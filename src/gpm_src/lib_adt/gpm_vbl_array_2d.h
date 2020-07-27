// -- Schlumberger Private --

#ifndef gpm_vbl_array_2d_h_
#define gpm_vbl_array_2d_h_
//:
// \file
// \brief Contains class for a templated 2d array
// \author Andrew W. Fitzgibbon, Oxford RRG
// \date   05 Aug 96
//
// \verbatim
// Modifications
// Peter Vanroose -13nov98- added copy constructor and assignment operator
// AWF 10/12/98 Added row/column store.  The old split was just too pedantic.
// PDA (Manchester) 21/03/2001: Tidied up the documentation
// \endverbatim

#include "gpm_int_extent_2d.h"
#include "gpm_array_common_defs.h"
#include <functional>
#include <cassert>
#include <array>

namespace Slb { namespace Exploration { namespace Gpm{


// we just copy this file for now, since we want a better controlled version of the matrix stuff we already have
template <class T>
class gpm_array_2d_base: public gpm_array_common_defs<2>
{
public:
    using value_type = T;
    using array_extent_type = int_extent_2d;

	enum direction {ROW, COL};

	bool is_valid_row(index_type i) const {return (i>=row_begin_) && (i<=row_end_);}
    bool is_valid_column(index_type j) const {return (j>=col_begin_) && (j<=col_end_);}
	bool is_valid_index(index_type i , index_type j) const {return is_valid_row(i) && is_valid_column(j);}
	bool has_data()const { return !empty();}
	bool empty()const {return rows_ == nullptr;}
	T const& operator() (index_type i, index_type j) const {
		assert(is_valid_index(i,j));
		return impl_at(i,j); 
	}
	T      & operator() (index_type i, index_type j) {
		assert(is_valid_index(i,j));
		return impl_at(i,j); 
	}

	void put(index_type i, index_type j, T const &x) { impl_at(i,j) = x; }
	T    get(index_type i, index_type j) const { return impl_at(i,j); }

	T const& at(index_type i, index_type j) const {
		assert(is_valid_index(i,j));
		return impl_at(i,j); 
	}
	T      & at(index_type i, index_type j)       {
		assert(is_valid_index(i,j));
		return impl_at(i,j); 
	}

	//  T const* operator[] (int i) const { return rows_[i-row_begin_]-col_begin_; }
	//  T      * operator[] (int i) { return rows_[i-row_begin_]-col_begin_; }

	bool operator==(gpm_array_2d_base<T> const &that) const {
		if (num_rows_ != that.num_rows_ || num_cols_ != that.num_cols_)
			return false;
		for (size_type i=0; i<num_rows_; ++i)
			for (size_type j=0; j<num_cols_; ++j)
				if (!( rows_[i][j] == that.rows_[i][j] )) // do not assume we have operator!=
					return false;
		return true;
	}

	//:
	bool operator!=(gpm_array_2d_base<T> const &that) const {
		return ! operator==(that);
	}

	void fill_core(T val ){
	    if ((core_row_begin_ <= core_row_end_) && (core_col_begin_ <= core_col_end_)){
		    int_extent_2d ext(core_row_begin_, core_row_end_+1, core_col_begin_, core_col_end_+1);
			fill(ext, val);
		}
	}

	//: fill with `value'
	void fill(T value) {
		assert(has_data());
		std::fill( begin(), end(), value );
	}

	void fill_border(direction dir, T val){
        assert(has_data());
		if ( dir == COL){
            fill_column(col_begin_, val);
			fill_column(col_end_, val);
		}
		else {// Should be row
			fill_row(row_begin_, val);
			fill_row(row_end_, val);
		}
	}

	void fill_edge_rows(T val){
		fill_border(ROW, val);
	}

	void fill_edge_cols(T val){
		fill_border(COL, val);
	}
	void fill_row(index_type i, T val){
		assert(is_valid_row(i));
		std::fill(rows_[i-row_begin_], rows_[i-row_begin_]+num_cols_, val);
    }
	void fill_column(index_type j, T val){
		assert(is_valid_column(j));
		for (size_type i=0; i<num_rows_; ++i){
			rows_[i][j-col_begin_] = val;
		}
    }
	void fill(const int_extent_2d& extent, T val){
		int_extent_2d res = extent & extents();
		if (!res.is_empty()){
			int_extent_2d::extent_type loc_rows=res.row_extent();
			int_extent_2d::extent_type loc_cols=res.col_extent();
			size_t len = loc_cols.size();
			auto j = loc_cols.lower();
			for(auto i = loc_rows.lower(); i < loc_rows.upper() ;++i){
				iterator start(&impl_at(i,j));
				iterator end(start + len);
				std::fill(start, end, val);
			}
		}
	}
	// fill where the data intersects
	void fill(gpm_array_2d_base<T> const &that){
		if (that.extents() == extents()){
			std::copy(that.begin(), that.end(), begin());
		}
		else{
			int_extent_2d res = that.extents() & extents();
			do_copy_by_index(that, res);
		}
	}

