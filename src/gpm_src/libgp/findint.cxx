// -- Schlumberger Private --

#include "gpm_parm_methods.h"
#include "mapstc.h"
#include "mapstc_descr.h"
#include "libgp.h"
#include "gpm_logger.h"
#include "gpm_string_utils.h"
#include <numeric>
#include <boost/algorithm/string.hpp>
#include <boost/core/null_deleter.hpp>
#include <set>

namespace Slb { namespace Exploration { namespace Gpm {namespace {

void clip_parameters(parm::index_type& from_index, parm::index_type& to_index, parm::index_type lower, parm::index_type upper) {
    if (from_index < lower) from_index = lower;
    if (to_index > upper) to_index = upper;
}
}

void find_arrays_for_timestep(parm* p, int prop_id,
                              int time_step, int from_top, int to_top,
                              std::vector<float_2darray_base_type*>* functions_p,
                              const Tools::gpm_logger& logger) {
    if (prop_id < 0 || prop_id >= p->propnam.size()) {
        logger.print(Tools::LOG_NORMAL, "Error finding index %d in property\n", prop_id);
    }
    else {
        int iret = find_grid_functions_for_timestep(p, prop_id, time_step, from_top, to_top, functions_p);
        if (iret != 0) {
            logger.print(Tools::LOG_NORMAL, "Error finding grid for index %d\n", prop_id);
        }
    }
}

void find_arrays_for_timestep(parm* p, const std::string& id,
                              int time_step, int from_top, int to_top,
                              std::vector<float_2darray_base_type*>* functions_p,
                              const Tools::gpm_logger& logger) {
    int indsed1 = get_property_index(*p, id);
    if (indsed1 < 0) { logger.print(Tools::LOG_NORMAL, "Error finding %s in property\n", id.c_str()); }
    int iret = find_grid_functions_for_timestep(p, indsed1, time_step, from_top, to_top, functions_p);
    if (iret != 0) { logger.print(Tools::LOG_NORMAL, "Error finding grid for %s\n", id.c_str()); }
}

int find_functions_for_timestep(const parm& p, int prop_id,
                                int time_step, int from_top, int to_top,
                                std::vector<function_2d>* functions_p,
                                const Tools::gpm_logger& logger) {
    int iret = find_grid_functions_for_timestep(p, prop_id, time_step, from_top, to_top, functions_p);
    if (iret != 0) { logger.print(Tools::LOG_NORMAL, "Error finding grid for id %d\n", prop_id); }
    return iret;
}
std::vector<int> find_non_unique_arrays_for_timestep(const parm& p, int prop_id,
	parm::index_type time_step, parm::index_type from_top, parm::index_type to_top)
{
	std::vector<int> res;
	std::vector<bool> is_unique;
	is_array_unique_for_timestep(p, prop_id, p.numtims() - 1, 0, p.numtops() - 1, &is_unique);
	if (std::any_of(is_unique.begin(), is_unique.end(), [](bool val) {return !val; })) {
		// Need to create the non unique ones
		for (auto i = 0; i < is_unique.size(); ++i) {
			if (!is_unique[i]) {
				res.push_back(i);
			}
		}
	}
	return res;
}
void is_array_unique_for_timestep(const parm& p, int prop_id,
	parm::index_type time_step, parm::index_type from_top, parm::index_type to_top,
	std::vector<bool>* array_p) {

	clip_parameters(from_top, to_top, 0, p.numtops() - 1);

	if (to_top >= array_p->size()) {
		array_p->resize(to_top + 1);
	}
	std::fill(array_p->begin(), array_p->end(), false);
	auto& prop(p.indprop(prop_id));
	auto time_ext = prop.row_extent();
	auto age_ext = prop.col_extent();

	std::set<parm::index_type> prev;
    for(auto t = time_ext.lower(); t < time_step;++t) {
		for (auto a = age_ext.lower(); a < age_ext.upper(); ++a) {
			if (prop(t, a).is_index()) {
				prev.insert(prop(t, a).index_value());
			}
		}   
    }
	for (auto a = from_top; a <= to_top; ++a) {
		if (prop(time_step, a).is_index() && prev.find(prop(time_step, a).index_value()) == prev.end()) {
			array_p->at(a) = true;
		}
	}
}

int gpm_find_sediment_functions_for_timestep(const parm& p,
                                             const std::vector<int>& sed_indexes,
                                             int timestep, int from_top, int to_top,
                                             std::vector<std::vector<function_2d>>* sed_functions_p) {
    std::vector<int> ret_vals;
    for (int it = 0; it < sed_indexes.size(); ++it) {
        int ret2 = find_grid_functions_for_timestep(p, sed_indexes[it], timestep, from_top, to_top, &(sed_functions_p->at(it)));
        ret_vals.push_back(ret2);
    }
    return std::accumulate(ret_vals.begin(), ret_vals.end(), 0);
}

void gpm_find_sediment_arrays_for_timestep(parm* p_p,
                                           const std::vector<int>& sed_indexes,
                                           int timestep, int from_top, int to_top,
                                           std::vector<std::vector<float_2darray_base_type*>>* sed_functions_p,
                                           const Tools::gpm_logger& logger)
{
    for (int it = 0; it < sed_indexes.size(); ++it) {
        find_arrays_for_timestep(p_p, sed_indexes[it], timestep, from_top, to_top, &(sed_functions_p->at(it)), logger);
    }
}

// First is the number of the index , like SED1, SED2 etc.
// Second is the index into the INDPROP array
namespace { // Local implementation , do not export out

int find_property_indexes_for_prefix(const parm& p,
                                     const std::string& prefix,
                                     std::vector<property_range_item>* indinsed_p,
                                     const Tools::gpm_logger& logger) {
    std::vector<int> indexes = find_properties_with_name_prefix(p, prefix);
    std::vector<property_range_item> sort_thing;
    for (auto iprop : indexes) {
        int loc_int = -1;
        std::string loc_str;
        if (Tools::split_into_alpha_number(p.propnam[iprop], loc_str, loc_int) && prefix == loc_str) {
			sort_thing.emplace_back(property_range_item{ loc_int, iprop });
        }
    }
    std::sort(sort_thing.begin(), sort_thing.end(), is_range_sorted);
    const auto it = std::unique(sort_thing.begin(), sort_thing.end(), is_range_equal);
    if (it != sort_thing.end()) {
        logger.print(Tools::LOG_NORMAL, "ERROR: Preexisting property types %sn defined more than once\n", prefix.c_str());
        return 2;
    }
    indinsed_p->assign(sort_thing.begin(), sort_thing.end());
    return 0;
}

} // End of anonymous namespace

int gpm_find_residence_time_property_indexes(const parm& p,
                                             std::vector<property_range_item>* restime_p,
                                             const Tools::gpm_logger& logger) {
    const std::string prefix = parm_descr_holder::get_grid_map_1_index_name_prefix(RES_TIME_j);
    const int ret_val = find_property_indexes_for_prefix(p, prefix, restime_p, logger);
    if (ret_val != 0) {
        return ret_val;
    }
    return 0;
}

int gpm_find_diagenesis_property_indexes(const parm& p,
                                         std::vector<property_range_item>* restime_p,
                                         const Tools::gpm_logger& logger) {
    const std::string prefix = parm_descr_holder::get_grid_map_1_index_name_prefixes().at(DIAGENESIS_j);
    const int ret_val = find_property_indexes_for_prefix(p, prefix, restime_p, logger);
    if (ret_val != 0) {
        return ret_val;
    }
    return 0;
}

int gpm_find_insitu_growth_property_indexes(const parm& p,
                                            std::vector<property_range_item>* indsed_p,
                                            const Tools::gpm_logger& logger) {
    const std::string prefix = parm_descr_holder::get_grid_map_1_index_name_prefix(INSITU_GROWTH_j);
    const int ret_val = find_property_indexes_for_prefix(p, prefix, indsed_p, logger);
    if (ret_val != 0) {
        return ret_val;
    }
    return 0;
}

// This returns erodability properties that are needed
std::vector<std::pair<int, property_description>> gpm_find_erodability_property_ids(
    const std::map<std::string, int>& sed_indexes,
	const std::vector<std::string>& sed_names)
{
	std::vector<std::pair<int, property_description>> res;
	const auto& prefix = parm_descr_holder::get_erodability_property_prefix();
    for(auto& it: sed_indexes) {
		auto id = parm_descr_holder::grid_property_postfix_index_name(prefix, it.second + 1);
		auto name = *prefix.begin() + boost::to_lower_copy(std::string(prefix.begin() + 1, prefix.end())) +
			+ " " + sed_names[it.second];
		res.push_back(std::make_pair(it.second, property_description(id, name, false)));
    }
	return res;
}

int gpm_find_sediment_property_indexes(const std::map<std::string, int>& sed_indexes,
                                       // The ones that are defined in sed
                                       const std::map<std::string, std::pair<std::string, int>>& model_sed_indexes,
                                       // Has the id, together with the sed name and index into indprop
                                       std::vector<property_range_item>* indsed_p,
                                       const Tools::gpm_logger& logger)
{
    const std::string prefix = parm_descr_holder::get_grid_map_1_index_name_prefix(SED_j);
    std::vector<std::pair<std::string, int>> sorted(sed_indexes.begin(), sed_indexes.end());
    std::sort(sorted.begin(), sorted.end(),
              [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b)
              {
                  return a.second < b.second ;
              });
    for (const auto& it: sorted) {
        int loc_int = -1;
        std::string loc_str;
        const auto& item = model_sed_indexes.at(it.first);
        if (Tools::split_into_alpha_number(item.first, loc_str, loc_int) && prefix == loc_str) {
			indsed_p->push_back(property_range_item{ loc_int, item.second });
        }
        else {
            return 2;
        }
    }
    return 0;
}

// Need to have arrays preallocated to be able to contain the elements as described;;
int find_base_functions_for_timestep(const parm& p,
                                     int top_index, const std::vector<int>& sed_indexes,
                                     int timestep, int from_top, int to_top,
                                     std::vector<function_2d>* top_functions_p,
                                     std::vector<std::vector<function_2d>>* sed_functions_p) {
    const int ret1 = find_grid_functions_for_timestep(p, top_index, timestep, from_top, to_top, top_functions_p);
    const int ret2 = gpm_find_sediment_functions_for_timestep(p, sed_indexes, timestep, from_top, to_top, sed_functions_p);
    return ret1 + ret2;
}

int find_grid_functions_for_timestep(parm* p, parm::index_type prop_index, parm::index_type time_index, parm::index_type ito_top, float_2darray_base_type** prop_p)
{
    std::vector<float_2darray_base_type*> tmp_vec;
    const int retval = find_grid_functions_for_timestep(p, prop_index, time_index, ito_top, ito_top, &tmp_vec);
    (*prop_p) = tmp_vec[ito_top];
    return retval;
}

int find_grid_functions_for_timestep(const parm& p, parm::index_type prop_index, parm::index_type time_index, parm::index_type ito_top, function_2d* prop_p)
{
    std::vector<function_2d> tmp_vec;
    const int retval = find_grid_functions_for_timestep(p, prop_index, time_index, ito_top, ito_top, &tmp_vec);
    (*prop_p) = tmp_vec[ito_top];
    return retval;
}
// Finds grid pointers for property iprop and time itim in ts matrix
// ifm, ito are inclusive stratigraphic bounds.
int find_grid_functions_for_timestep(parm* p, parm::index_type prop_index, parm::index_type time_index, parm::index_type ifrom_top, parm::index_type ito_top, std::vector<float_2darray_base_type*>* prop_p) {
    int iret = 0;
    std::vector<float_2darray_base_type*>& prop(*prop_p);
    clip_parameters(ifrom_top, ito_top, 0, p->numtops() - 1);

    if (ito_top >= prop.size()) {
        prop.resize(ito_top + 1);
    }
    for (auto itop = ifrom_top; itop <= ito_top; itop++) {
        if (p->indprop(prop_index, time_index, itop).is_index()) {
            const auto ival = p->indprop(prop_index, time_index, itop).index_value();
            prop[itop] = &(p->zgrid[ival]);
        }
        else {
            if (p->indprop(prop_index, time_index, itop).is_constant()) {
                prop[itop] = nullptr;
                iret = -1; // Property is single value but val not provided
            }
        }
    }
    return iret;
}

int find_grid_functions_for_surface_index(const parm& p, parm::index_type prop_index, parm::index_type ifrom_time, parm::index_type ito_time, parm::index_type
                                          surface_index, std::vector<function_2d>* prop_p) {
    const int iret = 0;
	std::vector<function_2d>& prop(*prop_p);
	clip_parameters(ifrom_time, ito_time, 0, p.numtims() - 1);

	if (ito_time >= prop.size()) {
		prop.resize(ito_time + 1);
	}
	for (auto itop = ifrom_time; itop <= ito_time; itop++) {
		prop[itop] = p.indprop(prop_index, itop, surface_index);
	}

	return iret;

}
int find_grid_functions_for_timestep(const parm& p, parm::index_type prop_index, parm::index_type time_index, parm::index_type ifrom_top, parm::index_type
                                     ito_top, std::vector<function_2d>* prop_p) {
    const int iret = 0;
    std::vector<function_2d>& prop(*prop_p);
    clip_parameters(ifrom_top, ito_top, 0, p.numtops() - 1);

    if (ito_top >= prop.size()) {
        prop.resize(ito_top + 1);
    }
    for (auto itop = ifrom_top; itop <= ito_top; itop++) {
        prop[itop] = p.indprop(prop_index, time_index, itop);
    }

    return iret;
}

bool has_allocated_arrays_for_timestep(const parm& p, parm::index_type prop_index, parm::index_type time_index) {
	bool has_arrays = false;
	for (auto itop = 0; itop < p.ageset.size() && !has_arrays; ++itop) {
		auto& prop = p.indprop(prop_index, time_index, itop);
		has_arrays = has_arrays || (prop.is_initialized() && !prop.is_constant());
	}
	return has_arrays;
}
int find_grid_diagonal_indexes(const parm& p, parm::index_type prop_index, std::vector<int>* prop_p)
{
    std::vector<int>& prop(*prop_p);
    if (p.numtims() > prop.size() ) {
        prop.resize(p.numtims());
    }
    for(auto i = 0; i < p.numtims();++i) {
        auto last_ind = -1;
        for(auto j = 0; j < p.numtops();++j) {
            if (p.indprop(prop_index,i,j).is_initialized()) {
                last_ind=j;
            }
        }
        prop[i]=last_ind;
    }
    return 0;
}

// Create the grids for the tops in the given timestep
void create_grids_for_timestep(parm* p, parm::index_type iprop, parm::index_type itim, parm::index_type ifm, parm::index_type ito, float init_val,
                               const std::vector<std::string>& names)
{
    // Finds grid pointers for property iprop and time itim in ts matrix
    // ifm, ito are inclusive stratigraphic bounds.
    // prop and val (if val is not NULL) must have at least ito elements
    // If iappend is true, then overwrites matrix entries and creates new grids
    //    (i.e. increments grid count, grids must have been previously allocated,
    //     and copies values from previous time if applicable)
    // Assigns grid pointers to prop, which must be allocated by caller
    //   to p->numtops() elements of type float**
    // If entry in ts matrix is a value instead of a pointer, prop[i] is returned as NULL and we return -1
    // If grids are new, copies values from previous time, if available
    // Return value: 0: retuned prop or val OK, -1: Error

    // Needs revision: could produce memory leak if prop allocated outside

    if ((ifm < 0) || (ito > p->numtops() - 1)){
        return;//ito = p->numtops() - 1;
    }


    const auto num_to_append = ito - ifm + 1;
    if (num_to_append > 0 && num_to_append == names.size()) {
        const auto old_levels = p->zgrid.layers(); // Assuming no change in row col layout
        // Set them to znull;
        p->zgrid.resize(old_levels+num_to_append, p->zgrid.rows(), p->zgrid.cols(), true);
        p->gridnam.resize(old_levels+num_to_append);
        // Append the file names
        auto out_it = p->gridnam.rbegin();
        for (auto it = names.rbegin(); it != names.rend(); ++it, ++out_it) {
            *out_it = *it;
        }
		auto i1 = old_levels;
        for (auto itop = ifm; itop <= ito; ++itop, ++i1) {
            // Todo couple this to creation somehow
            const auto zgrid_index = i1;
            p->indprop(iprop, itim, itop).set_index(zgrid_index);
            p->indprop(iprop, itim, itop).set_array(
                std::shared_ptr<float_3darray_vec_base_type>(&(p->zgrid), boost::null_deleter()));
            // Allocate new grid if needed

            float_2darray_base_type& prop(p->zgrid[zgrid_index]); // Append // *#*#
            // Copy from previous time
            if (itim > 0) {
                if (p->indprop(iprop, itim - 1, itop).is_index()) {
                    const auto ival = p->indprop(iprop, itim - 1, itop).index_value();
                    float_2darray_base_type& old_prop(p->zgrid[ival]);
                    prop.fill(old_prop);
                }
                else if (p->indprop(iprop, itim - 1, itop).is_constant()) {
                    prop.fill(p->indprop(iprop, itim - 1, itop).constant_value());
                }
                else {
                    prop.fill(init_val);
                }
            }
            else {
                prop.fill(init_val);
            }
        }

    }
}

}}}
