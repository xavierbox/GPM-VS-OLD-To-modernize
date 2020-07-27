// -- Schlumberger Private --


#ifndef gpm_vbl_vector_array_3d_h_
#define gpm_vbl_vector_array_3d_h_

#include "gpm_vbl_array_2d.h"
#include "gpm_int_extent_3d.h"
#include "gpm_simple_stable_vector.h"
#include <array>
#include <cassert>
#include <algorithm>

namespace Slb { namespace Exploration { namespace Gpm {
// we just copy this file for now, since we want a better controlled version of the matrix stuff we already have


//: simple 3D array base with no copy/assign just to avoid that issue

template <class T>
class gpm_array_3d_vec_base : public gpm_array_common_defs<3> {
public:
	using value_type = T;
    using array_2d_type = gpm_array_2d_base<T>;


    //: Comparison
    bool operator==(gpm_array_3d_vec_base<T> const& that) const
    {
        if (num_layers_ != that.num_layers_ || num_rows_ != that.num_rows_ || num_cols_ != that.num_cols_)
            return false;
        for (size_type i = 0; i < num_layers_; ++i)
            if (!(arr_[i] == that.arr_[i])) // do not assume we have operator!=
                return false;
        return true;
    }

    //:
    bool operator!=(gpm_array_3d_vec_base<T> const& that) const
    {
        return ! operator==(that);
    }

    //: fill with `value'
    void fill(T value)
    {
        for (size_type l = 0; l < num_layers_; ++l)
            arr_[l].fill(value);
    }

    // fill where the data intersects
    void fill(gpm_array_3d_vec_base<T> const& that)
    {
        auto min_l = std::max(that.layer_begin_, layer_begin_);
        auto max_l = std::min(that.layer_end_, layer_end_);
        auto min_i = std::max(that.row_begin_, row_begin_);
        auto max_i = std::min(that.row_end_, row_end_);
        auto min_j = std::max(that.col_begin_, col_begin_);
        auto max_j = std::min(that.col_end_, col_end_);
        for (auto l = min_l; l <= max_l; ++l)
            for (auto i = min_i; i <= max_i; ++i) {
                for (auto j = min_j; j <= max_j; ++j) {
                    (*this)(l, i, j) = that(l, i, j);
                }
            }
    }

    bool is_valid_index(index_type i, index_type j, index_type k) const
    {
        return (i >= layer_begin_) && (i <= layer_end_) && (j >= row_begin_) && (j <= row_end_) && (k >= col_begin_) &&
            (k <= col_end_);
    }

    // Data Access---------------------------------------------------------------
    // These are the only legal to index according to the begin end setup
    T const& operator()(index_type i, index_type j, index_type k) const
    {
        assert(i>=layer_begin_);
        assert(i<=layer_end_);
        return arr_[i - layer_begin_](j, k);
    }

    T& operator()(index_type i, index_type j, index_type k)
    {
        assert(i>=layer_begin_);
        assert(i<=layer_end_);
        return arr_[i - layer_begin_](j, k);
    }

    T const& at(index_type i, index_type j, index_type k) const
    {
        assert(i>=layer_begin_);
        assert(i<=layer_end_);
        return arr_[i - layer_begin_](j, k);
    }

    T& at(index_type i, index_type j, index_type k)
    {
        assert(i>=layer_begin_);
        assert(i<=layer_end_);
        return arr_[i - layer_begin_](j, k);
    }

    //void put(int i, int j, T const &x) { arr_[i-row_begin_][j][k] = x; }
    //T    get(int i, int j) const { return arr_[i-row_begin_][j][k]; }

    const array_2d_type& operator[](index_type i) const { return arr_[i - layer_begin_]; }
    array_2d_type& operator[](index_type i) { return arr_[i - layer_begin_]; }

    const array_2d_type& operator()(index_type i) const
    {
        assert(i>=layer_begin_);
        assert(i<=layer_end_);
        return arr_[i - layer_begin_];
    }

