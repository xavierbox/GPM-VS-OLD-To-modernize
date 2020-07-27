// -- Schlumberger Private --

#include "gpm_int_extent_3d.h"

namespace Slb { namespace Exploration { namespace Gpm {


int_extent_3d::int_extent_3d()
= default;

int_extent_3d::int_extent_3d(size_type num_lays, size_type num_rows, size_type num_cols) {
    _lay_extent = extent_type(num_lays);
    _row_extent = extent_type(num_rows);
    _col_extent = extent_type(num_cols);
}

int_extent_3d::int_extent_3d(index_type lay_begin, index_type lay_open_end, index_type row_beg, index_type row_open_end, index_type col_beg, index_type col_open_end) {
    _lay_extent = extent_type(lay_begin, lay_open_end);
    _row_extent = extent_type(row_beg, row_open_end);
    _col_extent = extent_type(col_beg, col_open_end);
}

int_extent_3d::int_extent_3d(const extent_type& lay, const extent_type& row, const extent_type& col) : _lay_extent(lay), _row_extent(row), _col_extent(col) {

}

bool int_extent_3d::is_empty() const {
    return _lay_extent.is_empty() || _row_extent.is_empty() || _col_extent.is_empty();
}

int_extent_3d::extent_type int_extent_3d::lay_extent() const {
    return _lay_extent;
}

int_extent_3d::extent_type int_extent_3d::row_extent() const {
    return _row_extent;
}

int_extent_3d::extent_type int_extent_3d::col_extent() const {
    return _col_extent;
}

int_extent_3d int_extent_3d::make_index(index_type i, index_type j, index_type k) {
    return int_extent_3d(extent_type::make_index(i), extent_type::make_index(j), extent_type::make_index(k));
}

int_extent_3d int_extent_3d::operator&(const int_extent_3d& rhs) const {
    return int_extent_3d(lay_extent() & rhs.lay_extent(), row_extent() & rhs.row_extent(), col_extent() & rhs.col_extent());
}

int_extent_3d int_extent_3d::operator|(const int_extent_3d& rhs) const {
    extent_type lay = _lay_extent | rhs._lay_extent;
    extent_type row = _row_extent | rhs._row_extent;
    extent_type col = _col_extent | rhs._col_extent;
    return int_extent_3d(lay, row, col);
}

bool int_extent_3d::operator==(const int_extent_3d& that) const {
    return _lay_extent == that._lay_extent && _row_extent == that._row_extent && _col_extent == that._col_extent;
}

bool int_extent_3d::operator!=(const int_extent_3d& that) const {
    return !operator==(that);
}

bool int_extent_3d::contains(const int_extent_3d& interval) const {
    return _lay_extent.contains(interval._lay_extent) && _row_extent.contains(interval._row_extent) && _col_extent.contains(interval._col_extent);
}

bool int_extent_3d::contains(index_type i, index_type j, index_type k) const {
    return _lay_extent.contains(i) && _row_extent.contains(j) && _col_extent.contains(k);
}

}}}
