// -- Schlumberger Private --

#include "libgp.h"
#include "gpm_vbl_array_2d.h"

namespace Slb { namespace Exploration { namespace Gpm {

void gradn_cells(const float_2darray_base_type& z, float deltx, float delty, float_2darray_base_type* gradx_p, float_2darray_base_type* grady_p) {
    // Cell-centered version of gradn. i corresponds with x. j corresponds with y
    // Calculates gradients from a grid z
    // z is node centered, nrows by ncols
    // Gradients are cell centered, nrows-1 by ncols-1
    /* nrows (in) = number of rows of nodes (minimum 2)
    ncols (in) = number of columns of nodes (minimum 2)
    z     (in) = output grid (must be allocated by caller)
    gradx (ou), grady(ou) = gradient grids in vertical units per horizontal distance
    Return value = 0: OK
    -1: Error
    */

    assert(!z.empty() && !gradx_p->empty() && !grady_p->empty());
    float_2darray_base_type& gradx(*gradx_p);
    float_2darray_base_type& grady(*grady_p);
    int_extent_2d::extent_type row_extent = gradx.row_extent();
    int_extent_2d::extent_type col_extent = gradx.col_extent();
    // Z grid has to be one larger
    assert(z.extents().contains(row_extent.upper(), col_extent.upper()));
    for (auto i = row_extent.lower(); i < row_extent.upper(); i++) {
        for (auto j = col_extent.lower(); j < col_extent.upper(); j++) {
            gradx(i, j) = (z(i, j + 1) - z(i, j) + z(i + 1, j + 1) - z(i + 1, j)) / 2.0F / deltx;
            grady(i, j) = (z(i + 1, j) - z(i, j) + z(i + 1, j + 1) - z(i, j + 1)) / 2.0F / delty;
        }
    }
}

}}}
