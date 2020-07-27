// -- Schlumberger Private --

#include "gpm_int_extent_1d.h"
#include <boost/icl/discrete_interval.hpp>


namespace Slb { namespace Exploration { namespace Gpm {
namespace {
    typedef boost::icl::discrete_interval<int_extent_1d::index_type> extent_impl_type;
    extent_impl_type make_extent(int_extent_1d::index_type lower, int_extent_1d::index_type upper) {
        return extent_impl_type::right_open(lower, upper);
    }
    extent_impl_type make_extent(const int_extent_1d& ext) {
        return extent_impl_type::right_open(ext.lower(), ext.upper());
    }
}

int_extent_1d::int_extent_1d(index_type lower, index_type upper):
    _lower(lower),_upper(upper) {
    assert(lower <= upper);
}

int_extent_1d::int_extent_1d(size_type num): _lower(0),_upper(num) {
    assert(num >= 0);
}

int_extent_1d::int_extent_1d():_lower(0),_upper(0) {

}

bool int_extent_1d::is_empty() const {
    return boost::icl::is_empty(make_extent(*this));
}


bool int_extent_1d::at_border(index_type i) const {
    return (i == _lower || i + 1 ==_upper);
}

int_extent_1d int_extent_1d::make_extent_with_borders(index_type border_lower, index_type border_upper) const {
    return int_extent_1d(_lower + border_lower,
                         _upper + border_upper);
}

int_extent_1d int_extent_1d::make_extent_with_symmetric_borders(index_type row_border) const {
    return make_extent_with_borders(row_border, -row_border);
}

int_extent_1d int_extent_1d::inner_extent(index_type row_border) const {
    return make_extent_with_borders(row_border, -row_border);
}


int_extent_1d int_extent_1d::make_index(index_type i) {
    return int_extent_1d(i, i + 1);
}

int_extent_1d int_extent_1d::operator&(const int_extent_1d& rhs) const {
    extent_impl_type tmp =make_extent(*this) & make_extent(rhs);
    return int_extent_1d(tmp.lower(), tmp.upper());
}

int_extent_1d int_extent_1d::operator|(const int_extent_1d& rhs) const {
    extent_impl_type row = boost::icl::hull(make_extent(*this), make_extent(rhs));
    return int_extent_1d(row.lower(), row.upper());
}

int_extent_1d int_extent_1d::left_subtract(const int_extent_1d& rhs) const {
    extent_impl_type tmp = boost::icl::left_subtract(make_extent(*this), make_extent(rhs));
    return int_extent_1d(tmp.lower(), tmp.upper());
}

int_extent_1d int_extent_1d::right_subtract(const int_extent_1d& rhs) const {
    extent_impl_type tmp = boost::icl::right_subtract(make_extent(*this), make_extent(rhs));
    return int_extent_1d(tmp.lower(), tmp.upper());
}

bool int_extent_1d::operator==(const int_extent_1d& that) const {
    return make_extent(*this) == make_extent(that);
}

bool int_extent_1d::operator!=(const int_extent_1d& that) const {
    return !operator==(that);
}

bool int_extent_1d::contains(const int_extent_1d& interval) const {
    return boost::icl::contains(make_extent(*this), make_extent(interval));
}

bool int_extent_1d::contains(index_type i) const {
    return boost::icl::contains(make_extent(*this), i);
}

// assert that extent is not empty
int_extent_1d::index_type int_extent_1d::nearest_index(index_type i) const {
    if (contains(i)) {
        return i;
    }
    // Outside
    if (i < _lower) {
        return _lower;
    }
    return _upper - 1;
}

int_extent_1d::index_type int_extent_1d::signed_distance_to_legal_index(index_type i) const {
    if (contains(i)) {
        return 0;
    }
    if (i < _lower) {
        return i - _lower;
    }
    return i - (_upper - 1);
}
int_extent_1d::size_type int_extent_1d::size() const
{
    return boost::icl::size(make_extent(*this));
}

}}}
