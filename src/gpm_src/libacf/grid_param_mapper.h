// -- Schlumberger Private --

#ifndef grid_param_mapper_h
#define grid_param_mapper_h

#include "mapstc.h"
#include "mapstc_descr.h"
#include "param_wrapper_helper.h"
#include "gpm_logger.h"
#include "acf_base_reader.h"


namespace Slb { namespace Exploration { namespace Gpm {

// This class should be moved to own class
class grid_param_wrapper {
public:
    typedef std::map<std::string, std::vector<int>> array_size_mapper_type;

    grid_param_wrapper(grid_parameter_type& param, bool initialize = true);

    grid_param_wrapper(grid_parameter_type& param, const parameter_descr_type& type_info, bool initialize = true);
    void initialize_struct(const parameter_descr_type& type_info);
    void initialize_struct();
    void free_struct();

    grid_parameter_type& m_param;
    param_wrapper_helper m_helper;
private:
    void free_arrays() const;
    void initialize_params() const;
    void initialize_pod_params() const;
    void initialize_params_in_arrays() const;
};


class grid_param_mapper:public acf_base_io_types {
public:

    // Do all the mapping needed
    grid_param_mapper(grid_param_wrapper& param, acf_base_reader& parser);
    // When we adjust, we need to also set the allocated array_sizes correct for now
    void adjust_arrays_to_read_size() const;
	void connect_indprop_to_zgrid() const;
    int check_sediment_entries(const Tools::gpm_logger& logger) const;
private:
    typedef acf_base_reader::array_dim_size_type::value_type fixed_array_type;

    void string_mapper(const key_type& key, const std::string& val) const;
    void int_mapper(const key_type& key, int val) const;
    void float_mapper(const key_type& key, float val) const;
    void double_mapper(const key_type& key, double val) const;

    void string_array_item_mapper(const key_type& key, const std::vector<index_type>& indexes, const std::string& val) const;
    void int_array_item_mapper(const key_type& key, const std::vector<index_type>& indexes, int val) const;
    void float_array_item_mapper(const key_type& key, const std::vector<index_type>& indexes, float val) const;
    void double_array_item_mapper(const key_type& key, const std::vector<index_type>& indexes, double val) const;

    void string_array_mapper(const key_type& key, const std::vector<index_type>& offset, const std::vector<size_type>& size, const std::vector<std::string>& values) const;
    void int_array_mapper(const key_type& key, const std::vector<index_type>& offset, const std::vector<size_type>& size, const std::vector<int>& values) const;
    void float_array_mapper(const key_type& key, const std::vector<index_type>& offset, const std::vector<size_type>& size, const std::vector<float>& values) const;
    void double_array_mapper(const key_type& key, const std::vector<index_type>& offset, const std::vector<size_type>& size, const std::vector<double>& values) const;

    // Return the newly trimmed dimension sizes
    void adjust_string_arrays(const TypeDescr& loc_type, const fixed_array_type& read_dims) const;
    void adjust_integer_arrays(const TypeDescr& loc_type, const fixed_array_type& read_dims) const;
    void adjust_float_arrays(const TypeDescr& loc_type, const fixed_array_type& read_dims) const;

    grid_param_wrapper& m_param;
    acf_base_reader& m_parser;
};
// To read the file
int gpm_read_map_file(const std::string& map_file_name, const std::string& file_type, parm& p, acf_base_reader::parameter_num_type& inumr, acf_base_reader::array_dim_size_type& idimr, const Tools::gpm_logger& logger);
}}}
#endif
