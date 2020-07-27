// -- Schlumberger Private --

#ifndef GPM_INT_EXTENT_1D_H
#define GPM_INT_EXTENT_1D_H

#include <memory>
#include <iterator>

namespace Slb { namespace Exploration { namespace Gpm {
// Right open interval as wrapper for boost as external
class int_extent_1d {
public:
    using index_type = std::ptrdiff_t;
	using size_type = std::size_t;
    // Inline these just for convenience
    class const_iterator {
		index_type _num;
    public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = index_type;
		using difference_type = index_type;
		using pointer = const index_type *;
		using reference = const index_type &;

        explicit const_iterator(index_type num) : _num(num){}
        const_iterator& operator++() { _num = _num + 1; return *this; }
        const_iterator operator++(int) {
            const const_iterator retval = *this; ++(*this); return retval;
        }
        bool operator==(const_iterator other) const { return _num == other._num; }
        bool operator!=(const_iterator other) const { return !(*this == other); }
        reference operator*() const { return _num; }
    };
    const_iterator begin() const { return const_iterator(lower()); }
    const_iterator end() const { return const_iterator(upper()); }

    int_extent_1d();
    int_extent_1d(index_type lower, index_type upper);
    explicit int_extent_1d(size_type num);

    bool is_empty() const;

	size_type size() const; // Number of items, same as cardinality

    index_type lower() const {
        return _lower;
    }
    index_type upper() const {
        return _upper;
    }
    // Check if we are on border
    bool at_border(index_type i) const;

    // Positive numbers decrease the size of the extent
    // border > 0
    int_extent_1d make_extent_with_borders(index_type border_lower, index_type border_upper) const;
    // Negative will increase size, positive will decrease
    int_extent_1d make_extent_with_symmetric_borders(index_type row_border) const;
    int_extent_1d inner_extent(index_type row_border) const;

    static int_extent_1d make_index(index_type i);
    // Intersection
    int_extent_1d operator&(const int_extent_1d& rhs) const;
    //Union of extents
    int_extent_1d operator|(const int_extent_1d& rhs) const;

    //- left_subtract --------------------------------------------------------------
    /** subtract \c left_minuend from the \c right interval on it's left side. 
    Return the difference: The part of \c right right of \c left_minuend.
    \code
    right_over = right - left_minuend; //on the left.
    ...      d) : right
    ... c)      : left_minuend
    [c  d) : right_over
    \endcode
    */
    int_extent_1d left_subtract(const int_extent_1d& rhs) const;

    //- right_subtract -------------------------------------------------------------
    /** subtract \c right_minuend from the \c left interval on it's right side. 
    Return the difference: The part of \c left right of \c right_minuend.
    \code
    left_over = left - right_minuend; //on the right side.
    [a      ...  : left
    [b ...  : right_minuend
    [a  b)       : left_over
    \endcode
    */
    int_extent_1d right_subtract(const int_extent_1d& rhs) const;
    bool operator==(const int_extent_1d& that) const;

    bool operator !=(const int_extent_1d& that) const;

    bool contains(const int_extent_1d& interval) const;
    bool contains(index_type i) const;
    // If index is contained, return that
    // Otherwise return index that is closest in distance
	index_type nearest_index(index_type i) const;

    // If on left side, distance is negative
    // if on right side, distance is positive
    // otherwise return 0
	index_type signed_distance_to_legal_index(index_type i) const;
private:
    // Implementation to be PIMPL'ed
    index_type _lower;
	index_type _upper;
};

}}}

#endif
