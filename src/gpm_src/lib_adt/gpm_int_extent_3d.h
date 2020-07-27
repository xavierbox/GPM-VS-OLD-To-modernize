// -- Schlumberger Private --

#ifndef GPM_INT_EXTENT_3D_H
#define GPM_INT_EXTENT_3D_H

#include "gpm_int_extent_1d.h"

namespace Slb { namespace Exploration { namespace Gpm {

class int_extent_3d {
public:
    using extent_type = int_extent_1d;
	using index_type = std::ptrdiff_t;
	using size_type = std::size_t;
	int_extent_3d();
    int_extent_3d(size_type num_lays, size_type num_rows, size_type num_cols);
    int_extent_3d(index_type lay_begin, index_type lay_open_end,
		index_type row_beg, index_type row_open_end,
		index_type col_beg, index_type col_open_end);
    int_extent_3d(const extent_type& lay, const extent_type& row, const extent_type& col);

    bool is_empty() const;

    extent_type lay_extent() const;
    extent_type row_extent() const;
    extent_type col_extent() const;
    static int_extent_3d make_index(index_type i, index_type j, index_type k);
    // Intersection
    int_extent_3d operator&(const int_extent_3d& rhs) const;
    //Union of extents
    int_extent_3d operator|(const int_extent_3d& rhs) const;
    // Equality
    bool operator==(const int_extent_3d& that) const;
    bool operator !=(const int_extent_3d& that) const;

    bool contains(const int_extent_3d& interval) const;
    bool contains(index_type i, index_type j, index_type k) const;
private:
    extent_type _lay_extent;
    extent_type _row_extent;
    extent_type _col_extent;
};

}}}

#endif
