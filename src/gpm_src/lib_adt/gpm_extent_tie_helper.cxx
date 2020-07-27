// -- Schlumberger Private --

#include "gpm_extent_tie_helper.h"

namespace Slb { namespace Exploration { namespace Gpm {

gpm_extent_tie_helper::gpm_extent_tie_helper()
= default;

gpm_extent_tie_helper::gpm_extent_tie_helper(int_extent_2d::size_type nr, int_extent_2d::size_type nc)
{
    initialize(nr, nc);
}

gpm_extent_tie_helper::gpm_extent_tie_helper(const int_extent_2d& ext)
{
    initialize(ext);
}

void gpm_extent_tie_helper::initialize(int_extent_2d::size_type nr, int_extent_2d::size_type nc)
{
    initialize(int_extent_2d(nr, nc));
}

void gpm_extent_tie_helper::initialize(const int_extent_2d& ext)
{
    full_extent = ext;
    col_offset = int_extent_2d::make_index(0, -1);
    row_offset = int_extent_2d::make_index(-1, 0);
    full_extent_with_col_boundary_minus_1 = col_offset | full_extent;
    full_extent_with_row_boundary_minus_1 = full_extent | row_offset;
}

}}}
