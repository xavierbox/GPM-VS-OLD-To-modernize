#ifndef gpm_plugin_api_holder_h
#define gpm_plugin_api_holder_h

#include "gpm_plugin_description.h"
#include "gpm_logger.h"
#include <string>

namespace Slb { namespace Exploration { namespace Gpm {
class ppm_plugin_holder {
public:
    ppm_plugin_holder(const std::string& id, const std::string& libname, const Tools::gpm_logger& logger);
    static std::string get_exe_folder();
    ppm_plugin_holder(const ppm_plugin_holder& rhs) = delete;
	ppm_plugin_holder& operator=(const ppm_plugin_holder& rhs) = delete;
	~ppm_plugin_holder();

//private:
	create_plugin_func _factory;
	set_model_extents_func _set_model_extents_func;
	set_sediment_func _set_sediment_func;
	read_parameters_func _read_parameters_func;
	current_install_dir_func _current_install_dir_func;
	get_needed_model_attributes_func _get_needed_model_attributes_func;
	get_write_model_attribute_num_func _get_write_model_attribute_num_func;
	get_write_model_attribute_sizes_func _get_write_model_attribute_sizes_func;
	get_write_model_attributes_func _get_write_model_attributes_func;

	initialize_display_func _initialize_display_func;
	process_top_sediment_sea_timestep_func _process_top_sediment_sea_timestep_func;

	process_model_multiple_of_timestep_func _process_multiple_of_timestep_func;
	process_model_timestep_func _process_model_timestep_func;
	update_attributes_timestep_func _update_attributes_timestep_func;

	delete_plugin_func _deleter;
	std::string id;
	void* dll_handle;
};

}}}
#endif
