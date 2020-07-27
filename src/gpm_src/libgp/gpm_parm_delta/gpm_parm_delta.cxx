
#include "gpm_hdf5_reader.h"
#include "grid_param_mapper.h"
#include "gpm_format_helper.h"
#include "gpm_2darray_functions.h"
#include "acf_base_helper.h"
#include "gpm_hdf5_writer.h"
#include "grid_param_reader.h"
#include "gpm_parm_helper_methods.h"
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/core/null_deleter.hpp>
#include <string>

using namespace Slb::Exploration::Gpm;
namespace  bmi = boost::multi_index;

using index_list_type = boost::multi_index_container<
	std::string,
	bmi::indexed_by<
	bmi::sequenced<bmi::tag<struct ordered_by_insertion> >,
	bmi::ordered_non_unique<
	bmi::tag<struct ordered_by_id>,
	bmi::identity<std::string>
	>
	>
>;

int find_max_num_arrays(const gpm_array_2d_base<function_2d>& first, const gpm_array_2d_base <function_2d>& second, 
	std::set<std::pair<std::ptrdiff_t, std::ptrdiff_t>>& found_indexes)
{
    auto row_ext = first.extents().row_extent();
    auto col_ext = first.extents().col_extent();
	int num = 0;
    for (auto i = row_ext.lower(); i != row_ext.upper(); ++i) {
        for (auto j = col_ext.lower(); j != col_ext.upper(); ++j) {
			if(first(i, j).is_index() && second(i, j).is_index() ) {
				found_indexes.insert(std::make_pair(first(i, j).index_value(),second(i, j).index_value()));
			}
			else if (first(i, j).is_index()) {
				++num;
			}
			else if (second(i, j).is_index()) {
				++num;
			}
        }
    }
	return num;
}
void setup_diff_coupling(const parm& first, const parm& second, const std::vector<std::pair<std::ptrdiff_t, std::ptrdiff_t>>& prop_indexes, parm& diff)
{
	std::ptrdiff_t z_index = 0;
	std::ptrdiff_t prop_index = 0;
	std::map<std::pair<std::ptrdiff_t, std::ptrdiff_t>, std::ptrdiff_t> z_grid_mapper;
	int_extent_2d ext(first.zgrid.row_extent(), first.zgrid.col_extent());
    for (const auto loc_prop_index : prop_indexes) {
		diff.propnam[prop_index] = first.propnam[loc_prop_index.first];
		auto row_ext = first.indprop.extents().row_extent();
		auto col_ext = first.indprop.extents().col_extent();
		for (auto i = row_ext.lower(); i != row_ext.upper(); ++i) {
			for (auto j = col_ext.lower(); j != col_ext.upper(); ++j) {
			    const auto& first_func = first.indprop(loc_prop_index.first);
			    const auto& second_func = second.indprop(loc_prop_index.second);
				if(first_func(i, j).is_index() || second_func(i, j).is_index() ) {
                    if (first_func(i, j).is_index() && second_func(i, j).is_index()) {
						auto item = std::make_pair(first_func(i, j).index_value(), second_func(i, j).index_value());
                        auto z_index_used = z_grid_mapper.find(item);
                        if (z_index_used == z_grid_mapper.end()) { // need to make diff
							std::string name = first.gridnam[item.first] + std::string("-") + second.gridnam[item.second];
							diff.gridnam[z_index] = name;
							diff.indprop(prop_index, i, j).set_index(z_index);
							diff.indprop(prop_index, i, j).set_array(
								std::shared_ptr<float_3darray_vec_base_type>(&(diff.zgrid), boost::null_deleter()));
							gpm_subtract(first_func(i, j).const_array_holder(), second_func(i, j).const_array_holder(), &diff.zgrid[z_index]);
							z_grid_mapper.insert(std::make_pair(item, z_index));
							++z_index;
                        }
						else {
							diff.indprop(prop_index, i, j).set_index(z_index_used->second);
							diff.indprop(prop_index, i, j).set_array(
								std::shared_ptr<float_3darray_vec_base_type>(&(diff.zgrid), boost::null_deleter()));
						}
                    }
					else if (first_func(i, j).is_index() ) {
						std::string name = first.gridnam[first_func(i, j).index_value()] + std::string("-constant");
						diff.gridnam[z_index] = name;
						diff.indprop(prop_index, i, j).set_index(z_index);
						diff.indprop(prop_index, i, j).set_array(
							std::shared_ptr<float_3darray_vec_base_type>(&(diff.zgrid), boost::null_deleter()));
						gpm_subtract(first_func(i, j).const_array_holder(), second_func(i, j).constant_value(), &diff.zgrid[z_index]);
						++z_index;
					}
					else {
						std::string name = std::string("constant-") + second.gridnam[second_func(i, j).index_value()];
						diff.gridnam[z_index] = name;
						diff.indprop(prop_index, i, j).set_index(z_index);
						diff.indprop(prop_index, i, j).set_array(
							std::shared_ptr<float_3darray_vec_base_type>(&(diff.zgrid), boost::null_deleter()));
						gpm_subtract(first_func(i, j).constant_value(), second_func(i, j).const_array_holder(), &diff.zgrid[z_index]);
						++z_index;
					}
				}
				else if ((first_func(i, j).is_constant() && second_func(i, j).is_constant())) {
					diff.indprop(prop_index, i, j).set_constant(first_func(i, j).constant_value() - second_func(i, j).constant_value(), ext);
				}
			}
		}
		++prop_index;
    }
}
int make_diff(const std::string & first_file, const std::string & second_file, const std::string& diff_file)
{
	const parameter_descr_type parm_descr = parm_descr_holder::get_parm_descr();
	gpm_hdf5_reader::array_dim_size_type in_idimr;
	gpm_hdf5_reader::parameter_num_type inumr;
	struct parm first;
	{
		gpm_hdf5_reader first_reader;
		grid_param_wrapper first_grid_param_wrap(first, parm_descr);
		first_reader.set_parse_type_info(parm_descr);
		grid_param_mapper actual_results_map_grid_it(first_grid_param_wrap, first_reader);
		first_reader.parse_file(first_file);
		in_idimr = first_reader.array_sizes_read();
		inumr = first_reader.num_read();
	}
	assign_zgrid_to_indprop(&first); // So we get the right setup

	//compare hdf5 file vs hdf5 org
	struct parm second;
	{
		gpm_hdf5_reader second_reader;
		grid_param_wrapper second_grid_param_wrap(second, parm_descr);
		second_reader.set_parse_type_info(parm_descr);
		grid_param_mapper expected_org_results_map_grid_it(second_grid_param_wrap, second_reader);
		second_reader.parse_file(second_file);
	}
	assign_zgrid_to_indprop(&second); // So we get the right setup

    if (first.indprop.row_extent()!= second.indprop.row_extent() || first.indprop.col_extent() != second.indprop.col_extent()) {
		return 1;
    }

    // Now we can diff the two, and see what they have
	index_list_type first_prop_names{ first.propnam.begin(), first.propnam.end() };
	index_list_type second_prop_names{ second.propnam.begin(), second.propnam.end() };
	std::vector<std::string> prop_intersection;
	auto& first_it = first_prop_names.get<ordered_by_id>();
	auto& second_it = second_prop_names.get<ordered_by_id>();
	std::set_intersection(first_it.begin(), first_it.end(), second_it.begin(), second_it.end(), std::back_inserter(prop_intersection));

	// Lets find the indexes for use
	std::vector<std::pair<std::ptrdiff_t, std::ptrdiff_t>> indprop_indexes;
	auto& exp_by_ins = first_prop_names.get<ordered_by_insertion>();
	auto& act_by_ins = second_prop_names.get<ordered_by_insertion>();
	for (const auto& item : prop_intersection) {
		const auto it1 = first_it.find(item);
		const auto it2 = second_it.find(item);
		const auto pos1 = std::distance(exp_by_ins.begin(), bmi::project<ordered_by_insertion>(first_prop_names, it1));
		const auto pos2 = std::distance(act_by_ins.begin(), bmi::project<ordered_by_insertion>(second_prop_names, it2));
		indprop_indexes.emplace_back(static_cast<int>(pos1), static_cast<int>(pos2));
	}
	std::set<std::pair<std::ptrdiff_t, std::ptrdiff_t>> index_pairs;
	std::size_t num_odd_pairs = 0;
	for (const auto item : indprop_indexes) {
		const auto first_layer = item.first;
		const auto second_layer = item.second;
		num_odd_pairs += find_max_num_arrays(first.indprop(first_layer), second.indprop(second_layer), index_pairs);
	}
    // Now we have the number of arrays and the number of props
    // Allocate
	auto num = index_pairs.size()+num_odd_pairs;
	struct parm diff;
	diff.zgrid.resize(int_extent_1d(0, num), first.zgrid.row_extent(), first.zgrid.col_extent());
	diff.gridnam.resize(num);
	diff.indprop.resize(int_extent_1d(0, prop_intersection.size()), first.indprop.row_extent(), first.indprop.col_extent());
	diff.propnam.resize(prop_intersection.size());
	diff.prop_display_name.resize(int_extent_2d(2, prop_intersection.size()));
	diff.ageset = first.ageset;
	diff.timeset = first.timeset;
	diff.xcorners = first.xcorners;
	diff.ycorners = first.ycorners;
	diff.base_layer_map = first.base_layer_map;
	diff.sediment_prop_map = first.sediment_prop_map;
	diff.znull = first.znull;
	diff.indprop_stats.resize(diff.propnam.size(), diff.timeset.size(), 2, true, diff.znull); // Min and max statistics
	diff.history = first.history;
	diff.history.insert(diff.history.end(), second.history.begin(), second.history.end());
	setup_diff_coupling(first, second, indprop_indexes, diff);
	Tools::gpm_default_logger my_logger(Slb::Exploration::Gpm::Tools::LOG_VERBOSE, true);
	for (int i = 0; i < diff.timeset.size(); ++i) {
		calculate_statistics(&diff, i, my_logger);
	}
    // Need the props now
    for(int i = 0; i < indprop_indexes.size(); ++i) {
		diff.prop_display_name(0, i) = first.prop_display_name(0, indprop_indexes[i].first);
		diff.prop_display_name(1, i) = first.prop_display_name(1, indprop_indexes[i].first);
    }
	acf_base_writer::array_dim_size_type idimr = acf_base_helper::convert(in_idimr);
	adjust_dimensions(diff, inumr, idimr, my_logger);
	gpm_hdf5_writer out_file;
	out_file.set_write_info(parm_descr, inumr, idimr);
	if (!out_file.open_file(diff_file, true)) {
		my_logger.print(Tools::LOG_NORMAL, "Error: Unable to open output file '%s'\n", diff_file.c_str());
		return 1;
	}

	grid_param_reader read_hdf5_grid_it(diff, out_file);
	out_file.write();

    // assume that 
	return 0;
}
int main(int argc, char* argv[])
{
    // We should compare ages also
    if (argc < 4) {
        printf("Need two file names as arguments to diff against and one file name for output\n");
    }
	const int tmp = make_diff(argv[1], argv[2], argv[3]);
    return tmp;
}
