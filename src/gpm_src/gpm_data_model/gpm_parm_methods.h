// -- Schlumberger Private --

#ifndef GPM_PARM_METHODS_H
#define GPM_PARM_METHODS_H

#include "mapstc.h"
#include "parm_prop_descr.h"
#include "gpm_indprop_indexes.h"
#include "gpm_function_2d.h"
#include "gpm_logger.h"
#include <map>
#include <string>
#include <vector>

// This is where we add the methods that operate on struct parm
// That way it may become a bit more clear that parm is a class that should be like that
// In essence there are two major data structures in parm
//  - zgrid, which keeps all the arrays currently in the model, in
//        essence it models a vector of 2d arrays that hold a model property for parm
//  - indprop,  which is a vector of 2d arrays where the size of the 2d array is the number of surfaces in the model (number of columns
//        and the number of rows are the number of ages or times in the mode
//        The items in a 2d array have pointers to an index in zgrid, or a constant or an empty value
//  - gridnam has as many name entires as zgrid has 2d arrays, so the sixe of gridman should be the same as layers of z grid
//  - propnam, similar to gridnam, but is for the properties in the model

// These methods are spread across different files to 
namespace Slb {namespace Exploration {namespace Gpm {

// Index between the property it represents
// and the property in indprop
struct property_range_item {
    int range_index;
    int indprop_index;
};

