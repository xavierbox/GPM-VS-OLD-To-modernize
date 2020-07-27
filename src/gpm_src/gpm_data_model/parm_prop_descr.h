// -- Schlumberger Private --

#ifndef parm_prop_descr_h
#define parm_prop_descr_h

#include "mapstc.h"
#include <string>
#include <vector>
#include <map>

namespace Slb { namespace Exploration { namespace Gpm{

// Include the enum defs
#include "parm_prop_enum_def.h"

// Our stuff
class parm_descr_holder{
public:
    static std::string get_grid_map_name(PARM_GRID_MAP_NAMES id);
    static std::string get_current_version();
    static const std::map<PARM_GRID_MAP_NAMES, std::string>& get_grid_map_names();
    static const std::map<PARM_GRID_VECTOR_2D_NAMES, std::pair<PARM_GRID_MAP_NAMES, PARM_GRID_MAP_NAMES> >& get_grid_map_vector_2d();
    static const std::map<PARM_GRID_VECTOR_2D_NAMES, std::string >& get_grid_map_vector_2d_names();
    static const std::vector<PARM_GRID_MAP_NAMES>& get_grid_map_time_indexes();
    static std::string get_grid_map_1_index_name_prefix(PARM_GRID_MAP_1_INDEX_NAMES id);

	static const std::string& get_sediment_property_prefix();
	static const std::string& get_insitu_growth_property_prefix();
	static const std::string& get_residence_time_property_prefix();
	static const std::string& get_residence_time_surface_property_prefix();
	static const std::string& get_deposition_contribution_property_prefix();
	static const std::string& get_diagenesis_property_prefix();
	static const std::string& get_erodability_property_prefix();

    static const std::map<PARM_GRID_MAP_1_INDEX_NAMES, std::string>& get_grid_map_1_index_name_prefixes();
    static std::vector<std::string> get_grid_map_1_index_name_prefixes_as_strings();
    static const std::vector<PARM_GRID_MAP_NAMES>& get_grid_map_top_layer_only_properties();
    static const std::map<PARM_DEPOSITIONAL_ENERGY_PROCESS_NAMES, std::string>& get_process_map_depo_energies();

    static std::string grid_property_postfix_index_name( PARM_GRID_MAP_1_INDEX_NAMES id, int sed);
    static std::string grid_instance_postfix_name(PARM_GRID_MAP_1_INDEX_NAMES id, int index);
    static std::string grid_instance_postfix_name(PARM_GRID_MAP_1_INDEX_NAMES id, int index, int cycle, int top);
    static std::string grid_instance_postfix_name(PARM_GRID_MAP_1_INDEX_NAMES id, int index, int time_index);
	static std::string grid_property_postfix_index_name(const std::string& id, int sed);
	static std::string grid_instance_postfix_name(const std::string& id, int index);
	static std::vector<std::string> grid_instance_postfix_name(const std::string& id, int index, int time_index, int first_surface_index, long last_surface_index);
	static std::string grid_instance_postfix_name(const std::string& id, int index, int cycle, int top);
	static std::string grid_instance_postfix_name(const std::string& id, int index, int time_index);
	static std::string grid_instance_name(const std::string& id);
    static std::string grid_instance_name(PARM_GRID_MAP_NAMES id);
    static std::vector<std::string> grid_instance_name(PARM_GRID_MAP_NAMES id, int time_index, int first_surface_index, long last_surface_index);
    static std::string grid_instance_name(PARM_GRID_MAP_NAMES id, int cycle, int top);
    static std::string grid_instance_name(const std::string& id, int time_index) ;
    static std::string grid_instance_name(PARM_GRID_MAP_NAMES id, int time_index);
	static property_description make_property_description(PARM_GRID_MAP_NAMES id, const std::string& display_name, bool nodal);
};

}}}

#endif
