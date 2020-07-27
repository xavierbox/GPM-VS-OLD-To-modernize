#ifndef GPM_ARMA_H
#define GPM_ARMA_H

#include <armadillo>
#include "gpm_basic_defs.h"
#include <gpm_int_extent_3d.h>
#include <gpm_int_extent_2d.h>

namespace Slb {
	namespace Exploration {
		namespace Gpm {

			namespace gpm_arma {

				template<typename eT, int capacity>
				struct tinyvec {
					eT mem[capacity];
					int nel;

                    GPM_DO_INLINE tinyvec()
                    {
                        nel = 0;
                    }
                    GPM_DO_INLINE explicit tinyvec(int n) {
                        assert(n <= capacity);
                        nel = n;
                        for (int is = 0; is < n; is++)
                            mem[is] = eT();
                    }

                    GPM_DO_INLINE tinyvec(int n, eT f) {
						assert(n <= capacity);
						nel = n;
						for (int is = 0; is < n; is++)
							mem[is] = f;
					}

					GPM_DO_INLINE eT& operator[] (const int i) {
						assert(i >= 0 && i < nel);
						return mem[i];
					}

					GPM_DO_INLINE const eT& operator[] (const int i) const {
						assert(i >= 0 && i < nel);
						return mem[i];
					}

					GPM_DO_INLINE eT* begin() {
						return mem;
					}

					GPM_DO_INLINE eT* end() {
						return mem + nel;
					}

					GPM_DO_INLINE int size() const {
						return nel;
					}
                    GPM_DO_INLINE void resize(int n) {
                        assert(n <= capacity);
                        int old_nen = nel;
                        nel = n;
                        for (int is = old_nen; is < n; is++)
                            mem[is] = eT();
                    }
				};

				

				//since GPM uses row-dominant ordering, make a small wrapper around armadillo

				template<typename eT>
				struct Mat {
					arma::Mat<eT> d;

					GPM_DO_INLINE Mat() {}
					GPM_DO_INLINE Mat(const int nr, const int nc) : d(nc, nr) {};
					template<typename fT>
					GPM_DO_INLINE Mat(const int nr, const int nc, const arma::fill::fill_class<fT>& f) : d(nc, nr, f) {};
					GPM_DO_INLINE Mat(eT* mem, int nr, int nc, bool copy = true) : d(mem, nc, nr, copy) {}

					GPM_DO_INLINE const eT& operator() (const int i, const int j) const {
						return d(j, i);
					}

					GPM_DO_INLINE eT& operator() (const int i, const int j) {
						return d(j, i);
					}

					GPM_DO_INLINE arma::uword n_rows() const {
						return d.n_cols;
					}

					GPM_DO_INLINE arma::uword n_cols() const {
						return d.n_rows;
					}

					GPM_DO_INLINE eT* memptr() const {
						return const_cast<eT*>(d.memptr());
					}

               GPM_DO_INLINE void set_size(int nr, int nc) {
                  d.set_size(nc, nr);
               }

               GPM_DO_INLINE void bind(gpm_arma::Mat<eT>& a) {
                  d.clear();
                  arma::Mat<eT> a_(a.memptr(), a.n_cols(), a.n_rows(), false);
                  memcpy(&d, &a_, sizeof(a_));
               }

               GPM_DO_INLINE void bind(eT* mem, int nrow, int ncol) {
                  d.clear();
                  arma::Mat<eT> a_(mem, ncol, nrow, false);
                  memcpy(&d, &a_, sizeof(a_));
               }

               GPM_DO_INLINE void bind(const gpm_vbl_array_2d<eT>& a) {
                  bind((eT*)&a(int(a.row_begin()), int(a.col_begin())), int(a.rows()), int(a.cols()));
               }

               GPM_DO_INLINE void bind(const gpm_array_2d_base<eT>& a) {
                  bind((eT*)&a(int(a.row_begin()), int(a.col_begin())), int(a.rows()), int(a.cols()));
               }

               
				};

				typedef Mat<float> fmat;
				typedef Mat<int> imat;

				template<typename eT>
				struct Cube {
					arma::Cube<eT> d;

					Cube() {}

					Cube(eT* mem, int nd, int nr, int nc, bool copy = true)
						: d(mem, nd, nc, nr, copy)
					{
				
					}
                    explicit Cube(const int_extent_3d& extent)
                        : d(extent.col_extent().upper()- extent.col_extent().lower(), extent.row_extent().upper() - extent.row_extent().lower(), extent.lay_extent().upper() - extent.lay_extent().lower())
                    {

                    }
                    Cube(const int_extent_3d& extent, eT init_val)
                        : d(extent.col_extent().upper() - extent.col_extent().lower(), extent.row_extent().upper() - extent.row_extent().lower(), extent.lay_extent().upper() - extent.lay_extent().lower())
                    {
                        fill(init_val);
                    }

