// -- Schlumberger Private --

#include "gpm_h5_reader_impl_v2.h"
#include "parm_prop_descr.h"
#include <memory>
#include "gpm_h5cpp_wrapper.h"
#include "gpm_format_helper.h"
#include <cmath>

using namespace H5;

namespace Slb {	namespace Exploration { namespace Gpm {

			gpm_h5_reader_impl_v2::gpm_h5_reader_impl_v2(const std::shared_ptr<gpm_h5cpp_wrapper>& file_p) : gpm_h5_reader_impl(file_p) {
			}

			bool gpm_h5_reader_impl_v2::initialize() {		
				if(!gpm_h5_reader_impl::initialize())
				{
					return false;
				}

				try {
					const auto prop_id = parm_descr_holder::get_grid_map_name(TOP);
					const auto num_times = size_timesteps();
					const auto num_ages = size_ages();
					std::vector<index_type> diagonal(num_times, -1);
					for (auto i = 0; i < num_times; ++i) {
						const std::vector<hsize_t> count = { 1, static_cast<hsize_t>(num_ages) };
						const std::vector<hsize_t> offset = { static_cast<hsize_t>(i), static_cast<hsize_t>(0) };
						const auto row_values = get_property_data(prop_id, offset, count);
						diagonal[i] = get_count_of_defined_surfaces(0, num_ages, row_values) - 1;
					}

					m_diagonal_indexes = diagonal;
				}
				catch (...) {
					return false;
				}				
			}		

			std::shared_ptr<gpm_h5_reader_impl_v2::gpm_property_data> gpm_h5_reader_impl_v2::get_property_data(const std::string& property_id, std::vector<hsize_t>& dimensions) const {
				const auto property_index = get_property_index(property_id);
				auto references = get_props_references(m_file_p);
				const auto property_dataset = m_file_p->dereference_dataset(&references[property_index]);
				std::vector<hsize_t> scalar(1, 1);
				const auto const_ref = m_file_p->read_attribute<hobj_ref_t>(property_dataset, "CONSTANTS_REF", scalar)[0];
				const auto const_dataset = m_file_p->dereference_dataset(&const_ref);
				std::vector<hsize_t> const_dims;

				gpm_property_data data;
				data.references = m_file_p->get_references(property_dataset, dimensions);
				data.constants = m_file_p->read_dataset<float>(const_dataset, const_dims);
				return std::make_shared<gpm_property_data>(data);
			}

			std::shared_ptr<gpm_h5_reader_impl_v2::gpm_property_data> gpm_h5_reader_impl_v2::get_property_data(const std::string& property_id, const int& time_index, const int& surface_index) const {
				const auto property_index = get_property_index(property_id);
				auto references = get_props_references(m_file_p);
				const auto property_dataset = m_file_p->dereference_dataset(&references[property_index]);

				std::vector<hsize_t> scalar(1, 1);
				const auto const_ref = m_file_p->read_attribute<hobj_ref_t>(property_dataset, "CONSTANTS_REF", scalar)[0];
				const auto const_dataset = m_file_p->dereference_dataset(&const_ref);

				const std::vector<hsize_t> offset = { static_cast<hsize_t>(time_index), static_cast<hsize_t>(surface_index) };
				const std::vector<hsize_t> count(2, 1);

				gpm_property_data data;
				data.references = m_file_p->get_references(property_dataset, offset, count);
				data.constants = m_file_p->read_dataset<float>(const_dataset, offset, count);
				return std::make_shared<gpm_property_data>(data);
			}

			std::shared_ptr<gpm_h5_reader_impl_v2::gpm_property_data> gpm_h5_reader_impl_v2::get_property_data(const std::string& property_id, const std::vector<hsize_t>& offset, const std::vector<hsize_t>& count) const {
				const auto property_index = get_property_index(property_id);
				auto references = get_props_references(m_file_p);
				const auto property_dataset = m_file_p->dereference_dataset(&references[property_index]);

				std::vector<hsize_t> scalar(1, 1);
				const auto const_ref = m_file_p->read_attribute<hobj_ref_t>(property_dataset, "CONSTANTS_REF", scalar)[0];
				const auto const_dataset = m_file_p->dereference_dataset(&const_ref);
				gpm_property_data data;
				data.references = m_file_p->get_references(property_dataset, offset, count);
				data.constants = m_file_p->read_dataset<float>(const_dataset, offset, count);
				return std::make_shared<gpm_property_data>(data);
			}

			bool gpm_h5_reader_impl_v2::is_surface_constant(const std::string& prop_id, const int& time_index, const int& surface_index) {
				const auto refs = get_property_data(prop_id, time_index, surface_index);
				return is_surface_constant(refs);
			}			

			bool gpm_h5_reader_impl_v2::is_surface_defined(const std::string& prop_id, const int& time_index, const int& surface_index) {
				const auto refs = get_property_data(prop_id, time_index, surface_index);
				return is_surface_defined(refs);
			}

			void gpm_h5_reader_impl_v2::get_property_at_index(const std::string& prop_id, const int& time_index, const int& surface_index, lin_span<float>* surface) {
				const auto refs = get_property_data(prop_id, time_index, surface_index);
				const auto reference = refs->references[0];
				const auto constant = refs->constants[0];
				if (is_surface_defined(refs)) {
					if (is_surface_constant(refs)) {
						std::fill(surface->begin(), surface->end(), constant);
					}
					else {
						const auto dataset = m_file_p->dereference_dataset(&reference);
						std::vector<hsize_t> dimensions;
						std::vector<float> values = m_file_p->read_dataset<float>(dataset, dimensions);
						std::copy(values.begin(), values.end(), surface->begin());
					}
				}
				else {
					const auto no_value = get_novalue();
					std::fill(surface->begin(), surface->end(), no_value);
				}
			}

			bool gpm_h5_reader_impl_v2::is_surface_constant(const std::shared_ptr<gpm_property_data>& refs, const int& index) {
				const auto reference = refs->references[index];
				const auto constant = refs->constants[index];
				return reference == NULL && !std::isnan(constant);
			}

			bool gpm_h5_reader_impl_v2::is_surface_defined(const std::shared_ptr<gpm_property_data>& refs, const int& index) {
				const auto reference = refs->references[index];
				const auto constant = refs->constants[index];
				return reference != NULL || !std::isnan(constant);
			}

			int gpm_h5_reader_impl_v2::get_count_of_defined_surfaces(const int left, const int right, const std::shared_ptr<gpm_property_data>& data) const {
				auto l = left;
				auto r = right;
				while (l < r) {
					const auto m = std::floor((l + r) / 2);
					if (is_surface_defined(data, m)) {
						l = m + 1;
					}
					else {
						r = m;
					}
				}

				return l;
			}

			int gpm_h5_reader_impl_v2::size_property(const int time_index, const std::string& prop_id) const {
				if (get_property_index(prop_id) == -1 || time_index >= size_timesteps()) {
					return 0;
				}

				const auto num_ages = size_ages();
			    const std::vector<hsize_t> count = { 1, static_cast<hsize_t>(num_ages) };
				const std::vector<hsize_t> offset = { static_cast<hsize_t>(time_index), static_cast<hsize_t>(0) };
				const auto row_values = get_property_data(prop_id, offset, count);
				return get_count_of_defined_surfaces(0, num_ages, row_values);
			}
		}
	}
}