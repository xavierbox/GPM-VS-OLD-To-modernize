
#include "gpm_water_transport.h"
#include "gpm_vbl_array_2d.h"

// Should really move the transport to its own impl class
namespace Slb { namespace Exploration { namespace Gpm {

gpm_water_transport::gpm_water_transport()
= default;

gpm_water_transport::gpm_water_transport(double transport_coeff) : trans_coeff(transport_coeff)
{
}

gpm_water_transport::gpm_water_transport(double transport_coeff,
                                         float epsilon_height,
                                         float epsilon_tie_height) : trans_coeff(transport_coeff),
                                                              min_tie_depth(epsilon_tie_height),
                                                              min_depth(epsilon_height)
{
}

float gpm_water_transport::calc_coefficient(double time_delta, const float ht, const float ht_offset, float velocity) const
{
    auto deptie = (ht + ht_offset) / 2.0F;
    if (deptie < min_tie_depth) deptie = min_tie_depth;
    float cddiff = velocity * velocity / deptie * trans_coeff;
    // Dan T.: 1E-6 is an arbitrary constant to bring user coefficient within manageable bounds
    if (cddiff > deptie / time_delta) cddiff = deptie / time_delta;
    // Limit load by depth // Needs to go sometime soon JT
    if (velocity < 0) cddiff = -cddiff;
    return cddiff;
}

void gpm_water_transport::calculate_transport_coeffs(const int_extent_1d& row_ext, const int_extent_1d& col_ext,
                                                     ind_type i_offset, ind_type j_offset,
                                                     const float_2darray_base_type& water_height,
                                                     const float_2darray_base_type& water_velocity,
                                                     double time_delta,
                                                     const float_2darray_base_type* surf_vel,
                                                     float_2darray_base_type* diffusion_coeff_p) const
{
    for (auto i = row_ext.lower(), nr = row_ext.upper(); i < nr; i++) {
        for (auto j = col_ext.lower(), nc = col_ext.upper(); j < nc; j++) {
            const auto ht = water_height(i, j); // was htw
            const auto ht_offset = water_height(i + i_offset, j + j_offset); // was htw
            if (ht > min_depth && ht_offset > min_depth) {
                auto vel = water_velocity(i, j);
                if (surf_vel != nullptr) vel += surf_vel->at(i, j);
                const auto cddiff = calc_coefficient(time_delta, ht, ht_offset, vel);
                (*diffusion_coeff_p)(i, j) = cddiff;
            }
        }
    }
}

void gpm_water_transport::calculate_transport_coeffs(const int_extent_1d& row_ext, const int_extent_1d& col_ext,
                                                     ind_type i_offset, ind_type j_offset,
                                                     const float_2darray_base_type& water_height,
                                                     const float_2darray_base_type& water_velocity, double time_delta,
                                                     const pycnal_check_func& pyc_check,
                                                     float_2darray_base_type* diffusion_coeff_p) const
{
    for (auto i = row_ext.lower(), nr = row_ext.upper(); i < nr; i++) {
        for (auto j = col_ext.lower(), nc = col_ext.upper(); j < nc; j++) {
            // Steady flow
            const auto ht = water_height(i, j);
            const auto ht_offset = water_height(i + i_offset, j + j_offset);
            if (ht > min_depth && ht_offset > min_depth) {
                // Prevent hypopycnal erosion from node of origin
                const auto vel = water_velocity(i, j);
                if (pyc_check(i, j, i_offset, j_offset)) {
                    const auto cddiff = calc_coefficient(time_delta, ht, ht_offset, vel);
                    (*diffusion_coeff_p)(i, j) = cddiff;
                }
            }
        }
    }
}

}}}