                    GPM_DO_INLINE void fill(eT val) { d.fill(val); }
                    GPM_DO_INLINE void fill(const Cube<eT>& rhs) { d = rhs.d; }
               //GPM_DO_INLINE void bind(eT* mem, int ndepth, int nrows, int ncol) {
               //   d.clear();
               //   arma::Cube<eT> a_(mem, ndepth, ncol, nrows, false);
               //   matlib::swap(d, a_, true);
               //}

               GPM_DO_INLINE void bind(const gpm_arma::Cube<eT>& a_) {
                  bind(a_.memptr(), a_.n_depth(), a_.n_rows(), a_.n_cols());
               }

					GPM_DO_INLINE const eT& operator() (const int k, const int i, const int j) const {
						return d(k, j, i);
					}

					GPM_DO_INLINE eT& operator() (const int k, const int i, const int j) {
						return d(k, j, i);
					}

               //GPM_DO_INLINE eT& operator() (const int i, const int j) {
               //   return d(i + j * n_depth());
               //}
               //
               //GPM_DO_INLINE const eT& operator() (const int i, const int j) const {
               //   return d(i + j * n_depth());
               //}

                    GPM_DO_INLINE lin_span<eT> operator() (const int i, const int j) {
                        lin_span<eT> r;
                        r.nel = int(d.n_rows);

                        r.mem = (&((*this)(0, i, j)));
                        return r;
                    }
                    GPM_DO_INLINE lin_span<eT const> operator() (const int i, const int j) const {
                        lin_span<eT const> r;
						r.nel = int(d.n_rows);

						r.mem = const_cast<float*>(&((*this)(0, i, j)));
						return r;
					}

					GPM_DO_INLINE arma::uword n_rows() const {
						return d.n_slices;
					}

					GPM_DO_INLINE arma::uword n_cols() const {
						return d.n_cols;
					}

					GPM_DO_INLINE arma::uword n_depth() const {
						return d.n_rows;
					}

					GPM_DO_INLINE eT* memptr() const {
						return const_cast<eT*>(d.memptr());
					}

					GPM_DO_INLINE eT* data() const {
						return const_cast<eT*>(d.memptr());
					}

                    GPM_DO_INLINE bool in_range(int i, int j) const
					{
                        if (i >= 0 && j >= 0) {
                            return d.in_range(0, j, i);
                        }
                        return false;
					}
                    inline void resize(int nseds, int ncols, int nrows)
					{
                        d.resize(nseds, ncols, nrows);
					}
					inline void resize(const int_extent_2d& extent, int ns) {
						resize(extent.row_extent(), extent.col_extent(), int_extent_1d(0, ns));
					}
                    inline void resize(const int_extent_1d& row, const int_extent_1d& col, const int_extent_1d& ns) {
                        int ncol = int(col.upper() - col.lower());
                        int nrow = int(row.upper() - row.lower());
                        int nc = int(ns.upper() - ns.lower());
                        resize(nc, ncol, nrow);
                    }
                    //inline void resize(const int_extent_1d& row, const int_extent_1d& col, const int_extent_1d& ns, bool dummy, eT init_val) {
                    //    resize(row, col, ns);
                    //    d.fill(init_val);
                    //}
                    inline void resize(const int_extent_3d& extent) {
                        resize(extent.lay_extent(), extent.row_extent(), extent.col_extent());
                    }
                    inline void resize(const int_extent_3d& extent, eT init_val) {
                        resize(extent);
                        d.fill(init_val);
                    }
     //               template<int max_num>
					//inline void resize(const int_extent_2d& extent, int ns, const boost::container::static_vector<eT, max_num>& s) {
					//	
					//	resize(extent, ns);

					//	for (arma::uword ic = 0; ic < n_cols(); ic++)
					//		for (arma::uword ir = 0; ir < n_rows(); ir++)
					//			for (int is = 0; is < ns; is++)
					//				(*this)(is, ir, ic) = s[is];
					//}

					template<int n>
					inline void fill(const tinyvec<eT, n>& s) {

						for (arma::uword ic = 0; ic < n_cols(); ic++)
							for (arma::uword ir = 0; ir < n_rows(); ir++)
								for (arma::uword is = 0; is < n_depth(); is++)
									(*this)(is, ir, ic) = s[is];
					}
                    inline bool empty() const { return d.empty(); }
				};

				typedef Cube<float> fcube;

            template<typename eT>
            GPM_DO_INLINE void bind(arma::Col<eT>& out, const arma::Col<eT>& in) {
               arma::Col<eT> a_((eT*)in.memptr(), in.n_elem, false);
               memcpy(&out, &a_, sizeof(a_));
            }

            template<typename eT>
            GPM_DO_INLINE void bind(arma::Mat<eT>& out, const arma::Mat<eT>& in) {
               arma::Mat<eT> a_((eT*)in.memptr(), in.n_rows, in.n_cols, false);
               memcpy(&out, &a_, sizeof(a_));
            }
			}

         //typedef gpm_arma::col_iterator<float> sediment_mix_iterator_opt;
         //typedef gpm_arma::fcube sediment_layer_type_opt;
         //typedef gpm_arma::tinyvec<float, max_num_sediments> sediment_mix_type_opt;

		}
	}
}


#endif

