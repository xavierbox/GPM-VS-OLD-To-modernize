// -- Schlumberger Private --

#include "gpm_parm_methods.h"
#include "acf_base_reader.h"
#include "acf_base_writer.h"
#include "acf_base_helper.h"
#include "gpm_logger.h"
#include "libgp.h"
#include "gpm_numerics.h"
#include <boost/numeric/conversion/cast.hpp>
#include <boost/range/combine.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/core/null_deleter.hpp>
#include <numeric>
#include "gpm_string_utils.h"

namespace Slb { namespace Exploration { namespace Gpm {
// All the locals
namespace {
void check_and_set_nodal(std::vector<int>* is_nodal, int index, int val)
{
    if (index >= is_nodal->size()) {
        is_nodal->resize(index + 1);
    }
    is_nodal->at(index) = val;
}

std::vector<int> find_indexes_equal_to(const std::string& name, const std::vector<std::string>& vec) {
    std::vector<int> res;
    for (int i = 0; i < vec.size(); ++i) {
        if (boost::iequals(vec[i], name)) {
            res.push_back(i);
        }
    }
    return res;
}


}
void assign_zgrid_to_indprop(parm* p)
{
	const bool do_extents = p->zgrid.size() > 0;
	int_extent_2d exts{ p->zgrid.row_extent(), p->zgrid.col_extent() };
	for (auto i = 0u; i < p->indprop.layers(); ++i) {
		function_2darray_type& tmp(p->indprop[i]);
        for (auto j = 0u; j < tmp.rows(); ++j) {
            for (auto k = 0u; k < tmp.cols(); ++k) {
                tmp(j, k).set_array(std::shared_ptr<float_3darray_vec_base_type>(&(p->zgrid), boost::null_deleter()));
                if (tmp(j,k).is_constant() && do_extents) {
					tmp(j, k).set_constant(tmp(j, k).constant_value(), exts);
                }
            }
        }
    }
}

void gpm_check_erodability_properties(parm* p_p, const std::vector<float>& erodibility, const std::vector<property_range_item>& needs, int last_index)
{
    const auto last_time = p_p->numtims() - 1;
	for(auto& index:needs) {
		if (!p_p->indprop(index.indprop_index)(last_time, 0).is_initialized()) {
			p_p->indprop(index.indprop_index)(last_time, 0).set_constant(p_p->base_erodability);
		}
        for( int top=1; top <= last_index;++top) {
			if (!p_p->indprop(index.indprop_index)(last_time, top).is_initialized()) {
				p_p->indprop(index.indprop_index)(last_time, top).set_constant(erodibility[index.range_index]);
			}
        }
    }
}

std::vector<int> gpm_add_needed_properties(parm* p_p, const std::vector<property_description>& needs,
                                           std::map<std::string, bool>* created, const Tools::gpm_logger& /*logger*/)
{
    std::vector<int> iret;
    std::map<std::string, bool>& create_map(*created);
    for (const auto& need : needs) {
        int indx;
        bool isnew;
        iret.push_back(find_or_add_property(p_p, need.m_prop_id_, need.m_prop_display_name_, 0, &indx, &isnew));
        create_map[need.m_prop_id_] = isnew;
        check_and_set_nodal(&p_p->isnodal, indx, need.m_is_nodal_);
    }
    return iret;
}

void find_min_max_values(const function_2d& func, float no_val, float* min_val_p, float* max_val_p)
{
    float min_old = *min_val_p;
    float max_old = *max_val_p;
    float min_new = no_val;
    float max_new = no_val;
    if (func.is_initialized()) {
        if (func.is_constant()) {
            min_new = func.constant_value();
            max_new = func.constant_value();
        }
        else {
            for (auto i = func.extents().row_extent().lower(), nr = func.extents().row_extent().upper(); i < nr; ++i) {
                for (auto j = func.extents().col_extent().lower(), nc = func.extents().col_extent().upper(); j < nc; ++j) {
                    auto val = func(i, j);
                    if (val != no_val) {
                        min_new = (min_new != no_val) ? std::min(val, min_new) : val;
                        max_new = (max_new != no_val) ? std::max(val, max_new) : val;
                    }
                }
            }
        }
    }
    *min_val_p = min_old != no_val && min_new != no_val
                     ? std::min(min_old, min_new)
                     : min_new != no_val
                     ? min_new
                     : min_old;
    *max_val_p = max_old != no_val && max_new != no_val
                     ? std::max(max_old, max_new)
                     : max_new != no_val
                     ? max_new
                     : max_old;
}

void calculate_statistics(parm* p_p, int time_step,  const Tools::gpm_logger& logger)
{
    auto layers = p_p->indprop.layer_extent(); // The properties
    auto cols = p_p->indprop.col_extent();
    for (auto l = layers.lower(), nl = layers.upper(); l < nl; ++l) {
        float min_val = p_p->znull;
        float max_val = p_p->znull;
        for (auto j = cols.lower(), nc = cols.upper(); j < nc; ++j) {
            auto& func(p_p->indprop(l, time_step, j));
            find_min_max_values(func, p_p->znull, &min_val, &max_val);
        }
        p_p->indprop_stats(l, time_step, 0) = min_val;
        p_p->indprop_stats(l, time_step, 1) = max_val;
    }
}

void free_memory_from_previous_timestep(parm* p_p, const Tools::gpm_logger& logger, long curr_ts_)
{
	auto curr_ts = curr_ts_;
	if (curr_ts == 0)
		curr_ts = p_p->numtims();

	// Will free all arrays used in previous timestep if not referenced in current timestep
	if (curr_ts > 1) {
		const auto curr_time = curr_ts - 1;
		auto prev_time = curr_time - 1;
		std::vector<function_2d::index_type> curr_indexes;
		std::vector<function_2d::index_type> prev_indexes;
        for(auto i = 0; i < p_p->indprop.layers();++i) {
			const auto& propss = p_p->indprop(i);
            for(auto age=0; age < propss.cols();++age) {
                if (propss(curr_time, age).is_index()) {
					curr_indexes.push_back(propss(curr_time, age).index_value());
                }
				if (propss(prev_time, age).is_index()) {
					prev_indexes.push_back(propss(prev_time, age).index_value());
				}
			}
        }
		std::sort(curr_indexes.begin(), curr_indexes.end());
		std::sort(prev_indexes.begin(), prev_indexes.end());
		std::vector<function_2d::index_type> free_indexes;
		std::set_difference(prev_indexes.begin(), prev_indexes.end(), curr_indexes.begin(), curr_indexes.end(), std::back_inserter(free_indexes));
        for(auto i: free_indexes) {
			p_p->zgrid.clear(i);
        }
		logger.print(Tools::LOG_VERBOSE, "Released %d arrays\n", free_indexes.size());
    }
}

void preserve_only_last_timestep(parm* p_p, const Tools::gpm_logger& logger)
{
	// Will free all arrays used in previous timestep if not referenced in current timestep
	if (p_p->numtims() > 1) {
		const auto curr_time = p_p->numtims() - 1;
		std::vector<function_2d::index_type> curr_indexes;
		for (auto i = 0; i < p_p->indprop.layers(); ++i) {
			auto& propss = p_p->indprop(i);
			for (auto age = 0; age < propss.cols(); ++age) {
				if (propss(curr_time, age).is_index()) {
					curr_indexes.push_back(propss(curr_time, age).index_value());
				}
				propss(0, age) = propss(curr_time, age);
			}
		}
        // Fix it
		p_p->timeset.front() = p_p->timeset.back();
		p_p->timeset.resize(1);
		p_p->indprop.resize(p_p->indprop.layers(), 1, p_p->indprop.cols());
		p_p->indprop_stats.resize(p_p->indprop_stats.layers(), 1, p_p->indprop_stats.cols());

        // Find src as well
		std::vector<int> inds = find_indexes_equal_to("SRC", p_p->gridnam);
		if (!inds.empty()) {
			curr_indexes.push_back(inds.front());
		}

        // Now we got all the indexes we need, and the src
        // Sort by increasing, so we can add them
		std::sort(curr_indexes.begin(), curr_indexes.end());
		int count = 0;
		std::map<function_2d::index_type, function_2d::index_type> old_to_new_indexes;
        for(auto i: curr_indexes) {
			old_to_new_indexes.insert(std::make_pair(i, count));
			++count;
        }
		float_3darray_vec_type new_zgrid(int_extent_1d(curr_indexes.size()),p_p->zgrid.row_extent(), p_p->zgrid.col_extent());
		std::vector<std::string> grid_names(curr_indexes.size());
		for (auto i : curr_indexes) {
			grid_names[old_to_new_indexes[i]] = p_p->gridnam[i];
			new_zgrid(old_to_new_indexes[i]).fill(p_p->zgrid(i));
		}
		for (auto i = 0; i < p_p->indprop.layers(); ++i) {
			auto& propss = p_p->indprop(i);
			for (auto age = 0; age < propss.cols(); ++age) {
				if (propss(0, age).is_index()) {
					propss(0, age).set_index(old_to_new_indexes[propss(0, age).index_value()]);
				}
			}
		}
		p_p->zgrid = new_zgrid;
		p_p->gridnam = grid_names;
		assign_zgrid_to_indprop(p_p);
	}
}

void find_source_grid(const parm& p, int_2darray_type& srcgrid, const Tools::gpm_logger& logger)
{
    int intsrc = -1; // Index to sources grid in map file */

    std::vector<int> inds = find_indexes_equal_to("SRC", p.gridnam);
    if (!inds.empty()) {
        intsrc = inds.front();
        const float_2darray_base_type& src_holder(p.zgrid[intsrc]);
        srcgrid.resize(src_holder.rows(), src_holder.columns());
        for (int i = 0; i < src_holder.rows(); ++i) {
            for (int j = 0; j < src_holder.columns(); ++j) {
                const float float_val = src_holder(i, j);
                const float adjusted_val = (gpm_isnan(float_val) || float_val == p.znull)
                                               ? -1.0F
                                               : (float_val < 0 ? float_val - 0.5f : float_val + 0.5f);
                srcgrid(i, j) = static_cast<int>(adjusted_val);
            }
        }
    }
    logger.print(Tools::LOG_VERBOSE, "intsrc (index to sources grid in map file) = %d\n", intsrc);
}

// 
void gpm_set_erodability_factors(gpm_array_3d_vec_base<function_2d>* indprop_p,
                                 const std::vector<int>& erod_prop_indexes,
                                 int time_index, int age_index,
                                 const std::vector<float>& erodabilities)
{
	gpm_array_3d_vec_base<function_2d>& indprop(*indprop_p);
	for(auto tup:  boost::combine(erod_prop_indexes, erodabilities)) {
		int index;
		float erodability;
		boost::tie(index, erodability) = tup;
		if (!indprop(index, time_index, age_index).is_initialized() || indprop(index, time_index, age_index).is_constant()) {
			indprop(index, time_index, age_index).set_constant(erodability);
		}
		else {
			indprop(index, time_index, age_index).array_holder().fill(erodability);
		}
	}
}
int gpm_copy_previous_timestep(gpm_array_3d_vec_base<function_2d>& indprop, int to_time, int from_time, int num_tops,
                               const std::vector<int>& indexes)
{
    for (auto index : indexes) {
        for (int itop = 0; itop < num_tops; itop++) {
            indprop(index, to_time, itop) = indprop(index, from_time, itop);
        }
    }
    return 0;
}

int gpm_copy_property_previous_timestep(gpm_array_3d_vec_base<function_2d>& indprop, int to_time, int from_time,
                                        int num_tops, int indexes)
{
    for (int itop = 0; itop < num_tops; itop++) {
        const int index = indexes;
        indprop(index, to_time, itop) = indprop(index, from_time, itop);
    }

    return 0;
}

int gpm_copy_previous_tops_for_timesteps(gpm_array_3d_vec_base<function_2d>& indprop, int to_top, int from_top,
                                         int num_times, const std::vector<int>& indexes)
{
    for (auto index : indexes) {
        for (int itim = 0; itim < num_times; itim++) {
            indprop(index, itim, to_top) = indprop(index, itim, from_top);
        }
    }
    return 0;
}

std::map<std::string, std::pair<std::string, int>> gpm_find_sediment_id_propnam_map(const parm& p)
{
    std::map<std::string, std::pair<std::string, int>> amap;
    auto col_ext = p.sediment_prop_map.col_extent();
    if (p.sediment_prop_map.row_extent().size() > 2) {
        for (auto j = col_ext.lower(); j != col_ext.upper(); ++j) {
            const auto id = p.sediment_prop_map(0, j);
            const auto prop_name = p.sediment_prop_map(2, j);
            const auto it = std::find(p.propnam.begin(), p.propnam.end(), prop_name);
            if (it != p.propnam.end()) {
                amap[id] = std::make_pair(prop_name, static_cast<int>(std::distance(p.propnam.begin(), it)));
            }
            else {
                amap[id] = std::make_pair(prop_name, -1);
            }
        }
    }
    return amap;
}

std::map<std::string, std::pair<std::string, int>> gpm_find_sediment_id_prop_display_name_map(const parm& p)
{
	std::map<std::string, std::pair<std::string, int>> amap;
	auto col_ext = p.sediment_prop_map.col_extent();
	if (p.sediment_prop_map.row_extent().size() > 2) {
		for (auto j = col_ext.lower(); j != col_ext.upper(); ++j) {
			const auto id = p.sediment_prop_map(0, j);
			const auto prop_name = p.sediment_prop_map(2, j);
            const auto prop_display_name = p.sediment_prop_map(1, j);
			const auto it = std::find(p.propnam.begin(), p.propnam.end(), prop_name);
			if (it != p.propnam.end()) {
				amap[id] = std::make_pair(prop_display_name, static_cast<int>(std::distance(p.propnam.begin(), it)));
			}
			else {
				amap[id] = std::make_pair(prop_display_name, -1);
			}
		}
	}
	return amap;
}
int gpm_check_propnam_index_items(const parm& p, const std::vector<std::string>& prefixes, const Tools::gpm_logger& logger)
{
	int ret_val = 0;
	for (const auto it : prefixes) {
		auto prefix = it;
		auto indexes = find_properties_with_name_prefix(p, prefix);
		for (auto ind : indexes) {
			auto name = p.propnam[ind];
			int loc_int = -1;
			std::string loc_str;
			if (!(Tools::split_into_alpha_number(name, loc_str, loc_int) && prefix == loc_str)) {
				logger.print(Tools::LOG_NORMAL, "ERROR: Preexisting property index types %s wrong. Name is %s\n", prefix.c_str(), name.c_str());
				ret_val += 1;
			}
		}
		// Lets check that we have no duplicates
		if (!indexes.empty()) {
			std::vector<std::string> names;
			std::transform(indexes.begin(), indexes.end(), std::back_inserter(names), [&p](int ind) {return p.propnam[ind]; });
			std::sort(names.begin(), names.end());
			const auto res = std::unique(names.begin(), names.end());
			if (res != names.end()) {
				logger.print(Tools::LOG_NORMAL, "ERROR: Preexisting property index types are duplicated", prefix.c_str());
				ret_val += 1;
			}
		}
	}
	return ret_val;
}

}}}