    array_2d_type& operator()(index_type i)
    {
        assert(i>=layer_begin_);
        assert(i<=layer_end_);
        return arr_[i - layer_begin_];
    }

    bool has_data() const { return arr_.size() != 0; }

    // Index ranges
    int_extent_3d extents() const { return int_extent_3d(layer_extent(), row_extent(), col_extent()); }

    extent_type layer_extent() const { return extent_type(layer_begin_, layer_end_ + 1); }
    extent_type row_extent() const { return extent_type(row_begin_, row_end_ + 1); }
    extent_type col_extent() const { return extent_type(col_begin_, col_end_ + 1); }

    //: Return number of rows
    size_type layers() const { return num_layers_; }

    //: Return number of rows
    size_type rows() const { return num_rows_; }

    //: Return number of columns
    size_type cols() const { return num_cols_; }

    //: Return number of columns
    size_type columns() const { return num_cols_; }

    //: Return size = (number of rows) * (number of columns)
    size_type size() const { return num_layers_ * num_rows_ * num_cols_; }

	//: Return dimensions of array
	dimension_type dimensions() const { return dimension_type{ num_layers_, num_rows_, num_cols_ }; }

    // TODO fix these 
    //T      *      * get_rows() { return rows_; }
    //T const* const* get_rows() const { return rows_; }

    //typedef T       *iterator;
    //iterator begin() { return rows_[0]; }
    //iterator end  () { return rows_[0] + num_cols_ * num_rows_; }

    //typedef T const *const_iterator;
    //const_iterator begin() const { return rows_[0]; }
    //const_iterator end  () const { return rows_[0] + num_cols_ * num_rows_; }

protected:
    //: Default constructor
    gpm_array_3d_vec_base() { construct(); }

    //: Construct m-by-n array.
    gpm_array_3d_vec_base(size_type l, size_type m, size_type n) { construct(l, m, n); }

    //: Construct and fill an m-by-n array.
    gpm_array_3d_vec_base(size_type l, size_type m, size_type n, const T& v)
    {
        construct(l, m, n);
        fill(v);
    }

    //: Construct m-by-n array.
    // begin is the first legal idex, end is the last legal index
    // End should probably been an illegal index
    //gpm_array_3d_base(int layer_begin, int layer_end, int row_begin, int row_end, int col_begin, int col_end) { construct(layer_begin, layer_end, row_begin, row_end, col_begin, col_end); }

    //: Construct and fill an m-by-n array.
    //gpm_array_3d_base(int layer_begin, int layer_end, int row_begin, int row_end, int col_begin, int col_end, const T &v) { construct(layer_begin, layer_end, row_begin, row_end, col_begin, col_end); fill(v);}

    //: Construct from a 2d array
    gpm_array_3d_vec_base(gpm_array_3d_vec_base<T> const& that)
    {
        if (that.size() > 0) {
            construct(that.extents());
            operator=(that);
        }
        else {
            construct();
        }
    }

    //: Destructor
    virtual ~gpm_array_3d_vec_base() { destruct(); }

    //: Assignment
    gpm_array_3d_vec_base<T>& operator=(gpm_array_3d_vec_base<T> const& that)
    {
        if (this == &that) return *this; // Self assignment

        if (that.size() > 0) {
            impl_resize(that.extents(), false);
            for (size_type i = 0; i < num_layers_; ++i)
                arr_[i] = that.arr_[i];
        }
        else {
            impl_clear();
        }
        return *this;
    }

    //: make as if default-constructed.
    void impl_clear()
    {
        initialize();
        arr_.clear();
    }
	void impl_clear(const typename extent_type::index_type& i)
	{
		assert(i >= layer_begin_);
		assert(i <= layer_end_);
		arr_[i - layer_begin_].clear();
	}
    //: change size.
    void impl_resize(size_type l, size_type m, size_type n, bool keep_old_data = true, T val = T())
    {
        resize(0, static_cast<int>(l - 1), 0, static_cast<int>(m - 1), 0, static_cast<int>(n - 1), keep_old_data, val);
    }

