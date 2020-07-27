// -- Schlumberger Private --

#ifndef VTK_READER_VTU_H
#define VTK_READER_VTU_H

// A wrapper for the acf writer currently used, but where we use signals to call back to the struct

#include "mapstc.h"
#include "gpm_model_definition.h"
#include "acf_base_writer.h"
#include "gpm_logger.h"

#include <vector>

#include <boost/geometry.hpp>

namespace Slb { namespace Exploration { namespace Gpm {

class vtk_reader_vtu {
public:
    // Perhaps we should do it all by array?
    typedef std::vector<acf_base_writer::io_array_dim_type> arrray_dim_size_type;
    typedef acf_base_writer::parameter_num_type parameter_num_type;
	typedef boost::geometry::model::point <double, 3, boost::geometry::cs::cartesian> point;
	typedef std::vector<point > vec_point;

	struct property_stack_holder {
		std::string name;
		std::vector<float_2darray_base_type*> stack;
		property_stack_holder() = default;
		property_stack_holder(std::string aname, float_2darray_base_type* reader)
		{
			name = std::move(aname);
			stack.push_back(reader);
		}

		property_stack_holder(std::string aname, const std::vector<float_2darray_base_type*>& reader)
		{
			name = std::move(aname);
			stack = reader;
		}
	};
    vtk_reader_vtu();
    ~vtk_reader_vtu() = default;
	int reader(const std::string& prop_name/*, const gpm_model_definition& model*/,
		      std::vector<double>& pressure_array,const Tools::gpm_logger& logger);
	bool get_ogs_result_property(const gpm_model_definition& model,
                                 const Tools::gpm_logger& logger, const std::vector<property_stack_holder>& stack, const std::string& fil_name, const std::string& ogs_binary);

    void set_read_info(const std::string& fil_name, grid_parameter_type* param);
    void set_file_name(const std::string& fil_name) { m_file_name_ = fil_name; }
    static int get_data_for_file(const std::string& fil_name,
                                 std::vector<boost::geometry::model::point<double, 3, boost::geometry::cs::cartesian>>&
                                 gpoints,
                                 std::vector<std::vector<int>>& cellindexes,
                                 std::vector<std::string>& propname,
                                 std::vector<std::vector<double>>& props);
    static int do_compare_files(const std::string& fil_name_1,
                                const std::string& fil_name_2,
                                const Slb::Exploration::Gpm::Tools::gpm_logger& logger);

private:
    std::string m_file_name_;
    grid_parameter_type* m_parm_;
};

}}}
#endif
