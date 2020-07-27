// -- Schlumberger Private --

#ifndef gpm_h5_reader_impl_v2_h
#define gpm_h5_reader_impl_v2_h

#include <memory>
#include "gpm_h5cpp_wrapper.h"
#include "gpm_h5_reader_impl.h"

namespace Slb {	namespace Exploration {	namespace Gpm {
			/*
			 * Class to encapsulate access to H5Cpp libraries API
			 */
			class gpm_h5_reader_impl_v2 final : public gpm_h5_reader_impl {
			public:
				explicit gpm_h5_reader_impl_v2(const std::shared_ptr<gpm_h5cpp_wrapper>& file_p);
                gpm_h5_reader_impl_v2(const gpm_h5_reader_impl_v2& rhs) = delete;
				gpm_h5_reader_impl_v2& operator=(const gpm_h5_reader_impl_v2& rhs) = delete;
				~gpm_h5_reader_impl_v2() override = default;

				void get_property_at_index(const std::string& prop_id, const int& time_index, const int& surface_index, lin_span<float>* surface) override;
				bool is_surface_constant(const std::string& prop_id, const int& time_index, const int& surface_index) override;
				bool is_surface_defined(const std::string& prop_id, const int& time_index, const int& surface_index) override;
				int size_property(const int time_index, const std::string& prop_id) const override;

			protected:
				bool initialize() override;
			private:
				struct gpm_property_data {
					gpm_property_data() = default;
					~gpm_property_data() = default;
					std::vector<hobj_ref_t> references;
					std::vector<float> constants;
				};

				std::shared_ptr<gpm_property_data> get_property_data(const std::string& property_id, std::vector<hsize_t>& dimensions) const;
				std::shared_ptr<gpm_property_data> get_property_data(const std::string& property_id, const int& time_index, const int& surface_index) const;
				std::shared_ptr<gpm_property_data> get_property_data(const std::string& property_id, const std::vector<hsize_t>& offset, const std::vector<hsize_t>& count) const;

                static bool is_surface_constant(const std::shared_ptr<gpm_property_data>& refs, const int& index = 0);
                static bool is_surface_defined(const std::shared_ptr<gpm_property_data>& refs, const int& index = 0);

				int get_count_of_defined_surfaces(int left, int right, const std::shared_ptr<gpm_property_data>& data) const;
			};
		}
	}
}

#endif