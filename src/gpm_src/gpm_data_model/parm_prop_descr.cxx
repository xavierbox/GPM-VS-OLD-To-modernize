// -- Schlumberger Private --

#include "parm_prop_descr.h"
#include "gpm_string_utils.h"
#include <boost/algorithm/string/case_conv.hpp>

namespace Slb { namespace Exploration { namespace Gpm {


// TODO downgrade the version to 1.1 so 2017 and 2018 commercial can read the results of the simulation
// Has to do with the statistics that is added
std::string parm_descr_holder::get_current_version() { return ("1.1.0.0"); }

const std::map<PARM_GRID_MAP_NAMES, std::string>& parm_descr_holder::get_grid_map_names()
{
    static std::map<PARM_GRID_MAP_NAMES, std::string> grid_map_name =
    {
        {TOP, "TOP"},
        {SV_X_OBSOLETE, "SVX"},
        {SV_Y_OBSOLETE, "SVY"},
        {PVX, "PVX"},
        {PVY, "PVY"},
        {PVZ, "PVZ"},
        {POR, "POR"},
        {PRS, "PRS"},
        {DEPODEPTH, "DEPODEPTH"},
        {EROSION, "EROSION"},
        {SRC, "SRC"},
        {TEC, "TEC"},
        {TOP_SLOPE_X, "TOP_SLOPE_X"},
        {TOP_SLOPE_Y, "TOP_SLOPE_Y"},
        {WATER_TOP_FLUID, "WATER_TOP_FLUID"},
        {WATER_TOP_FLUID_VEL_X, "WATER_TOP_FLUID_VEL_X"},
        {WATER_TOP_FLUID_VEL_Y, "WATER_TOP_FLUID_VEL_Y"},
        {WATER_TOP_TURB, "WATER_TOP_TURB"},
        {WATER_TOP_TURB_VEL_X, "WATER_TOP_TURB_VEL_X"},
        {WATER_TOP_TURB_VEL_Y, "WATER_TOP_TURB_VEL_Y"},
        {LITHOLOGY, "LITHOLOGY"},
        {WAVE_ENERGY_DISSIP, "WAVE_ENERGY_DISSIP"},
        {WAVE_TRAVEL_TIME, "WAVE_TRAVEL_TIME"},
        {WAVE_DIR_X, "WAVE_DIR_X"},
        {WAVE_DIR_Y, "WAVE_DIR_Y"},
        {WAVE_VEL_EFF_X, "WAVE_VEL_EFF_X"},
        {WAVE_VEL_EFF_Y, "WAVE_VEL_EFF_Y"},
        {WAVE_LONGSHORE_DIR_X, "WAVE_LONGSHORE_DIR_X"},
        {WAVE_LONGSHORE_DIR_Y, "WAVE_LONGSHORE_DIR_Y"},
        {WAVE_LONGSHORE_VEL_X, "WAVE_LONGSHORE_VEL_X"},
        {WAVE_LONGSHORE_VEL_Y, "WAVE_LONGSHORE_VEL_Y"},
        {SED_MOVE_X, "SED_MOVE_X"},
        {SED_MOVE_Y, "SED_MOVE_Y"},
        {SEALEVEL, "SEALEVEL"},
        {REL_SEALEVEL_CHANGE_RATE, "REL_SEALEVEL_CHANGE_RATE"},
        {RES_TIME_DEF, "RES_TIME_DEF"},
        {WAVE_INDUCED_ZONE_FLOW, "WAVE_INDUCED_ZONE_FLOW"},
        {WAVE_INDUCED_ZONE_SURFACE, "WAVE_INDUCED_ZONE_SURFACE"},
        {RESIDENCE_LATTORAL, "RESIDENCE_LATTORAL"},
        {PERMEABILITY_HORI, "PERMEABILITY_HORI"},
        {PERMEABILITY_VERT, "PERMEABILITY_VERT"},
        {OGS_PRESSURE, "OGS_PRESSURE"},
        {OGS_VELOCITY_X, "OGS_VELOCITY_X"},
        {OGS_VELOCITY_Y, "OGS_VELOCITY_Y"},
        {OGS_VELOCITY_Z, "OGS_VELOCITY_Z"},
        {CARBONATE_DROWNING_HEIGHT, "CARBONATE_DROWNING_HEIGHT"},
        {DIFFUSION_ENERGY, "DIFFUSION_ENERGY"},
        {STEADY_FLOW_ENERGY, "STEADY_FLOW_ENERGY"},
        {UNSTEADY_FLOW_ENERGY, "UNSTEADY_FLOW_ENERGY"},
        {WAVE_DISSIPATION_ENERGY, "WAVE_DISSIPATION_ENERGY"},
        {WAVE_ONSHORE_ENERGY, "WAVE_ONSHORE_ENERGY"},
        {WAVE_LONGSHORE_ENERGY, "WAVE_LONGSHORE_ENERGY"},
        {WATER_BOTTOM_VELOCITY, "WATER_BOTTOM_VELOCITY"},
        {WAVE_HORIZONTAL_DIFFUSION_ENERGY, "WAVE_HORIZONTAL_DIFFUSION_ENERGY"}
    };
    return grid_map_name;
}

std::string parm_descr_holder::get_grid_map_name(PARM_GRID_MAP_NAMES id)
{
    return get_grid_map_names().at(id);
}

const std::map<PARM_GRID_VECTOR_2D_NAMES, std::string>& parm_descr_holder::get_grid_map_vector_2d_names()
{
    static std::map<PARM_GRID_VECTOR_2D_NAMES, std::string> grid_map_vector_2d_name =
    {
        {SV, "SV"},
        {TOP_SLOPE, "TOP_SLOPE"},
        {WATER_TOP_FLUID_VEL, "WATER_TOP_FLUID_VEL"},
        {WATER_TOP_TURB_VEL, "WATER_TOP_TURB_VEL"},
        {WAVE_DIR, "WAVE_DIR"},
        {WAVE_LONGSHORE_VEL, "WAVE_LONGSHORE_VEL"},
        {WAVE_LONGSHORE_DIR, "WAVE_LONGSHORE_DIR"},
        {WAVE_VEL_EFF, "WAVE_VEL_EFF"},
        {SED_MOVE, "SED_MOVE"}
    };
    return grid_map_vector_2d_name;
}

const std::map<PARM_GRID_VECTOR_2D_NAMES, std::pair<PARM_GRID_MAP_NAMES, PARM_GRID_MAP_NAMES>>& parm_descr_holder::
get_grid_map_vector_2d()
{
    static std::map<PARM_GRID_VECTOR_2D_NAMES, std::pair<PARM_GRID_MAP_NAMES, PARM_GRID_MAP_NAMES>> grid_map_vector_2d =
    {
        {SV, std::make_pair(SV_X_OBSOLETE, SV_Y_OBSOLETE)},
        {TOP_SLOPE, std::make_pair(TOP_SLOPE_X, TOP_SLOPE_Y)},
        {WATER_TOP_FLUID_VEL, std::make_pair(WATER_TOP_FLUID_VEL_X, WATER_TOP_FLUID_VEL_Y)},
        {WATER_TOP_TURB_VEL, std::make_pair(WATER_TOP_TURB_VEL_X, WATER_TOP_TURB_VEL_Y)},
        {WAVE_DIR, std::make_pair(WAVE_DIR_X, WAVE_DIR_Y)},
        {WAVE_LONGSHORE_VEL, std::make_pair(WAVE_LONGSHORE_VEL_X, WAVE_LONGSHORE_VEL_Y)},
        {WAVE_LONGSHORE_DIR, std::make_pair(WAVE_LONGSHORE_DIR_X, WAVE_LONGSHORE_DIR_Y)},
        {WAVE_VEL_EFF, std::make_pair(WAVE_VEL_EFF_X, WAVE_VEL_EFF_Y)},
        {SED_MOVE, std::make_pair(SED_MOVE_X, SED_MOVE_Y)}
    };
    return grid_map_vector_2d;
}

const std::vector<PARM_GRID_MAP_NAMES>& parm_descr_holder::get_grid_map_top_layer_only_properties()
{
    static std::vector<PARM_GRID_MAP_NAMES> grid_map_top_layer_only_properties =
    {
        SV_X_OBSOLETE,
        SV_Y_OBSOLETE,
        DEPODEPTH,
        EROSION,
        TOP_SLOPE_X,
        TOP_SLOPE_Y,
        WATER_TOP_FLUID,
        WATER_TOP_FLUID_VEL_X,
        WATER_TOP_FLUID_VEL_Y,
        WATER_TOP_TURB,
        WATER_TOP_TURB_VEL_X,
        WATER_TOP_TURB_VEL_Y,
        LITHOLOGY,
        WAVE_ENERGY_DISSIP,
        WAVE_TRAVEL_TIME,
        WAVE_DIR_X,
        WAVE_DIR_Y,
        WAVE_VEL_EFF_X,
        WAVE_VEL_EFF_Y,
        WAVE_LONGSHORE_DIR_X,
        WAVE_LONGSHORE_DIR_Y,
        WAVE_LONGSHORE_VEL_X,
        WAVE_LONGSHORE_VEL_Y,
        SED_MOVE_X,
        SED_MOVE_Y,
        SEALEVEL,
        REL_SEALEVEL_CHANGE_RATE,
        WATER_BOTTOM_VELOCITY
    };
    return grid_map_top_layer_only_properties;
}

const std::string& parm_descr_holder::get_sediment_property_prefix()
{
    static std::string tmp("SED");
    return tmp;
}

const std::string& parm_descr_holder::get_insitu_growth_property_prefix()
{
    static std::string tmp("INSITU_GROWTH");
    return tmp;
}

const std::string& parm_descr_holder::get_residence_time_property_prefix()
{
    static std::string tmp("RES_TIME");
    return tmp;
}

const std::string& parm_descr_holder::get_residence_time_surface_property_prefix()
{
    static std::string tmp("RES_TIME_SURF");
    return tmp;
}

const std::string& parm_descr_holder::get_deposition_contribution_property_prefix()
{
    static std::string tmp("DEPOSITION_CONTRIBUTION");
    return tmp;
}

const std::string& parm_descr_holder::get_diagenesis_property_prefix()
{
    static std::string tmp("DIAGENESIS");
    return tmp;
}

const std::string& parm_descr_holder::get_erodability_property_prefix()
{
    static std::string tmp("ERODABILITY");
    return tmp;
}

const std::map<PARM_GRID_MAP_1_INDEX_NAMES, std::string>& parm_descr_holder::get_grid_map_1_index_name_prefixes()
{
    static std::map<PARM_GRID_MAP_1_INDEX_NAMES, std::string> grid_map_1_index_name_prefix =
    {
        {SED_j, get_sediment_property_prefix()},
        {INSITU_GROWTH_j, get_insitu_growth_property_prefix()},
        {RES_TIME_j, get_residence_time_property_prefix()},
        {RES_TIME_SURF_j, get_residence_time_surface_property_prefix()},
        {DEPOSITION_CONTRIBUTION_j, get_deposition_contribution_property_prefix()},
        {DIAGENESIS_j, get_diagenesis_property_prefix()},
        {ERODABILITY_j, get_erodability_property_prefix()}
    };
    return grid_map_1_index_name_prefix;
}

std::vector<std::string> parm_descr_holder::get_grid_map_1_index_name_prefixes_as_strings()
{
    std::vector<std::string> res;
    for (auto it : get_grid_map_1_index_name_prefixes()) {
        res.push_back(it.second);
    }
    return res;
}

std::string parm_descr_holder::get_grid_map_1_index_name_prefix(PARM_GRID_MAP_1_INDEX_NAMES id)
{
    return get_grid_map_1_index_name_prefixes().at(id);
}

const std::map<PARM_DEPOSITIONAL_ENERGY_PROCESS_NAMES, std::string>& parm_descr_holder::get_process_map_depo_energies()
{
    static std::map<PARM_DEPOSITIONAL_ENERGY_PROCESS_NAMES, std::string> process_map_depo_energy =
    {
        {DIFFUSION_PROCESS, "DIFFUSION_PROCESS"},
        {STEADY_FLOW_PROCESS, "STEADY_FLOW_PROCESS"},
        {UNSTEADY_FLOW_PROCESS, "UNSTEADY_FLOW_PROCESS"},
        {CARBONATE_PROCESS, "CARBONATE_PROCESS"},
        {WAVE_PROCESS_ENERGY_DISSIPATION, "WAVE_PROCESS_ENERGY_DISSIPATION"},
        {WAVE_PROCESS_ONSHORE, "WAVE_PROCESS_ONSHORE"},
        {WAVE_PROCESS_LONGSHORE, "WAVE_PROCESS_LONGSHORE"}
    };
    return process_map_depo_energy;
}

std::string parm_descr_holder::grid_property_postfix_index_name(PARM_GRID_MAP_1_INDEX_NAMES id, int sed)
{
    const std::string key = get_grid_map_1_index_name_prefixes().at(id);
    return key + std::to_string(sed);
}

std::string parm_descr_holder::grid_instance_postfix_name(PARM_GRID_MAP_1_INDEX_NAMES id, int index)
{
    std::string key = get_grid_map_1_index_name_prefixes().at(id);
    boost::to_lower(key);
    auto alpha_str = key + std::to_string(index) + "_";
    return alpha_str;
}


std::string parm_descr_holder::grid_instance_postfix_name(PARM_GRID_MAP_1_INDEX_NAMES id, int index, int time_index,
                                                          int top)
{
    const auto alpha_str = grid_instance_postfix_name(id, index);
    return Tools::two_number_format(alpha_str, time_index, top);
}

std::string parm_descr_holder::grid_instance_postfix_name(PARM_GRID_MAP_1_INDEX_NAMES id, int index, int time_index)
{
    const auto alpha_str = grid_instance_postfix_name(id, index);
    return Tools::single_number_format(alpha_str, time_index);
}

std::string parm_descr_holder::grid_property_postfix_index_name(const std::string& id, int sed)
{
    return id + std::to_string(sed);
}

std::string parm_descr_holder::grid_instance_postfix_name(const std::string& id, int index)
{
    std::string key = id;
    boost::to_lower(key);
    auto alpha_str = key + std::to_string(index) + "_";
    return alpha_str;
}


std::vector<std::string> parm_descr_holder::grid_instance_postfix_name(const std::string& id, int index,
                                                                       int time_index, int first_surface_index,
                                                                       long last_surface_index)
{
    const auto alpha_str = grid_instance_postfix_name(id, index);
    std::vector<std::string> res;
    for (int i = first_surface_index; i <= last_surface_index; ++i) {
        res.push_back(Tools::two_number_format(alpha_str, time_index, i));
    }
    return res;
}

std::string parm_descr_holder::grid_instance_postfix_name(const std::string& id, int index, int time_index, int top)
{
    const auto alpha_str = grid_instance_postfix_name(id, index);
    return Tools::two_number_format(alpha_str, time_index, top);
}

std::string parm_descr_holder::grid_instance_postfix_name(const std::string& id, int index, int time_index)
{
    const auto alpha_str = grid_instance_postfix_name(id, index);
    return Tools::single_number_format(alpha_str, time_index);
}

std::string parm_descr_holder::grid_instance_name(const std::string& id)
{
    std::string key = id + "_";
    boost::to_lower(key);
    return key;
}

std::string parm_descr_holder::grid_instance_name(PARM_GRID_MAP_NAMES id)
{
    return grid_instance_name(get_grid_map_names().at(id));
}

std::vector<std::string> parm_descr_holder::grid_instance_name(PARM_GRID_MAP_NAMES id, int time_index,
                                                               int first_surface_index, long last_surface_index)
{
    const std::string key = grid_instance_name(id);
    std::vector<std::string> res;
    for (int i = first_surface_index; i <= last_surface_index; ++i) {
        res.push_back(Tools::two_number_format(key, time_index, i));
    }
    return res;
}

std::string parm_descr_holder::grid_instance_name(PARM_GRID_MAP_NAMES id, int time_index, int top)
{
    const std::string key = grid_instance_name(id);
    return Tools::two_number_format(key, time_index, top);
}

std::string parm_descr_holder::grid_instance_name(const std::string& id, int time_index)
{
    const std::string key = grid_instance_name(id);
    return Tools::single_number_format(key, time_index);
}

std::string parm_descr_holder::grid_instance_name(PARM_GRID_MAP_NAMES id, int time_index)
{
    const std::string key = grid_instance_name(id);
    return Tools::single_number_format(key, time_index);
}

property_description parm_descr_holder::make_property_description(PARM_GRID_MAP_NAMES id,
                                                                  const std::string& display_name, bool nodal)
{
    return property_description(parm_descr_holder::get_grid_map_names().at(id), display_name, nodal);
}


}}}
