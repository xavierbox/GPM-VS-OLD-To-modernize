// -- Schlumberger Private --

#ifndef VTK_WRITER_VTU_H
#define VTK_WRITER_VTU_H

// A wrapper for the acf writer currently used, but where we use signals to call back to the struct

#include "mapstc.h"
#include "gpm_model_definition.h"
#include "acf_base_writer.h"
#include "gpm_vbl_array_2d.h"
#include "gpm_logger.h"

#include <utility>
#include <vector>

#include <boost/geometry.hpp>

namespace Slb { namespace Exploration { namespace Gpm {

class vtk_writer_vtu {
public:
    // Perhaps we should do it all by array?
    using arrray_dim_size_type = std::vector<acf_base_writer::io_array_dim_type>;
    using parameter_num_type = acf_base_writer::parameter_num_type;
    using point = boost::geometry::model::point <double, 3, boost::geometry::cs::cartesian>;
    using vec_point = std::vector<point >;
    // We are writing it out in double, may be a bit extravagant?
    using function_2d_read_type = std::function<double(acf_base_writer::index_type, acf_base_writer::index_type)>;

    struct property_stack_holder {
		std::string name;
		std::vector<function_2d_read_type> stack;
		property_stack_holder() = default;
		property_stack_holder(std::string aname, const function_2d_read_type& reader)
		{
			name = std::move(aname);
			stack.push_back(reader);
		}
		property_stack_holder(std::string aname, const function_2d& reader, float default_value)
		{
			name = std::move(aname);
			if (reader.is_initialized()) {
				stack.emplace_back(reader);
			}
			else {
				stack.emplace_back([default_value](acf_base_writer::index_type, acf_base_writer::index_type) {return default_value; });
			}
		}
		property_stack_holder(std::string aname, const std::vector<function_2d_read_type>& reader)
		{
			name = std::move(aname);
			stack=reader;
		}
    };
    vtk_writer_vtu();
    ~vtk_writer_vtu() = default;
    int write(int cycle_time, const gpm_model_definition& model,
               const Tools::gpm_logger& logger) ;
    // Number of properties in each property must macth the number of surfaces 
    int write_model_for_timestep(const std::vector<function_2d_read_type>& surface_stack,
                                 const std::vector<property_stack_holder>& property_stacks,
                                 const gpm_model_definition& model,
                                 const Tools::gpm_logger& logger);
    // This is based on the fact that the index regime from the call above is valid
	int write_tops_for_timestep(const function_2d_read_type& surface_stack,
		const std::vector<property_stack_holder>& property_stack,
		const gpm_model_definition& model) const;

    void set_write_info(const std::string& fil_name, grid_parameter_type* param, bool binary_write, bool origional_write);
    void set_file_name(const std::string& fil_name) { m_file_name_ = fil_name; }
    void set_write_mode_to_binary() { m_binary_write_ = true; }
    void set_write_mode_to_ascii() { m_binary_write_ = false; }
	std::vector<std::string> get_properties_to_write() const;
	void set_properties_to_write(const std::vector<std::string>& props);
	void add_properties_to_write(const std::vector<std::string>& props);

private:
    static bool convert_to_cell_property(const std::string& property);
	int write_vtu_model_file(int cycle_time, const gpm_model_definition& model,
                                const Tools::gpm_default_logger::gpm_logger& logger) ;
    int write_vtu_tops_file(int cycle_time, const gpm_model_definition& model,
        const Tools::gpm_default_logger::gpm_logger& logger) const;
    bool is_property_to_be_included(const std::string& property) const;
    static void get_tetra_cells(std::vector<std::pair<int, double>>& v_dist,
                                std::vector<int>& t1, std::vector<int>& t2, std::vector<int>& t3, std::vector<int>& t4);
	static void add_valid_cell_to_mapping(std::vector<bool>& used_point_index, const std::vector<int>& cell_ptr);
    // Parse info
    std::string m_file_name_;
    grid_parameter_type* m_parm_;
    std::vector<std::string> prop_names{ "POR","PERMEA","SED" };
    //std::vector<std::string> prop_names{ "POR","PERMEA","SED","WAVE" };
    bool m_binary_write_;
    bool m_origional_write_;
	int current_stack_size = 0;
    // Hold info needed
	gpm_vbl_array_2d<long long> _top_model_index_map;
	std::vector<long long> _origin_to_compressed_map;
};

}}}
#endif
