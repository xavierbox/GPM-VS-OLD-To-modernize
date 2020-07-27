// -- Schlumberger Private --

#include "gpm_h5_incremental_reader_v1.h"
#include <memory>
#include "gpm_format_helper.h"
#include "gpm_h5_reader_impl_v1.h"

namespace Slb {
	namespace Exploration {
		namespace Gpm {

			gpm_h5_incremental_reader_v1::gpm_h5_incremental_reader_v1(const std::shared_ptr<gpm_h5cpp_wrapper>& file_ptr,
				                                                       const std::string& main_file_name,
				                                                       const int last_time_index) : gpm_h5_reader_impl_v1(file_ptr) {
				m_main_file_name = main_file_name;
				m_last_time_index = last_time_index;
			}

			void gpm_h5_incremental_reader_v1::get_timesteps(lin_span<double>* timesteps) const {
				std::copy(m_timesteps.begin(), m_timesteps.end(), timesteps->begin());
			}

			int gpm_h5_incremental_reader_v1::size_timesteps() const {
				return m_timesteps.size();
			}

			void gpm_h5_incremental_reader_v1::get_ages(lin_span<double>* ages) const {
				std::copy(m_ages.begin(), m_ages.end(), ages->begin());
			}

			int gpm_h5_incremental_reader_v1::size_ages() const {
				return m_ages.size();
			}			

			void gpm_h5_incremental_reader_v1::get_property_statistics(const std::string& prop_id, int time_index, float& min, float& max) {
				const auto incr_file_p = m_files[time_index];
				return gpm_h5_reader_impl::get_property_statistics(incr_file_p, prop_id, 0, min, max);
			}

			void gpm_h5_incremental_reader_v1::close() const
			{
				gpm_h5_reader_impl_v1::close();
				for (auto const& x : m_files)
				{
					x.second->close_file();
				}
			}

			std::shared_ptr<gpm_h5cpp_wrapper> gpm_h5_incremental_reader_v1::open_incremental_file(const int& time_index) const {
				const auto incremental_file_name = gpm_format_helper::get_incremental_file_name(m_main_file_name, time_index);
				const auto incremental_file_ptr = std::make_shared<gpm_h5cpp_wrapper>();
				if (incremental_file_ptr->open_file(incremental_file_name, false, false)) {
					return incremental_file_ptr;
				}
				return nullptr;
			}			

			bool gpm_h5_incremental_reader_v1::initialize() {
				const auto start_index = parm_type_descr_holder::get_parm_description(GREF_START_INDEX);
				const auto end_index = parm_type_descr_holder::get_parm_description(GREF_END_INDEX);
				auto start = m_file_p->read_scalar_attribute<int>(start_index.name);
				auto end = m_file_p->read_scalar_attribute<int>(end_index.name);
				m_indpropmap_indexes[0] = gpm_indprop_map_indexes{ start, end };
				m_files[0] = m_file_p;

				fetch_times_and_ages(m_file_p);
				for (auto increment = 1; increment <= m_last_time_index; increment++) {
					auto file_ptr = open_incremental_file(increment);
					if (file_ptr == nullptr) {
						return false;
					}
					m_files[increment] = file_ptr;
					start = file_ptr->read_scalar_attribute<int>(start_index.name);
					end = file_ptr->read_scalar_attribute<int>(end_index.name);
					m_indpropmap_indexes[increment] = gpm_indprop_map_indexes{ start, end };
					fetch_times_and_ages(file_ptr);					
				}
                
				return gpm_h5_reader_impl_v1::initialize();
			}

			int gpm_h5_incremental_reader_v1::get_num_tops()
			{
				const auto last_incr_file_ptr = m_files.at(m_last_time_index);
				return gpm_h5_reader_impl::get_num_tops(last_incr_file_ptr);
			}

			std::vector<std::string> gpm_h5_incremental_reader_v1::get_property_map_values_at(const int property_index, const int time_index) const
			{
				const auto file_ptr = m_files.at(time_index);
				const auto property_map_ds = file_ptr->get_dataset(gpm_h5cpp_wrapper::PROPERTIESDATASET);
				const auto property_map_ref = file_ptr->get_references(property_map_ds)[property_index];
				const auto property_map = file_ptr->dereference_dataset(&property_map_ref);
				std::vector<hsize_t> dimensions;
				return file_ptr->read_string_dataset(property_map, dimensions);
			}         

			std::vector<float> gpm_h5_incremental_reader_v1::transform_map_to_grid(const std::string& property_map)
			{
				const auto absolute_grid_index = gpm_format_helper::get_function_index(property_map);
				const auto increment_time_index = find_increment_index(absolute_grid_index);
				if(increment_time_index < 0)
				{
					return std::vector<float>();
				}
				
				const auto relative_index = absolute_grid_index - m_indpropmap_indexes.at(increment_time_index).m_start_index;

				const auto inc_file_ptr = m_files.at(increment_time_index);
				const auto grids_ds = inc_file_ptr->get_dataset(gpm_h5cpp_wrapper::GRIDSDATASET);
				const auto reference = inc_file_ptr->get_references(grids_ds)[relative_index];
				const auto ds = inc_file_ptr->dereference_dataset(&reference);
				std::vector<hsize_t> dimensions;
				return inc_file_ptr->read_dataset<float>(ds, dimensions);
			}

			int gpm_h5_incremental_reader_v1::find_increment_index(const int absolute_grid_index) const {
				for (auto time_index = 0; time_index <= m_last_time_index; ++time_index) {
					const auto interval = m_indpropmap_indexes.at(time_index);
					if (absolute_grid_index >= interval.m_start_index 
						&& absolute_grid_index <= interval.m_end_index) {
						return time_index;
					}
				}
				return -1;
			}

			void gpm_h5_incremental_reader_v1::fetch_times_and_ages(const std::shared_ptr<gpm_h5cpp_wrapper>& file_ptr) {
				const auto time_typedesc = parm_type_descr_holder::get_parm_description(TIMESET);
				const auto age_typedesc = parm_type_descr_holder::get_parm_description(AGESET);
				auto timesteps = get_type_descr_values(file_ptr, time_typedesc);
				m_timesteps.insert(m_timesteps.end(), timesteps.begin(), timesteps.end());
				auto ages = get_type_descr_values(file_ptr, age_typedesc);
				m_ages.insert(m_ages.end(), ages.begin(), ages.end());
			}
			
			std::vector<double> gpm_h5_incremental_reader_v1::get_type_descr_values(const std::shared_ptr<gpm_h5cpp_wrapper>& file_ptr, const TypeDescr& type_descr) const {
				const auto name = type_descr.name;
				const auto size = get_num_1d_array(name);
				std::vector<double> data(size);
				auto values = lin_span<double>(data.data(), size);
				const auto dataset = file_ptr->get_dataset(name);
				file_ptr->read_dataset<double>(dataset, &values);
				return data;
			}
		}
	}
}
