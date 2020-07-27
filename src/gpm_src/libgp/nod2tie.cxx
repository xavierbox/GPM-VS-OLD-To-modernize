// -- Schlumberger Private --

#include "libgp.h"
#include "gpm_vbl_array_2d.h"

namespace Slb { namespace Exploration { namespace Gpm {

inline void loc_nod2tie(int di, int dj, float_2darray_base_type* grad_p) {
    int_extent_2d ext = grad_p->extents().make_extent_with_borders(0, -di, 0, -dj);
    int_extent_2d::extent_type row_extent = ext.row_extent();
    int_extent_2d::extent_type col_extent = ext.col_extent();
    float_2darray_base_type& gradx(*grad_p);
    for (auto i = row_extent.lower(); i < row_extent.upper(); i++) {
        for (auto j = col_extent.lower(); j < col_extent.upper(); j++) {
            gradx(i, j) = (gradx(i + di, j + dj) + gradx(i, j)) / 2.0F;
        }
    }
}

void nod2tie(float_2darray_base_type* gradx_p, float_2darray_base_type* grady_p) {
    // Converts node gradients to tie gradients in place

    assert(!grady_p->empty()&&!gradx_p->empty());
    loc_nod2tie(0, 1, gradx_p);// One less in cols
    loc_nod2tie(1, 0, grady_p);// One less in rows
}

void cell2nod(const float_2darray_base_type& cellprop, float_2darray_base_type* nodeprop_p) { // Converts a cell-based property to a node-based property by interpolation of adjacent cells
    // nrows, ncols: rows and columns of nodes

    float_2darray_base_type& nodeprop(*nodeprop_p);
    int nrows = nodeprop.extents().row_extent().upper();
    int ncols = nodeprop.extents().col_extent().upper();
    nodeprop(0, 0) = cellprop(0, 0);
    for (int j = 1; j < ncols - 1; j++) {
        nodeprop(0, j) = (cellprop(0, j - 1) + cellprop(0, j)) / 2.0F;
    }
    nodeprop(0, ncols - 1) = cellprop(0, ncols - 2);
    for (int i = 1; i < nrows - 1; i++) {
        nodeprop(i, 0) = (cellprop(i - 1, 0) + cellprop(i, 0)) / 2.0F;
        for (int j = 1; j < ncols - 1; j++) {
            nodeprop(i, j) = (cellprop(i - 1, j - 1) + cellprop(i, j - 1) + cellprop(i - 1, j) + cellprop(i, j)) / 4.0F;
        }
        nodeprop(i, ncols - 1) = (cellprop(i - 1, ncols - 2) + cellprop(i, ncols - 2)) / 2.0F;
    }
    nodeprop(nrows - 1, 0) = cellprop(nrows - 2, 0);
    for (int j = 1; j < ncols - 1; j++) {
        nodeprop(nrows - 1, j) = (cellprop(nrows - 2, j - 1) + cellprop(nrows - 2, j)) / 2.0F;
    }
    nodeprop(nrows - 1, ncols - 1) = cellprop(nrows - 2, ncols - 2);
}

}}}
