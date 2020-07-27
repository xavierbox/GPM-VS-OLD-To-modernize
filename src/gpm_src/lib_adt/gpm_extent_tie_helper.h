// -- Schlumberger Private --

#ifndef GPM_EXTENT_TIE_HELPER_H
#define GPM_EXTENT_TIE_HELPER_H

#include "gpm_int_extent_2d.h"


namespace Slb { namespace Exploration { namespace Gpm {

struct gpm_extent_tie_helper {
    int_extent_2d full_extent;
    int_extent_2d col_offset;
    int_extent_2d row_offset;
    int_extent_2d full_extent_with_col_boundary_minus_1;
    int_extent_2d full_extent_with_row_boundary_minus_1;
    gpm_extent_tie_helper();
    gpm_extent_tie_helper(int_extent_2d::size_type nr, int_extent_2d::size_type nc);
    explicit gpm_extent_tie_helper(const int_extent_2d& ext);
    void initialize(const int_extent_2d& ext);
    void initialize(int_extent_2d::size_type nr, int_extent_2d::size_type nc);
};
}}}

#endif
