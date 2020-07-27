
#include "gpm_hdf5_reader.h"
#include "grid_param_mapper.h"
#include "gpm_parm_methods.h"
#include <string>
#include "gpm_2darray_functions.h"
#include "gpm_vbl_array_3d.h"
#include "acf_base_helper.h"
#include "gpm_hdf5_writer.h"
#include "mapstc.h"
#include "gpm_model_definition.h"
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/min.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <iostream>

using namespace Slb::Exploration::Gpm;
using  namespace boost::accumulators;
float find_positive_volume(const function_2d& top, const function_2d& base)
{
	float sum = 0.0F;
	auto row_ext = top.extents().row_extent();
    auto col_ext = top.extents().col_extent();
    for(auto i = row_ext.lower(); i != row_ext.upper()-1;++i) {
		for (auto j = col_ext.lower(); j != col_ext.upper() - 1; ++j) {
			std::vector<float> dh = { top(i,j) - base(i,j) , top(i, j + 1) - base(i, j + 1), top(i + 1, j + 1) - base(i + 1, j + 1), top(i + 1, j) - base(i + 1, j) };
			std::vector<float> pos;
			std::copy_if(dh.begin(), dh.end(), std::back_inserter(pos), [](float val) {return val > 0.0F; });
            if (!pos.empty()) {
				sum += std::accumulate(pos.begin(), pos.end(), 0.0F) *0.25f;
            }
		}
    }
	return sum;
}

void find_intersection_indexes(const std::vector<function_2d>& tops, const function_2d& intersect, int_2darray_base_type* indx_p)
{
	auto row_ext = tops.front().extents().row_extent();
	auto col_ext = tops.front().extents().col_extent();
	for (auto i = row_ext.lower(); i != row_ext.upper() ; ++i) {
		for (auto j = col_ext.lower(); j != col_ext.upper() ; ++j) {
            const auto index = std::find_if(tops.begin(), tops.end(), [=](const function_2d& top) {return top(i, j)> intersect(i,j); });
            const auto distance = std::distance(tops.begin(),index);
			indx_p->at(i, j) = distance;
		}
	}
}

void find_sediment_sum_of_clipped_model(const std::vector<function_2d>& tops,
                                        const std::vector<std::vector<function_2d>>& seds,
                                        const function_2d& basement,
                                        const int_2darray_base_type& indx,
                                        gpm_array_3d_base<float>& sed_stack)
{
    auto row_ext = tops.front().extents().row_extent();
    auto col_ext = tops.front().extents().col_extent();
    for (auto i = row_ext.lower(); i != row_ext.upper(); ++i) {
        for (auto j = col_ext.lower(); j != col_ext.upper(); ++j) {
            const auto start_index = indx(i, j);
            if (start_index < tops.size()) {
                std::vector<float> top_samples;
                top_samples.push_back(basement(i, j));
                std::transform(tops.begin() + start_index, tops.end(), std::back_inserter(top_samples),
                               [=](const function_2d& top) { return top(i, j); });
                std::vector<float> delta;
                std::adjacent_difference(top_samples.begin(), top_samples.end(), std::back_inserter(delta),
                                         [](const float& top, const float& bot) { return top - bot; });
                // Now the seds
                std::vector<std::vector<float>> all_seds;
                for (const auto& sed : seds) {
                    std::vector<float> sed_parts;
                    std::transform(sed.begin() + start_index, sed.end(), std::back_inserter(sed_parts),
                                   [=](const function_2d& top) { return top(i, j); });
                    all_seds.push_back(sed_parts);
                }
                std::vector<float> sed_sum;
                for (const auto& sed_it : all_seds) {
                    sed_sum.push_back(std::inner_product(delta.begin() + 1, delta.end(), sed_it.begin(), 0.0F));
                }
                auto part = sed_stack(i, j);
                std::copy(sed_sum.begin(), sed_sum.end(), part.begin());
                int uu = 0;
            }
        }
    }
}

