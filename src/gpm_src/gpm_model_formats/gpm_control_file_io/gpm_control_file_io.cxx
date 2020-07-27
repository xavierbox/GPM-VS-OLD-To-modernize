// -- Schlumberger Private --

#include "gpm_control_file_io.h"
#include "ssmstc_descr.h"
#include "gpm_version_number.h"
#include "gpm_control_file_io_sc.h"
#include "gpm_json_doc_mapper.h"
#include <fstream>
#include <boost/algorithm/string.hpp>
#include "acf_format_selector.h"

namespace Slb { namespace Exploration { namespace Gpm {namespace {
int check_file_name(std::string file_name, const std::string& error_text, const Tools::gpm_logger& logger) {
    boost::trim(file_name);
    if (file_name.empty()) {
        logger.print(Tools::LOG_NORMAL, "%s file not given\n", error_text.c_str());
        return 2;
    }
    else if (!std::ifstream(file_name, std::ios_base::in)) {
        logger.print(Tools::LOG_IMPORTANT, "%s file not found\n", error_text.c_str());
        return 2;
    }
    return 0;
}
}

// Here is the implemenatation
gpm_control_file_io::gpm_control_file_io(): _parameters_ok(false), _parsed(false){
}

gpm_control_file_io::~gpm_control_file_io() = default;


int gpm_control_file_io::read_parameters(const std::string& param_file, const Tools::gpm_logger& logger,
                                         bool check_files)
{
    _parsed = false;
    gpm_json_doc_mapper mapper(get_process_name(), Plugins::MainProcesses::gpm_control_file_io_schema);
    int ret_val = mapper.parse_file(param_file, logger);
    _parsed = ret_val == 0;
    if (_parsed) {
        _mapper.initialize_parameters(parc_descr_holder::get_parc_descr());
        mapper.find_mapping(&_mapper);
        _parameters_ok = are_parameters_ok(logger, check_files);
        ret_val = _parameters_ok ? 0 : 1;
    }
    return ret_val;
}

int gpm_control_file_io::read_parameters_from_string(const std::string& param_string, const Tools::gpm_logger& logger,
                                                     bool check_files)
{
    _parsed = false;
    gpm_json_doc_mapper mapper(get_process_name(), Plugins::MainProcesses::gpm_control_file_io_schema);
    int ret_val = mapper.parse_text(param_string, logger);
    _parsed = ret_val == 0;
    if (_parsed) {
        _mapper.initialize_parameters(parc_descr_holder::get_parc_descr());
        mapper.find_mapping(&_mapper);
        _parameters_ok = are_parameters_ok(logger, check_files);
        ret_val = _parameters_ok ? 0 : 1;
    }
    return ret_val;
}

std::string gpm_control_file_io::get_process_name() {
    return "Control process";
}

bool gpm_control_file_io::are_parameters_ok(const Tools::gpm_logger& logger, bool check_files) const {
    std::vector<int> retval;
    Tools::gpm_version_number ver(version());
    if (!ver.is_number_correct()) {
        logger.print(Tools::LOG_IMPORTANT, "ERROR: file version of control file incorrect\n");
        retval.push_back(2);
    }
    else {

        Tools::gpm_version_number found(version());
        Tools::gpm_version_number supported(parc_descr_holder::get_current_version());
        if (!found.version_compatible(supported)) {
            logger.print(Tools::LOG_IMPORTANT, "Control file versions not compatible\n");
            retval.push_back(1);
        }
        else {
            if (check_files) {
                retval.push_back(check_file_name(map_file(), "Map", logger));
                retval.push_back(check_file_name(flow_file(), "Flow", logger));
                retval.push_back(check_file_name(sed_file(), "Sediment", logger));
                if (carb_flag()) retval.push_back(check_file_name(carb_file(), "Carbonate", logger));
                if (do_area_sediment()) retval.push_back(check_file_name(area_sediment_parms(), "Area sediment", logger));
                if (do_litho_classification()) retval.push_back(check_file_name(litho_classification_parms(), "Litho classification", logger));
                if (do_waves()) retval.push_back(check_file_name(wave_parms(), "Wave", logger));
                if (do_steady_flow()) retval.push_back(check_file_name(steady_flow_parms(), "Steady flow", logger));
                if (do_diffusion()) retval.push_back(check_file_name(diffusion_parms(), "Diffusion", logger));
                if (do_unsteady_flow()) retval.push_back(check_file_name(unsteady_flow_parms(), "Unsteady flow", logger));
                if (do_model_sampling()) retval.push_back(check_file_name(model_sampling_parms(), "Model sampling", logger));
            }
        }
    }
    return std::all_of(retval.begin(), retval.end(), [](int num) { return num == 0; });
}

void gpm_control_file_io::get_parms(const program_option_params& opts, parc* ctl_parms_p) const {
	get_parms(opts.fluid_sim_, opts.write_ogs_project_file, ctl_parms_p);
	update_with_options(opts, ctl_parms_p);
}
void gpm_control_file_io::get_parms(bool fluidsim_is_set, bool do_ogs_workflow, parc* ctl_parms_p) const {
    auto& m_param(*ctl_parms_p);
    m_param.seaflag = sea_flag() != 0; // SEAFLAG: off, on

    m_param.do_top_slope = arrows_flag()!=0; 
    m_param.porflag = por_flag() != 0; // PORFLAG: off, on
    m_param.prsflag = prs_flag() != 0; // PRSFLAG: off, on

    m_param.compflag = static_cast<parc::COMPFLAG_OPTIONS>(comp_flag()); // COMPFLAG: Compaction: off, load, load and water flow
    m_param.do_carbonate = carb_flag() != 0; // CARBFLAG: Carbonates: off, on
    m_param.carbonate_parms = carb_file();

    m_param.timbeg = timbeg(); // TIMBEG
    m_param.timend = timend(); // TIMEND
    m_param.tdisplay = tdisplay(); // TDISPLAY

    m_param.althick = althick(); // ALTHICK

    m_param.dtime = dtime(); // DTIME

    m_param.isave = isave_flag(); // ISAVE
	m_param.save_last_timestep_only = save_last_timestep_only_flag()!= 0;
    m_param.depodepthflag = depodepth_flag();
    m_param.erosionflag = erosion_flag() != 0; // Default off erosion flag for now JT

    m_param.version = version();

    m_param.mapfile = map_file();
    m_param.flowfile = flow_file();
    m_param.sedfile = sed_file();
    m_param.outfile = out_file();
    m_param.do_postprocess = postprocess_flag() != 0;
    m_param.postprocess_file = postprocess_file();
    m_param.postprocess_parms = postprocess_parms();
    m_param.do_model_sampling = do_model_sampling() != 0;
    m_param.model_sampling_parms = model_sampling_parms();
    m_param.do_diffusion = do_diffusion() != 0;
    m_param.diffusion_parms = diffusion_parms();
    m_param.do_area_sediment = do_area_sediment() != 0;
    m_param.area_sediment_parms = area_sediment_parms();
    m_param.do_compaction = do_compaction() != 0;
    m_param.compaction_parms = compaction_parms();
    m_param.do_steady_flow = do_steady_flow() != 0;
    m_param.steady_flow_parms = steady_flow_parms();
    m_param.do_unsteady_flow = do_unsteady_flow() != 0;
    m_param.unsteady_flow_parms = unsteady_flow_parms();
    m_param.do_tectonics = do_tectonics() != 0;
    m_param.tectonics_parms = tectonics_parms();
    m_param.do_litho_classification = do_litho_classification() != 0;
    m_param.litho_classification_parms = litho_classification_parms();
    m_param.do_waves = do_waves() != 0;
    m_param.wave_parms = wave_parms();
	m_param.do_log_process_energy = do_log_process_energy() != 0;
	m_param.do_isostatic_tectonics = do_isostatic_tectonics() != 0;
	m_param.isostatic_tectonics_parms = isostatic_tectonics_parms() ;
    m_param.do_flow_simulator_output = fluidsim_is_set || (do_flow_simulator_output() != 0);
    m_param.flow_simulator_output_parms = (fluidsim_is_set) ? "Flow_parms.fld.json" : flow_simulator_output_parms();
    m_param.do_pelagic_rain = do_pelagic_rain() != 0;
    m_param.pelagic_rain_parms = pelagic_rain_parms(); 
    m_param.do_plugins = do_plugins() != 0;
	m_param.plugins_ctrl_parms = plugins_ctrl_parms();
	m_param.do_bathymetry = do_bathymetry() != 0;
	m_param.do_ogs_workflow = do_ogs_workflow;
	m_param.output_incremental_files = do_output_incremental_files();
	m_param.save_last_timestep_only = do_save_last_timestep_only();
	m_param.do_include_active_layer = do_include_active_layer() != 0;
}

void gpm_control_file_io::update_with_options(const program_option_params& opts, parc* ctl_parms_p) const
{
	auto& m_param(*ctl_parms_p);
	// Use this out file instead of the one in the control file
	if (!opts.out_file_name.empty() || !opts.output_directory.empty()) {
		if (!opts.output_directory.empty() && !opts.out_file_name.empty()) {
			m_param.outfile = opts.output_directory + "/" + opts.out_file_name;
		}
		else if (!opts.output_directory.empty()) {
			m_param.outfile = opts.output_directory + "/" + m_param.outfile;
		}
		else {
			m_param.outfile = opts.out_file_name;
		}
	}
	if (m_param.do_postprocess && !opts.output_directory.empty()) {
		// Add directory
		m_param.postprocess_file = opts.output_directory + "/" + m_param.postprocess_file;
	}

    if (opts.output_only_last_timestep) {
		m_param.save_last_timestep_only = true;
    }
	if (opts.output_incremental_files) {
		m_param.output_incremental_files = true;
	}
}
bool gpm_control_file_io::has_incremental_time_interval() const {
    return _mapper.has_key(TIMRUN) && !(_mapper.has_key(TIMBEG) && _mapper.has_key(TIMEND));
}

std::string gpm_control_file_io::version() const {
    return _mapper.get_value<STRING>(PARC_VERSION, "1.1.1.0");
}

std::string gpm_control_file_io::map_file() const {
    return _mapper.get_value<STRING>(MAPFILE, "");
}

std::string gpm_control_file_io::flow_file() const {
    return _mapper.get_value<STRING>(FLOWFILE, "");
}

std::string gpm_control_file_io::sed_file() const {
    return _mapper.get_value<STRING>(SEDFILE, "");
}

std::string gpm_control_file_io::carb_file() const {
    return _mapper.get_value<STRING>(CARBFILE, "");
}

std::string gpm_control_file_io::out_file() const {
    return _mapper.get_value<STRING>(OUTFILE, "");
}

int gpm_control_file_io::sea_flag() const {
    return _mapper.get_value<INTEGER>(SEAFLAG, 1);
}

int gpm_control_file_io::arrows_flag() const {
    return _mapper.get_value<INTEGER>(ARROWS, static_cast<int>(ARROWS_OPTIONS::ARROWS_OFF));
}

int gpm_control_file_io::parrows_flag() const {
    return _mapper.get_value<INTEGER>(PARROWS, PARROWS_OFF);
}

int gpm_control_file_io::por_flag() const {
    return _mapper.get_value<INTEGER>(PORFLG, 0);
}

int gpm_control_file_io::prs_flag() const {
    return _mapper.get_value<INTEGER>(PRSFLG, 0);
}

int gpm_control_file_io::comp_flag() const {
    return _mapper.get_value<INTEGER>(COMPFLAG, COMPFLAG_OFF);
}

int gpm_control_file_io::carb_flag() const {
    return _mapper.get_value<INTEGER>(CARBFLAG, 0);
}

int gpm_control_file_io::permun() const {
    return _mapper.get_value<INTEGER>(PERMUN, 0);
}

int gpm_control_file_io::cmpun() const {
    return _mapper.get_value<INTEGER>(CMPUN, 0);
}

int gpm_control_file_io::isave_flag() const {
    return _mapper.get_value<INTEGER>(ISAVE, 0);
}

int gpm_control_file_io::save_last_timestep_only_flag() const {
	return _mapper.get_value<INTEGER>(SAVE_LAST_TIMESTEP_ONLY, 0);
}

int gpm_control_file_io::depodepth_flag() const {
    return _mapper.get_value<INTEGER>(DEPODEPTHFLAG, 0);
}

int gpm_control_file_io::erosion_flag() const {
    return _mapper.get_value<INTEGER>(EROSIONFLAG, 0); // Put out erosion surface for timestep
}

double gpm_control_file_io::timbeg() const {
    return _mapper.get_value<DOUBLE>(TIMBEG, 0.0);
}

double gpm_control_file_io::timend() const {
    return _mapper.get_value<DOUBLE>(TIMEND, 1000);
}

double gpm_control_file_io::tdisplay() const {
    return _mapper.get_value<DOUBLE>(TDISPLAY, 1000);
}

float gpm_control_file_io::althick() const {
    return _mapper.get_value<FLOAT>(ALTHICK, 1.0F);
}

float gpm_control_file_io::dtime() const {
    return _mapper.get_value<FLOAT>(DTIME, 1e-4f);
}

int gpm_control_file_io::postprocess_flag() const {
    return _mapper.get_value<INTEGER>(POSTPROCESSFLAG, 0);// Shall we do postprocessing and what type
}

std::string gpm_control_file_io::postprocess_file() const {
    return _mapper.get_value<STRING>(POSTPROCESSFILE, "");// Name of the outputfile
}

std::string gpm_control_file_io::postprocess_parms() const {
    return _mapper.get_value<STRING>(POSTPROCESSPARMS, "");// The file with the input parms
}

std::string gpm_control_file_io::area_sediment_parms() const {
    return _mapper.get_value<STRING>(AREA_SEDIMENT_PARMS, "");
}

int gpm_control_file_io::do_area_sediment() const {
    return _mapper.get_value<INTEGER>(DO_AREA_SEDIMENT, 0);
}

std::string gpm_control_file_io::diffusion_parms() const {
    return _mapper.get_value<STRING>(DIFFUSION_PARMS, "");
}

int gpm_control_file_io::do_diffusion() const {
    return _mapper.get_value<INTEGER>(DO_DIFFUSION, 0);
}

std::string gpm_control_file_io::steady_flow_parms() const {
    return _mapper.get_value<STRING>(STEADY_FLOW_PARMS, "");
}

int gpm_control_file_io::do_steady_flow() const {
    return _mapper.get_value<INTEGER>(DO_STEADY_FLOW, 0);
}

std::string gpm_control_file_io::unsteady_flow_parms() const {
    return _mapper.get_value<STRING>(UNSTEADY_FLOW_PARMS, "");
}

int gpm_control_file_io::do_unsteady_flow() const {
    return _mapper.get_value<INTEGER>(DO_UNSTEADY_FLOW, 0);
}

std::string gpm_control_file_io::wave_parms() const {
    return _mapper.get_value<STRING>(WAVE_PARMS, "");
}

int gpm_control_file_io::do_waves() const {
    return _mapper.get_value<INTEGER>(DO_WAVES, 0);
}

std::string gpm_control_file_io::litho_classification_parms() const {
    return _mapper.get_value<STRING>(LITHO_CLASSIFICATION_PARMS, "");
}

int gpm_control_file_io::do_litho_classification() const {
    return _mapper.get_value<INTEGER>(DO_LITHO_CLASSIFICATION, 0);
}

std::string gpm_control_file_io::compaction_parms() const {
    return _mapper.get_value<STRING>(COMPACTION_PARMS, "");
}

int gpm_control_file_io::do_compaction() const {
    return _mapper.get_value<INTEGER>(DO_COMPACTION, 0);
}

std::string gpm_control_file_io::tectonics_parms() const {
    return _mapper.get_value<STRING>(TECTONICS_PARMS, "");
}

int gpm_control_file_io::do_tectonics() const {
    return _mapper.get_value<INTEGER>(DO_TECTONICS, 0);
}

std::string gpm_control_file_io::model_sampling_parms() const {
    return _mapper.get_value<STRING>(MODEL_SAMPLING_PARMS, "");
}

int gpm_control_file_io::do_model_sampling() const {
    return _mapper.get_value<INTEGER>(DO_MODEL_SAMPLING, 0);
}

std::string gpm_control_file_io::point_sediment_parms() const {
    return _mapper.get_value<STRING>(POINT_SEDIMENT_PARMS, "");
}

int gpm_control_file_io::do_point_sediment() const {
    return _mapper.get_value<INTEGER>(DO_POINT_SEDIMENT, 0);
}

int gpm_control_file_io::run_optimized_code() const {
    return _mapper.get_value<INTEGER>(RUN_OPTIMIZED_CODE, 0);
}

int gpm_control_file_io::do_log_process_energy() const {
	return _mapper.get_value<INTEGER>(DO_LOG_PROCESS_ENERGY, 0);
}

int gpm_control_file_io::do_isostatic_tectonics() const {
    return _mapper.get_value<INTEGER>(DO_ISOSTATIC_TECTONICS, 0);
}

std::string gpm_control_file_io::isostatic_tectonics_parms() const {
		return _mapper.get_value<STRING>(ISOSTATIC_TECTONICS_PARMS, "");
	}

int gpm_control_file_io::do_flow_simulator_output() const {
    return _mapper.get_value<INTEGER>(DO_FLOW_SIMULATOR_OUTPUT, 0);
}

std::string gpm_control_file_io::flow_simulator_output_parms() const {
    return _mapper.get_value<STRING>(FLOW_SIMULATOR_OUTPUT_PARMS, "");
}

int gpm_control_file_io::do_pelagic_rain() const {
    return _mapper.get_value<INTEGER>(DO_PELAGIC_RAIN, 0);
}

std::string gpm_control_file_io::pelagic_rain_parms() const {
    return _mapper.get_value<STRING>(PELAGIC_RAIN_PARMS, "");
}

int gpm_control_file_io::do_plugins() const {
	return _mapper.get_value<INTEGER>(DO_PLUGINS, 0);
}

std::string gpm_control_file_io::plugins_ctrl_parms() const {
	return _mapper.get_value<STRING>(PLUGINS_CTRL_PARMS, "");
}

int gpm_control_file_io::do_bathymetry() const {
	return _mapper.get_value<INTEGER>(DO_BATHYMETRY, 0);
}
int gpm_control_file_io::do_include_active_layer() const {
	return _mapper.get_value<INTEGER>(DO_INCLUDE_ACTIVE_LAYER, 1);
}
int gpm_control_file_io::do_save_last_timestep_only() const {
	return _mapper.get_value<INTEGER>(DO_SAVE_LAST_TIMESTEP_ONLY, 0);
}
int gpm_control_file_io::do_output_incremental_files() const {
	return _mapper.get_value<INTEGER>(DO_OUTPUT_INCREMENTAL_FILES, 0);
}
}}}
