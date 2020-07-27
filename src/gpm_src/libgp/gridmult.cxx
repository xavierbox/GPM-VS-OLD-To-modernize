// -- Schlumberger Private --

#include "libgp.h"
#include "gpm_vbl_array_2d.h"

namespace Slb { namespace Exploration { namespace Gpm {

void grid_filter(float_2darray_base_type& qx, const float_2darray_base_type& qxb, float vfacs) {
    return grid_filter(qx.extents(), qx, qxb, vfacs);
}

void grid_filter(const int_extent_2d& ext, float_2darray_base_type& qx, const float_2darray_base_type& qxb, float vfacs) {
    // Clip according to the input grids
    int_extent_2d intersect1 = ext & qxb.extents() & qx.extents();
    // The border regions and the inner regions
    int_extent_2d inner_range = intersect1.inner_extent(1, 1);
    std::vector<int_extent_2d> border_extents = intersect1.border_extents(1, 1);
    int_extent_2d::extent_type row_extent = inner_range.row_extent();
    int_extent_2d::extent_type col_extent = inner_range.col_extent();

    // Inner region
    for (auto i = row_extent.lower(); i < row_extent.upper(); ++i) {
        for (auto j = col_extent.lower(); j < col_extent.upper(); ++j) {
            int num = 0;
            float sum = qxb(i - 1, j);
            num++;
            sum += qxb(i + 1, j);
            num++;
            sum += qxb(i, j - 1);
            num++;
            sum += qxb(i, j + 1);
            num++;
            qx(i, j) = qxb(i, j) + (sum / num - qxb(i, j)) * 0.5f * vfacs;
        }
    }
    // Filter velocities spatially
    // Do the border part
    for (std::vector<int_extent_2d>::const_iterator it = border_extents.begin(); it != border_extents.end(); ++it) {
        int_extent_2d loc(*it);
        int_extent_2d::extent_type row_extent1 = loc.row_extent();
        int_extent_2d::extent_type col_extent1 = loc.col_extent();
        for (auto i = row_extent1.lower(); i < row_extent1.upper(); ++i) {
            for (auto j = col_extent1.lower(); j < col_extent1.upper(); ++j) {
                int num = 0;
                float sum = 0;
                if (loc.contains(i - 1, j)) {
                    sum += qxb(i - 1, j);
                    num++;
                }
                if (loc.contains(i + 1, j)) {
                    sum += qxb(i + 1, j);
                    num++;
                }
                if (loc.contains(i, j - 1)) {
                    sum += qxb(i, j - 1);
                    num++;
                }
                if (loc.contains(i, j + 1)) {
                    sum += qxb(i, j + 1);
                    num++;
                }
                if (num > 0) {
                    qx(i, j) = qxb(i, j) + (sum / num - qxb(i, j)) * 0.5f * vfacs;
                }
                else {
                    qx(i, j) = qxb(i, j);
                }
            }
        }
    }
}

}}}
