// -- Schlumberger Private --

#ifndef gpm_format_helper_h
#define gpm_format_helper_h

#include "gpm_basic_defs.h"
#include "gpm_logger.h"
#include "gpm_1d_interpolator.h"
#include <string>
#include <vector>
#include <memory>

namespace Slb { namespace Exploration { namespace Gpm {

class gpm_format_helper {
public:
    static std::string index_prefix;
    // Method to map index array to function 2d array. 
    // Assume that we deal with float arrays in 3d array
    // The props are either constants or indexes into the arr array
    // returns number if something went wrong in the parsing
    static int check_function_indexes(const gpm_array_2d_base<std::string>& area_prop,
                                      const gpm_array_3d_vec_base<float>& area_func,
                                      const std::string& error_txt,
                                      const Tools::gpm_logger& logger);
	static int check_if_value_correct(const gpm_array_2d_base<std::string>& property_map,
		                             const gpm_array_3d_vec_base<float>& property_functions,
		                             const std::function<bool(const float_2darray_base_type& values)>& validation_func,
		                             const std::function<bool(float)>& const_validation_func,
		                             const std::string& error_txt,
		                             const Tools::gpm_logger& logger);
	static std::shared_ptr<gpm_1d_interpolator> make_function(const gpm_vbl_vector_array_3d<float>& funcs, float znull, const std::string& txt);
    static std::shared_ptr<gpm_1d_integration> make_integrator(const gpm_vbl_vector_array_3d<float>& funcs, float znull, const std::string& txt);
    static std::vector<std::shared_ptr<gpm_1d_interpolator>> make_functions(const std::vector<std::string>& props, const gpm_vbl_vector_array_3d<float>& funcs, float znull);
    static int check_function_indexes(const std::vector<std::string>& area_prop, const gpm_array_3d_vec_base<float>& area_func, const std::string& error_txt, const Tools::gpm_logger& logger);
    static bool has_function_indexes(const std::vector<std::string>& area_prop);
    static bool has_function_indexes(const gpm_array_2d_base<std::string>& area_prop);
    static int check_for_valid_entries(const gpm_array_2d_base<std::string>& area_prop,
                                       const std::vector<std::string>& valid_entries,
                                       const std::string& error_txt,
                                       const Tools::gpm_logger& logger);
    static int get_function_index(const std::string& text_index);
    static int check_integer_indexes(const gpm_array_2d_base<std::string>& src_id_prop, const gpm_array_3d_vec_base<int>& src_id_pos, const std::string& error_txt, const Tools::gpm_logger& logger);
    static std::shared_ptr<gpm_1d_interpolator> setup_1d_interpolator(std::string prop, const std::shared_ptr<gpm_array_3d_vec_base<float>>& arr, float znull);
    static void setup_function_2d(std::string prop, const std::shared_ptr<gpm_array_3d_vec_base<float>>& arr, function_2d* func);
    static void set_extent_for_constants(const int_extent_2d& ext, gpm_array_2d_base<function_2d>* res);
    static int check_if_valid_index(const std::string& txt, const gpm_array_3d_vec_base<float>& area_func, const Slb::Exploration::Gpm::Tools::gpm_logger& logger, const std::string& error_txt);
	static int check_if_value_correct(const std::string& txt, const gpm_array_3d_vec_base<float>& property_functions, const std::function<bool(const float_2darray_base_type& values)>& validation_func,
		const std::function<bool(float)>& const_validation_func, const Slb::Exploration::Gpm::Tools::gpm_logger& logger, const std::string& error_txt);
    static std::string to_string(const function_2d& func);
    static bool is_empty_string(const std::string& str);
    static bool has_index_prefix(const std::string& txt);
    static int find_index(const std::string& txt);
    static float find_constant(const std::string& txt);

	static std::string get_incremental_file_name(const std::string& filename, const int incremental_index);
    static std::vector<std::shared_ptr<gpm_1d_interpolator>> make_functions(const gpm_vbl_vector_array_3d<float>& funcs, float znull);
    static std::vector<std::shared_ptr<gpm_1d_integration>> make_integrators(const gpm_vbl_vector_array_3d<float>& funcs, float znull);
    static std::vector<std::shared_ptr<gpm_1d_integration>> make_integrators(const std::vector<std::string>& props, const gpm_vbl_vector_array_3d<float>& funcs, float znull);

    static std::vector<std::vector<std::string>> copy_to_vector_of_vector(const gpm_array_2d_base<std::string>& src_id_prop);
    static std::vector<std::string> copy_row_to_vector(const gpm_array_2d_base<std::string>& prop, int row_num);

    static int copy_row_to_vector(const gpm_array_2d_base<std::string>& src_id_prop,
                                  int row_num,
                                  std::vector<std::string>* ids,
                                  const std::string& error_txt,
                                  const Tools::gpm_logger& logger);

    static int convert_row_to_vector(const gpm_array_2d_base<std::string>& src_id_prop,
                                     int row_num,
                                     std::vector<int>* ids,
                                     const std::string& error_txt,
                                     const Tools::gpm_logger& logger);

    // res array must have same size as props, at least
    static void setup_function_array(const gpm_array_2d_base<std::string>& props,
                                     const std::shared_ptr<gpm_array_3d_vec_base<float>>& arr,
                                     gpm_array_2d_base<function_2d>* res);
    static void setup_function_array(const std::vector<std::string>& props,
                                     const std::shared_ptr<gpm_array_3d_vec_base<float>>& arr,
                                     std::vector<function_2d>* res);

    template <typename T>
    static int check_existance_and_size(const std::vector<T>* input, int size,
                                        const std::string& error_txt,
                                        const Tools::gpm_logger& logger) {
        int num_errors = 0;
        if (input == nullptr) {
            logger.print(Tools::LOG_IMPORTANT, "%s is missing.\n", error_txt.c_str());
            ++num_errors;
        }
        else {
            if (input->size() != size) {
                logger.print(Tools::LOG_IMPORTANT, "Number of %s are wrong.\n", error_txt.c_str());
                ++num_errors;
            }
        }
        return num_errors;
    }

    template <typename T>
    static int check_existance_and_size(const gpm_array_2d_base<T>* input, int size,
                                        const std::string& error_txt,
                                        const Tools::gpm_logger& logger) {
        int num_errors = 0;
        if (input == nullptr) {
            logger.print(Tools::LOG_IMPORTANT, "%s is missing.\n", error_txt.c_str());
            ++num_errors;
        }
        else {
            if (input->rows() != size) {
                logger.print(Tools::LOG_IMPORTANT, "Number of %s are wrong.\n", error_txt.c_str());
                ++num_errors;
            }
        }
        return num_errors;
    }

    template <typename T>
    static int check_existance_and_size(const gpm_array_3d_vec_base<T>* input, int size,
                                        const std::string& error_txt,
                                        const Tools::gpm_logger& logger) {
        int num_errors = 0;
        if (input == nullptr) {
            logger.print(Tools::LOG_IMPORTANT, "%s is missing.\n", error_txt.c_str());
            ++num_errors;
        }
        else {
            if (input->layers() != size) {
                logger.print(Tools::LOG_IMPORTANT, "Number of %s are wrong.\n", error_txt.c_str());
                ++num_errors;
            }
        }
        return num_errors;
    }
};

}}}
#endif
