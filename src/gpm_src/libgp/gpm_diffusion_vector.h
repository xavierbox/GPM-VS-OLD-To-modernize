// -- Schlumberger Private --

#ifndef GPM_DIFFUSION_VECTOR_H
#define GPM_DIFFUSION_VECTOR_H

#include "gpm_2dvector_tie_array.h"
#include "gpm_basic_defs.h"


namespace Slb { namespace Exploration { namespace Gpm {

    class diffusion_vector : public gpm_2dvector_tie_array {
    public:
        diffusion_vector();
        diffusion_vector(const diffusion_vector& rhs) = delete;
        diffusion_vector& operator=(const diffusion_vector& rhs) = delete;
        ~diffusion_vector();
        float_2darray_base_type& get_cdx();
        float_2darray_base_type& get_cdy();
        const float_2darray_base_type& get_cdx() const;
        const float_2darray_base_type& get_cdy() const;
        float calculate_tie_harmonic_average_diffusion(const float_2darray_base_type& coeff, const float_2darray_base_type& za, float dx2, float dy2, float transportability);
        float calculate_tie_average_diffusion(const float_2darray_base_type& coeff, const float_2darray_base_type& za, float factor, float dx2, float dy2, float transportability);
    private:
        static void calculate_tie_harmonic_diffusion_coeffs(const float_2darray_base_type& coeff, const int_extent_1d& row_extent, const int_extent_1d& col_extent, int i_offset, int j_offset, float d2_reciproc, float_2darray_base_type& cdx);
        static void calculate_tie_diffusion_coeffs(const float_2darray_base_type& coeff, const int_extent_1d& row_extent, const int_extent_1d& col_extent, int i_offset, int j_offset, float factor, float d2_reciproc, float_2darray_base_type& cdx);
        static void add_height_difference(const int_extent_1d& row_extent, const int_extent_1d& col_extent, int i_offset, int j_offset, const float_2darray_base_type& za, float_2darray_base_type& cdx);
        float find_max_velocity(const int_extent_2d& extents);
        float calculate_tie_harmonic_average_diffusion_serial(const float_2darray_base_type & coeff, const float_2darray_base_type & za, float dx2, float dy2, float transportability);
        float calculate_tie_harmonic_average_diffusion_TBB(const float_2darray_base_type & coeff, const float_2darray_base_type & za, float dx2, float dy2, float transportability);
        float calculate_tie_average_diffusion_serial(const float_2darray_base_type & coeff, const float_2darray_base_type & za, float factor, float dx2, float dy2, float transportability);
        float calculate_tie_average_diffusion_TBB(const float_2darray_base_type & coeff, const float_2darray_base_type & za, float factor, float dx2, float dy2, float transportability);
    };
}}}

#endif
