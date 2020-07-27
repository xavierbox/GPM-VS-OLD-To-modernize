// -- Schlumberger Private --

#include "gpm_indprop_indexes_helper.h"

namespace Slb { namespace Exploration { namespace Gpm {


void gpm_set_indprop_indexes(const std::map<PARM_GRID_MAP_NAMES, std::string>& prop_mapper, const std::vector<std::string>& index_names, indprop_indexes* holder_p) {
    // may have a mismatch sometime here
    indprop_indexes& holder(*holder_p);
    if (indprop_indexes::find_index(index_names, prop_mapper.at(TOP_SLOPE_X)) != -1) {
        holder.itop_slope_x = indprop_indexes::find_index(index_names, prop_mapper.at(TOP_SLOPE_X));
        holder.itop_slope_y = indprop_indexes::find_index(index_names, prop_mapper.at(TOP_SLOPE_Y));
    }
    if (indprop_indexes::find_index(index_names, prop_mapper.at(PVX)) != -1) {
        holder.ipvx = indprop_indexes::find_index(index_names, prop_mapper.at(PVX));
        holder.ipvy = indprop_indexes::find_index(index_names, prop_mapper.at(PVY));
        holder.ipvz = indprop_indexes::find_index(index_names, prop_mapper.at(PVZ));
    }

    if (indprop_indexes::find_index(index_names, prop_mapper.at(POR)) != -1) {
        holder.ipor = indprop_indexes::find_index(index_names, prop_mapper.at(POR));
    }
 
    if (indprop_indexes::find_index(index_names, prop_mapper.at(PERMEABILITY_HORI)) != -1) {
        holder.iperm_hor = indprop_indexes::find_index(index_names, prop_mapper.at(PERMEABILITY_HORI));
    }
    if (indprop_indexes::find_index(index_names, prop_mapper.at(PERMEABILITY_VERT)) != -1) {
        holder.iperm_vert = indprop_indexes::find_index(index_names, prop_mapper.at(PERMEABILITY_VERT));
    }

    if (indprop_indexes::find_index(index_names, prop_mapper.at(PRS)) != -1) {
        holder.iprs = indprop_indexes::find_index(index_names, prop_mapper.at(PRS));
    }

    holder.idepodep = indprop_indexes::find_index(index_names, prop_mapper.at(DEPODEPTH));
    holder.ierosion = indprop_indexes::find_index(index_names, prop_mapper.at(EROSION));
    holder.ilithology = indprop_indexes::find_index(index_names, prop_mapper.at(LITHOLOGY));
    holder.isealevel = indprop_indexes::find_index(index_names, prop_mapper.at(SEALEVEL));
    holder.irel_sealevel_change_rate = indprop_indexes::find_index(index_names, prop_mapper.at(REL_SEALEVEL_CHANGE_RATE));
    holder.idiffusion_energy = indprop_indexes::find_index(index_names, prop_mapper.at(DIFFUSION_ENERGY));
    holder.isteady_flow_energy = indprop_indexes::find_index(index_names, prop_mapper.at(STEADY_FLOW_ENERGY));
    holder.iunsteady_flow_energy = indprop_indexes::find_index(index_names, prop_mapper.at(UNSTEADY_FLOW_ENERGY));
    holder.iwave_dissipation_energy = indprop_indexes::find_index(index_names, prop_mapper.at(WAVE_DISSIPATION_ENERGY));
    holder.iwave_onshore_energy = indprop_indexes::find_index(index_names, prop_mapper.at(WAVE_ONSHORE_ENERGY));
    holder.iwave_longshore_energy = indprop_indexes::find_index(index_names, prop_mapper.at(WAVE_LONGSHORE_ENERGY));
    holder.iwave_horizontal_diffusion_energy = indprop_indexes::find_index(index_names, prop_mapper.at(WAVE_HORIZONTAL_DIFFUSION_ENERGY));

	if (indprop_indexes::find_index(index_names, prop_mapper.at(OGS_PRESSURE)) != -1) {
        holder.ogs_pressure = indprop_indexes::find_index(index_names, prop_mapper.at(OGS_PRESSURE));
	}
    if (indprop_indexes::find_index(index_names, prop_mapper.at(OGS_VELOCITY_X)) != -1) {
        holder.ogs_velocity_x = indprop_indexes::find_index(index_names, prop_mapper.at(OGS_VELOCITY_X));
    }
    if (indprop_indexes::find_index(index_names, prop_mapper.at(OGS_VELOCITY_Y)) != -1) {
        holder.ogs_velocity_y = indprop_indexes::find_index(index_names, prop_mapper.at(OGS_VELOCITY_Y));
    }
    if (indprop_indexes::find_index(index_names, prop_mapper.at(OGS_VELOCITY_Z)) != -1) {
        holder.ogs_velocity_z = indprop_indexes::find_index(index_names, prop_mapper.at(OGS_VELOCITY_Z));
    }

}

}}}
