// -- Schlumberger Private --

#include "libgp.h"
#include "gpm_vbl_array_2d.h"
#include <cmath>

namespace Slb { namespace Exploration { namespace Gpm {

void normvec(float_2darray_base_type* vecx_p, float_2darray_base_type* vecy_p) {
    // Normalizes vectors to make them length 1
    // vectors assumed node centered
    // can be cell centered by using nrows and ncols of cells
    // If original length is 0, it stays 0
    // nrows, ncols: number of rows and columns of nodes or cells

    assert(!vecx_p->empty() &&!vecy_p->empty());
    float_2darray_base_type& vecx(*vecx_p);
    float_2darray_base_type& vecy(*vecy_p);
    int_extent_2d::extent_type row_extent = vecx.row_extent();
    int_extent_2d::extent_type col_extent = vecx.col_extent();
    for (auto i = row_extent.lower(); i < row_extent.upper(); i++) {
        for (auto j = col_extent.lower(); j < col_extent.upper(); j++) {
            float vecx_item = vecx(i, j);
            float vecy_item = vecy(i, j);
            float vlen = static_cast<float>(sqrt(vecx_item * vecx_item + vecy_item * vecy_item));
            if (vlen > 0) {
                vecx(i, j) /= vlen;
                vecy(i, j) /= vlen;
            }
        }
    }
}

}}}
