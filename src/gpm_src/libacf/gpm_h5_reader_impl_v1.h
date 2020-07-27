// -- Schlumberger Private --

#ifndef gpm_h5_reader_impl_v1_h
#define gpm_h5_reader_impl_v1_h

#include "gpm_lin_span.h"
#include <memory>
#include "gpm_h5_reader_impl.h"

namespace Slb { namespace Exploration { namespace Gpm {
/*
 * Class to encapsulate access to H5Cpp libraries API
 */
class gpm_h5_reader_impl_v1 : public gpm_h5_reader_impl {

public:
	explicit gpm_h5_reader_impl_v1(const std::shared_ptr<gpm_h5cpp_wrapper>& file_p);
	gpm_h5_reader_impl_v1(const gpm_h5_reader_impl_v1& rhs) = delete;
	gpm_h5_reader_impl_v1& operator=(const gpm_h5_reader_impl_v1& rhs) = delete;
	~gpm_h5_reader_impl_v1() override = default;

	void get_property_at_index(const std::string& property_id, const int& time_index, const int& surface_index, lin_span<float>* surface) override;
	bool is_surface_constant(const std::string& property_id, const int& time_index, const int& surface_index) override;
	bool is_surface_defined(const std::string& property_id, const int& time_index, const int& surface_index) override;
	int size_property(const int time_index, const std::string& prop_id) const override;

	static int get_count_of_defined_surfaces(int left, int right, const std::vector<std::string>& data);
	static bool is_surface_constant(const std::string& surface_ref_str);
	static bool is_surface_defined(const std::string& surface_ref_str);
protected:
	bool initialize() override;
	virtual std::vector<float> transform_map_to_grid(const std::string& property_map);
	virtual std::vector<std::string> get_property_map_values_at(const int property_index, const int time_index) const;
private:
	std::vector<hobj_ref_t> m_grid_references;
};
}}}

#endif