	// fill where the data intersects
	void fill(gpm_array_2d_base<T> const &that, const int_extent_2d& extent){
		if (that.extents() == extents() && extent == extents()){
			std::copy(that.begin(), that.end(), begin());
		}
		else{      
			int_extent_2d res = that.extents() & extents() & extent;
			do_copy_by_index(that, res);
		}
	}

	// Index ranges
	int_extent_2d extents() const {return int_extent_2d(row_extent(), col_extent());}
	int_extent_2d core_extents() const {return int_extent_2d(core_row_extent(), core_col_extent());}

	extent_type core_row_extent() const {return extent_type(core_row_begin_, core_row_end_+1);}
	extent_type core_col_extent() const {return extent_type(core_col_begin_, core_col_end_+1);}
	extent_type row_extent() const {return extent_type(row_begin_, row_end_+1);}
	extent_type col_extent() const {return extent_type(col_begin_, col_end_+1);}
	index_type row_begin() const { return row_begin_;}
	index_type row_end() const { return row_end_;}
	index_type col_begin() const { return col_begin_;}
	index_type col_end() const { return col_end_;}

	//: Return number of rows
	size_type rows() const { return num_rows_; }

	//: Return number of columns
	size_type cols() const { return num_cols_; }
	//: Return number of columns
	size_type columns() const { return num_cols_; }

	//: Return size = (number of rows) * (number of columns)
	size_type size() const { return num_rows_ * num_cols_; }

    //: Return dimensions of array
	dimension_type dimensions() const { return dimension_type{ num_rows_, num_cols_ }; }
	stride_type strides() const { return stride_type{ num_cols_, 1 }; }

    // Pointers to the data
    T*   data(){return  rows_[0];}
	T const *   data() const { return  rows_[0]; }

    typedef T       *iterator;
	iterator begin() { return rows_[0]; }
	iterator end  () { return rows_[0] + num_cols_ * num_rows_; }

    typedef T const *const_iterator;
	const_iterator begin() const { return rows_[0]; }
	const_iterator end  () const { return rows_[0] + num_cols_ * num_rows_; }

	virtual ~gpm_array_2d_base() { 
		destruct(); 
	}


protected:
	gpm_array_2d_base(){ construct();}
	gpm_array_2d_base(const gpm_array_2d_base<T>& that) {   
		if ( that.size() > 0 ){
			construct(that.extents());
			operator=(that);
		}
		else { 
			// Default constructor
			construct(); 
		}
	}
	gpm_array_2d_base& operator=(const gpm_array_2d_base<T>& that){
		if (this == &that) return *this; // Self assignment

		if (that.size() > 0 ){
			do_resize(that.extents(), false);
			fill(that);
		}
		else {
			do_clear();
		}
		return *this;
	}
	gpm_array_2d_base&  assign_rhs(const gpm_array_2d_base<T>& that){
		operator=(that);
		return *this;
	}

	inline T const& impl_at(index_type i, index_type j) const {
		return rows_[i-row_begin_][j-col_begin_]; 
	}
	inline T      & impl_at(index_type i, index_type j)       {
		return rows_[i-row_begin_][j-col_begin_]; 
	}
	void construct() {
		initialize();
	}
	void construct(size_type m, size_type n, T val=T()) {
		construct(0, m-1, 0, n-1, val);
	}

	void construct(const int_extent_2d& ext, T val=T()){
		construct(ext.row_extent(), ext.col_extent(), val);
	}

	void construct(const extent_type& m, const extent_type& n, T val=T()) {
		construct(m.lower(), m.upper()-1, n.lower(), n.upper()-1, val);
	}

	void impl_set_core_size(index_type row_begin, index_type row_end, index_type col_begin, index_type col_end){
		core_row_begin_ = row_begin;
		core_row_end_ = row_end;
		core_col_begin_ = col_begin;
		core_col_end_ = col_end;
	}
	//: Comparison
	//: change size.
	void do_resize(size_type m, size_type n, bool keep_old_data = true, T val=T()) {
		do_resize(0, static_cast<index_type>(m-1), 0, static_cast<index_type>(n-1), keep_old_data, val);
	}
	void do_resize(const int_extent_2d& ext,  bool keep_old_data = true, T val=T()) {
		do_resize(ext.row_extent(), ext.col_extent(), keep_old_data, val);
	}
	void do_resize(const extent_type& rows, const extent_type& cols, bool keep_old_data = true, T val=T()) {
		do_resize(rows.lower(), rows.upper()-1, cols.lower(), cols.upper()-1, keep_old_data, val);
	}
	void do_clear() {
		if (rows_) {
			destruct();
			construct();
		}
	}

