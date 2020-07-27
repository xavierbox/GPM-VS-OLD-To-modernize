// -- Schlumberger Private --

#include "libgp.h"
#include "gpm_vbl_array_2d.h"

namespace Slb { namespace Exploration { namespace Gpm {

void dirgrad(const float_2darray_base_type& z, const float_2darray_base_type& dirx, const float_2darray_base_type& diry, float deltx, float delty, float_2darray_base_type* grad_p) {
    // Finds the directional gradient of z
    // in the direction of unit vectors dirx, diry
    // with sign changed (i.e. positive when z decreases)

    float dx, dy;

    assert(!z.empty()&& !dirx.empty()&& !diry.empty() && !grad_p->empty());
    float_2darray_base_type& grad(*grad_p);
    int_extent_2d::extent_type row_extent = grad.row_extent();
    int_extent_2d::extent_type col_extent = grad.col_extent();
    auto row_upper = row_extent.upper();
    auto row_low = row_extent.lower();
    auto col_upper = col_extent.upper();
    auto col_low = col_extent.lower();

    for (auto i = row_low; i < row_upper; i++) {
        for (auto j = col_low; j < col_upper; j++) {
            // dx
            if (j == col_low) dx = (z(i, j + 1) - z(i, j)) / deltx;
            else if (j == col_upper - 1) dx = (z(i, j) - z(i, j - 1)) / deltx;
            else dx = (z(i, j + 1) - z(i, j - 1)) / (2.0F * deltx);
            // dy
            if (i == row_low) dy = (z(i + 1, j) - z(i, j)) / delty;
            else if (i == row_upper - 1) dy = (z(i, j) - z(i - 1, j)) / delty;
            else dy = (z(i + 1, j) - z(i - 1, j)) / (2.0F * delty);
            // Find projection
            grad(i, j) = - dx * dirx(i, j) - dy * diry(i, j);
            // TEMPORARY check for negative values
            // Final fix should consider how energy dissipation is calculated so that
            // negative dissipation will be impossible
            if (grad(i, j) < 0.0) {
                grad(i, j) = 0.0;
            }
        }
    }
}

}}}
