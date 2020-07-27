#ifndef GPM_MODEL_READER_API_H
#define GPM_MODEL_READER_API_H
#include "gpm_lin_span.h"
#include <vector>
#include <memory>
#include <string>

namespace Slb { namespace Exploration { namespace Gpm {

class gpm_model_reader {
public:
    /*
     * Fills pre-allocated timesteps vector with timestep values. Vector should be of correct size.
     */
    void get_timesteps(lin_span<double>* timesteps) const;
    /*
     * Returns size of timesteps vector
     */
    int  get_num_timesteps() const;
	/*
	 * Fills pre-allocated ages vector with ages values. Vector should be of correct size.
	 */
    void get_ages(lin_span<double>* ages) const;
	/*
	 * Returns size of ages vector
	 */
    int  get_num_ages() const;
	/*
	 * Returns corners points coordinates using a counter clock-wise order.
	 */
    void get_world_corners(lin_span<float>* x_values, lin_span<float>* y_values) const;
	/*
	 * Returns surfaces size, rows first.
	 */
    void get_model_size(int* num_rows, int* num_cols) const;
	/*
	 * Returns base erodability in a model
	 */
	float get_base_erodability() const;
	/*
	 * Returns number of sediments in a model
	 */
	int get_num_sediment_info_in_model() const;
	/*
	 * Fills pre-allocated sediments vectors with data.
	 */
	void get_sediment_info_in_model(std::vector<std::string>* sed_ids, std::vector<std::string>* sed_names, std::vector<std::string>* sed_prop) const;	
	/*
	 * Returns number of sediments in a base layer
	 */
	int get_num_sediment_ids_in_baselayer() const;
	/*
	 * Fills pre-allocated sediments vectors with data.
	 */
	void get_sediment_ids_in_baselayer(std::vector<std::string>* sed_ids) const;
	/*
	 * Returns no value
	 */
    float get_novalue() const;
	/*
	 * Fills pre-allocated surfaces vector with values for a given timestep index
	 */
    void get_geometry(const int& time_index, std::vector<lin_span<float>>* surfaces) const;
    /*
     * Returns number of geometry surfaces at a given timestep index
     */
    int get_num_geometry(const int& time_index) const;
    /*
     * Returns geometry statistics for a given timestep index
     */
	void get_geometry_statistics(const int& time_index, float& min, float& max) const;
    /*
     * Fills pre-allocated vector of property ids with values
     */
    void get_property_ids(std::vector<std::string>* property_ids) const;
    /*
     * Returns property ids vector size
     */
	int get_num_property_ids() const;
    /*
     * Fills pre-allocated vector of property names with values
     */
	void get_property_display_names(std::vector<std::string>* ids, std::vector<std::string>* names) const;
	/*
	 * Returns property names vector size
	 */
	int get_num_property_display_names() const;
	/*
     * Fills pre-allocated vector of property surfaces with values at a given timestep. diagonal_indexes vector will be populated with diagonal indexes for each surface.
     */
	void get_property(const std::string& prop_id, const int& time_index, std::vector<lin_span<float>>* surfaces, std::vector<int>* diagonal_indexes) const;

	bool is_surface_constant(const std::string& prop_id, const int& time_index, const int& surface_index) const;

	bool is_surface_defined(const std::string& prop_id, const int& time_index, const int& surface_index) const;

	/*
	 * Validate prop_id 
	*/
	std::string validate_property_id(const std::string& prop_id) const;
	/*
	 * Validate time_index
	*/
	std::string validate_time_index(const int& time_index) const;
	/*
	 * Validate surface_index
	*/
	std::string validate_surface_index(const int& surface_index) const;
	/*
	 * Returns number of surfaces for a given property id at a given timestep index
	 */
    int get_num_property(const std::string& prop_id, const int& time_index) const;
    /*
     * Returns property statistics for a given timestep index
     */
	void get_property_statistics(const std::string& prop_id, const int& time_index, float& min, float& max) const;
	/*
	 * Fills pre-allocated vector of property surface values
	 */
    void get_property_at_index(const int& time_index, const int& surface_index, const std::string& prop_id, lin_span<float>* surface) const;
	/*
	 * Fills pre-allocated vector of property surfaces with values at a given time and surface indexes
	 */
    void get_all_timesteps_surfaces_at_index(const int& surface_index, const std::string& prop_id, std::vector<lin_span<float>>* surfaces) const;
	/*
	 * Fills pre-allocated vector of property top surfaces with values
	 */
    void get_all_timesteps_top_surfaces(const std::string& prop_id, std::vector<lin_span<float>>* surfaces) const;
	/*
	 * Fills pre-allocated top surfaces of a given property at a given time index
	 */
    void get_top_surface(const int& time_index, const std::string& prop_id, lin_span<float>* surface) const;
	/*
	 * Returns nodal vector size
	 */
    int get_num_isnodal() const;
	/*
	 * Fills pre-allocated nodal vector with values
	 */
	void get_isnodal(lin_span<int>* nodal) const;
	/*
	 * Returns number of grid names
	 */
	int get_num_grid_names() const;
	/*
    * Fills pre-allocated grid names vector
    */
	void get_grid_names(std::vector<std::string>* grid_names) const;
	/*
	* Fills pre-allocated grid vector with values
	*/
	void get_grid(const int grid_index, lin_span<float>* z_nodes) const;

	gpm_model_reader();
    gpm_model_reader(const gpm_model_reader& rhs)=delete;
    gpm_model_reader& operator=(const gpm_model_reader& rhs)=delete;
    ~gpm_model_reader();

    void close() const;
	bool open(const std::string& filename) const;
	bool open(const std::string& main_file, const int last_increment_index) const;

private:   
    struct api_impl;
	std::unique_ptr<api_impl> m_impl;
};
}}}
#endif
