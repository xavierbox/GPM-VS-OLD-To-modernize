// -- Schlumberger Private --

#include "gpm_vbl_array_2d.h"
#include "libgp.h"
#include "gpm_2darray_functions.h"
#include <boost/range/iterator_range.hpp>
#include <boost/range/algorithm/transform.hpp>
#include <ctime>
#include <iostream>
using namespace Slb::Exploration::Gpm;

bool has_same_value(const float_2darray_type& c, float val, float rel_diff) {
    int_extent_2d::extent_type row_extent = c.row_extent();
    int_extent_2d::extent_type col_extent = c.col_extent();
    bool isOk = true;

    for (float_2darray_type::const_iterator it = c.begin(); it != c.end(); ++it) {
        float f1_loc = *it;
        float diff = f1_loc - val;
        isOk = isOk && std::abs(diff) < rel_diff;
    }
    return isOk;
}

int grid_mult_functor_test(int argc, char* argv[]) {
    float_2darray_type a(int_extent_2d(0, 2100, 0, 2000));
    float_2darray_type b(int_extent_2d(0, 2100, 0, 2000));
    float_2darray_type c(int_extent_2d(0, 2100, 0, 2000));
    a.fill(2);
    b.fill(3);
    gpm_transform(std::multiplies<float>(), a, b, &c);
    return has_same_value(c, 6, 1e-3f) ? 0 : 1;

}

int grid_mult_test(int argc, char* argv[]) {
    float_2darray_type a(int_extent_2d(0, 2100, 0, 2000));
    float_2darray_type b(int_extent_2d(0, 2100, 0, 2000));
    float_2darray_type c(int_extent_2d(0, 2100, 0, 2000));
    a.fill(2);
    b.fill(3);
    gpm_multiply(a, b, &c);
    return has_same_value(c, 6, 1e-3f) ? 0 : 1;
}

int grid_mult2_test(int argc, char* argv[]) {
    float_2darray_type a(int_extent_2d(0, 2100, 0, 2000));
    float_2darray_type b(int_extent_2d(0, 2100, 0, 2000));
    float_2darray_type c(int_extent_2d(0, 2100, 0, 2000));
    a.fill(2);
    b.fill(3);
    float_2darray_type::iterator c_it = c.begin();
    float_2darray_type::const_iterator b_it = b.begin();
    float_2darray_type::const_iterator a_it = a.begin();
    for (; a_it != a.end(); ++a_it , ++b_it , ++c_it) {
        float tmp1 = *a_it;
        float tmp2 = *b_it;
        float res = tmp1 * tmp2;
        *c_it = res;
    }
    return has_same_value(c, 6, 1e-3f) ? 0 : 1;
}

int grid_filter_test(int argc, char* argv[]) {
    // 
    float_2darray_type f1(int_extent_2d(0, 21, -1, 20));
    float_2darray_type f2(int_extent_2d(0, 21, -1, 20));
    int_extent_2d tmp = f1.extents();
    int_extent_2d::extent_type row_extent = tmp.row_extent();
    int_extent_2d::extent_type col_extent = tmp.col_extent();
    f1.fill(1);
    f2.fill(0);
    grid_filter(f2, f1, 1);
    bool isOk = true;
    for (auto i = row_extent.lower(); i < row_extent.upper(); ++i) {
        for (auto j = col_extent.lower(); j < col_extent.upper(); ++j) {
            float f1_loc = f1(i, j);
            float f2_loc = f2(i, j);
            float diff = f2_loc - f1_loc;
            isOk = isOk && std::abs(diff) < 1e-3;
        }
    }

    //gpm_vbl_array_1d<int> i1;
    //std::vector<float> container(10,2);
    //gpm_vbl_array_1d<float> f2(10,2);
    //gpm_vbl_array_1d<float> c2(container);
    //gpm_vbl_array_1d<float> a2(&(container[0]), container.size());
    //bool is1=f2==c2;
    //bool is2=f2==a2;
    return isOk ? 0 : 1;
}