void find_sediment_sum_of_model(const std::vector<function_2d>& tops,
                                const std::vector<std::vector<function_2d>>& seds,
                                gpm_array_3d_base<float>& sed_stack)
{
	auto row_ext = tops.front().extents().row_extent();
	auto col_ext = tops.front().extents().col_extent();
	for (auto i = row_ext.lower(); i != row_ext.upper(); ++i) {
		for (auto j = col_ext.lower(); j != col_ext.upper(); ++j) {
			std::vector<float> top_samples;
			std::transform(tops.begin(), tops.end(), std::back_inserter(top_samples), [=](const function_2d& top) {return top(i, j); });
			std::vector<float> delta;
			std::adjacent_difference(top_samples.begin(), top_samples.end(), std::back_inserter(delta), [](const float& top, const float& bot) {return top - bot; });
			// Now the seds
			std::vector < std::vector<float>> all_seds;
			for (const auto& sed : seds) {
				std::vector<float> sed_parts;
				std::transform(sed.begin(), sed.end(), std::back_inserter(sed_parts), [=](const function_2d& top) {return top(i, j); });
				all_seds.push_back(sed_parts);
			}
			std::vector<float> sed_sum;
			for (const auto& sed_it : all_seds) {
				sed_sum.push_back(std::inner_product(delta.begin() + 1, delta.end(), sed_it.begin() + 1, 0.0F));
			}
			auto part = sed_stack(i, j);
			std::copy(sed_sum.begin(), sed_sum.end(), part.begin());
			int uu = 0;
		}
	}
}

void find_geometry_and_sediments_for_timestep(const parm& first, const int geometry_id, const std::vector<int> sed_ids, std::vector<int> ids, int time_index, std::vector<function_2d>& current_geometry, std::vector<std::vector<function_2d>>& current_sediemnts)
{
    find_grid_functions_for_timestep(first, geometry_id, time_index, 0, ids[time_index], &current_geometry);
    for (auto sed : sed_ids) {
        std::vector<function_2d> sed_vec;
        find_grid_functions_for_timestep(first, sed, time_index, 0, ids[time_index], &sed_vec);
        current_sediemnts.push_back(sed_vec);
    }
}

void find_the_sediments(const parm& first)
{
	const auto geometry_id = get_property_index(first, "TOP");
	const auto sed_ids = find_properties_with_name_prefix(first, "SED");
	assert(geometry_id >= 0);
	std::vector<int> ids;
	find_grid_diagonal_indexes(first, geometry_id, &ids);
	gpm_model_definition my_model(first.indprop(geometry_id, 0, 0).extents());

	my_model.set_world_coordinates(first.xcorners, first.ycorners);
	auto area = my_model.cell_area();
	auto& geoms(first.indprop(geometry_id));
	auto row_ext = my_model.model_extent().row_extent();
	auto col_ext = my_model.model_extent().col_extent();
	std::vector<float> diffs;
	for (int time_index = 1; time_index < ids.size(); ++time_index) {
        std::vector<function_2d> current_geometry;
        std::vector<std::vector<function_2d>> current_sediemnts;
        find_geometry_and_sediments_for_timestep(first, geometry_id, sed_ids, ids, time_index, current_geometry, current_sediemnts);
        gpm_vbl_array_3d<float> sed_sum_last(row_ext, col_ext, int_extent_1d(sed_ids.size()));
		find_sediment_sum_of_model(current_geometry, current_sediemnts, sed_sum_last);
		auto current_sum = std::accumulate(sed_sum_last.begin(), sed_sum_last.end(), 0.0F, std::plus<float>());

        // Prev
		std::vector<function_2d> prev_geometry;
		std::vector<std::vector<function_2d>> previous_sediments;
		find_geometry_and_sediments_for_timestep(first, geometry_id, sed_ids, ids, time_index-1, prev_geometry, previous_sediments);
		int_2darray_type indexes(my_model.model_extent());
		find_intersection_indexes(prev_geometry, current_geometry.front(), &indexes);
		gpm_vbl_array_3d<float> sed_sum_eroded(row_ext, col_ext, int_extent_1d(sed_ids.size()));
		find_sediment_sum_of_clipped_model(prev_geometry, previous_sediments, current_geometry.front(), indexes, sed_sum_eroded);
		auto eroded_sum = std::accumulate(sed_sum_eroded.begin(), sed_sum_eroded.end(), 0.0F, std::plus<float>());
		auto diff = (eroded_sum - current_sum) / eroded_sum;
		diffs.push_back(diff);
	}
    // Do last and first part
    std::vector<function_2d> last_geometry;
    find_grid_functions_for_timestep(first, geometry_id, ids.size() - 1, 0, ids.back(), &last_geometry);
    std::vector<function_2d> first_geometry;
    find_grid_functions_for_timestep(first, geometry_id, 0, 0, ids.front(), &first_geometry);
    int_2darray_type indexes(my_model.model_extent());
    find_intersection_indexes(first_geometry, last_geometry.front(), &indexes);
    std::vector<std::vector<function_2d>> start_sediments;
    for (auto sed : sed_ids) {
        std::vector<function_2d> sed_vec;
        find_grid_functions_for_timestep(first, sed, ids.size() - 1, 0, ids.back(), &sed_vec);
        start_sediments.push_back(sed_vec);
    }
    gpm_vbl_array_3d<float> sed_sum_eroded(row_ext, col_ext, int_extent_1d(sed_ids.size()));
    find_sediment_sum_of_clipped_model(first_geometry, start_sediments, last_geometry.front(), indexes, sed_sum_eroded);
    auto eroded_sum  = std::accumulate(sed_sum_eroded.begin(), sed_sum_eroded.end(), 0.0F, std::plus<float>());

    gpm_vbl_array_3d<float> sed_sum_last(row_ext, col_ext, int_extent_1d(sed_ids.size()));
    std::vector<std::vector<function_2d>> all_last_seds;
    for (auto sed : sed_ids) {
        std::vector<function_2d> sed_vec;
        find_grid_functions_for_timestep(first, sed, ids.size() - 1, 0, ids.back(), &sed_vec);
        all_last_seds.push_back(sed_vec);
    }
    find_sediment_sum_of_model(last_geometry, all_last_seds, sed_sum_last);
    auto conatined_sum  = std::accumulate(sed_sum_last.begin(), sed_sum_last.end(), 0.0F, std::plus<float>());
	auto diff = (conatined_sum - eroded_sum) / eroded_sum;
	std::cout << "Relative sediment volume diff between first and last step:   " << diff << std::endl;
	accumulator_set<float, features<tag::mean, tag::max, tag::min>> acc;
	std::for_each(diffs.begin(), diffs.end(), [&acc](float val) {acc(val); });
	std::cout << "Relative sediment volume diffs between time steps: Mean:   " << mean(acc) << "  Min:   " << min(acc) << "  Max:   " << max(acc) << std::endl;
 }

