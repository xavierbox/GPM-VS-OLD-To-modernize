// -- Schlumberger Private --

#include "gpm_h5_reader_impl_v1.h"
#include "parm_prop_descr.h"
#include <memory>
#include "gpm_format_helper.h"
using namespace H5;

namespace Slb {
	namespace Exploration {
		namespace Gpm {

			gpm_h5_reader_impl_v1::gpm_h5_reader_impl_v1(const std::shared_ptr<gpm_h5cpp_wrapper>& file_p) : gpm_h5_reader_impl(file_p)	{
			}

			bool gpm_h5_reader_impl_v1::initialize() {
				if(!gpm_h5_reader_impl::initialize())
				{
					return false;
				}

				const auto grid_dataset = m_file_p->get_dataset(gpm_h5cpp_wrapper::GRIDSDATASET);
				m_grid_references = m_file_p->get_references(grid_dataset);

				const auto prop_id = parm_descr_holder::get_grid_map_name(TOP);
				const auto num_times = size_timesteps();
				std::vector<index_type> diagonal(num_times, -1);
				for (auto i = 0; i < num_times; ++i) {
					const auto num = size_property(i, prop_id);
					diagonal[i] = num - 1;
				}
				m_diagonal_indexes = diagonal;
			}		
            
			bool gpm_h5_reader_impl_v1::is_surface_constant(const std::string& property_id, const int& time_index, const int& surface_index) {
				const auto property_index = get_property_index(property_id);
				const auto values = get_property_map_values_at(property_index, time_index);
				return is_surface_constant(values[surface_index]);
			}

			bool gpm_h5_reader_impl_v1::is_surface_defined(const std::string& property_id, const int& time_index, const int& surface_index) {
				const auto property_index = get_property_index(property_id);
				const auto values = get_property_map_values_at(property_index, time_index);
				return is_surface_defined(values[surface_index]);
			}

			void gpm_h5_reader_impl_v1::get_property_at_index(const std::string& property_id, const int& time_index, const int& surface_index, lin_span<float>* surface) {
			    const auto property_index = get_property_index(property_id);				
				const auto values = get_property_map_values_at(property_index, time_index);
				std::string surface_ref_str;
                if(surface_index < values.size()) {
					surface_ref_str = values[surface_index];
                }

				if (is_surface_defined(surface_ref_str)) {
					if (is_surface_constant(surface_ref_str)) {
						const auto constant = gpm_format_helper::find_constant(surface_ref_str);
						std::fill(surface->begin(), surface->end(), constant);
					}
					else {
                        auto grid = transform_map_to_grid(surface_ref_str);
						std::copy(grid.begin(), grid.end(), surface->begin());
					}
				}
				else {
					const auto no_value = get_novalue();
					std::fill(surface->begin(), surface->end(), no_value);
				}
			}

			std::vector<float> gpm_h5_reader_impl_v1::transform_map_to_grid(const std::string& property_map)
			{
				const auto index = gpm_format_helper::find_index(property_map);
				const auto dataset = m_file_p->dereference_dataset(&m_grid_references[index]);
				std::vector<hsize_t> dimensions;
				return m_file_p->read_dataset<float>(dataset, dimensions);
			}

			bool gpm_h5_reader_impl_v1::is_surface_constant(const std::string& surface_ref_str) {
				return !gpm_format_helper::is_empty_string(surface_ref_str) && !gpm_format_helper::has_index_prefix(surface_ref_str);
			}

			bool gpm_h5_reader_impl_v1::is_surface_defined(const std::string& surface_ref_str) {
				return !gpm_format_helper::is_empty_string(surface_ref_str);
			}

			int gpm_h5_reader_impl_v1::get_count_of_defined_surfaces(const int left, const int right, const std::vector<std::string>& data) {
				auto l = left;
				auto r = right;
				while (l < r) {
					const auto m = std::floor((l + r) / 2);
					if (is_surface_defined(data.at(m))) {
						l = m + 1;
					}
					else {
						r = m;
					}
				}

				return l;
			}

			int gpm_h5_reader_impl_v1::size_property(const int time_index, const std::string& prop_id) const {						    
			    const auto property_index = get_property_index(prop_id);
				if (property_index == -1 || time_index > size_timesteps()) {
					return 0;
				}
				
				const auto row_values = get_property_map_values_at(property_index, time_index);
				return get_count_of_defined_surfaces(0, row_values.size(), row_values);
			}

			std::vector<std::string> gpm_h5_reader_impl_v1::get_property_map_values_at(const int property_index, const int time_index) const {
				auto references = get_props_references(m_file_p);
				const auto property_dataset = m_file_p->dereference_dataset(&references[property_index]);
				DataSpace dataspace = property_dataset.getSpace();
		        const int rank = dataspace.getSimpleExtentNdims();
		        hsize_t sdim[64];
		        int ndims = dataspace.getSimpleExtentDims(sdim);
				std::vector<hsize_t> dimensions;
				std::copy(sdim, sdim + rank, std::back_inserter(dimensions));
				const std::vector<hsize_t> count = { 1, static_cast<hsize_t>(dimensions[1]) };
				const std::vector<hsize_t> offset = { static_cast<hsize_t>(time_index), static_cast<hsize_t>(0) };
				return  m_file_p->read_string_dataset(property_dataset, offset, count);
			}
		}
	}
}