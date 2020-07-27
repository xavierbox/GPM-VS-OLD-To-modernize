#ifndef GPM_PLUGIN_MAPPER_H
#define GPM_PLUGIN_MAPPER_H
#include "gpm_plugin_holder.h"
#include "gpm_pars_methods.h"
#include "gpm_model_definition.h"
#include "gpm_logger.h"

// TODO need a better way to handle armadillo
#if defined(USE_ARMA)
#include "gpm_arma.h"
using sediment_layer_type = Slb::Exploration::Gpm::gpm_arma::fcube;
using sediment_layer_base_type = sediment_layer_type;
#else
#include "gpm_vbl_array_3d.h"
using sediment_layer_base_type = Slb::Exploration::Gpm::gpm_array_3d_base<float>;
using sediment_layer_type = Slb::Exploration::Gpm::gpm_vbl_array_3d<float>;
#endif


namespace Slb { namespace Exploration { namespace Gpm {
class gpm_plugin_mapper {
public:
    struct attribute_const_descriptor {
		std::string id;
		std::vector<function_2d> funcs;
    };
	struct attribute_descriptor {
		std::string id;
		std::vector<float_2darray_base_type*> funcs;
	};

    struct attribute_type {
		std::string id;
		bool top_only;
    };
    explicit gpm_plugin_mapper(const std::shared_ptr<ppm_plugin_holder>& holder);
    gpm_plugin_mapper(const gpm_plugin_mapper& rhs) = delete;
    gpm_plugin_mapper& operator=(const gpm_plugin_mapper& rhs) = delete;
    ~gpm_plugin_mapper();
    std::string id();
    bool has_valid_handle();
    void set_model_extents(const gpm_model_definition& in_model);
    void set_current_install_dir(const std::string& curr_dir);
    int get_multiples_of_timestep(double display_time) const;
    // Call process_model_timestep at each nth display time;
    int read_parameters(const std::string& file_name, const Tools::gpm_logger& logger);
	int read_needed_properties(const std::vector<std::string>& prop_ids, const Tools::gpm_logger& logger);
    void set_sediments(const std::vector<sediment_props>& seds) const;
    void initialize_display_time(double time);
	int process_sediments(double time_prev, double time_now,
		const float_2darray_base_type& top,
        float sea_level,
		sediment_layer_base_type* seds_p,
		const Tools::gpm_logger&  logger) const;

	int process_model_timestep(double time_prev, double time_now, const std::vector< attribute_const_descriptor>& arrs, const Tools::gpm_logger& logger) const;
	int update_attributes_timestep(double time_prev, double time_now, const std::vector< attribute_descriptor>& arrs, const Tools::gpm_logger& logger);
	std::vector<std::string> find_input_properties() const;
	std::vector<attribute_type> find_output_properties() const;

private:
	std::vector<std::string> _model_properties;
	std::vector<attribute_type> _write_properties;
	int_extent_2d _extents;

    std::shared_ptr<ppm_plugin_holder> _holder;
    void* _plugin_handle;
};
}}}
#endif // GPM_PLUGIN_MAPPER_H