    void impl_resize(const int_extent_3d& ext, bool keep_old_data = true, T val = T())
    {
        impl_resize(ext.lay_extent(), ext.row_extent(), ext.col_extent(), keep_old_data, val);
    }

    void impl_resize(const extent_type& layers, const extent_type& rows, const extent_type& cols,
                     bool keep_old_data = true, T val = T())
    {
        resize(layers.lower(), layers.upper() - 1, rows.lower(), rows.upper() - 1, cols.lower(), cols.upper() - 1,
               keep_old_data, val);
    }

    //: change size.

    void construct()
    {
        initialize();
    }

    void construct(size_type l, size_type m, size_type n, T val = T())
    {
        construct(0, static_cast<int>(l - 1), 0, static_cast<int>(m - 1), 0, static_cast<int>(n - 1), val);
    }

    void construct(const int_extent_3d& ext, T val = T())
    {
        construct(ext.lay_extent(), ext.row_extent(), ext.col_extent(), val);
    }

    void construct(const extent_type& layers, const extent_type& rows, const extent_type& cols, T val = T())
    {
        construct(layers.lower(), layers.upper() - 1, rows.lower(), rows.upper() - 1, cols.lower(), cols.upper() - 1,
                  val);
    }

    void destruct()
    {
        impl_clear();
    }

    void initialize()
    {
        num_layers_ = 0;
        num_rows_ = 0;
        num_cols_ = 0;
        layer_begin_ = 0;
        layer_end_ = -1;
        row_begin_ = 0;
        row_end_ = -1;
        col_begin_ = 0;
        col_end_ = -1;
    }

protected: // Should really make access methods to these
    size_type num_layers() const { return num_layers_; }
    typedef gpm_vbl_array_2d<T> array_2d_impl_type;
    gpm_simple_stable_vector<array_2d_impl_type> arr_;


private:

    void resize(index_type layer_begin, index_type layer_end, index_type row_begin, index_type row_end, index_type col_begin, index_type col_end,
                bool keep_old_data = true, T val = T())
    {
        if (!(layer_begin == layer_begin_ && layer_end == layer_end_ && row_begin == row_begin_ && row_end == row_end_
            && col_begin == col_begin_ && col_end == col_end_)) {
            if (row_begin == row_begin_ && row_end == row_end_ && col_begin == col_begin_ && col_end == col_end_) {
                arr_.resize(layer_end - layer_begin + 1,
                            array_2d_impl_type(int_extent_2d(row_begin, row_end + 1, col_begin, col_end + 1), val));
                num_layers_ = layer_end - layer_begin + 1;
                layer_begin_ = layer_begin;
                layer_end_ = layer_end;
            }
            else {
                if (keep_old_data && (size() != 0)) {
                    gpm_array_3d_vec_base<T> tmp(*this);
                    destruct();
                    construct(layer_begin, layer_end, row_begin, row_end, col_begin, col_end, val);
                    fill(tmp);
                }
                else {
                    destruct();
                    construct(layer_begin, layer_end, row_begin, row_end, col_begin, col_end, val);
                }
            }
        }
    }

    void construct(index_type layer_begin, index_type layer_end, index_type row_begin, index_type row_end, index_type col_begin, index_type col_end, T val = T())
    {
        size_type l = layer_end - layer_begin + 1;
		size_type m = row_end - row_begin + 1;
		size_type n = col_end - col_begin + 1;
        num_layers_ = l;
        num_rows_ = m;
        num_cols_ = n;
        layer_begin_ = layer_begin;
        layer_end_ = layer_end;
        row_begin_ = row_begin;
        row_end_ = row_end;
        col_begin_ = col_begin;
        col_end_ = col_end;
        if (l && m && n) {
            array_2d_impl_type tmp(m, n, val);
            arr_.resize(l, tmp);
        }
        else {
            impl_clear();
        }
    }

    size_type num_layers_;
    size_type num_rows_;
    size_type num_cols_;

