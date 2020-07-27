// -- Schlumberger Private --

#ifndef GPM_INT_EXTENT_2D_H
#define GPM_INT_EXTENT_2D_H


#include "gpm_int_extent_1d.h"
#include <vector>


namespace Slb { namespace Exploration { namespace Gpm {
// Right open interval 
class int_extent_2d {
public:
    using extent_type = int_extent_1d;
	using index_type = std::ptrdiff_t;
	using size_type = std::size_t;
    int_extent_2d();
	int_extent_2d(index_type row_beg, index_type row_open_end, index_type col_beg, index_type col_open_end);
    int_extent_2d(size_type num_rows, size_type num_cols);

    int_extent_2d(const extent_type& row, const extent_type& col);
    bool is_empty() const;
	size_type size() const;

    extent_type row_extent() const;
    extent_type col_extent() const;

    // Check if we are on border
    bool at_row_border(index_type i) const;
    bool at_col_border(index_type j) const;

    // Positive numbers decrease the size of the extent
    // border > 0
    int_extent_2d make_extent_with_borders(index_type row_border_lower, index_type row_border_upper,
                                           index_type col_border_lower, index_type col_border_upper) const;
    // Negative will increase size, positive will decrease
    int_extent_2d make_extent_with_symmetric_borders(index_type row_border, index_type col_border) const;
    int_extent_2d inner_extent(index_type row_border, index_type col_border) const;

    // The eight outer extents, border assumed to be positive >0
    std::vector<int_extent_2d> border_extents(index_type row_border, index_type col_border);

    // The clipped border of a radius R and the origin index of the radius
    std::vector<int_extent_2d> radius_extent(index_type i_o, index_type j_o, index_type radius) const;

        static int_extent_2d make_index(index_type i, index_type j);
    // Intersection
    int_extent_2d operator&(const int_extent_2d& rhs) const;
    //Union of extents
    int_extent_2d operator|(const int_extent_2d& rhs) const;
    int_extent_2d left_subtract(const int_extent_2d& rhs) const;
    int_extent_2d right_subtract(const int_extent_2d& rhs) const;
    bool operator==(const int_extent_2d& that) const;

    bool operator!=(const int_extent_2d& that) const;

    bool contains(const int_extent_2d& interval) const;
    bool contains(index_type i, index_type j) const;

private:
    extent_type _row_extent;
    extent_type _col_extent;
};

}}}

#endif
