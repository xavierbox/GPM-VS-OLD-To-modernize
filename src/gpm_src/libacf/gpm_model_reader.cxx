#include "gpm_model_reader.h"
#include "mapstc_descr.h"
#include "parm_prop_descr.h"
#include "gpm_lin_span.h"
#include "gpm_format_helper.h"
#include "gpm_h5_reader_impl_v2.h"
#include "gpm_h5_reader_impl_v1.h"
#include "gpm_h5_incremental_reader_v1.h"

namespace Slb {
	namespace Exploration {
		namespace Gpm {
		
			struct gpm_model_reader::api_impl {
				api_impl();
				bool open(const std::string& filename);
				bool open(const std::string& main_file_name, const int last_increment_index);
				std::unique_ptr<gpm_h5_reader_impl> m_reader_impl;
			};

			gpm_model_reader::api_impl::api_impl() = default;

			bool gpm_model_reader::api_impl::open(const std::string& main_file_name, const int last_increment_index) {

				if (last_increment_index < 0) {
					return false;
				}
				
				try {
					const auto file_ptr = std::make_shared<gpm_h5cpp_wrapper>();
					const auto increment_file_name = gpm_format_helper::get_incremental_file_name(main_file_name, 0);
					if (!file_ptr->open_file(increment_file_name, false, false)) {
						return false;
					}

					m_reader_impl.reset(new gpm_h5_incremental_reader_v1(file_ptr, main_file_name, last_increment_index));
					return m_reader_impl->initialize();
				}
				catch (...) {
					return false;
				}
			}

			bool gpm_model_reader::api_impl::open(const std::string& filename) {	
				const auto file_ptr = std::make_shared<gpm_h5cpp_wrapper>();
				auto res = file_ptr->open_file(filename, false, false);
				if (!res) {
					return false;
				}

				try {
					const auto version = file_ptr->get_version();
					const std::string prefix = "1.";
					if (version.substr(0, prefix.size()) == prefix) {
						m_reader_impl.reset(new gpm_h5_reader_impl_v1(file_ptr));
					}
					else {
						m_reader_impl.reset(new gpm_h5_reader_impl_v2(file_ptr));
					}
					
					return m_reader_impl->initialize();
				}
				catch(...) {
					file_ptr->close_file();
					return false;
				}
			}

			void gpm_model_reader::get_timesteps(lin_span<double>* timesteps) const
			{
				m_impl->m_reader_impl->get_timesteps(timesteps);
			}

			int gpm_model_reader::get_num_timesteps() const
			{
				return m_impl->m_reader_impl->size_timesteps();
			}

			void gpm_model_reader::get_ages(lin_span<double>* ages) const
			{
				return m_impl->m_reader_impl->get_ages(ages);
			}

			int gpm_model_reader::get_num_ages() const
			{
				return m_impl->m_reader_impl->size_ages();
			}

			void gpm_model_reader::get_world_corners(lin_span<float>* x_values, lin_span<float>* y_values) const
			{
				return m_impl->m_reader_impl->get_world_corners(x_values, y_values);
			}

			void gpm_model_reader::get_model_size(int* num_rows, int* num_cols) const
			{
				return m_impl->m_reader_impl->get_model_size(num_rows, num_cols);
			}

			float gpm_model_reader::get_base_erodability() const
			{
				return m_impl->m_reader_impl->get_base_erodability();
			}

			float gpm_model_reader::get_novalue() const
			{
				return m_impl->m_reader_impl->get_novalue();
			}

			void gpm_model_reader::get_geometry(const int& time_index, std::vector<lin_span<float>>* surfaces) const
			{
				const auto prop_id = parm_descr_holder::get_grid_map_name(TOP);
				std::vector<int> pos(surfaces->size());
				m_impl->m_reader_impl->get_property(prop_id, time_index, surfaces, &pos);
			}

			int gpm_model_reader::get_num_geometry(const int& time_index) const
			{
				const auto prop_id = parm_descr_holder::get_grid_map_name(TOP);
				assert(!gpm_format_helper::is_empty_string(prop_id));
				return m_impl->m_reader_impl->size_property(time_index, prop_id);
			}

			void gpm_model_reader::get_geometry_statistics(const int& time_index, float& min, float& max) const
			{
				const auto prop_id = parm_descr_holder::get_grid_map_name(TOP);
				get_property_statistics(prop_id, time_index, min, max);
			}

			void gpm_model_reader::get_all_timesteps_surfaces_at_index(const int& surface_index, const std::string& prop_id, std::vector<lin_span<float>>* surfaces) const
			{
				const int num_times = get_num_timesteps();
				assert(surfaces->size() == num_times);

				for (int time_index = 0; time_index < num_times; ++time_index) {
					m_impl->m_reader_impl->get_property_at_index(prop_id, time_index, surface_index, &surfaces->at(time_index));
				}
			}

			void gpm_model_reader::get_all_timesteps_top_surfaces(const std::string& prop_id, std::vector<lin_span<float>>* surfaces) const
			{
				const int num_times = get_num_timesteps();
				assert(surfaces->size() == num_times);

				for (int time_index = 0; time_index < num_times; ++time_index) {
					get_top_surface(time_index, prop_id, &surfaces->at(time_index));
				}
			}

