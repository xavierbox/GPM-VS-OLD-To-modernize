// -- Schlumberger Private --

#ifndef GPM_INDPROP_INDEXES_H
#define GPM_INDPROP_INDEXES_H

#include "mapstc_descr.h"
#include "gpm_basic_defs.h"
#include <map>
#include <vector>
#include <string>
#include <functional>

namespace Slb { namespace Exploration { namespace Gpm {
// Get names in scope
// These indexes are used for possibly pointing to 
// zgrid instances or indprop indexes
class parm_base_indexes {
public:
	using index_type = int;
	index_type itop;
    std::vector<index_type> sed_indexes;
	std::vector<index_type> erodability;
    std::vector<index_type> insitu_growth;
    std::vector<index_type> residence_time;
    std::vector<index_type> residence_def_time;
    std::vector<index_type> residence_surf_time;
    std::vector<index_type> wave_induced_zone_flow_time;
    std::vector<index_type> wave_induced_zone_surface_time;
    std::vector<index_type> residence_lattoral;
    std::vector<index_type> diagenesis;

    parm_base_indexes();
};

// Here we map in the scalar outputs
struct parm_scalar_output {
	using index_type = int;
	index_type prop_id;
	std::string name;
	std::string display_name;
	bool is_nodal{false};
	index_type zgrid_id;
	std::function<void(float, float_2darray_base_type*)> func;
};

// Here we map in the vector outputs
struct parm_vector_output {
	using index_type = int;
	index_type prop_x_id;
	index_type prop_y_id;
	std::string name_x;
	std::string name_y;
	std::string display_name_x;
	std::string display_name_y;
	bool is_nodal{ false };
	index_type zgrid_x_id;
	index_type zgrid_y_id;
	std::function<void(float, float_2darray_base_type*, float_2darray_base_type*)> func;
};


class parm_indexes: public parm_base_indexes {
public:
    index_type iscl;
    index_type isvx_obsolete, isvy_obsolete; // 2d velocities
    index_type ipvx, ipvy, ipvz; // 3d velocities
    index_type ipor; // Index of porosity
    index_type iperm_hor; // Horizontal permeability
    index_type iperm_vert; // Vertical permeability
    index_type iprs; //Index of pressure
    index_type idepodep; // Index of depositional depth JT
    index_type ierosion; // Index of erosion surface in indprop
    index_type ilithology; // Index of litho in indprop
    index_type itop_slope_x;
    index_type itop_slope_y;
    index_type isealevel; // Sealev surface
	std::vector<parm_scalar_output> scalar_outputs;
	std::vector<parm_vector_output> vector_outputs;
    index_type irel_sealevel_change_rate;
    index_type idiffusion_energy;
    index_type isteady_flow_energy;
    index_type iunsteady_flow_energy;
    index_type iwave_dissipation_energy;
    index_type iwave_onshore_energy;
    index_type iwave_longshore_energy;
	index_type iwave_horizontal_diffusion_energy;
    index_type ogs_pressure;
    index_type ogs_velocity_x;
    index_type ogs_velocity_y;
    index_type ogs_velocity_z;
    parm_indexes();
};

// Almost just a tagging interface
class zgrid_indexes: public parm_indexes {
public:
    zgrid_indexes();
};

class indprop_indexes: public parm_indexes {
public:
    indprop_indexes();
    static int find_index(const std::vector<std::string>& index_names, const std::string& name);
    void set_residence_time_indexes(std::vector<std::string>& prop_mapper, const std::vector<std::string>& index_names);
    void set_residence_time_def_indexes(std::vector<std::string>& prop_mapper, const std::vector<std::string>& index_names);
    void set_wave_induced_zone_flow_indexes(std::vector<std::string>& prop_mapper, const std::vector<std::string>& index_names);
    void set_wave_induced_zone_surface_indexes(std::vector<std::string>& prop_mapper, const std::vector<std::string>& index_names);
    void set_lattoral_residence_indexes(std::vector<std::string>& prop_mapper, const std::vector<std::string>& index_names);
    void set_residence_time_surf_indexes(std::vector<std::string>& prop_mapper, const std::vector<std::string>& index_names);
    void set_diagenesis_indexes(std::vector<std::string>& prop_mapper, const std::vector<std::string>& index_names);
    void set_insitu_growth_indexes(std::vector<std::string>& prop_mapper, const std::vector<std::string>& index_names);
    // Will setup the indexes according to the map and lists
    void set_indexes(const std::map<PARM_GRID_MAP_NAMES, std::string>& prop_mapper, const std::vector<std::string>& index_names);
	void set_indexes(const std::vector<parm_scalar_output>& scalars, const std::vector<parm_vector_output>& vectors, const std::vector<std::string>& index_names);
};

class grid_creation_options {
public:
    grid_creation_options();
    grid_creation_options(bool make_layer, bool make_sediments, bool make_surfaces, bool compaction, bool pressure);
    bool make_new_layer=false;
    bool make_new_sediments=false;
    bool make_all_surfaces=false;
    bool has_compaction = false;
	bool make_pressure = false;
};

}}}
#endif
