// -- Schlumberger Private --

#ifndef gpm_h5_reader_v1_h
#define gpm_h5_reader_v1_h

#include "gpm_h5cpp_wrapper.h"
#include "acf_base_reader.h"
#include "mapstc_descr.h"
#include "gpm_lin_span.h"
#include <memory>

namespace Slb { namespace Exploration { namespace Gpm {
/*
 * Abstract class to encapsulate H5 Read API, for version interdependent logic
 */
class gpm_h5_reader_impl {

public:
	explicit gpm_h5_reader_impl(std::shared_ptr<gpm_h5cpp_wrapper> file_p);
    gpm_h5_reader_impl(const gpm_h5_reader_impl& rhs) = delete;
    gpm_h5_reader_impl& operator=(const gpm_h5_reader_impl& rhs) = delete;
    virtual ~gpm_h5_reader_impl();

    virtual void get_timesteps(lin_span<double>* timesteps) const;
	virtual int size_timesteps() const;

	virtual void get_ages(lin_span<double>* ages) const;
	virtual int  size_ages() const;

	void get_world_corners(lin_span<float>* x_values, lin_span<float>* y_values) const;
	void get_model_size(int* num_rows, int* num_cols) const;
	float get_base_erodability() const;
	float get_novalue() const;
	void get_isnodal(lin_span<int>* nodal) const;
	int size_isnodal() const;

	int size_sediment_info_in_model() const;
	void get_sediment_info_in_model(std::vector<std::string>* sed_ids, std::vector<std::string>* sed_names, std::vector<std::string>* sed_prop) const;
	int size_sediment_info_in_baselayer() const;
	void get_sediment_ids_in_baselayer(std::vector<std::string>* sed_ids) const;
    	
    virtual int size_property(const int time_index, const std::string& prop_id) const=0;
	int size_property_display_names() const;

	virtual void get_property(const std::string& prop_id, int time_index, std::vector<lin_span<float>>* surfaces, std::vector<int>* diagonal_indexes);
	virtual void get_property_at_index(const std::string& property_id, const int& time_index, const int& surface_index, lin_span<float>* surface)=0;
	virtual bool is_surface_constant(const std::string& prop_id, const int& time_index, const int& surface_index) = 0;
	virtual bool is_surface_defined(const std::string& prop_id, const int& time_index, const int& surface_index) = 0;
	
	virtual void get_property_statistics(const std::string& prop_id, int time_index, float& min, float& max);
    void get_property_display_names(std::vector<std::string>* ids, std::vector<std::string>* names) const;
	
	void get_property_ids(std::vector<std::string>* property_ids);
	int size_property_ids() const;
	virtual int get_num_grid_names() const;
	virtual void get_grid_names(std::vector<std::string>* grid_names) const;
	virtual void get_grid(int grid_index, lin_span<float>* z_nodes) const;
	virtual void close() const;
    virtual bool initialize() = 0;

	std::string validate_property_id(const std::string& property_id) const;
	std::string validate_time_index(const int& time_index) const;
	std::string validate_surface_index(const int& surface_index) const;

protected:
	void get_property_statistics(const std::shared_ptr<gpm_h5cpp_wrapper>& file_ptr, const std::string& prop_id, int time_index, float& min, float& max);
	virtual int get_num_tops();
	int get_num_tops(const std::shared_ptr<gpm_h5cpp_wrapper>& file_ptr) const;
    using index_type = acf_base_reader::index_type;
    std::shared_ptr<gpm_h5cpp_wrapper> m_file_p;
    std::vector<hsize_t> get_dataset_dims(const std::string& dataset_name) const;
    int get_num_1d_array(const std::string& dataset_name) const;
    std::map<std::string, int> m_indprop_map;
    std::map<std::string, std::vector<std::vector<float>>> m_indprop_stats_cache;
	std::vector<index_type> m_diagonal_indexes;
	int m_num_tops;
    void get_property_values(int time_index, const std::string& prop_id, std::vector<lin_span<float>>* surfaces);
	void calculate_statistics(int time_index, const std::string& prop_id, float& min, float& max);
	int get_property_index(const std::string& prop_id) const;
	static std::vector<hobj_ref_t> get_props_references(const std::shared_ptr<gpm_h5cpp_wrapper>& file_ptr);
};

}}}

#endif
