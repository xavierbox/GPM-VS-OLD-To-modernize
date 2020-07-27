// -- Schlumberger Private --

#ifndef vtk_writer_legacy_h
#define vtk_writer_legacy_h

// A wrapper for the acf writer currently used, but where we use signals to call back to the struct

#include "mapstc.h"
#include "gpm_type_descriptor.h"
#include "gpm_model_definition.h"
#include "acf_base_writer.h"

namespace Slb { namespace Exploration { namespace Gpm {

class vtk_writer_legacy {
public:
    // Perhaps we should do it all by array?
    typedef std::vector<acf_base_writer::io_array_dim_type> arrray_dim_size_type;
	typedef acf_base_writer::parameter_num_type parameter_num_type;
    vtk_writer_legacy();
    ~vtk_writer_legacy() = default;
    void write(const gpm_model_definition& model);

    void set_write_info(const std::string& fil_name, grid_parameter_type* param,
                        const std::vector<TypeDescr>& type_info,
                        const parameter_num_type& inumr, const arrray_dim_size_type& idimr);
private:
    int writevtk(const gpm_model_definition& model);

    // Parse info
    FILE* m_filin;
    std::string m_fil_name;
    grid_parameter_type* m_parm;
    std::vector<TypeDescr> m_type_info;
    parameter_num_type m_inumr;
    arrray_dim_size_type m_idimr;

};

}}}
#endif
