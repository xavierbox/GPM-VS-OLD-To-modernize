// -- Schlumberger Private --

#include "gpm_indprop_indexes.h"
#include <algorithm>

namespace Slb { namespace Exploration { namespace Gpm {

parm_base_indexes::parm_base_indexes(): itop(-1) {
}

parm_indexes::parm_indexes(): parm_base_indexes(), iscl(-1),
                              isvx_obsolete(-1), isvy_obsolete(-1),
                              ipvx(-1), ipvy(-1), ipvz(-1),
                              ipor(-1), iperm_hor(-1), iperm_vert(-1), iprs(-1),
                              idepodep(-1), ierosion(-1), ilithology(-1),
                              itop_slope_x(-1), itop_slope_y(-1),
                              isealevel(-1),
                              irel_sealevel_change_rate(-1),
                              idiffusion_energy(-1), isteady_flow_energy(-1),
                              iunsteady_flow_energy(-1), iwave_dissipation_energy(-1), iwave_onshore_energy(-1),
                              iwave_longshore_energy(-1), iwave_horizontal_diffusion_energy(-1), ogs_pressure(-1), 
                              ogs_velocity_x(-1), ogs_velocity_y(-1), ogs_velocity_z(-1)
{
}

zgrid_indexes::zgrid_indexes(): parm_indexes() {
}

indprop_indexes::indprop_indexes(): parm_indexes() {
}


int indprop_indexes::find_index(const std::vector<std::string>& index_names, const std::string& name) {
    const std::vector<std::string>::const_iterator it = std::find(index_names.begin(), index_names.end(), name);
    const int index = static_cast<int>(it != index_names.end() ? it - index_names.begin() : -1);
    return index;
}

void indprop_indexes::set_residence_time_indexes(std::vector<std::string>& prop_mapper, const std::vector<std::string>& index_names) {
    for (const auto& it: prop_mapper) {
        residence_time.push_back(find_index(index_names, it));
    }
}

void indprop_indexes::set_residence_time_def_indexes(std::vector<std::string>& prop_mapper, const std::vector<std::string>& index_names) {
    for (const auto& it : prop_mapper) {
        residence_def_time.push_back(find_index(index_names, it));
    }
}

void indprop_indexes::set_wave_induced_zone_flow_indexes(std::vector<std::string>& prop_mapper, const std::vector<std::string>& index_names) {
    for (const auto& it : prop_mapper) {
        wave_induced_zone_flow_time.push_back(find_index(index_names, it));
    }
}

void indprop_indexes::set_wave_induced_zone_surface_indexes(std::vector<std::string>& prop_mapper, const std::vector<std::string>& index_names) {
    for (const auto& it : prop_mapper) {
        wave_induced_zone_surface_time.push_back(find_index(index_names, it));
    }
}

void indprop_indexes::set_lattoral_residence_indexes(std::vector<std::string>& prop_mapper, const std::vector<std::string>& index_names) {
    for (const auto& it : prop_mapper) {
        residence_lattoral.push_back(find_index(index_names, it));
    }
}

void indprop_indexes::set_residence_time_surf_indexes(std::vector<std::string>& prop_mapper, const std::vector<std::string>& index_names) {
    for (const auto& it : prop_mapper) {
        residence_surf_time.push_back(find_index(index_names, it));
    }
}

void indprop_indexes::set_diagenesis_indexes(std::vector<std::string>& prop_mapper, const std::vector<std::string>& index_names) {
    for (const auto& it : prop_mapper) {
        diagenesis.push_back(find_index(index_names, it));
    }
}

void indprop_indexes::set_insitu_growth_indexes(std::vector<std::string>& prop_mapper, const std::vector<std::string>& index_names) {
    for (const auto& it: prop_mapper) {
        insitu_growth.push_back(find_index(index_names, it));
    }
}


void indprop_indexes::set_indexes(const std::vector<parm_scalar_output>& scalars,
    const std::vector<parm_vector_output>& vectors, const std::vector<std::string>& index_names)
{
    for(const auto& it: scalars) {
		parm_scalar_output tmp;
		tmp.prop_id=find_index(index_names, it.name);
		tmp.name = it.name;
		tmp.func = it.func;
		tmp.is_nodal = it.is_nodal;
		scalar_outputs.push_back(tmp);
    }
	for (const auto& it : vectors) {
		parm_vector_output tmp;
		tmp.prop_x_id = find_index(index_names, it.name_x);
		tmp.name_x = it.name_x;
		tmp.prop_y_id = find_index(index_names, it.name_y);
		tmp.name_y = it.name_y;
		tmp.func = it.func;
		tmp.is_nodal = it.is_nodal;
		vector_outputs.push_back(tmp);
	}
}

grid_creation_options::grid_creation_options()
= default;

grid_creation_options::
grid_creation_options(bool make_layer, bool make_sediments, bool make_surfaces, bool compaction, bool pressure): make_new_layer(make_layer),
                                                                                 make_new_sediments(make_sediments),
                                                                                 make_all_surfaces(make_surfaces), has_compaction(compaction), make_pressure(pressure)
{
}
}}}