    // GPM additions for offsize indexing
	index_type layer_begin_;
	index_type layer_end_;
	index_type row_begin_;
	index_type row_end_;
	index_type col_begin_;
	index_type col_end_;

};

//: simple 3D array that can be created and resized
template <class T>
class gpm_vbl_vector_array_3d : public gpm_array_3d_vec_base<T> {
public:
    using base_type = gpm_array_3d_vec_base<T>;
    using size_type = typename base_type::size_type;
    using extent_type = typename base_type::extent_type;

    //: Default constructor
    gpm_vbl_vector_array_3d() { base_type::construct(); }

    //: Construct m-by-n array. public:
    //gpm_vbl_vector_array_3d(size_type l, size_type m, size_type n, T v=T()) { base_type::construct(l, m, n, v); }

    //: Construct and fill an m-by-n array.
    gpm_vbl_vector_array_3d(size_type l, size_type m, size_type n, T v = T()) { base_type::construct(l, m, n, v); }

    //: Construct m-by-n array.
    // begin is the first legal idex, end is the last legal index
    // End should probably been an illegal index
    //gpm_vbl_vector_array_3d(int layer_begin, int layer_end, int row_begin, int row_end, int col_begin, int col_end) { base_type::construct(layer_begin, layer_end, row_begin, row_end, col_begin, col_end); }

    gpm_vbl_vector_array_3d(const int_extent_3d& ext, T v = T()) { base_type::construct(ext, v); }

    gpm_vbl_vector_array_3d(const extent_type& layers, const extent_type& rows, const extent_type cols, T v = T())
    {
        base_type::construct(layers, rows, cols, v);
    }

    //: Construct and fill an m-by-n array.
    // gpm_vbl_vector_array_3d(int layer_begin, int layer_end, int row_begin, int row_end, int col_begin, int col_end, T v=T()) { base_type::construct(layer_begin, layer_end, row_begin, row_end, col_begin, col_end, v); }

    //: Construct from a 2d array
    gpm_vbl_vector_array_3d(gpm_vbl_vector_array_3d<T> const& that)
    {
        if (that.size() > 0) {
            base_type::construct(that.extents());
            base_type::operator=(that);
        }
        else {
            base_type::construct();
        }
    }

    //: Destructor
    ~gpm_vbl_vector_array_3d()
    = default;

    //: Assignment
    gpm_vbl_vector_array_3d<T>& operator=(gpm_vbl_vector_array_3d<T> const& that)
    {
        if (this == &that) return *this; // Self assignment

        if (that.size() > 0) {
            resize(that.extents(), false);
            for (size_type i = 0; i < base_type::num_layers(); ++i)
                base_type::arr_[i] = that.arr_[i];
        }
        else {
            clear();
        }
        return *this;
    }

    //: change size.
    void resize(size_type l, size_type m, size_type n, bool keep_old_data = true, T val = T())
    {
        base_type::impl_resize(l, m, n, keep_old_data, val);
    }

    void resize(const int_extent_3d& extents, bool keep_old_data = true, T val = T())
    {
        base_type::impl_resize(extents, keep_old_data, val);
    }

    void resize(const extent_type& layers, const extent_type& rows, const extent_type& cols, bool keep_old_data = true,
                T val = T())
    {
        base_type::impl_resize(layers, rows, cols, keep_old_data, val);
    }

    //: change size.
    //  void resize(int layer_begin, int layer_end, int row_begin, int row_end, int col_begin, int col_end, bool keep_old_data = true, T val=T()) {
    //    base_type::resize(layer_begin, layer_end, row_begin, row_end, col_begin, col_end, keep_old_data, val);
    //   }

    //: make as if default-constructed.
    void clear()
    {
        base_type::impl_clear();
    }
    // Release memory of array
    // Simple way of reducing memory if it is known it will not be used again
    // BUT USE WITH CARE
    void clear(const typename extent_type::index_type& i)
    {
		base_type::impl_clear(i);
    }
};

}}} // Namespace
#endif
