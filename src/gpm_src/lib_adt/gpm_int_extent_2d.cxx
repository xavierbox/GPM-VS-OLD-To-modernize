// -- Schlumberger Private --

#include "gpm_int_extent_2d.h"
#include <boost/icl/discrete_interval.hpp>
#include <boost/icl/split_interval_set.hpp>

namespace Slb { namespace Exploration { namespace Gpm {namespace {
// Some boost helpers
typedef boost::icl::discrete_interval<int_extent_2d::index_type> extent_type;

extent_type convert_extent_to_boost(const int_extent_1d& val) {
    return extent_type::right_open(val.lower(), val.upper());
}

int_extent_1d convert_extent_from_boost(const extent_type& val) {
    return int_extent_1d(val.lower(), val.upper());
}
}

int_extent_2d::int_extent_2d() = default;

int_extent_2d::int_extent_2d(index_type row_beg, index_type row_open_end, index_type col_beg, index_type col_open_end) {
    _row_extent = extent_type(row_beg, row_open_end);
    _col_extent = extent_type(col_beg, col_open_end);
}

int_extent_2d::int_extent_2d(size_type num_rows, size_type num_cols) {
    _row_extent = extent_type(num_rows);
    _col_extent = extent_type(num_cols);
}

int_extent_2d::int_extent_2d(const extent_type& row, const extent_type& col) : _row_extent(row), _col_extent(col) {

}

bool int_extent_2d::is_empty() const {
    return _row_extent.is_empty() || _col_extent.is_empty();
}

int_extent_2d::size_type int_extent_2d::size() const
{
	return _row_extent.size()*_col_extent.size();
}

int_extent_2d::extent_type int_extent_2d::row_extent() const {
    return _row_extent;
}

int_extent_2d::extent_type int_extent_2d::col_extent() const {
    return _col_extent;
}

bool int_extent_2d::at_row_border(index_type i) const {
    return (i == _row_extent.lower() || i + 1 == _row_extent.upper());
}

bool int_extent_2d::at_col_border(index_type j) const {
    return (j == _col_extent.lower() || j + 1 == _col_extent.upper());
}

int_extent_2d int_extent_2d::make_extent_with_borders(index_type row_border_lower, index_type row_border_upper, index_type col_border_lower, index_type col_border_upper) const {
    return int_extent_2d(_row_extent.lower() + row_border_lower,
                         _row_extent.upper() + row_border_upper,
                         _col_extent.lower() + col_border_lower,
                         _col_extent.upper() + col_border_upper);
}

int_extent_2d int_extent_2d::make_extent_with_symmetric_borders(index_type row_border, index_type col_border) const {
    return make_extent_with_borders(row_border, -row_border, col_border, -col_border);
}

int_extent_2d int_extent_2d::inner_extent(index_type row_border, index_type col_border) const {
    return make_extent_with_borders(row_border, -row_border, col_border, -col_border);
}

std::vector<int_extent_2d> int_extent_2d::border_extents(index_type row_border, index_type col_border) {
    assert(row_border > 0);
    assert(col_border > 0);
    assert(_row_extent.upper()-_row_extent.lower()>2*row_border);
    assert(_col_extent.upper()-_col_extent.lower()>2*col_border);
    int_extent_2d inner_range = inner_extent(row_border, col_border);
    boost::icl::split_interval_set<index_type> split_rows;
    boost::icl::split_interval_set<index_type> split_cols;
    split_rows += convert_extent_to_boost(row_extent());
    split_rows += convert_extent_to_boost(inner_range.row_extent());
    split_cols += convert_extent_to_boost(col_extent());
    split_cols += convert_extent_to_boost(inner_range.col_extent());
    std::vector<int_extent_2d> outers;
    typedef std::pair<boost::icl::split_interval_set<int>::iterator, boost::icl::split_interval_set<int>::iterator> my_pair;
    auto row_iter = split_rows.lower_bound(convert_extent_to_boost(inner_range.row_extent()));
    auto col_iter = split_cols.lower_bound(convert_extent_to_boost(inner_range.col_extent()));
    for (auto a_row = split_rows.begin(); a_row != split_rows.end(); ++a_row) {
        for (auto a_col = split_cols.begin(); a_col != split_cols.end(); ++a_col) {
            int_extent_2d tmp(convert_extent_from_boost(*a_row), convert_extent_from_boost(*a_col));
            if (!(row_iter == a_row && col_iter == a_col)) {
                outers.push_back(tmp);
            }
        }
    }
    return outers;
}

std::vector<int_extent_2d> int_extent_2d::radius_extent(index_type i_o, index_type j_o, index_type radius) const
{
    if (contains(i_o, j_o)) {
        std::vector<int_extent_2d> res;
        if (_row_extent.contains(i_o - radius)) {
            const int_extent_2d bottom(int_extent_1d::make_index(i_o - radius), int_extent_1d(j_o - radius, j_o + radius+1));
            res.push_back(*this & bottom);
        }
        if (_row_extent.contains(i_o + radius)) {
            const int_extent_2d top(int_extent_1d::make_index(i_o + radius), int_extent_1d(j_o - radius, j_o + radius+1));
            res.push_back(*this & top);
        }
        if (_col_extent.contains(j_o - radius)) {
            const int_extent_2d left_side(int_extent_1d(j_o - radius + 1, j_o + radius), int_extent_1d::make_index(j_o - radius));
            res.push_back(*this & left_side);
        }
        if (_col_extent.contains(j_o + radius)) {
            const int_extent_2d right_side(int_extent_1d(j_o - radius + 1, j_o + radius), int_extent_1d::make_index(j_o + radius));
            res.push_back(*this & right_side);
        }
        return res;
    }
	return std::vector<int_extent_2d>();
}

int_extent_2d int_extent_2d::make_index(index_type i, index_type j) {
    return int_extent_2d(extent_type::make_index(i), extent_type::make_index(j));
}

int_extent_2d int_extent_2d::operator&(const int_extent_2d& rhs) const {
    return int_extent_2d(row_extent() & rhs.row_extent(), col_extent() & rhs.col_extent());
}

int_extent_2d int_extent_2d::operator|(const int_extent_2d& rhs) const {
    extent_type row = _row_extent | rhs._row_extent;
    extent_type col = _col_extent | rhs._col_extent;
    return int_extent_2d(row, col);
}

int_extent_2d int_extent_2d::left_subtract(const int_extent_2d& rhs) const {
    extent_type row = _row_extent.left_subtract(rhs._row_extent);
    extent_type col = _col_extent.left_subtract(rhs._col_extent);
    return int_extent_2d(row, col);
}

int_extent_2d int_extent_2d::right_subtract(const int_extent_2d& rhs) const {
    extent_type row = _row_extent.right_subtract(rhs._row_extent);
    extent_type col = _col_extent.right_subtract(rhs._col_extent);
    return int_extent_2d(row, col);
}

bool int_extent_2d::operator==(const int_extent_2d& that) const {
    return row_extent() == that.row_extent() && col_extent() == that.col_extent();
}

bool int_extent_2d::operator!=(const int_extent_2d& that) const {
    return !operator==(that);
}

bool int_extent_2d::contains(const int_extent_2d& interval) const {
    return _row_extent.contains(interval._row_extent) && _col_extent.contains(interval._col_extent);
}

bool int_extent_2d::contains(index_type i, index_type j) const {
    return _row_extent.contains(i) && _col_extent.contains(j);
}

}}}
