// -- Schlumberger Private --

#ifndef grid_param_reader_h
#define grid_param_reader_h

#include "mapstc.h"
#include "acf_base_writer.h"
#include <string>

namespace Slb { namespace Exploration { namespace Gpm {

class grid_param_reader {
public:
    // Do all the mapping needed
    template <typename T>
    grid_param_reader(grid_parameter_type& param, T& writer): m_param(param) {
        writer.connect_string(std::bind(&grid_param_reader::string_reader, this, std::placeholders::_1));
        writer.connect_int(std::bind(&grid_param_reader::int_reader, this, std::placeholders::_1));
        writer.connect_float(std::bind(&grid_param_reader::float_reader, this, std::placeholders::_1));
        writer.connect_double(std::bind(&grid_param_reader::double_reader, this, std::placeholders::_1));
        writer.connect_string_array(std::bind(&grid_param_reader::string_array_reader, this, std::placeholders::_1, std::placeholders::_2));
        writer.connect_int_array(std::bind(&grid_param_reader::int_array_reader, this, std::placeholders::_1, std::placeholders::_2));
        writer.connect_float_array(std::bind(&grid_param_reader::float_array_reader, this, std::placeholders::_1, std::placeholders::_2));
        writer.connect_double_array(std::bind(&grid_param_reader::double_array_reader, this, std::placeholders::_1, std::placeholders::_2));
    }

private:
    typedef int key_type;
	using index_type = acf_base_writer::index_type;
    std::string string_reader(const key_type& key) const;
    int int_reader(const key_type& key) const;
    float float_reader(const key_type& key) const;
    double double_reader(const key_type& key) const;
    std::string string_array_reader(const key_type& key, const std::vector<index_type>& indexes) const;
    int int_array_reader(const key_type& key, const std::vector<index_type>& indexes) const;
    float float_array_reader(const key_type& key, const std::vector<index_type>& indexes) const;
    double double_array_reader(const key_type& key, const std::vector<index_type>& indexes) const;

    grid_parameter_type& m_param;
};

}}}
#endif
