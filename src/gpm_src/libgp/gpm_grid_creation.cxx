// -- Schlumberger Private --

#include "gpm_parm_helper_methods.h"
#include "gpm_indprop_indexes.h"
#include "mapstc.h"
#include "gpm_logger.h"
#include <boost/numeric/conversion/cast.hpp>
#include <boost/core/null_deleter.hpp>

namespace Slb { namespace Exploration { namespace Gpm{

using namespace Tools;

namespace {

void gpm_create_new_zgrid_range(parm* p_p, PARM_GRID_MAP_NAMES id, int prop_id, int time_index, int surf_first, int surface_last)
{
    const auto res_names = parm_descr_holder::grid_instance_name(id, time_index, surf_first, surface_last);
    //printf("gpm_create_new_zgrid_range: name %s \n", res_names.)
    create_grids_for_timestep(p_p, prop_id, time_index, surf_first, surface_last,p_p->znull, res_names);
}
int add_zgrid_assign_to_index(parm* p, const std::string& name, int prop_index, int time_index, int top_index)
{
    const int old_num = boost::numeric_cast<long>(p->zgrid.layers()); // Assuming no change in row col layout
    const int new_num = old_num + 1;
    // Set them to znull;
    p->zgrid.resize(new_num, p->zgrid.rows(), p->zgrid.cols(), true, p->znull);
    // Really bad, out of sync
    p->gridnam.resize(new_num);
    p->gridnam[old_num] = name;
    p->indprop(prop_index, time_index, top_index).set_index(old_num);
    p->indprop(prop_index, time_index, top_index).set_array(
        std::shared_ptr<float_3darray_vec_base_type>(&(p->zgrid), boost::null_deleter()));
    return old_num;
}

}

int gpm_create_new_zgrid(parm* p_p, const std::string& id, int prop_id, int time_index, int surface_index)
{
	const auto name = parm_descr_holder::grid_instance_name(id, time_index);
	return add_zgrid_assign_to_index(p_p, name, prop_id, time_index, surface_index);
}

int gpm_create_new_zgrid(parm* p_p, PARM_GRID_MAP_NAMES id, int prop_id, int time_index, int surface_index)
{
	const auto name = parm_descr_holder::grid_instance_name(id, time_index);
    return add_zgrid_assign_to_index(p_p, name, prop_id, time_index, surface_index);
}

void gpm_create_residence_time_grids(parm* p_p, const indprop_indexes& prop_indexes, int new_time_index, int first_surface_index, long last_surface_index) {
	const auto prefix = parm_descr_holder::get_residence_time_property_prefix();
    for(auto residence_time_ind = 0u; residence_time_ind < prop_indexes.residence_time.size(); ++residence_time_ind){
        const auto res_names = parm_descr_holder::grid_instance_postfix_name(prefix, residence_time_ind + 1, new_time_index, first_surface_index, last_surface_index);
        create_grids_for_timestep(p_p, prop_indexes.residence_time[residence_time_ind], new_time_index, first_surface_index, last_surface_index,p_p->znull, res_names);
    }
}

void gpm_create_residence_time_def_grids(parm* p_p, const indprop_indexes& prop_indexes, int new_time_index) {
    const std::vector<std::string> res_names = {parm_descr_holder::grid_instance_name(PARM_GRID_MAP_NAMES::RES_TIME_DEF, new_time_index)};
    // The prop_index could just be an integer, not a list
    create_grids_for_timestep(p_p, prop_indexes.residence_def_time[0], new_time_index, 0, 0, p_p->znull, res_names);
}

void gpm_create_residence_time_surf_grids(parm* p_p, const indprop_indexes& prop_indexes, int new_time_index) {
	const auto prefix = parm_descr_holder::get_residence_time_surface_property_prefix();
    for (auto residence_time_ind = 0u; residence_time_ind < prop_indexes.residence_surf_time.size(); ++residence_time_ind) {
        // Only one per surface, and we store them in col 0 as well
        const std::vector<std::string> res_names = { parm_descr_holder::grid_instance_postfix_name(prefix, residence_time_ind + 1, new_time_index) };
        create_grids_for_timestep(p_p, prop_indexes.residence_surf_time[residence_time_ind], new_time_index, 0, 0, p_p->znull, res_names);
    }
}

void gpm_create_wave_induced_zone_flow_grids(parm* p_p, const indprop_indexes& prop_indexes, int new_time_index, int first_surface_index, int last_surface_index){
    const std::vector<std::string> res_names = { parm_descr_holder::grid_instance_name(PARM_GRID_MAP_NAMES::WAVE_INDUCED_ZONE_FLOW, new_time_index, first_surface_index, last_surface_index) };
    // The prop_index could just be an integer, not a list
    create_grids_for_timestep(p_p, prop_indexes.wave_induced_zone_flow_time[0], new_time_index, first_surface_index, last_surface_index, p_p->znull, res_names);
}

void gpm_create_wave_induced_zone_surface_grids(parm* p_p, const indprop_indexes& prop_indexes, int new_time_index){
    const std::vector<std::string> res_names = { parm_descr_holder::grid_instance_name(PARM_GRID_MAP_NAMES::WAVE_INDUCED_ZONE_SURFACE, new_time_index) };
    // The prop_index could just be an integer, not a list
    create_grids_for_timestep(p_p, prop_indexes.wave_induced_zone_surface_time[0], new_time_index, 0, 0, p_p->znull, res_names);
}

void gpm_create_residence_lattoral_grids(parm* p_p, const indprop_indexes& prop_indexes, int new_time_index) {
    const std::vector<std::string> res_names = { parm_descr_holder::grid_instance_name(PARM_GRID_MAP_NAMES::RESIDENCE_LATTORAL, new_time_index) };
// The prop_index could just be an integer, not a list
    create_grids_for_timestep(p_p, prop_indexes.residence_lattoral[0], new_time_index, 0, 0, p_p->znull, res_names);
}
void gpm_create_diagenesis_grids(parm* p_p, const indprop_indexes& prop_indexes, int new_time_index, int first_surface_index, long last_surface_index) {
	const auto prefix = parm_descr_holder::get_diagenesis_property_prefix();
	for (auto residence_time_ind = 0u; residence_time_ind < prop_indexes.diagenesis.size(); ++residence_time_ind) {
        const auto res_names = parm_descr_holder::grid_instance_postfix_name(prefix, residence_time_ind+1, new_time_index, first_surface_index, last_surface_index);
        create_grids_for_timestep(p_p, prop_indexes.diagenesis[residence_time_ind], new_time_index, first_surface_index, last_surface_index, p_p->znull, res_names);
    }
}

    

int 
gpm_grid_create_new_timestep(parm* p_p, const indprop_indexes& prop_indexes, const grid_creation_options& create_objs, zgrid_indexes* z_indexes_p, const Tools::gpm_logger& logger)
{
    auto base_indexes(prop_indexes.sed_indexes);
	base_indexes.insert(base_indexes.begin(), prop_indexes.itop);
	std::vector<int> all_indexes=base_indexes;
	// Find all < 0 and remove them
	{
        const auto it = std::remove_if(all_indexes.begin(), all_indexes.end(), std::bind(std::less<int>(), std::placeholders::_1, 0));
		if (it != all_indexes.end()){
			all_indexes.erase(it, all_indexes.end());
		}
	}
	// Expand indprop for a new time and top
    const auto old_top_size = p_p->numtops();
	{
        const auto prop_size = boost::numeric_cast<int>(p_p->propnam.size());
        const auto time_size = p_p->numtims() + 1;
        const auto top_size = create_objs.make_new_layer ? p_p->numtops() + 1: p_p->numtops(); // Allow for sea level
		p_p->indprop.resize(prop_size, time_size, top_size);
		//p_p->numtims()++;  // These should be calls to indprop now
		//p_p->numtops() = top_size;
	}
    const auto new_time_index = p_p->numtims() - 1;
	// Set output indices (should use intxxx instead of ixxx)
	// Gets one extra empty, shouldn't matter
	gpm_copy_previous_timestep(p_p->indprop, new_time_index, new_time_index-1, old_top_size, all_indexes);

    const auto last_surface_index = p_p->numtops()-1;
	// If an actual new layer needs to be defined,
	// copy water, velx, vely and top one up for all times
    if (create_objs.make_new_layer) {

        // Create one new top */
        const auto top_name = parm_descr_holder::grid_instance_name(PARM_GRID_MAP_NAMES::TOP, new_time_index,
                                                                    last_surface_index);
        // Create one new top */
        const auto ival = add_zgrid_assign_to_index(p_p, top_name, prop_indexes.itop, new_time_index,
                                                    last_surface_index);
        if (ival < 0) {
            logger.print(LOG_NORMAL, "Error: Constant topography, inttop[%d] = %d\n", last_surface_index, ival);
            return -1;
        }
        z_indexes_p->itop = ival;
        logger.print(LOG_VERBOSE, "new top: p.numgrds=%d\n", p_p->numgrds());

        if (create_objs.make_new_sediments) {
            // Rename to sed1, sed2 sed3
            z_indexes_p->sed_indexes.resize(prop_indexes.sed_indexes.size());
			auto prefix = parm_descr_holder::get_sediment_property_prefix();
            for (auto sed_ind = 0u; sed_ind < prop_indexes.sed_indexes.size(); ++sed_ind) {
                const auto sed_id = parm_descr_holder::grid_instance_postfix_name(
					prefix, sed_ind + 1, new_time_index, last_surface_index);
                z_indexes_p->sed_indexes[sed_ind] = add_zgrid_assign_to_index(
                    p_p, sed_id, prop_indexes.sed_indexes[sed_ind], new_time_index, last_surface_index);
            }
            z_indexes_p->erodability.resize(prop_indexes.erodability.size());
			prefix = parm_descr_holder::get_erodability_property_prefix(); 
            for (auto sed_ind = 0u; sed_ind < prop_indexes.erodability.size(); ++sed_ind) {
                const auto sed_id = parm_descr_holder::grid_instance_postfix_name(
                    prefix, sed_ind + 1, new_time_index, last_surface_index);
                z_indexes_p->erodability[sed_ind] = add_zgrid_assign_to_index(
                    p_p, sed_id, prop_indexes.erodability[sed_ind], new_time_index, last_surface_index);
            }

            logger.print(LOG_VERBOSE, "new sediments: p.numgrds=%d\n", p_p->numgrds());
        }
        else {
            // Todo this should not be neccessary JT
            // Copy previous seds
            for (auto sed_ind = 0u; sed_ind < prop_indexes.sed_indexes.size(); ++sed_ind) {
                p_p->indprop(prop_indexes.sed_indexes[sed_ind], new_time_index, last_surface_index) = p_p->indprop(
                    prop_indexes.sed_indexes[sed_ind], new_time_index, p_p->numtops() - 2);
            }
            for (auto sed_ind = 0u; sed_ind < prop_indexes.erodability.size(); ++sed_ind) {
                p_p->indprop(prop_indexes.erodability[sed_ind], new_time_index, last_surface_index) = p_p->indprop(
                    prop_indexes.erodability[sed_ind], new_time_index, p_p->numtops() - 2);
            }
        }
        if (prop_indexes.idepodep >= 0) {
            const auto name = parm_descr_holder::grid_instance_name(PARM_GRID_MAP_NAMES::DEPODEPTH, new_time_index);
            z_indexes_p->idepodep = add_zgrid_assign_to_index(p_p, name, prop_indexes.idepodep, new_time_index, 0);
            logger.print(LOG_VERBOSE, "new depodep: p.numgrds=%d\n", p_p->numgrds());
        }
    }

	if (!prop_indexes.insitu_growth.empty())
	{
		z_indexes_p->insitu_growth.resize(prop_indexes.insitu_growth.size());
		const auto prefix = parm_descr_holder::get_insitu_growth_property_prefix();
        for (auto insitu_growth_ind = 0u; insitu_growth_ind < prop_indexes.insitu_growth.size(); ++insitu_growth_ind) {
            const auto sed_id = parm_descr_holder::grid_instance_postfix_name(prefix, insitu_growth_ind + 1, new_time_index);
            z_indexes_p->insitu_growth[insitu_growth_ind] = add_zgrid_assign_to_index(p_p, sed_id, prop_indexes.insitu_growth[insitu_growth_ind], new_time_index, 0);
        }
	}

	// Create one new vector x and one new vector y */
	if(prop_indexes.itop_slope_x >= 0 &&  prop_indexes.itop_slope_y >= 0 ){ 
        z_indexes_p->itop_slope_x = gpm_create_new_zgrid(p_p, TOP_SLOPE_X, prop_indexes.itop_slope_x, new_time_index,0);
		z_indexes_p->itop_slope_y = gpm_create_new_zgrid(p_p, TOP_SLOPE_Y,prop_indexes.itop_slope_y, new_time_index,0);
	}

	z_indexes_p->scalar_outputs = prop_indexes.scalar_outputs;
    for (auto& it : z_indexes_p->scalar_outputs) {
        it.zgrid_id = gpm_create_new_zgrid(p_p, it.name, it.prop_id, new_time_index, 0);
        logger.print(LOG_VERBOSE, "new %s: p.numgrds=%d\n", it.name.c_str(), p_p->numgrds());
    }
	z_indexes_p->vector_outputs = prop_indexes.vector_outputs;
	for (auto& it : z_indexes_p->vector_outputs) {
		it.zgrid_x_id = gpm_create_new_zgrid(p_p, it.name_x, it.prop_x_id, new_time_index, 0);
		it.zgrid_y_id = gpm_create_new_zgrid(p_p, it.name_y, it.prop_y_id, new_time_index, 0);
	}

	if (prop_indexes.ierosion >= 0 ) {
        z_indexes_p->ierosion = gpm_create_new_zgrid(p_p, EROSION, prop_indexes.ierosion,new_time_index,0);
		logger.print(LOG_VERBOSE,"new erosion: p.numgrds=%d\n",p_p->numgrds());
	}
	if ( prop_indexes.ilithology >= 0 ){
        z_indexes_p->ilithology = gpm_create_new_zgrid(p_p, LITHOLOGY, prop_indexes.ilithology,new_time_index,0);
		logger.print(LOG_VERBOSE, "new lithology: p.numgrds=%d\n",p_p->numgrds());
	}
	if(prop_indexes.irel_sealevel_change_rate >= 0){
        z_indexes_p->irel_sealevel_change_rate = gpm_create_new_zgrid(p_p, REL_SEALEVEL_CHANGE_RATE, prop_indexes.irel_sealevel_change_rate,new_time_index,0);
		logger.print(LOG_VERBOSE, "new relative sea level change: p.numgrds=%d\n",p_p->numgrds());
	}
    if(prop_indexes.idiffusion_energy >= 0) {
        z_indexes_p->idiffusion_energy = gpm_create_new_zgrid(p_p, DIFFUSION_ENERGY, prop_indexes.idiffusion_energy, new_time_index, 0);
        logger.print(LOG_VERBOSE, "new diffusion energy: p.numgrds=%d\n", p_p->numgrds());
    }
    if (prop_indexes.isteady_flow_energy >= 0) {
        z_indexes_p->isteady_flow_energy = gpm_create_new_zgrid(p_p, STEADY_FLOW_ENERGY, prop_indexes.isteady_flow_energy, new_time_index, 0);
        logger.print(LOG_VERBOSE, "new steady flow energy: p.numgrds=%d\n", p_p->numgrds());
    }
    if (prop_indexes.iunsteady_flow_energy >= 0) {
        z_indexes_p->iunsteady_flow_energy = gpm_create_new_zgrid(p_p, UNSTEADY_FLOW_ENERGY, prop_indexes.iunsteady_flow_energy, new_time_index, 0);
        logger.print(LOG_VERBOSE, "new unsteady flow energy: p.numgrds=%d\n", p_p->numgrds());
    }
    if (prop_indexes.iwave_dissipation_energy >= 0) {
        z_indexes_p->iwave_dissipation_energy = gpm_create_new_zgrid(p_p, WAVE_DISSIPATION_ENERGY, prop_indexes.iwave_dissipation_energy, new_time_index, 0);
        logger.print(LOG_VERBOSE, "new wave dissipation energy: p.numgrds=%d\n", p_p->numgrds());
    }
    if (prop_indexes.iwave_onshore_energy >= 0) {
        z_indexes_p->iwave_onshore_energy = gpm_create_new_zgrid(p_p, WAVE_ONSHORE_ENERGY, prop_indexes.iwave_onshore_energy, new_time_index, 0);
        logger.print(LOG_VERBOSE, "new wave onshore energy: p.numgrds=%d\n", p_p->numgrds());
    }
    if (prop_indexes.iwave_longshore_energy >= 0) {
        z_indexes_p->iwave_longshore_energy = gpm_create_new_zgrid(p_p, WAVE_LONGSHORE_ENERGY, prop_indexes.iwave_longshore_energy, new_time_index, 0);
        logger.print(LOG_VERBOSE, "new wave longshore energy: p.numgrds=%d\n", p_p->numgrds());
    }
	if (prop_indexes.iwave_horizontal_diffusion_energy >= 0) {
		z_indexes_p->iwave_horizontal_diffusion_energy = gpm_create_new_zgrid(p_p, WAVE_HORIZONTAL_DIFFUSION_ENERGY, prop_indexes.iwave_horizontal_diffusion_energy, new_time_index, 0);
		logger.print(LOG_VERBOSE, "new wave horizontal diffusion energy: p.numgrds=%d\n", p_p->numgrds());
	}

    if (create_objs.make_all_surfaces) {
        const auto idx = p_p->numtims() - 1; // remove warning
        if (prop_indexes.ogs_pressure >= 0) {
            const int last_index = p_p->numtops()-1;
            const std::vector<std::string> new_grids = parm_descr_holder::grid_instance_name(
                PARM_GRID_MAP_NAMES::OGS_PRESSURE, idx, 0, last_index);
            create_grids_for_timestep(p_p, prop_indexes.ogs_pressure, idx, 0, last_index, p_p->znull, new_grids);
        }
        if (prop_indexes.ogs_velocity_x >= 0) {
            const int last_index = p_p->numtops() - 1;
            const std::vector<std::string> new_grids = parm_descr_holder::grid_instance_name(
                PARM_GRID_MAP_NAMES::OGS_VELOCITY_X, idx, 0, last_index);
            create_grids_for_timestep(p_p, prop_indexes.ogs_velocity_x, idx, 0, last_index, p_p->znull, new_grids);
        }
        if (prop_indexes.ogs_velocity_y >= 0) {
            const int last_index = p_p->numtops() - 1;
            const std::vector<std::string> new_grids = parm_descr_holder::grid_instance_name(
                PARM_GRID_MAP_NAMES::OGS_VELOCITY_Y, idx, 0, last_index);
            create_grids_for_timestep(p_p, prop_indexes.ogs_velocity_y, idx, 0, last_index, p_p->znull, new_grids);
        }
        if (prop_indexes.ogs_velocity_z >= 0) {
            const int last_index = p_p->numtops() - 1;
            const std::vector<std::string> new_grids = parm_descr_holder::grid_instance_name(
                PARM_GRID_MAP_NAMES::OGS_VELOCITY_Z, idx, 0, last_index);
            create_grids_for_timestep(p_p, prop_indexes.ogs_velocity_z, idx, 0, last_index, p_p->znull, new_grids);
        }
    }

    if (create_objs.make_all_surfaces) {
        const int last_top = create_objs.make_new_layer ? p_p->numtops() - 2 : p_p->numtops() - 1;
        gpm_grid_creation_for_surface_stack(p_p, prop_indexes.itop, last_top);
    }

    if (prop_indexes.ipor >= 0) {
        gpm_grid_creation_for_initial_porosity_and_permeabilities(p_p, create_objs.has_compaction, prop_indexes,
                                                                  p_p->numtims() - 1, p_p->numtops() - 1, logger);
    }
    if (create_objs.has_compaction ) {
        gpm_grid_creation_for_compaction(p_p, false,
                                         create_objs.make_pressure, prop_indexes,
                                         logger);
    }
    return 0; // All done
}

void gpm_grid_creation_for_surface_stack(parm* p_p, int prop_index, int last_surface_index)
{
    const std::vector<std::string> new_grids = parm_descr_holder::grid_instance_name(
        PARM_GRID_MAP_NAMES::TOP, p_p->numtims() - 1, 0, last_surface_index);
    create_grids_for_timestep(p_p, prop_index, p_p->numtims() - 1, 0, last_surface_index, p_p->znull, new_grids);
}
void gpm_grid_creation_for_surface_stack(parm* p_p, int prop_index, const std::vector<int>& surface_indexes)
{
    for(auto i:surface_indexes) {
		const std::string key = parm_descr_holder::grid_instance_name(PARM_GRID_MAP_NAMES::TOP, p_p->numtims() - 1, i);
		auto func_old=p_p->indprop(prop_index, p_p->numtims() - 1, i);
		add_zgrid_assign_to_index(p_p, key, prop_index, p_p->numtims() - 1, i);
        if (func_old.is_initialized()) {
            // Copy in old value
			auto func_new = p_p->indprop(prop_index, p_p->numtims() - 1, i);
            if (func_old.is_constant()) {
				p_p->zgrid[func_new.index_value()].fill(func_old.constant_value());
            }
			else {
				p_p->zgrid[func_new.index_value()].fill(p_p->zgrid[func_old.index_value()]);
			}
        }
    }
}

void gpm_grid_creation_for_compaction(parm* p, bool has_parrows, bool has_pressure, const indprop_indexes& prop_indexes, const gpm_logger& logger)
{
    const auto time_index = p->numtims()-1;
    const auto last_surface_index = p->numtops()-1;
	if(has_parrows){ // Create new set of velocities for this time
        const auto ngrids = p->numgrds(); // Save number of grids for debugging
        gpm_create_new_zgrid_range(p, PVX, prop_indexes.ipvx, time_index, 0, last_surface_index);
        gpm_create_new_zgrid_range(p, PVY, prop_indexes.ipvy, time_index, 0, last_surface_index);
        gpm_create_new_zgrid_range(p, PVZ, prop_indexes.ipvz, time_index, 0, last_surface_index);
		if (p->numgrds() != ngrids) {
			logger.print(LOG_VERBOSE,"findint added %d grid(s) for subsurface flow arrows, p.numgrds = %d\n", p->numgrds()-ngrids, p->numgrds());
		}
	}

	if(has_pressure){ // Create new set of pore pressures for this time
        const auto ngrids = p->numgrds();
        gpm_create_new_zgrid_range(p, PRS, prop_indexes.iprs, time_index, 0, last_surface_index);
		if (p->numgrds() != ngrids) {
			logger.print(LOG_VERBOSE,"findint added %d grid(s) for pore pressure, p.numgrds = %d\n", p->numgrds()-ngrids, p->numgrds());
		}
	}
}

void gpm_grid_creation_for_initial_porosity_and_permeabilities(parm* p, bool has_compaction, const indprop_indexes& prop_indexes, int time_index, int surface_index, const gpm_logger& logger)
{
    gpm_grid_creation_for_initial_porosity(p, has_compaction, prop_indexes, time_index, surface_index, logger);
    gpm_grid_creation_for_initial_permeabilities(p, has_compaction, prop_indexes, time_index, surface_index, logger);
}

void gpm_grid_creation_for_initial_permeabilities(parm* p, bool has_compaction, const indprop_indexes& prop_indexes, int time_index, int surface_index, const gpm_logger& logger)
{
    const auto first_surface_index = has_compaction ? 0 : surface_index; // Do all or one depending on cmpaction is on
    const auto ngrids3 = p->numgrds();
    gpm_create_new_zgrid_range(p, PERMEABILITY_HORI, prop_indexes.iperm_hor, time_index, first_surface_index, surface_index);
    if (p->numgrds() != ngrids3) {
        logger.print(LOG_VERBOSE, "findint added %d grid(s) for horizontal permeability, p.numgrds = %d\n", p->numgrds() - ngrids3, p->numgrds());
    }
    const auto ngrids2 = p->numgrds();
    gpm_create_new_zgrid_range(p, PERMEABILITY_VERT, prop_indexes.iperm_vert, time_index, first_surface_index, surface_index);
    if (p->numgrds() != ngrids2) {
        logger.print(LOG_VERBOSE, "findint added %d grid(s) for vertical permeability, p.numgrds = %d\n", p->numgrds() - ngrids2, p->numgrds());
    }
}

void gpm_grid_creation_for_ogs_pressure(parm* p, const indprop_indexes& prop_indexes, int time_index, int surface_index, const gpm_logger& logger)
{
    return;
    const auto ngrids = p->numgrds(); // Save number of grids for debugging
    const auto first_surface_index = 0; // Do all or one depending on compaction is on
    gpm_create_new_zgrid_range(p, OGS_PRESSURE, prop_indexes.ogs_pressure, time_index, first_surface_index, surface_index);
    if (p->numgrds() != ngrids) {
        logger.print(LOG_VERBOSE, "findint added %d grid(s) for ogs pressure, p.numgrds = %d\n", p->numgrds() - ngrids, p->numgrds());
    }
}


void gpm_grid_creation_for_initial_porosity (parm* p, bool has_compaction, const indprop_indexes& prop_indexes, int time_index, int surface_index, const gpm_logger& logger)
{
    const auto ngrids = p->numgrds(); // Save number of grids for debugging
    const auto first_surface_index = has_compaction ? 0 : surface_index; // Do all or one depending on compaction is on
    gpm_create_new_zgrid_range(p, POR, prop_indexes.ipor, time_index, first_surface_index, surface_index);
    if (p->numgrds() != ngrids) {
        logger.print(LOG_VERBOSE, "findint added %d grid(s) for porosity, p.numgrds = %d\n", p->numgrds() - ngrids, p->numgrds());
    }
    // Assign basement porosity unknown (symbollically 1.0)
}
int gpm_find_last_indprop_index(const parm& p, int time_index)
{
	// Find the highest index in indprop for the current time
	auto ind=std::ptrdiff_t(-1);
	if ( time_index < p.indprop.rows()){
		for(auto layer = 0u; layer < p.indprop.layers(); ++layer)
		{
			for(auto col = 0u; col < p.indprop.columns();++col)
			{
				if (!p.indprop(layer,time_index,col).is_constant()&& p.indprop(layer,time_index,col).is_initialized())
				{
					ind = std::max(ind, p.indprop(layer,time_index,col).index_value());
				}
			}
		}
	}
	return ind;
}
}}}