	void do_copy_by_index(gpm_array_2d_base<T> const &that, const int_extent_2d& fill_extent){
		if (!fill_extent.is_empty()){
			int_extent_2d::extent_type loc_rows=fill_extent.row_extent();
			int_extent_2d::extent_type loc_cols=fill_extent.col_extent();
			size_t len = loc_cols.size();
			auto j = loc_cols.lower();
			for(auto i = loc_rows.lower(); i < loc_rows.upper() ;++i){
				const T* start(&that.impl_at(i,j));
				const T*  end(start + len);
				iterator out_to(&impl_at(i,j));
				std::copy(start, end, out_to);
			}
		}
	}

private:
	T** rows_;
	size_type num_rows_;
	size_type num_cols_;

	// GPM additions for offsize indexing
	index_type row_begin_;
	index_type row_end_;
	index_type col_begin_;
	index_type col_end_;

	// Additions to the core size without padding
	index_type core_row_begin_;
	index_type core_row_end_;
	index_type core_col_begin_;
	index_type core_col_end_;

	void initialize(){
		rows_ = nullptr;
		num_rows_ = 0;
		num_cols_ = 0;
		row_begin_ = 0;
		row_end_ = -1;
		col_begin_ = 0;
		col_end_ = -1;
		core_row_begin_ = 0;
		core_row_end_ = -1;
		core_col_begin_ = 0;
		core_col_end_ = -1;
	}

	void allocate_data(T val){
		size_type m = num_rows_;
		size_type n = num_cols_;
		if (m && n) {
			rows_ = new T * [m];
			T* p = new T[m * n];
			for (size_type i = 0; i < m; ++i){
				rows_[i] = p + i * n;
			}
			std::fill( p, p + m*n, val );
		}
		else {
			rows_ = 0;
		}
	}  

	void set_indexes(index_type row_begin, index_type row_end, index_type col_begin, index_type col_end){
		num_rows_ = row_end - row_begin + 1;
		num_cols_ = col_end - col_begin + 1;
		row_begin_ = row_begin;
		row_end_ = row_end;
		col_begin_ = col_begin;
		col_end_ = col_end;
		core_row_begin_ = row_begin;
		core_row_end_ = row_end;
		core_col_begin_ = col_begin;
		core_col_end_ = col_end;
	}
	void destruct() {
		if (rows_ ) {
			delete [] rows_[0];
			delete [] rows_;
		}
		initialize();
	}
	void construct(index_type row_begin, index_type row_end, index_type col_begin, index_type col_end, T val=T()){
		set_indexes(row_begin, row_end, col_begin, col_end);
		allocate_data(val);
	}    
	//: change size.
	void do_resize(index_type row_begin, index_type row_end, index_type col_begin, index_type col_end, bool keep_old_data = true, T val=T()) {
		if (!(row_begin == row_begin_ && row_end == row_end_ && col_begin == col_begin_ && col_end == col_end_)) {
			if( keep_old_data &&  (size() != 0 )){
				gpm_array_2d_base<T> tmp(*this);
				destruct();
				construct(row_begin, row_end, col_begin, col_end, val);
				fill(tmp);
			}
			else {
				destruct();
				construct(row_begin, row_end, col_begin, col_end, val);
			}
		}
	}


};
//: simple 2D array
template <class T>
class gpm_vbl_array_2d: public gpm_array_2d_base<T>
{
public:
  // Redef to make linux happy
	using base = gpm_array_2d_base<T>;
	using size_type = typename base::size_type;
    using index_type = typename base::index_type;
	using extent_type = typename base::extent_type;

  //: Default constructor
  gpm_vbl_array_2d() = default;;

  //: Construct m-by-n array.
  //gpm_vbl_array_2d(size_type m, size_type n) { base::construct(m, n);fill(T()); }

  //: Construct and fill an m-by-n array.
  gpm_vbl_array_2d(size_type m, size_type n,  T v=T()) { base::construct(m, n,v); }

  // Extents are assumed to be right open
  gpm_vbl_array_2d(const int_extent_2d& extent, T v=T()){ base::construct(extent.row_extent(), extent.col_extent(),v);}
  gpm_vbl_array_2d(const extent_type& m, const extent_type& n,  T v=T()) { base::construct(m, n,v); }