int grid_timing_test(int argc, char* argv[]) {

    const int x_size = 400;
    const int y_size = 400;
    const int num_size = 10000;
    float_2darray_type a(y_size, x_size, 1);
    float_2darray_type b(y_size, x_size, 2);
    float_2darray_type c(y_size, x_size, 3);


    double start = clock();
    for (int num = 0; num < num_size; ++num) {
        gpm_transform(std::multiplies<float>(), a, b, &c);
        a.fill(1);
    }
    double current = clock();
    double duration = (double)(current - start) / CLOCKS_PER_SEC;
    std::cout << "binary took " << duration << " seconds" << "\n";
    start = clock();

    for (int num = 0; num < num_size; ++num) {
        gpm_multiply(a, b, &c);
        a.fill(1);
    }

    current = clock();
    duration = (double)(current - start) / CLOCKS_PER_SEC;
    std::cout << "mult took " << duration << " seconds" << "\n";
    typedef boost::iterator_range<float_2darray_type::const_iterator> icrange;
    typedef boost::iterator_range<float_2darray_type::iterator> irange;
    start = clock();
    double sum = 0;
    for (int num = 0; num < num_size; ++num) {
        icrange first = boost::make_iterator_range(a.begin(), a.end());
        icrange sec = boost::make_iterator_range(b.begin(), b.end());
        boost::transform(first, sec, c.begin(), std::multiplies<float>());
        a.fill(1);
        sum += 1;
    }
    current = clock();
    duration = (double)(current - start) / CLOCKS_PER_SEC;
    std::cout << "transform took " << duration << " seconds" << "\n";

    start = clock();
    for (int num = 0; num < num_size; ++num) {
        gpm_binary_sub_range(std::multiplies<float>(), a, b, a.extents(), &c);
        a.fill(1);
    }
    current = clock();
    duration = (double)(current - start) / CLOCKS_PER_SEC;
    std::cout << "binary full range took " << duration << " seconds" << "\n";

    start = clock();
    int_extent_2d extent = a.extents();
    for (int num = 0; num < num_size; ++num) {
        if (a.extents() == extent && b.extents() == extent) {
            std::transform(a.begin(), a.end(), b.begin(), c.begin(), std::multiplies<float>());
            a.fill(1);
        }
    }
    current = clock();
    duration = (double)(current - start) / CLOCKS_PER_SEC;
    std::cout << "std transform took " << duration << " seconds" << "\n";

    start = clock();
    for (int num = 0; num < num_size; ++num) {
        gpm_transform(std::multiplies<float>(), a, 10.0F, a.extents(), &c);
        a.fill(1);
    }
    current = clock();
    duration = (double)(current - start) / CLOCKS_PER_SEC;
    std::cout << "transform with constant took " << duration << " seconds" << "\n";

    return 0;
}
/*
The following code inverts the matrix input using LU-decomposition with backsubstitution of unit vectors. Reference: Numerical Recipies in C, 2nd ed., by Press, Teukolsky, Vetterling & Flannery.
you can solve Ax=b using three lines of ublas code:
permutation_matrix<> piv;
lu_factorize(A, piv);
lu_substitute(A, piv, x);
*/
#ifndef INVERT_MATRIX_HPP
#define INVERT_MATRIX_HPP

// REMEMBER to update "lu.hpp" header includes from boost-CVS
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/vector_proxy.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/triangular.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <boost/numeric/ublas/io.hpp>

namespace ublas = boost::numeric::ublas;

/* Matrix inversion routine.
Uses lu_factorize and lu_substitute in uBLAS to invert a matrix */
template<class T>
bool InvertMatrix(const ublas::matrix<T>& input, ublas::matrix<T>& inverse) {
    using namespace boost::numeric::ublas;
    typedef permutation_matrix<std::size_t> pmatrix;
    // create a working copy of the input
    matrix<T> A(input);
    // create a permutation matrix for the LU-factorization
    pmatrix pm(A.size1());

    // perform LU-factorization
    int res = lu_factorize(A, pm);
    if (res != 0) return false;

    // create identity matrix of "inverse"
    inverse.assign(ublas::identity_matrix<T>(A.size1()));

    // backsubstitute to get the inverse
    lu_substitute(A, pm, inverse);

    return true;
}

#endif //INVERT_MATRIX_HPP
int blas_invert_test(int argc, char* argv[])
{
    ublas::matrix<float> tmp(4, 4);
    ublas::matrix<float> res(4, 4);
    tmp = ublas::zero_matrix<float>(4, 4);
    tmp(0, 0) = tmp(0, 1) = tmp(0, 2) = 1;
    tmp(1, 1) = 3; tmp(1, 2) = 1; tmp(1, 3) = 2;
    tmp(2, 0) = 2; tmp(2, 1) = 3; tmp(2, 2) = 1;
    tmp(3, 0) = 1; tmp(3, 2) = 2; tmp(3, 3) = 1;
    if (InvertMatrix(tmp, res)) {
        std::cout << res << std::endl;
        return 0;
    }
    return 1;
}