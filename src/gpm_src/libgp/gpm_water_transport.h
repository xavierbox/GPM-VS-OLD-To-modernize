#ifndef GPM_WATER_TRANSPORT_H
#define GPM_WATER_TRANSPORT_H

#include "gpm_basic_defs.h"
#include "gpm_int_extent_1d.h"
#include "gpm_vbl_array_2d.h"
#include <functional>

namespace Slb { namespace Exploration { namespace Gpm {
class gpm_water_transport {
public:
    using ind_type = int_extent_1d::index_type;
    // Index i, index j, offset i, offset j
    using pycnal_check_func = std::function<bool(ind_type, ind_type, ind_type, ind_type)>;

    struct hypopycnal {
        hypopycnal(float sea_level, const float_2darray_base_type& top, const float_2darray_base_type& vel,
                   const float_2darray_base_type& depth)
            : slevnow(sea_level), za(top), qx(vel), ht(depth)
        {
        }

        bool operator()(ind_type i, ind_type j, ind_type i_offset, ind_type j_offset) const
        {
            const auto vel_ij = qx(i, j);
            const auto ht_ij = ht(i, j);
            const auto ht_offset = ht(i + i_offset, j + j_offset);

			return !(
				vel_ij > 0 && slevnow - za(i, j) > ht_ij ||
				vel_ij < 0 && slevnow - za(i + i_offset, j + j_offset) > ht_offset);
        }

        float slevnow;
        const float_2darray_base_type& za;
        const float_2darray_base_type& qx;
        const float_2darray_base_type& ht;
    };
    gpm_water_transport();
    explicit gpm_water_transport(double transport_coeff);
    gpm_water_transport(double transport_coeff, float epsilon_height, float epsilon_tie_height);
    float calc_coefficient(double time_delta, float ht, float ht_offset, float velocity) const;
    void calculate_transport_coeffs(const int_extent_1d& row_ext, const int_extent_1d& col_ext,
                                    ind_type i_offset, ind_type j_offset,
                                    const float_2darray_base_type& water_height,
                                    const float_2darray_base_type& water_velocity,
                                    double time_delta, const float_2darray_base_type* surf_vel,
                                    float_2darray_base_type* diffusion_coeff_p) const;
    void calculate_transport_coeffs(const int_extent_1d& row_ext, const int_extent_1d& col_ext, ind_type i_offset,
                                    ind_type j_offset,
                                    const float_2darray_base_type& water_height,
                                    const float_2darray_base_type& water_velocity,
                                    double time_delta,
                                    const pycnal_check_func& pyc_check,
                                    float_2darray_base_type* diffusion_coeff_p) const;

    double trans_coeff{};
    float min_tie_depth{};
    float min_depth{};
};

}}}
#endif
