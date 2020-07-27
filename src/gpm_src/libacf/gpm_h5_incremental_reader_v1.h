// -- Schlumberger Private --

#ifndef gpm_h5_reader_impl_incr_mode_h
#define gpm_h5_reader_impl_incr_mode_h

#include <memory>
#include "gpm_h5_reader_impl_v1.h"

namespace Slb { namespace Exploration { namespace Gpm {

class gpm_h5_incremental_reader_v1 final : public gpm_h5_reader_impl_v1 {

public:
	explicit gpm_h5_incremental_reader_v1(const std::shared_ptr<gpm_h5cpp_wrapper>& file_ptr, 
		                                  const std::string& main_file_name, 
		                                  const int last_time_index);
	gpm_h5_incremental_reader_v1(const gpm_h5_incremental_reader_v1& rhs) = delete;
	gpm_h5_incremental_reader_v1& operator=(const gpm_h5_incremental_reader_v1& rhs) = delete;
	~gpm_h5_incremental_reader_v1() override = default;

	void get_timesteps(lin_span<double>* timesteps) const override;
	int size_timesteps() const override;
	void get_ages(lin_span<double>* ages) const override;
	int size_ages() const override;
	void get_property_statistics(const std::string& prop_id, int time_index, float& min, float& max) override;
	void close() const override;

protected:
	bool initialize() override;
	std::vector<std::string> get_property_map_values_at(const int property_index, const int time_index) const override;
	int get_num_tops() override;

private:
	std::shared_ptr<gpm_h5cpp_wrapper> open_incremental_file(const int& time_index) const;
	std::vector<float> transform_map_to_grid(const std::string& property_map) override;
	int find_increment_index(const int absolute_grid_index) const;
	void fetch_times_and_ages(const std::shared_ptr<gpm_h5cpp_wrapper>& file_ptr);
	std::vector<double> get_type_descr_values(const std::shared_ptr<gpm_h5cpp_wrapper>& file_ptr, const TypeDescr& type_descr) const;

    struct gpm_indprop_map_indexes {
		int m_start_index;
		int m_end_index;
	};

	std::map<int, gpm_indprop_map_indexes> m_indpropmap_indexes;
	std::map<int, std::shared_ptr<gpm_h5cpp_wrapper>> m_files;
	std::vector<double> m_timesteps;
	std::vector<double> m_ages;

	std::string m_main_file_name;
	int m_last_time_index;
}; 
}}}

#endif