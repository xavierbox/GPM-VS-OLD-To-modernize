// -- Schlumberger Private --

#include "gpm_format_helper.h"
#include "gpm_function_2d.h"
#include "gpm_numerics.h"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <set>

namespace Slb { namespace Exploration { namespace Gpm {
       
std::string gpm_format_helper::to_string(const function_2d& func)
{
    if(func.is_initialized()) {
        if (func.is_constant()) {
			return std::to_string(func.constant_value());
        }
        return index_prefix+ std::to_string(func.index_value());
    }
	return std::string();
}

bool gpm_format_helper::is_empty_string(const std::string& txt) {
    bool res = txt.empty();
    if (!res) {
        std::string tmp(txt);
        boost::trim(tmp);
        res = tmp.empty();
    }
    return res;
}

bool gpm_format_helper::has_index_prefix(const std::string& txt) {
    return (!txt.empty() && boost::starts_with(txt, index_prefix));
}

int gpm_format_helper::find_index(const std::string& txt) {
    int ind1 = -1;
    if (has_index_prefix(txt)) {
        std::string tmp(txt);
        boost::erase_all(tmp, index_prefix);
        try {
            ind1 = boost::lexical_cast<int>(tmp);
        }
        catch (const boost::bad_lexical_cast& exp) {
			ind1 = -1; // Not needed
        }
    }
    return ind1;
}

float gpm_format_helper::find_constant(const std::string& txt) {
    float res;
    try {
        res = boost::lexical_cast<float>(txt);
    }
    catch (boost::bad_lexical_cast&) {
        res = std::numeric_limits<float>::quiet_NaN();
    }
    return res;
}

std::string gpm_format_helper::get_incremental_file_name(const std::string& filename, const int incremental_index)
{
	const auto suffix_string = "." + std::to_string(incremental_index) + "_incr.hdf5";
	const auto last_index = filename.find_last_of(".");
	const auto file_name_without_extension = filename.substr(0, last_index);
	return file_name_without_extension + suffix_string;
}

int gpm_format_helper::check_if_valid_index(const std::string& txt, const gpm_array_3d_vec_base<float>& area_func, const Slb::Exploration::Gpm::Tools::gpm_logger& logger, const std::string& error_txt) {

    int num_errors = 0;
    if (!txt.empty()) {
        if (has_index_prefix(txt)) {
            const int ind1 = find_index(txt);
            if (ind1 >= 0) {
                if (!area_func.layer_extent().contains(ind1)) {
                    logger.print(Tools::LOG_IMPORTANT, "%s index not in function array %s\n", error_txt.c_str(), txt.c_str());
                    ++num_errors;
                }
            }
            else {
                logger.print(Tools::LOG_IMPORTANT, "%s index wrongly spelled %s\n", error_txt.c_str(), txt.c_str());
                ++num_errors;
            }
        }
        else {
            // Must be a constant
            const float res = find_constant(txt);
            if (gpm_isnan(res)) {
                logger.print(Tools::LOG_IMPORTANT, "%s constant in function array %s is wrong\n", error_txt.c_str(), txt.c_str());
                ++num_errors;
            }
        }
    }
    return num_errors;
}

int gpm_format_helper::check_if_value_correct(const std::string& txt, 
	const gpm_array_3d_vec_base<float>& property_functions, 
	const std::function<bool(const float_2darray_base_type&)>& validation_func, 
	const std::function<bool(float)>& const_validation_func,
	const Slb::Exploration::Gpm::Tools::gpm_logger& logger, 
	const std::string& error_txt) {

	int num_errors = 0;
	if (!txt.empty()) {
		if (has_index_prefix(txt)) {
		    const int k = find_index(txt);
			if (k >= 0) {
				if (!property_functions.layer_extent().contains(k)) {
					logger.print(Tools::LOG_IMPORTANT, "%s index not in function array %s\n", error_txt.c_str(), txt.c_str());
					++num_errors;
				}
				else {
					const float_2darray_base_type& func(property_functions(k));
					if (!validation_func(func)) {
						logger.print(Tools::LOG_IMPORTANT, "%s property function %s is not valid.\n", error_txt.c_str(), txt.c_str());
						++num_errors;
					}
				}
			}
			else {
				logger.print(Tools::LOG_IMPORTANT, "%s index wrongly spelled %s\n", error_txt.c_str(), txt.c_str());
				++num_errors;
			}
		}
		else {
		    const float res = find_constant(txt);
			if (gpm_isnan(res)) {
				logger.print(Tools::LOG_IMPORTANT, "%s constant in function array %s is wrong\n", error_txt.c_str(), txt.c_str());
				++num_errors;
			}
			else {
				if (!const_validation_func(res)) {
					logger.print(Tools::LOG_IMPORTANT, "%s constant in function array %s is not valid.\n", error_txt.c_str(), txt.c_str());
					++num_errors;
				}
			}
		}
	}
	return num_errors;
}

// The helper class
int gpm_format_helper::check_for_valid_entries(const gpm_array_2d_base<std::string>& area_prop,
                                               const std::vector<std::string>& valid_entries,
                                               const std::string& error_txt,
                                               const Tools::gpm_logger& logger) {
    std::set<std::string> entries(area_prop.begin(), area_prop.end());
    std::set<std::string> valid(valid_entries.begin(), valid_entries.end());
    std::vector<std::string> possible_wrong;
    auto it = std::set_difference(entries.begin(), entries.end(), valid_entries.begin(), valid_entries.end(),
                                  std::back_inserter(possible_wrong));
    int num_errors = 0;
    if (!possible_wrong.empty()) {
        // Better have only empty stuff
        const auto it2 = std::stable_partition(possible_wrong.begin(), possible_wrong.end(), &is_empty_string);
        if (it2 < possible_wrong.end()) {
            const auto num = std::distance(it2, possible_wrong.begin());
            logger.print(Tools::LOG_IMPORTANT, "%s has %d invalid entries\n", error_txt.c_str(), num);
            num_errors += boost::numeric_cast<int>(num);
        }
    }
    return num_errors;
}

int gpm_format_helper::get_function_index(const std::string& text_index) {
    auto txt = text_index;
    boost::trim(txt); //No space at the end by accident
    return find_index(txt);
}

bool gpm_format_helper::has_function_indexes(const std::vector<std::string>& area_prop) {
    bool has_index = false;
    for (auto it = area_prop.begin(); it != area_prop.end() && !has_index; ++it) {
        // Find the indexes
        has_index = get_function_index(*it) >= 0;
    }
    return has_index;
}

bool gpm_format_helper::has_function_indexes(const gpm_array_2d_base<std::string>& area_prop) {
    bool has_index = false;
    for (auto it = area_prop.begin(); it != area_prop.end() && !has_index; ++it) {
        // Find the indexes
        auto txt = *it;
        boost::trim(txt); //No space at the end by accident
        has_index = find_index(txt) >= 0;
    }
    return has_index;
}

int gpm_format_helper::check_function_indexes(const std::vector<std::string>& area_prop,
                                              const gpm_array_3d_vec_base<float>& area_func,
                                              const std::string& error_txt,
                                              const Tools::gpm_logger& logger) {
    int num_errors = 0;
    for (auto txt : area_prop) {
        // Find the indexes
        boost::trim(txt); //No space at the end by accident
        num_errors += check_if_valid_index(txt, area_func, logger, error_txt);
    }
    return num_errors;
}

int gpm_format_helper::check_function_indexes(const gpm_array_2d_base<std::string>& area_prop,
                                              const gpm_array_3d_vec_base<float>& area_func,
                                              const std::string& error_txt,
                                              const Tools::gpm_logger& logger) {
    int num_errors = 0;
    for (auto txt : area_prop) {
        boost::trim(txt);
        num_errors += check_if_valid_index(txt, area_func, logger, error_txt);
    }

    return num_errors;
}

int gpm_format_helper::check_if_value_correct(const gpm_array_2d_base<std::string>& property_map,
											 const gpm_array_3d_vec_base<float>& property_functions,
	                                         const std::function<bool(const float_2darray_base_type& values)>& validation_func,
											 const std::function<bool(float)>& const_validation_func,
											 const std::string& error_txt,
											 const Tools::gpm_logger& logger) {
	int num_errors = 0;
	for (auto txt : property_map) {
	    boost::trim(txt);
		num_errors += check_if_value_correct(txt, property_functions, validation_func, const_validation_func, logger, error_txt);
	}

	return num_errors;
}

std::shared_ptr<gpm_1d_interpolator> gpm_format_helper::make_function(const gpm_vbl_vector_array_3d<float>& funcs, float znull, const std::string& txt) {
	std::shared_ptr<gpm_1d_interpolator> func;
	if (has_index_prefix(txt)) {
	    const auto ind1 = find_index(txt);
		if (ind1 >= 0) {
			func = std::shared_ptr<gpm_1d_interpolator>(new gpm_1d_linear_array2d(funcs(ind1), znull));
		}
	}
	else {
	    const float loc_res = find_constant(txt);
		if (!gpm_isnan(loc_res)) {
			func = std::shared_ptr<gpm_1d_interpolator>(new gpm_1d_constant(loc_res));
		}
	}
	return func;
}

std::shared_ptr<gpm_1d_integration> gpm_format_helper::make_integrator(const gpm_vbl_vector_array_3d<float>& funcs, float znull, const std::string& txt) {
    std::shared_ptr<gpm_1d_integration> func;
    if (has_index_prefix(txt)) {
        const auto ind1 = find_index(txt);
        if (ind1 >= 0) {
            const auto interp = std::shared_ptr<gpm_1d_linear_array2d>(new gpm_1d_linear_array2d(funcs(ind1), znull));
            func = std::shared_ptr<gpm_1d_integration>(new gpm_1d_trapezoid_integration_array2d(interp));
        }
    }
    else {
        const float loc_res = find_constant(txt);
        if (!gpm_isnan(loc_res)) {
            func = std::shared_ptr<gpm_1d_integration>(new gpm_1d_constant_integration(loc_res));
        }
    }
    return func;
}
std::vector<std::shared_ptr<gpm_1d_interpolator>>
gpm_format_helper::make_functions(const std::vector<std::string>& props, const gpm_vbl_vector_array_3d<float>& funcs, float znull) {
    std::vector<std::shared_ptr<gpm_1d_interpolator>> res;
    for (std::string txt : props) {
        boost::trim(txt); //No space at the end by accident
        if (!txt.empty()) {
			std::shared_ptr<gpm_1d_interpolator> func= make_function(funcs, znull, txt);
			if ( func)
			{
				res.push_back(func);
			}
        }
    }
    return res;
}

std::vector<std::shared_ptr<gpm_1d_interpolator>>
gpm_format_helper::make_functions(const gpm_vbl_vector_array_3d<float>& funcs, float znull) {
    std::vector<std::shared_ptr<gpm_1d_interpolator>> res;
    for (int i = 0; i < funcs.layers(); ++i) {
        res.push_back(std::shared_ptr<gpm_1d_interpolator>(new gpm_1d_linear_array2d(funcs(i), znull)));
    }
    return res;
}

std::vector<std::shared_ptr<gpm_1d_integration>>
gpm_format_helper::make_integrators(const gpm_vbl_vector_array_3d<float>& funcs, float znull) {
    std::vector<std::shared_ptr<gpm_1d_integration>> res;
    for (int i = 0; i < funcs.layers(); ++i) {
        auto interp = std::shared_ptr<gpm_1d_linear_array2d>(new gpm_1d_linear_array2d(funcs(i), znull));
        res.push_back(std::shared_ptr<gpm_1d_integration>(new gpm_1d_trapezoid_integration_array2d(interp)));
    }
    return res;
}

std::vector<std::shared_ptr<gpm_1d_integration>>
gpm_format_helper::make_integrators(const std::vector<std::string>& props, const gpm_vbl_vector_array_3d<float>& funcs, float znull) {
    std::vector<std::shared_ptr<gpm_1d_integration>> res;
    for (std::string txt : props) {
        boost::trim(txt); //No space at the end by accident
        if (!txt.empty()) {
            std::shared_ptr<gpm_1d_integration> func = make_integrator(funcs, znull, txt);
            if (func)
            {
                res.push_back(func);
            }
        }
    }
    return res;
}

std::vector<std::vector<std::string>> gpm_format_helper::copy_to_vector_of_vector(const gpm_array_2d_base<std::string>& src_id_prop) {
    bool row_ok = true;
    auto row_ext = src_id_prop.row_extent();
    auto col_ext = src_id_prop.col_extent();
    std::vector<std::vector<std::string>> res;
    for (auto i = row_ext.lower(); i < row_ext.upper() && row_ok; ++i) {
        bool col_ok = true;
        std::vector<std::string> col_res;
        for (auto j = col_ext.lower(); j < col_ext.upper() && col_ok; ++j) {
            // Find the indexes
            auto txt = src_id_prop(i, j);
            boost::trim(txt); //No space at the end by accident
            if (!txt.empty()) {
                // has to be an integer
                col_res.push_back(txt);
            }
            else {
                col_ok = false;
                if (j == 0) {
                    row_ok = false;
                }
            }
        }
        res.push_back(col_res);
    }
    return res;
}

std::vector<std::string> gpm_format_helper::copy_row_to_vector(const gpm_array_2d_base<std::string>& prop, int row_num)
{
    auto ext = prop.row_extent();
    std::vector<std::string> res;
    if (ext.contains(row_num)) {
        const auto start_it = prop.begin()+row_num*prop.cols();
        const auto end_it = start_it + prop.cols();
        std::copy_if(start_it, end_it, std::back_inserter(res),[](std::string val)
        {
            boost::trim(val); //No space at the end by accident
            return !val.empty();
        });
       std::for_each(res.begin(), res.end(), [](std::string& val){boost::trim(val);});
    }
    return res;
}
int gpm_format_helper::copy_row_to_vector(const gpm_array_2d_base<std::string>& src_id_prop,
                                          int row_num,
                                          std::vector<std::string>* ids,
                                          const std::string& error_txt,
                                          const Tools::gpm_logger& logger) {
    auto ext = src_id_prop.row_extent();
    int num_errors = 0;
    if (ext.contains(row_num)) {
        auto col_ext = src_id_prop.col_extent();
        for (auto j = col_ext.lower(); j < col_ext.upper(); ++j) {
            // Find the indexes
            auto txt = src_id_prop(row_num, j);
            boost::trim(txt); //No space at the end by accident
            if (!txt.empty()) {
                // has to be an integer
                ids->push_back(txt);
            }
        }
    }
    else {
        logger.print(Tools::LOG_IMPORTANT, "%s index out of range of extent \n", error_txt.c_str());
        ++num_errors;
    }
    return num_errors;
}

// First empty position stops filling
int gpm_format_helper::convert_row_to_vector(const gpm_array_2d_base<std::string>& src_id_prop,
                                             int row_num,
                                             std::vector<int>* ids,
                                             const std::string& error_txt,
                                             const Tools::gpm_logger& logger) {
    int num_errors = 0;
    auto col_ext = src_id_prop.col_extent();
    bool is_ok = true;
    for (auto j = col_ext.lower(); j < col_ext.upper() && is_ok; ++j) {
        // Find the indexes
        auto txt = src_id_prop(row_num, j);
        boost::trim(txt); //No space at the end by accident
        if (!txt.empty()) {
            // has to be an integer
            try {
                auto id = boost::lexical_cast<int>(txt);
                ids->push_back(id);
            }
            catch (const boost::bad_lexical_cast&) {
                logger.print(Tools::LOG_IMPORTANT, "%s index not integer %s\n", error_txt.c_str(), txt.c_str());
                ++num_errors;
                is_ok = false;
            }
        }
        else {
            is_ok = false;
        }
    }
    return num_errors;
}

int gpm_format_helper::check_integer_indexes(const gpm_array_2d_base<std::string>& src_id_prop,
                                             const gpm_array_3d_vec_base<int>& src_id_pos,
                                             const std::string& error_txt,
                                             const Tools::gpm_logger& logger) {
    int num_errors = 0;
    auto row_ext = src_id_prop.row_extent();
    auto col_ext = src_id_prop.col_extent();

    for (auto i = row_ext.lower(); i < row_ext.upper(); ++i) {
        std::set<int> ids;
        for (auto j = col_ext.lower(); j < col_ext.upper(); ++j) {
            // Find the indexes
            auto txt = src_id_prop(i, j);
            boost::trim(txt); //No space at the end by accident
            if (!txt.empty()) {
                // has to be an integer
                try {
                    auto id = boost::lexical_cast<int>(txt);
                    ids.insert(id);
                }
                catch (const boost::bad_lexical_cast&) {
                    logger.print(Tools::LOG_IMPORTANT, "%s index not integer %s\n", error_txt.c_str(), txt.c_str());
                    ++num_errors;
                }
            }
        }
        if (!ids.empty()) {
            const gpm_array_2d_base<int>& pos_array(src_id_pos[i]);
            std::set<int> found;
            std::copy_if(pos_array.begin(), pos_array.end(), std::inserter(found, found.end()), std::bind(std::greater<int>(), std::placeholders::_1, 0));
            std::vector<int> diff;
            std::set_difference(ids.begin(), ids.end(), found.begin(), found.end(), std::back_inserter(diff));
            if (!diff.empty()) {
                logger.print(Tools::LOG_IMPORTANT, "%s indexes not in position array %s\n", error_txt.c_str());
                ++num_errors;
            }
        }

    }
    return num_errors;
}

std::shared_ptr<gpm_1d_interpolator> gpm_format_helper::setup_1d_interpolator(std::string prop, const std::shared_ptr<gpm_array_3d_vec_base<float>>& arr, float znull) {
    boost::trim(prop); //No space at the end by accident
    std::shared_ptr<gpm_1d_interpolator> res = std::shared_ptr<gpm_1d_interpolator>(new gpm_1d_constant(1.0));
    if (!prop.empty()) {
        if (has_index_prefix(prop)) {
            const int ind1 = find_index(prop);
            if (ind1 >= 0) {
                if (arr->layer_extent().contains(ind1)) {
                    res = std::shared_ptr<gpm_1d_interpolator>(new gpm_1d_linear_array2d((*arr)[ind1], znull));
                }
            }
        }
        else {
            // Must be a constant
            const float loc_res = find_constant(prop);
            if (!gpm_isnan(loc_res)) {
                res = std::shared_ptr<gpm_1d_interpolator>(new gpm_1d_constant(loc_res));
            }
        }
    }
    return res;
}

void gpm_format_helper::setup_function_2d(std::string prop, const std::shared_ptr<gpm_array_3d_vec_base<float>>& arr, function_2d* func) {
    boost::trim(prop); //No space at the end by accident
    if (!prop.empty()) {
        if (has_index_prefix(prop)) {
            const int ind1 = find_index(prop);
            if (ind1 >= 0) {
                if (arr->layer_extent().contains(ind1)) {
                    func->set_index(ind1);
                    func->set_array(arr);
                }
            }
        }
        else {
            // Must be a constant
            const float loc_res = find_constant(prop);
            if (!gpm_isnan(loc_res)) {
                func->set_constant(loc_res);
            }
        }
    }

}

void gpm_format_helper::set_extent_for_constants(const int_extent_2d& ext, gpm_array_2d_base<function_2d>* res)
{
    for (function_2d& func: *res) {
        if (func.is_initialized() && func.is_constant()) {
            func.set_constant(func.constant_value(), ext);
        }
    }
}
void gpm_format_helper::setup_function_array(const std::vector<std::string>& props,
                                             const std::shared_ptr<gpm_array_3d_vec_base<float>>& arr,
                                             std::vector<function_2d>* res) {

    for (auto i = 0; i < props.size(); ++i) {
        // Find the indexes
        const auto& txt = props[i];
        setup_function_2d(txt, arr, &res->at(i));
    }
}

void gpm_format_helper::setup_function_array(const gpm_array_2d_base<std::string>& props,
                                             const std::shared_ptr<gpm_array_3d_vec_base<float>>& arr,
                                             gpm_array_2d_base<function_2d>* res) {
    auto row_ext = props.row_extent();
    auto col_ext = props.col_extent();

    for (auto i = row_ext.lower(); i < row_ext.upper(); ++i) {
        for (auto j = col_ext.lower(); j < col_ext.upper(); ++j) {
            // Find the indexes
            const auto& txt = props(i, j);
            setup_function_2d(txt, arr, &res->at(i, j));
        }
    }

}

std::string gpm_format_helper::index_prefix = "#";

}}}
