// -- Schlumberger Private --

#ifndef _ssmstc_h_
#define _ssmstc_h_

#include <string>

namespace Slb { namespace Exploration { namespace Gpm {

struct parc {
	enum class COMPFLAG_OPTIONS{ COMPFLAG_OFF = 0, COMPFLAG_LOAD, COMPFLAG_LOAD_AND_WATER_FLOW, COMPFLAG_LOAD_AND_WATER_FLOW_3D };

    std::string mapfile, flowfile, sedfile, outfile; // files
    bool seaflag;
    bool do_top_slope;
    bool porflag, prsflag;
    COMPFLAG_OPTIONS compflag;
    double timbeg, timend; // Times
    double tdisplay;
    float althick; // Active layer thickness
    float dtime; // Time increment

    int isave;
	bool save_last_timestep_only;
	bool output_incremental_files{};
    long depodepthflag; // Depositional flag. Should have been further up JT
    bool erosionflag; // Write erosion surface for timestep;

    std::string version;


    // Process parameters for the various processes
    std::string carbonate_parms;
    bool do_carbonate;
    std::string area_sediment_parms;
    bool do_area_sediment;
    std::string point_sediment_parms;
    bool do_point_sediment;
    std::string diffusion_parms;
    bool do_diffusion;
    std::string steady_flow_parms;
    bool do_steady_flow;
    std::string unsteady_flow_parms;
    bool do_unsteady_flow;
    std::string wave_parms;
    bool do_waves;
    std::string litho_classification_parms;
    bool do_litho_classification;
    std::string compaction_parms;
    bool do_compaction;
    std::string tectonics_parms;
    bool do_tectonics;
    std::string model_sampling_parms;
    bool do_model_sampling;
    bool do_postprocess;
    std::string postprocess_parms; // Params for the processor
    std::string postprocess_file; // Comes as a map file
	bool do_log_process_energy;
	std::string isostatic_tectonics_parms;
	bool do_isostatic_tectonics;
    std::string flow_simulator_output_parms;
    bool do_flow_simulator_output;
    std::string pelagic_rain_parms;
    bool do_pelagic_rain;
	bool do_plugins;
    std::string plugins_ctrl_parms;
	bool do_bathymetry;
	bool do_ogs_workflow;
	bool do_include_active_layer{ true };
    // Just to get the current version of this thing
    parc(): seaflag(false), do_top_slope(), porflag(false), prsflag(false),
            compflag(), timbeg(0), timend(0), tdisplay(0), althick(0),
            dtime(0), isave(0), save_last_timestep_only(false), depodepthflag(0), erosionflag(false),
            do_carbonate(false),
            do_area_sediment(false), do_point_sediment(false), do_diffusion(false), do_steady_flow(false),
            do_unsteady_flow(false), do_waves(false), do_litho_classification(false), do_compaction(false),
            do_tectonics(false), do_model_sampling(false), do_postprocess(false), 
            do_log_process_energy(false), do_isostatic_tectonics(false), do_flow_simulator_output(false),
            do_pelagic_rain(false), do_plugins(false), do_bathymetry(false), do_ogs_workflow(false)
    {
    }
};

typedef struct parc control_parameter_type;

}}} //Namespace GPM

#endif
