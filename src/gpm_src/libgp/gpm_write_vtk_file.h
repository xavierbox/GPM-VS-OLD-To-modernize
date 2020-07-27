// -- Schlumberger Private --

#ifndef GPM_WRITE_VTK_FILE_H
#define GPM_WRITE_VTK_FILE_H
#include "gpm_int_extent_2d.h"
#include "gpm_vbl_array_2d.h"
#include "gpm_model_definition.h"
#include <vector>
#include <functional>


namespace Slb { namespace Exploration { namespace Gpm {

// write a file to vtk
// collect them in one class for simplicity
class gpm_vtk_output {
public:
    typedef std::function<float(int, int)> accessor_2d_float_type; // Get the array or constant values
    typedef std::function<int(int, int)> accessor_2d_int_type; // Get the array or constant values
    typedef std::pair<accessor_2d_float_type, accessor_2d_float_type> vector_accessor_2d_type;
    typedef std::function<void(int, int, float*, float*)> coordinate_converter_type;
    typedef std::pair<std::string, std::vector<accessor_2d_float_type>> name_accessor_float_type;
    typedef std::pair<std::string, std::vector<accessor_2d_int_type>> name_accessor_int_type;
    typedef std::pair<std::string, std::vector<vector_accessor_2d_type>> name_vector_accessor_type;
    int_extent_2d extent;
    coordinate_converter_type convert_to_world;
    name_accessor_float_type geometries;
    std::vector<name_accessor_float_type> float_properties;
    std::vector<name_accessor_int_type> int_properties;
    std::vector<name_vector_accessor_type> vector_properties;
    // simple utility functions
    void setup_simple_conversion(const int_extent_2d& ext, float row_height, float column_width);
    void setup_constant_geometry(const std::string& name, float const_val);
    void setup_model_conversion(const gpm_model_definition& model);
    void push_back_float(const std::string& name, const gpm_array_2d_base<float>& data);
    void push_back_float(const std::string& name, const accessor_2d_float_type& data);
    void push_back_vector(const std::string& name, const gpm_array_2d_base<float>& vx, const gpm_array_2d_base<float>& vy);
    int write_file(const std::string& file_name);
    int write_file_to_unstructured(const std::string& file_name);
    int write_file_to_new_unstructured(const std::string& file_name);
};

}}}
#endif