			void gpm_model_reader::get_top_surface(const int& time_index, const std::string& prop_id, lin_span<float>* surface) const
			{
				const auto num_surfaces = get_num_geometry(time_index);
				m_impl->m_reader_impl->get_property_at_index(prop_id, time_index, num_surfaces - 1, surface);
			}

			void gpm_model_reader::get_property_ids(std::vector<std::string>* property_ids) const
			{
				m_impl->m_reader_impl->get_property_ids(property_ids);
			}

			int gpm_model_reader::get_num_sediment_info_in_model() const
			{
				return m_impl->m_reader_impl->size_sediment_info_in_model();
			}

			void gpm_model_reader::get_sediment_info_in_model(std::vector<std::string>* sed_ids, std::vector<std::string>* sed_names, std::vector<std::string>* sed_prop) const
			{
				m_impl->m_reader_impl->get_sediment_info_in_model(sed_ids, sed_names, sed_prop);
			}

			int gpm_model_reader::get_num_sediment_ids_in_baselayer() const
			{
				return m_impl->m_reader_impl->size_sediment_info_in_baselayer();
			}

			void gpm_model_reader::get_sediment_ids_in_baselayer(std::vector<std::string>* sed_ids) const
			{
				m_impl->m_reader_impl->get_sediment_ids_in_baselayer(sed_ids);
			}

			void gpm_model_reader::get_property_statistics(const std::string& prop_id, const int& time_index, float& min, float& max) const
			{
				assert(!gpm_format_helper::is_empty_string(prop_id));
				m_impl->m_reader_impl->get_property_statistics(prop_id, time_index, min, max);
			}

			void gpm_model_reader::get_isnodal(lin_span<int>* nodal) const
			{
				m_impl->m_reader_impl->get_isnodal(nodal);
			}

			gpm_model_reader::gpm_model_reader() : m_impl(new api_impl()) {
			}

			int gpm_model_reader::get_num_property_ids() const
			{
				return m_impl->m_reader_impl->size_property_ids();
			}

			void gpm_model_reader::get_property_display_names(std::vector<std::string>* ids, std::vector<std::string>* names) const
			{
				m_impl->m_reader_impl->get_property_display_names(ids, names);
			}

			int gpm_model_reader::get_num_property_display_names() const
			{
				return m_impl->m_reader_impl->size_property_display_names();
			}

			void gpm_model_reader::get_property(const std::string& prop_id, const int& time_index, std::vector<lin_span<float>>* surfaces, std::vector<int>* diagonal_indexes) const
			{
				m_impl->m_reader_impl->get_property(prop_id, time_index, surfaces, diagonal_indexes);
			}

			bool gpm_model_reader::is_surface_constant(const std::string & prop_id, const int& time_index, const int& surface_index) const
			{
				return m_impl->m_reader_impl->is_surface_constant(prop_id, time_index, surface_index);
			}			

			bool gpm_model_reader::is_surface_defined(const std::string & prop_id, const int& time_index, const int& surface_index) const
			{
				return m_impl->m_reader_impl->is_surface_defined(prop_id, time_index, surface_index);
			}

			int gpm_model_reader::get_num_property(const std::string& prop_id, const int& time_index) const
			{
				return m_impl->m_reader_impl->size_property(time_index, prop_id);
			}

			void gpm_model_reader::get_property_at_index(const int& time_index, const int& surface_index, const std::string& prop_id, lin_span<float>* surface) const
			{
				m_impl->m_reader_impl->get_property_at_index(prop_id, time_index, surface_index, surface);
			}

			int gpm_model_reader::get_num_grid_names() const
			{
				return m_impl->m_reader_impl->get_num_grid_names();
			}

			void gpm_model_reader::get_grid_names(std::vector<std::string>* grid_names) const
			{
				m_impl->m_reader_impl->get_grid_names(grid_names);
			}

			void gpm_model_reader::get_grid(const int grid_index, lin_span<float>* z_nodes) const
			{
				m_impl->m_reader_impl->get_grid(grid_index, z_nodes);
			}

			gpm_model_reader::~gpm_model_reader() = default;

			void gpm_model_reader::close() const
			{
				m_impl->m_reader_impl->close();
			}

			bool gpm_model_reader::open(const std::string& filename) const
			{
				return m_impl->open(filename);
			}

			bool gpm_model_reader::open(const std::string& main_file, const int last_increment_index) const
			{
				return m_impl->open(main_file, last_increment_index);
			}

			int gpm_model_reader::get_num_isnodal() const
			{
				return m_impl->m_reader_impl->size_isnodal();
			}
            		
			std::string gpm_model_reader::validate_surface_index(const int& surface_index) const
			{
				return m_impl->m_reader_impl->validate_surface_index(surface_index);
			}

			std::string gpm_model_reader::validate_time_index(const int& time_index) const
			{
				return m_impl->m_reader_impl->validate_time_index(time_index);
			}
			std::string gpm_model_reader::validate_property_id(const std::string & prop_id) const
			{
				return m_impl->m_reader_impl->validate_property_id(prop_id);
			}

		}
	}
}