  explicit gpm_vbl_array_2d(gpm_array_2d_base<T> const &that):base(that){}
  //: Construct from a 2d array
  gpm_vbl_array_2d(gpm_vbl_array_2d<T> const &that) {
    if ( that.size() > 0 ){
      base::construct(that.extents());
      operator=(that);
    }
    else { 
      // Default constructor
      base::construct(); 
    }
  }

  //: Destructor
  ~gpm_vbl_array_2d() = default;;

  //: Assignment
  gpm_vbl_array_2d<T>& operator=(gpm_vbl_array_2d<T> const &that) {
    if (this == &that) return *this; // Self assignment

    this->assign_rhs(that);
    return *this;
  }

  void set_core_size(const int_extent_2d& extents){
	  set_core_size(extents.row_extent().lower(), extents.row_extent().upper()-1,
    extents.col_extent().lower(), extents.col_extent().upper()-1);
  }
  void set_core_size(size_type m, size_type n){
    set_core_size(0, static_cast<int>(m-1), 0, static_cast<int>(n-1));
  }

  void set_core_size( const extent_type& rows, const extent_type& cols){
	  set_core_size(rows.lower(), rows.upper()-1, cols.lower(), cols.upper()-1);
   }
  //: Comparison
  //: change size.
  void resize(size_type m, size_type n, bool keep_old_data = true,  T v=T()) {
    base::do_resize(m,n,keep_old_data, v);
  }

  void resize(const int_extent_2d& extents, bool keep_old_data = true, T val=T()){    
	  base::do_resize(extents.row_extent(), extents.col_extent(), keep_old_data, val);
  }
  void resize(const extent_type& rows, const extent_type& cols, bool keep_old_data = true, T val=T()){    
    base::do_resize(rows, cols, keep_old_data, val);
  }
  //: make as if default-constructed.
  void clear() {
    base::do_clear();
  }

 private:
  void set_core_size(index_type row_begin, index_type row_end, index_type col_begin, index_type col_end){
    base::impl_set_core_size(row_begin, row_end, col_begin, col_end);
  }

};

//: simple 2D array
template <class T>
class gpm_boundary_condition_2d : public gpm_array_index_defs {
public:
    // Redef to make linux happy
    typedef gpm_array_2d_base<T> base;

    enum BOUNDARY_COND { REPLICATE, CONSTANT, FUNCTOR };

    typedef std::function<T(index_type i, index_type j)> functor_type;
    //: Default constructor
    explicit gpm_boundary_condition_2d(const gpm_array_2d_base<T>& rhs): _holder(rhs), _bound(REPLICATE)
    {
    }

    gpm_boundary_condition_2d(const gpm_array_2d_base<T>& rhs, T const_val) : _holder(rhs), _bound(CONSTANT),
                                                                              _constant(const_val)
    {
    }

    gpm_boundary_condition_2d(const gpm_array_2d_base<T>& rhs, const functor_type& func) : _holder(rhs),
                                                                                           _bound(FUNCTOR),
                                                                                           _functor(func)
    {
    }

    T operator()(index_type i, index_type j) const
    {
        return get_val(i, j);
    }

private:
    T get_val(index_type i, index_type j) const
    {
        if (_bound == REPLICATE) {
            auto loc_i = i >= _holder.row_begin()
                            ? (i <= _holder.row_end() ? i : _holder.row_end())
                            : _holder.row_begin();
            auto loc_j = j >= _holder.col_begin()
                            ? (j <= _holder.col_end() ? j : _holder.col_end())
                            : _holder.col_begin();
            return _holder(loc_i, loc_j);
        }
        auto off_index = index_type(-1);
        if (_bound == CONSTANT) {
            auto loc_i = i >= _holder.row_begin() ? (i <= _holder.row_end() ? i : off_index) : off_index;
            auto loc_j = j >= _holder.col_begin() ? (j <= _holder.col_end() ? j : off_index) : off_index;
            if (loc_i == off_index || loc_j == off_index) {
                return _constant;
            }
            return _holder(loc_i, loc_j);
        }
        if (_bound == FUNCTOR) {
            auto loc_i = i >= _holder.row_begin() ? (i <= _holder.row_end() ? i : off_index) : off_index;
            auto loc_j = j >= _holder.col_begin() ? (j <= _holder.col_end() ? j : off_index) : off_index;
            if (loc_i == off_index || loc_j == off_index) {
                return _functor(i, j);
            }
            return _holder(loc_i, loc_j);
        }
    }

    const gpm_array_2d_base<T>& _holder;
    BOUNDARY_COND _bound;
    T _constant;
    functor_type _functor;
};

}}} // Namespace
#endif // vbl_array_2d_h_
