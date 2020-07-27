// -- Schlumberger Private --

#ifndef gpm_surface_data_io_h
#define gpm_surface_data_io_h


#include "general_param_mapper.h"
#include "acf_base_reader.h"
#include "gpm_logger.h"
#include <string>
#include <vector>
#include "gpm_vbl_array_2d.h"
#include "gpm_vbl_array_3d.h"

namespace Slb { namespace Exploration { namespace Gpm {

class gpm_surface_data_io {
public:
    gpm_surface_data_io();
    virtual ~gpm_surface_data_io();
    int read_parameters(const std::string& param_file, const Tools::gpm_logger& logger);
    static std::string get_process_name();
    bool are_parameters_ok(const Tools::gpm_logger& logger) const;
    int read_parameters_from_string(const std::string& param_string, const Tools::gpm_logger& logger);
    int write_file(const std::string& param_file, const std::vector<std::pair<std::string, float_2darray_base_type*>>& surfaces, const std::vector<float>& x_corners, const std::vector<float>& y_corners, const Tools::gpm_logger& logger);
    std::vector<float> x_corners() const;
    std::vector<float> y_corners() const;
    std::vector<std::string> grid_names() const;
    bool get_surface_data(const std::string& id, float_2darray_type* res) const;
    std::string version() const;
    float no_val() const;
private:
    int check_parsed_state(acf_base_reader& sediment_param_parser, const std::string& param_file, const Tools::gpm_logger& logger);

    std::shared_ptr<float_3darray_vec_type> z_array() const;

    bool _parameters_ok;
    bool _parsed;
    int _num_read;
    std::vector<int> _missing_indexes;
    general_param_read_mapper _mapper;
//    std::shared_ptr<gpm_1d_linear_array2d> _sealevel_curve;
};

class gpm_surface_data_output_io {
public:
    gpm_surface_data_output_io();
    virtual ~gpm_surface_data_output_io();
    int write_file(const std::string& param_file, const std::vector<std::pair<std::string, float_2darray_base_type*>>& surfaces, const std::vector<float>& x_corners, const std::vector<float>& y_corners, float no_val, const Tools::gpm_logger& logger);
    int error_number() const;
    std::string error() const;
private:
    int _err_num;
    std::string _error;
};

}}}
#endif
