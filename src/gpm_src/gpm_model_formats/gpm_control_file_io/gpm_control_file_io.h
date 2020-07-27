// -- Schlumberger Private --

#ifndef gpm_control_file_io_h
#define gpm_control_file_io_h

#include "ssmstc.h"
#include "gpm_param_type_mapper.h"
#include "gpm_program_options.h"
#include "gpm_logger.h"
#include <string>
#include <vector>

namespace Slb { namespace Exploration { namespace Gpm {

class gpm_control_file_io {
public:
    gpm_control_file_io();
    virtual ~gpm_control_file_io();
    int read_parameters(const std::string& param_file, const Tools::gpm_logger& logger, bool check_files = true);
    static std::string get_process_name();
    bool are_parameters_ok(const Tools::gpm_logger& logger, bool check_files = true) const;
    int read_parameters_from_string(const std::string& param_string, const Tools::gpm_logger& logger, bool check_files = true);
    void get_parms(const program_option_params& opts, parc* ctl_parms_p) const;
    void get_parms(bool fluidsim_is_set, bool do_ogs_workflow, parc* ctl_parms_p) const;
    void update_with_options(const program_option_params& opts, parc* ctl_parms_p) const;
    bool has_incremental_time_interval() const;
private:
    std::string version() const;
    std::string map_file() const;
    std::string flow_file() const;
    std::string sed_file() const;
    std::string carb_file() const;
    std::string out_file() const;
    int sea_flag() const;
    int arrows_flag() const;
    int parrows_flag() const;
    int por_flag() const;
    int prs_flag() const;
    int comp_flag() const;
    int carb_flag() const;
    int permun() const;
    int cmpun() const;
    int isave_flag() const;
    int save_last_timestep_only_flag() const;
    int depodepth_flag() const;
    int erosion_flag() const;
    double timbeg() const;
    double timend() const;
    double tdisplay() const;
    float althick() const;
    float baserod() const;
    float dtime() const;
    int postprocess_flag() const;
    std::string postprocess_file() const;
    std::string postprocess_parms() const;
    std::string area_sediment_parms() const;
    int do_area_sediment() const;
    std::string diffusion_parms() const;
    int do_diffusion() const;
    std::string steady_flow_parms() const;
    int do_steady_flow() const;
    std::string unsteady_flow_parms() const;
    int do_unsteady_flow() const;
    std::string wave_parms() const;
    int do_waves() const;
    std::string litho_classification_parms() const;
    int do_litho_classification() const;
    std::string compaction_parms() const;
    int do_compaction() const;
    std::string tectonics_parms() const;
    int do_tectonics() const;
    std::string model_sampling_parms() const;
    int do_model_sampling() const;
    std::string point_sediment_parms() const;
    int do_point_sediment() const;
    int run_optimized_code() const;
	int do_log_process_energy() const;
	int do_isostatic_tectonics() const;
	std::string isostatic_tectonics_parms() const;
    int do_flow_simulator_output() const;
    std::string flow_simulator_output_parms() const;
    int do_pelagic_rain() const;
    std::string pelagic_rain_parms() const;
    int do_plugins() const;
    std::string plugins_ctrl_parms() const;
	int do_bathymetry() const;
    int do_include_active_layer() const;
    int do_save_last_timestep_only() const;
    int do_output_incremental_files() const;
    bool _parameters_ok;
    bool _parsed;
	gpm_param_type_mapper _mapper;
};
}}}
#endif