void check_the_volume_balance(const parm& first)
{
    const auto geometry_id = get_property_index(first, "TOP");
	const auto sed_ids = find_properties_with_name_prefix(first, "SED");
	assert(geometry_id >= 0);
	std::vector<int> ids;
	find_grid_diagonal_indexes(first, geometry_id, &ids);
	gpm_model_definition my_model(first.indprop(geometry_id,0,0).extents());

	my_model.set_world_coordinates(first.xcorners, first.ycorners);
	auto area = my_model.cell_area();
	auto& geoms(first.indprop(geometry_id));
	std::vector<float> diffs;
	for (int time_index = 1; time_index < ids.size(); ++time_index) {
        // Top and bottom
		auto sum = find_positive_volume(geoms(time_index, ids[time_index]), geoms(time_index, 0));
        // Prev top and current bottom
		auto sum2 = find_positive_volume(geoms(time_index-1, ids[time_index-1]), geoms(time_index, 0));
		auto vol1 = sum ;
		auto vol2 = sum2;
		auto diff = (vol1 - vol2) / vol2;
		diffs.push_back(diff);
	}
    // start and end
    {
		auto last = ids.size() - 1;
		auto sum = find_positive_volume(geoms(last, ids[last]), geoms(last, 0));
		// Prev top and current bottom
		auto sum2 = find_positive_volume(geoms(0, ids[0]), geoms(last, 0));
		auto vol1 = sum;
		auto vol2 = sum2;
		auto diff = (vol1 - vol2) / vol2;
		std::cout << "Relative volume diff between first and last step:   " << diff << std::endl;
		auto ii = 0;
    }
    accumulator_set<float, features<tag::mean, tag::max, tag::min>> acc;
	std::for_each(diffs.begin(), diffs.end(), [&acc](float val) {acc(val); });
	std::cout << "Relative volume diffs between time steps: Mean:   " << mean(acc) << "  Min:   " << min(acc) << "  Max:   "<< max(acc)<< std::endl;
}

// assumption is that we don't add anything
// if we did, we should make a note of them

int main(int argc, char* argv[])
{
    // We should compare ages also
    if (argc < 1) {
        printf("Need  file name as argument to check\n");
    }
	const parameter_descr_type parm_descr = parm_descr_holder::get_parm_descr();
    const std::string file_name = argv[1];
	struct parm first;
	{
        gpm_hdf5_reader first_reader;
        grid_param_wrapper first_grid_param_wrap(first, parm_descr);
		first_reader.set_parse_type_info(parm_descr);
        grid_param_mapper actual_results_map_grid_it(first_grid_param_wrap, first_reader);
		first_reader.parse_file(file_name);
		gpm_hdf5_reader::array_dim_size_type in_idimr = first_reader.array_sizes_read();
		gpm_hdf5_reader::parameter_num_type inumr = first_reader.num_read();
	}
	assign_zgrid_to_indprop(&first); // So we get the right setup
	find_the_sediments(first);
	check_the_volume_balance(first);
    return 0;
}
