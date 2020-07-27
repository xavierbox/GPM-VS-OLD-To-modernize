#include "gpm_plugin_holder.h"
#include "gpm_plugin_description.h"
#include <stdexcept>
#include <boost/type_index.hpp>
#ifdef _WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#include <unistd.h>
#endif



namespace Slb { namespace Exploration { namespace Gpm {
#ifdef _WIN32
	const std::string dso_extension = ".dll";
    HMODULE get_plugin_handle(const std::string& name)
    {
        return ::LoadLibrary(name.c_str());
    }
    FARPROC get_plugin_function(HMODULE handle, const std::string& name)
    {
		return handle ? ::GetProcAddress(handle, name.c_str()) : nullptr;
    }
	bool free_plugin_handle(void* handle)
    {
		return handle ? ::FreeLibrary(static_cast<HMODULE>(handle)) != 0 : true;
    }
#else
	const std::string dso_extension = ".so";
	void* get_plugin_handle(const std::string& name)
	{
		return ::dlopen(name.c_str(), RTLD_LAZY | RTLD_LOCAL);
	}
	void* get_plugin_function(void* handle, const std::string& name)
	{
		return handle ? dlsym(handle, name.c_str()) : nullptr;
	}
	bool free_plugin_handle(void* handle)
	{
		return handle ? dlclose(handle) == 0 : true;
	}
#endif

ppm_plugin_holder::ppm_plugin_holder(const std::string& loc_id, const std::string& libname, const Tools::gpm_logger& logger):id(loc_id)
{
    const std::string fullname = libname + dso_extension;

    const auto handle = get_plugin_handle(fullname);
	dll_handle = handle;
    const auto create_handle = get_plugin_function(handle, "gpm_plugin_api_create_plugin_handle") ;
    const auto parameter_handler = get_plugin_function(handle, "gpm_plugin_api_read_parameters") ;
	const auto install_dir_handle = get_plugin_function(handle, "gpm_plugin_api_current_install_directory");
    const auto model_handler = get_plugin_function(handle, "gpm_plugin_api_set_model_extents");
    const auto sediment_handler = get_plugin_function(handle, "gpm_plugin_api_set_sediments");
        // The more general ones                             
	const auto attribute_read_handler = get_plugin_function(handle, "gpm_plugin_api_get_needed_model_attributes");        
	const auto attribute_write_num_handler = get_plugin_function(handle, "gpm_plugin_api_get_write_model_attribute_num");
	const auto attribute_write_sizes_handler = get_plugin_function(handle, "gpm_plugin_api_get_write_model_attribute_sizes");
	const auto attribute_write_handler = get_plugin_function(handle, "gpm_plugin_api_get_write_model_attributes");

    const auto init_display_handler = get_plugin_function(handle, "gpm_plugin_api_initialize_display_step");
    const auto process_timestep_handle = get_plugin_function(handle, "gpm_plugin_api_process_top_sediment_sea_timestep");

	const auto process_model_multiple_of_timestep_handle = get_plugin_function(handle, "gpm_plugin_api_process_model_multiple_of_timestep");
	const auto process_model_timestep_handle = get_plugin_function(handle, "gpm_plugin_api_process_model_timestep");
	const auto process_modell_timestep_attributes_handle = get_plugin_function(handle, "gpm_plugin_api_update_attributes_timestep");

    const auto delete_handle = get_plugin_function(handle, "gpm_plugin_api_delete_plugin_handle");

    _factory = reinterpret_cast<create_plugin_func>(create_handle);
    _deleter = reinterpret_cast<delete_plugin_func>(delete_handle);
    _read_parameters_func = reinterpret_cast<read_parameters_func>(parameter_handler);
	_current_install_dir_func = reinterpret_cast<current_install_dir_func>(install_dir_handle);
    _set_model_extents_func = reinterpret_cast<set_model_extents_func>(model_handler);
    _set_sediment_func = reinterpret_cast<set_sediment_func>(sediment_handler);

	 _get_needed_model_attributes_func = reinterpret_cast<get_needed_model_attributes_func>(attribute_read_handler);
	 _get_write_model_attribute_num_func = reinterpret_cast<get_write_model_attribute_num_func>(attribute_write_num_handler);
	 _get_write_model_attribute_sizes_func = reinterpret_cast<get_write_model_attribute_sizes_func>(attribute_write_sizes_handler);
	 _get_write_model_attributes_func = reinterpret_cast<get_write_model_attributes_func>(attribute_write_handler);

    _initialize_display_func = reinterpret_cast<initialize_display_func>(init_display_handler);
    _process_top_sediment_sea_timestep_func = reinterpret_cast<process_top_sediment_sea_timestep_func>(process_timestep_handle);

	_process_multiple_of_timestep_func = reinterpret_cast<process_model_multiple_of_timestep_func>(process_model_multiple_of_timestep_handle);
	_process_model_timestep_func = reinterpret_cast<process_model_timestep_func>(process_model_timestep_handle);
	_update_attributes_timestep_func= reinterpret_cast<update_attributes_timestep_func>(process_modell_timestep_attributes_handle);

		// TODO change these to only mandatory ones JT 
    if (!handle)
        throw std::runtime_error("Cannot load \"" + fullname + "\".");
    if (!create_handle)
        throw std::runtime_error("Cannot find gpm_plugin_api_create_plugin_handle in \"" + fullname + "\".");
	if (!parameter_handler)
		throw std::runtime_error("Cannot find gpm_plugin_api_read_parameters in \"" + fullname + "\".");
	if (!model_handler)
		throw std::runtime_error("Cannot find gpm_plugin_api_set_model_extents in \"" + fullname + "\".");
	if (!sediment_handler)
		throw std::runtime_error("Cannot find gpm_plugin_api_set_sediments in \"" + fullname + "\".");
	if (!init_display_handler)
		throw std::runtime_error("Cannot find gpm_plugin_api_initialize_display_step in \"" + fullname + "\".");
	if (!process_timestep_handle)
		throw std::runtime_error("Cannot find gpm_plugin_api_process_top_sediment_sea_timestep in \"" + fullname + "\".");
	if (!delete_handle)
        throw std::runtime_error("Cannot find gpm_plugin_api_delete_plugin_handle in \"" + fullname + "\".");
}

ppm_plugin_holder::~ppm_plugin_holder()
{
   if (dll_handle != nullptr) {
       free_plugin_handle(dll_handle);
   }
}

std::string ppm_plugin_holder::get_exe_folder()
{
#ifdef _WIN32
	char buf[MAX_PATH];
    const size_t nbytes = GetModuleFileName(NULL, buf, sizeof(buf));
	std::string exe = nbytes > 0 ? std::string(buf, nbytes) : std::string();
    const auto pos = exe.find_last_of('\\');
	return exe.substr(0, pos);

#else
	char buf[8192];
	ssize_t nbytes = readlink("/proc/self/exe", buf, sizeof(buf));
	std::string exe = nbytes > 0 ? std::string(buf, nbytes) : std::string();
	size_t pos = exe.find_last_of("/");
	if (pos == 0) // e.g. "/foo"
		return std::string("/");
	else if (pos == std::string::npos) // e.g. "foo" or ""
		return std::string(".");
	else // e.g. "foo/bar" or "/foo/bar" but doesn't handle "/foo/bar/".
		return exe.substr(0, pos);
	// Note, "/foo/bar/" or "/foo/bar///" not handled correctly
	// but I don't think those are possible.
#endif
}

}}}