	// Helpers
inline bool is_range_sorted(const property_range_item& a, const property_range_item& b) { return a.range_index < b.range_index; }
inline bool is_indprop_sorted(const property_range_item& a, const property_range_item& b) { return a.indprop_index < b.indprop_index; }
inline bool is_range_equal(const property_range_item& a, const property_range_item& b) { return a.range_index == b.range_index; }
inline bool is_indprop_equal(const property_range_item& a, const property_range_item& b) { return a.indprop_index == b.indprop_index; }
inline int get_range(const property_range_item& a) { return a.range_index; }
inline int get_indprop(const property_range_item& a) { return a.indprop_index; }


bool has_property(const parm& p, const std::string& name);
int get_property_index(const parm& p, const std::string& name);
std::map<std::string, int> get_property_index_map(const parm& p);

std::vector<int> find_properties_with_name_prefix(const parm& p, const std::string& name);

int find_or_add_property(parm* p,
                         const std::string& name, const std::string& display_name, int is_nodal, int* indprop,
                         bool* isnew);

void find_arrays_for_timestep(parm* p, int prop_id,
                              int time_step, int from_top, int to_top,
                              std::vector<float_2darray_base_type*>* functions_p,
                              const Slb::Exploration::Gpm::Tools::gpm_logger& logger);

void find_arrays_for_timestep(parm* p, const std::string& id,
                              int time_step, int from_top, int to_top,
                              std::vector<float_2darray_base_type*>* functions_p,
                              const Slb::Exploration::Gpm::Tools::gpm_logger& logger);

int find_functions_for_timestep(const parm& p, int prop_id,
                                int time_step, int from_top, int to_top,
                                std::vector<function_2d>* functions_p,
                                const Tools::gpm_logger& logger);

void is_array_unique_for_timestep(const parm& p, int prop_id,
                                  parm::index_type time_step, parm::index_type from_top, parm::index_type to_top,
                                  std::vector<bool>* array_p);

std::vector<int> find_non_unique_arrays_for_timestep(const parm& p, int prop_id,
	parm::index_type time_step, parm::index_type from_top, parm::index_type to_top);

int gpm_find_sediment_functions_for_timestep(const parm& p,
                                             const std::vector<int>& sed_indexes,
                                             int timestep, int from_top, int to_top,
                                             std::vector<std::vector<function_2d>>* sed_functions_p);

void gpm_find_sediment_arrays_for_timestep(parm* p_p,
                                           const std::vector<int>& sed_indexes,
                                           int timestep, int from_top, int to_top,
                                           std::vector<std::vector<float_2darray_base_type*>>* sed_functions_p,
                                           const Tools::gpm_logger& logger);

int find_base_functions_for_timestep(const parm& p,
                                     int top_index, const std::vector<int>& sed_indexes,
                                     int timestep, int from_top, int to_top,
                                     std::vector<function_2d>* top_functions_p,
                                     std::vector<std::vector<function_2d>>* sed_functions_p);

int gpm_find_residence_time_property_indexes(const parm& p,
                                             std::vector<property_range_item>* restime_p,
                                             const Tools::gpm_logger& logger);

int gpm_find_diagenesis_property_indexes(const parm& p,
                                         std::vector<property_range_item>* restime_p,
                                         const Tools::gpm_logger& logger);

int gpm_find_insitu_growth_property_indexes(const parm& p,
                                            std::vector<property_range_item>* indsed_p,
                                            const Tools::gpm_logger& logger);

std::vector<std::pair<int, property_description>> gpm_find_erodability_property_ids(
    const std::map<std::string, int>& sed_indexes,
    const std::vector<std::string>& sed_names);


int gpm_find_sediment_property_indexes(const std::map<std::string, int>& sed_indexes,
                                       const std::map<std::string, std::pair<std::string, int>>& model_sed_indexes,
                                       std::vector<property_range_item>* indsed_p,
                                       const Slb::Exploration::Gpm::Tools::gpm_logger& logger);

int find_grid_functions_for_timestep(parm* p, parm::index_type prop_index, parm::index_type time_index, parm::index_type ito_top,
                                     float_2darray_base_type** prop_p);
int find_grid_functions_for_timestep(const parm& p, parm::index_type prop_index, parm::index_type time_index, parm::index_type ito_top,
                                     function_2d* prop_p);

int find_grid_functions_for_timestep(parm* p, parm::index_type prop_index, parm::index_type time_index, parm::index_type ifrom_top, parm::index_type ito_top,
                                     std::vector<float_2darray_base_type*>* prop_p);
int find_grid_functions_for_timestep(const parm& p, parm::index_type prop_index, parm::index_type time_index, parm::index_type ifrom_top, parm::index_type
                                     ito_top,
                                     std::vector<function_2d>* prop_p);

int find_grid_functions_for_surface_index(const parm& p, parm::index_type prop_index, parm::index_type ifrom_time, parm::index_type ito_time,
                                          parm::index_type surface_index, std::vector<function_2d>* prop_p);

bool has_allocated_arrays_for_timestep(const parm& p, parm::index_type prop_index, parm::index_type time_index);

// Find the diagonal, can use it to reorder the other props if needed
int find_grid_diagonal_indexes(const parm& p, parm::index_type prop_index, std::vector<int>* prop_p);

// Create and find instead of add_grids_to_timestep
void create_grids_for_timestep(parm* p, parm::index_type iprop, parm::index_type itim, parm::index_type ifm, parm::index_type ito, float init_val,
                               const std::vector<std::string>& names);

void assign_zgrid_to_indprop(parm* p);

int gpm_map_struct_property_check(parm* p_p, double time_begin, const Tools::gpm_logger& logger);
void gpm_check_erodability_properties(parm* p_p, const std::vector<float>& erodibility, const std::vector<property_range_item>& needs, int last_index);

std::vector<int> gpm_add_needed_properties(parm* p_p, const std::vector<property_description>& needs,
                                           std::map<std::string, bool>* created,
                                           const Tools::gpm_logger& /*logger*/);
void find_source_grid(const parm& p, int_2darray_type& srcgrid, const Tools::gpm_logger& logger);
void calculate_statistics(parm* p_p, int time_step,  const Tools::gpm_logger& logger);
void free_memory_from_previous_timestep(parm* p_p, const Tools::gpm_logger& logger ,long curr_ts=0);
void preserve_only_last_timestep(parm* p_p, const Tools::gpm_logger& logger);

// Assume index equality
void gpm_set_erodability_factors(gpm_array_3d_vec_base<function_2d>* indprop_p,
                                 const std::vector<int>& erod_prop_indexes,
                                 int time_index, int age_index,
                                 const std::vector<float>& erodabilities);

int gpm_create_new_zgrid(parm* p_p, PARM_GRID_MAP_NAMES id, int prop_id, int time_index, int surface_index);


void gpm_grid_creation_for_surface_stack(parm* p_p, int prop_index, int last_surface_index);
void gpm_grid_creation_for_surface_stack(parm* p_p, int prop_index, const std::vector<int>& surface_indexes);

int gpm_find_last_indprop_index(const parm& p, int time_index);
// Will copy data from the prev timestep to to_time
int gpm_copy_previous_timestep(gpm_array_3d_vec_base<function_2d>& indprop, int to_time, int from_time, int num_tops,
                               const std::vector<int>& indexes);
int gpm_copy_property_previous_timestep(gpm_array_3d_vec_base<function_2d>& indprop, int to_time, int from_time,
                                        int num_tops, int indexes);
// Will copy data from the prev top to to_top
int gpm_copy_previous_tops_for_timesteps(gpm_array_3d_vec_base<function_2d>& indprop, int to_top, int from_top,
                                         int num_times, const std::vector<int>& indexes);
std::map<std::string, std::pair<std::string, int>> gpm_find_sediment_id_propnam_map(const parm& p);
std::map<std::string, std::pair<std::string, int>> gpm_find_sediment_id_prop_display_name_map(const parm& p);

int gpm_check_propnam_index_items(const parm& p, const std::vector<std::string>& prefixes, const Tools::gpm_logger& logger);

void gpm_create_residence_time_grids(parm* p_p, const indprop_indexes& prop_indexes, int new_time_index,
    int first_surface_index, long last_surface_index);
void gpm_create_residence_time_def_grids(parm* p_p, const indprop_indexes& prop_indexes, int new_time_index);
void gpm_create_residence_time_surf_grids(parm* p_p, const indprop_indexes& prop_indexes, int new_time_index);
void gpm_create_wave_induced_zone_flow_grids(parm* p_p, const indprop_indexes& prop_indexes, int new_time_index, int first_surface_index = 0, int last_surface_index = 0);
void gpm_create_wave_induced_zone_surface_grids(parm* p_p, const indprop_indexes& prop_indexes, int new_time_index);
void gpm_create_residence_lattoral_grids(parm* p_p, const indprop_indexes& prop_indexes, int new_time_index);
void gpm_create_diagenesis_grids(parm* p_p, const indprop_indexes& prop_indexes, int new_time_index,
    int first_surface_index, long last_surface_index);

int gpm_grid_create_new_timestep(parm* p_p, const indprop_indexes& prop_indexes, const grid_creation_options& create_objs,
    zgrid_indexes* z_indexes_p, const Tools::gpm_logger& logger);
void gpm_grid_creation_for_compaction(parm* p_p, bool has_parrows, bool has_pressure,
    const indprop_indexes& prop_indexes, const Tools::gpm_logger& logger);
void gpm_grid_creation_for_initial_porosity_and_permeabilities(parm* p_p, bool has_porosity,
    const indprop_indexes& prop_indexes, int time_index,
    int surface_index, const Tools::gpm_logger& logger);
void gpm_grid_creation_for_initial_permeabilities(parm* p_p, bool has_porosity, const indprop_indexes& prop_indexes,
    int time_index, int surface_index, const Tools::gpm_logger& logger);
void gpm_grid_creation_for_initial_porosity(parm* p_p, bool has_porosity, const indprop_indexes& prop_indexes,
    int time_index, int surface_index, const Tools::gpm_logger& logger);
void gpm_grid_creation_for_ogs_pressure(parm* p_p, const indprop_indexes& prop_indexes, int time_index, int surface_index, const Tools::gpm_logger& logger);
}}}
#endif
