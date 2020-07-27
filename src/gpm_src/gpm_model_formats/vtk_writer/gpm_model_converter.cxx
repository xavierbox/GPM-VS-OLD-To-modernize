#include "gpm_model_converter.h"
#include <set>

namespace Slb { namespace Exploration { namespace Gpm {

std::vector<std::vector<int>> gpm_model_converter::check_cell_pointers(const vec_point& points,
                                                                       const quad_index_type& bot_mapped,
                                                                       const quad_index_type& top_mapped)
{
    std::vector<std::vector<int>> cell_ind;
    // set up cell pointer for a regular hex cell
    std::vector<int> point_ind;

    using quad_set_type = std::set<quad_index_type::value_type>;
    using quad_vector_type = std::vector<quad_index_type::value_type>;

    quad_set_type all(bot_mapped.begin(), bot_mapped.end());
    all.insert(top_mapped.begin(), top_mapped.end());
    if (all.size() == 8) {
        point_ind.insert(point_ind.end(), bot_mapped.begin(), bot_mapped.end());
        point_ind.insert(point_ind.end(), top_mapped.begin(), top_mapped.end());
        cell_ind.emplace_back(point_ind);
    }
    else {
        // Hard parts
        if (all.size() == 7) {
            quad_set_type bot_ordered(bot_mapped.begin(), bot_mapped.end());
            quad_set_type top_ordered(top_mapped.begin(), top_mapped.end());
            quad_vector_type common_item;
            std::set_intersection(bot_ordered.begin(), bot_ordered.end(), top_ordered.begin(), top_ordered.end(),
                                  std::back_inserter(common_item));
            const auto index = std::distance(top_mapped.begin(),
                                             std::find(top_mapped.begin(), top_mapped.end(), common_item.front()));

            // erase the item
            const quad_vector_type indexes = {0, 1, 2, 3};
            auto wedge_indexes = indexes;
            wedge_indexes.erase(wedge_indexes.begin() + index);
            std::transform(wedge_indexes.begin(), wedge_indexes.end(), std::back_inserter(point_ind),
                           [&top_mapped](quad_index_type::value_type ind) { return top_mapped[ind]; });
            std::transform(wedge_indexes.begin(), wedge_indexes.end(), std::back_inserter(point_ind),
                           [&bot_mapped](quad_index_type::value_type ind) { return bot_mapped[ind]; });
            cell_ind.emplace_back(point_ind);

            //Now the pyramid
            std::map<quad_index_type::value_type, quad_index_type> pyr_map;
            pyr_map[0] = quad_index_type(
                {
                    1, 5, 7, 3
                }
            );
            pyr_map[1] = quad_index_type(
                {
                    0, 2, 6, 4
                }
            );
            pyr_map[2] = quad_index_type(
                {
                    1, 3, 7, 5
                }
            );
            pyr_map[3] = quad_index_type(
                {
                    0, 4, 6, 2
                }
            );

            quad_vector_type all_indexes;
            all_indexes.insert(all_indexes.end(), bot_mapped.begin(), bot_mapped.end());
            all_indexes.insert(all_indexes.end(), top_mapped.begin(), top_mapped.end());
            std::vector<int> pyramid;
            std::transform(pyr_map[index].begin(), pyr_map[index].end(), std::back_inserter(pyramid),
                           [&all_indexes](quad_index_type::value_type ind) { return all_indexes[ind]; });

            pyramid.push_back(common_item.front());
            cell_ind.emplace_back(pyramid);
        }
        if (all.size() == 6) {
            // Wedge
            quad_vector_type top_diff;
            quad_set_type bot_ordered(bot_mapped.begin(), bot_mapped.end());
            quad_set_type top_ordered(top_mapped.begin(), top_mapped.end());
            std::set_intersection(top_ordered.begin(), top_ordered.end(), bot_ordered.begin(), bot_ordered.end(),
                                  std::back_inserter(top_diff));
            quad_vector_type top_dist;
            std::transform(top_diff.begin(), top_diff.end(), std::back_inserter(top_dist),
                           [&top_mapped](quad_index_type::value_type ind)
                           {
                               auto d = std::find(top_mapped.begin(), top_mapped.end(), ind);
                               return std::distance(top_mapped.begin(), d);
                           });
            std::sort(top_dist.begin(), top_dist.end());
            const auto dist = top_dist.back() - top_dist.front();
            auto key = std::make_pair(top_dist.front(), top_dist.back());
            using quad_pair_type = std::pair<quad_index_type::value_type, quad_index_type::value_type>;
            if (dist == 2) {
                // trouble
                auto first_diag = quad_pair_type(0, 2);
                auto second_diag = quad_pair_type(1, 3);
                if (key == first_diag) {
                    point_ind.push_back(bot_mapped[0]);
                    point_ind.push_back(bot_mapped[1]);
                    point_ind.push_back(bot_mapped[2]);
                    point_ind.push_back(top_mapped[1]);
                    cell_ind.emplace_back(point_ind);

                    point_ind.clear();
                    point_ind.push_back(bot_mapped[0]);
                    point_ind.push_back(bot_mapped[2]);
                    point_ind.push_back(bot_mapped[3]);
                    point_ind.push_back(top_mapped[3]);
                    cell_ind.emplace_back(point_ind);
                }
                else if (key == second_diag) {
                    point_ind.push_back(bot_mapped[0]);
                    point_ind.push_back(bot_mapped[1]);
                    point_ind.push_back(bot_mapped[3]);
                    point_ind.push_back(top_mapped[0]);
                    cell_ind.emplace_back(point_ind);
                    point_ind.clear();
                    point_ind.push_back(bot_mapped[1]);
                    point_ind.push_back(bot_mapped[2]);
                    point_ind.push_back(bot_mapped[3]);
                    point_ind.push_back(top_mapped[2]);
                    cell_ind.emplace_back(point_ind);
                }
            }
            else {
                // wedge
                static const std::map<std::pair<int, int>, std::pair<int, int>> wedge_map = {
                    {{0, 1}, {3, 2}},
                    {{1, 2}, {0, 3}},
                    {{2, 3}, {1, 0}},
                    {{0, 3}, {1, 2}}
                };
                auto vals = wedge_map.at(key);
                point_ind.push_back(bot_mapped[key.second]);
                point_ind.push_back(bot_mapped[vals.second]);
                point_ind.push_back(top_mapped[vals.second]);
                point_ind.push_back(bot_mapped[key.first]);
                point_ind.push_back(bot_mapped[vals.first]);
                point_ind.push_back(top_mapped[vals.first]);
                if (key == quad_pair_type(0, 3)) {
                    std::rotate(point_ind.begin(), point_ind.begin() + 3, point_ind.end());
                }
                cell_ind.emplace_back(point_ind);
            }
        }
        else if (all.size() == 5) {
            // Tetrahedra
            quad_set_type bot_ordered(bot_mapped.begin(), bot_mapped.end());
            quad_set_type top_ordered(top_mapped.begin(), top_mapped.end());
            quad_vector_type top_index;
            std::set_difference(top_ordered.begin(), top_ordered.end(), bot_ordered.begin(), bot_ordered.end(),
                                std::back_inserter(top_index));
            const auto index = std::distance(top_mapped.begin(),
                                             std::find(top_mapped.begin(), top_mapped.end(), top_index.front()));
            switch (index) {
            case 0:
                point_ind.push_back(bot_mapped[3]);
                point_ind.push_back(bot_mapped[0]);
                point_ind.push_back(bot_mapped[1]);
                break;
            case 1:
                point_ind.push_back(bot_mapped[0]);
                point_ind.push_back(bot_mapped[1]);
                point_ind.push_back(bot_mapped[2]);
                break;
            case 2:
                point_ind.push_back(bot_mapped[1]);
                point_ind.push_back(bot_mapped[2]);
                point_ind.push_back(bot_mapped[3]);
                break;
            case 3:
                point_ind.push_back(bot_mapped[2]);
                point_ind.push_back(bot_mapped[3]);
                point_ind.push_back(bot_mapped[0]);
                break;
            default:
                break;
            }
            point_ind.push_back(top_index.front());
            cell_ind.emplace_back(point_ind);
        }
    }

    return cell_ind;
}

void gpm_model_converter::convert_to_unstructured(const std::vector<function_2d_read_type>& surface_stack,
                                                  const gpm_model_definition& model)
{
    _top_model_index_map.resize(model.model_extent());
    _stack_extent = int_extent_3d(int_extent_1d(surface_stack.size()), model.model_extent().row_extent(), model.model_extent().col_extent());
    int_extent_1d lay_ext = _stack_extent.lay_extent();
    int_extent_1d row_ext = _stack_extent.row_extent();
    int_extent_1d col_ext = _stack_extent.col_extent();
    const auto numlayers = lay_ext.size();
    const auto numrows = row_ext.size();
    const auto numcols = col_ext.size();
    _grid_points.reserve(numlayers * numrows * numcols);
    gpm_model_definition::index_pt tmp_ij = {0, 0};

    for (const auto tmp : surface_stack) {
        for (auto j : row_ext) {
            tmp_ij.i = static_cast<double>(j);
            for (auto k : col_ext) {
                tmp_ij.j = static_cast<double>(k);
                const auto tmp_xy = model.convert_from_index_to_world(tmp_ij);
                point pp = point(tmp_xy.x, tmp_xy.y, tmp(j, k));
                _grid_points.emplace_back(pp);
            }
        }
    }
    const auto node_layer = numrows * numcols;

    for (auto i = row_ext.lower(); i < row_ext.upper() - 1; i++) {
        for (auto j = col_ext.lower(); j < col_ext.upper() - 1; j++) {
            unsigned long p0_bot = static_cast<unsigned long>(j + i * numcols);
            unsigned long p1_bot = p0_bot + 1;
            std::vector<std::vector<unsigned long>> p_index = {
                {p0_bot}, {p1_bot},
                {static_cast<unsigned long>(p1_bot + numcols)},
                {static_cast<unsigned long>(p0_bot + numcols)}
            };

            for (size_t layer = 1; layer < surface_stack.size(); ++layer) {
                unsigned long p0 = static_cast<unsigned long>(j + layer * node_layer + i * numcols);
                unsigned long p1 = p0 + 1;
                unsigned long p2 = static_cast<unsigned long>(p1 + numcols);
                unsigned long p3 = static_cast<unsigned long>(p0 + numcols);
                // Check the pointers
                std::array<unsigned long, 4> top = {
                    p0, p1, p2, p3
                };

                for (auto ind = 0UL; ind < top.size(); ++ind) {
                    auto dist = boost::geometry::
                        distance(_grid_points[p_index[ind].back()], _grid_points[top[ind]]);
                    if (dist > 0.001) {
                        p_index[ind].push_back(top[ind]);
                    }
                }
            }
            // This is to find the tops since we are going upwards
            _top_model_index_map(i, j) = p_index[0].back();
            _top_model_index_map(i, j + 1) = p_index[1].back();
            _top_model_index_map(i + 1, j + 1) = p_index[2].back();
            _top_model_index_map(i + 1, j) = p_index[3].back();

            std::array<unsigned long, 4> sizes{};
            std::transform(p_index.begin(), p_index.end(), sizes.begin(), [](const std::vector<unsigned long>& it)
            {
                return static_cast<unsigned long>(it.size());
            });
            const auto max_num = *std::max_element(sizes.begin(), sizes.end());
            std::array<unsigned long, 4> bottom = {
                p_index[0].front(), p_index[1].front(), p_index[2].front(), p_index[3].front()
            };
            for (auto count = 1UL; count < max_num; ++count) {
                std::array<unsigned long, 4> top{};
                for (auto tmp1 = 0; tmp1 < 4; ++tmp1) {
                    top[tmp1] = count < p_index[tmp1].size() ? p_index[tmp1][count] : p_index[tmp1].back();
                }
                auto cell_ptr = check_cell_pointers(_grid_points, bottom, top);
                // Add it to cells
                for (const auto& item : cell_ptr) {
                    _cell_indexes.push_back(item);
                }
                bottom = top;
            }
        }
    }
    clean_ugrid_points_and_cells();
}

void gpm_model_converter::clean_ugrid_points_and_cells()
{
    const auto number_of_old_points = _grid_points.size();
    std::vector<bool> point_referenced_by_cell(number_of_old_points, false);

    // filter out unused cells (org --> compressed)
    for (auto& cell_point_ids : _cell_indexes) {
        for (auto cell_index : cell_point_ids) {
            point_referenced_by_cell.at(cell_index) = true;
        }
    }
    const auto number_of_actual_points = count(point_referenced_by_cell.begin(), point_referenced_by_cell.end(),
                                               true);
    _is_compressed = number_of_actual_points != number_of_old_points;
    // check if there are any cleanup to do
    if (_is_compressed) {

        _compressed_to_origin_map.resize(number_of_actual_points, -1);
        _origin_to_compressed_map.resize(number_of_old_points, -1);
        std::fill(_origin_to_compressed_map.begin(), _origin_to_compressed_map.end(), -1);
        std::fill(_compressed_to_origin_map.begin(), _compressed_to_origin_map.end(), -1);
        // map geometry points
        auto new_index_mapping = 0;
        for (auto i = 0; i < _grid_points.size(); i++) {
            if (point_referenced_by_cell.at(i)) {
                const auto pd = _grid_points.at(i);
                // add only points that are referenced
                _compressed_grid_points.emplace_back(pd);
                _compressed_to_origin_map.at(new_index_mapping) = i;
                _origin_to_compressed_map.at(i) = new_index_mapping;
                new_index_mapping++;
            }
        }

        // generate new cell points
        for (const auto& new_cell_point_ids : _cell_indexes) {
            const auto cell_size = new_cell_point_ids.size();
            std::vector<int> ci;
            ci.reserve(cell_size);
            for (auto cell_index : new_cell_point_ids) {
                ci.emplace_back(_origin_to_compressed_map[cell_index]);
            }
            _compressed_cell_indexes.push_back(ci);
        }

    }
    // Here we should have copy the compressed to the original again
}
;

}}}
