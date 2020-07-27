// -- Schlumberger Private --

#ifndef GPM_2DARRAY_FUNCTIONS_H
#define GPM_2DARRAY_FUNCTIONS_H
#include "gpm_vbl_array_2d.h"
#include "gpm_basic_defs.h"
#include <algorithm>
#include <functional>

namespace Slb { namespace Exploration { namespace Gpm {

template <typename T, typename Func1, typename Pred1>
void gpm_transform_replace_if(const Func1& f1, const gpm_array_2d_base<T>& a, const gpm_array_2d_base<T>& b, const int_extent_2d& extent, const Pred1& p1, T new_val, gpm_array_2d_base<T>* out) {
    assert(b.extents().contains(extent));
    assert(a.extents().contains(extent));
    assert(out->extents().contains(extent));
    int_extent_2d::extent_type row_ext = extent.row_extent();
    int_extent_2d::extent_type col_ext = extent.col_extent();
    for (auto i = row_ext.lower(); i < row_ext.upper(); i++) {
        for (auto j = col_ext.lower(); j < col_ext.upper(); j++) {
            T res = f1(a(i, j), b(i, j));
            if (p1(res)) {
                res = new_val;
            }
            out->at(i, j) = res;
        }
    }
}

template <typename T, typename Func1, typename Pred1>
void gpm_transform_replace_if(const Func1& f1, const gpm_array_2d_base<T>& b, const int_extent_2d& extent, const Pred1& p1, T new_val, gpm_array_2d_base<T>* out) {
    assert(b.extents().contains(extent));
    assert(out->extents().contains(extent));
    int_extent_2d::extent_type row_ext = extent.row_extent();
    int_extent_2d::extent_type col_ext = extent.col_extent();
    for (auto i = row_ext.lower(); i < row_ext.upper(); i++) {
        for (auto j = col_ext.lower(); j < col_ext.upper(); j++) {
            T res = f1(b(i, j));
            if (p1(res)) {
                res = new_val;
            }
            out->at(i, j) = res;
        }
    }
}


template <typename T>
void gpm_subtract_replace_if_less(T top, const gpm_array_2d_base<T>& base, T level, gpm_array_2d_base<T>* out) {
    gpm_transform_replace_if(std::bind(std::minus<T>(), top, std::placeholders::_1), base, out->extents(), std::bind(std::less<T>(), std::placeholders::_1, level), level, out);
}

template <typename T>
void gpm_add_replace_if_less(const gpm_array_2d_base<T>& top, const gpm_array_2d_base<T>& base, T level, gpm_array_2d_base<T>* out) {
    gpm_transform_replace_if(std::plus<T>(), top, base, out->extents(), std::bind(std::less<T>(), std::placeholders::_1, level), level, out);
}

// Do function_ab on a and b
// do function_c_ab on c and result of func_a_b
// ex d = a + b - c => d= func_ab_c(func_a_b(a,b),c)
template <typename T, typename Func1, typename Func2>
void gpm_ternary_full_range(const Func1& func_ab_c,
                            const Func2& func_a_b,
                            const gpm_array_2d_base<T>& a,
                            const gpm_array_2d_base<T>& b,
                            const gpm_array_2d_base<T>& c,
                            gpm_array_2d_base<T>* d) {
    const T* a_first = a.begin();
    const T* a_last = a.end();
    const T* b_first = b.begin();
    const T* c_first = c.begin();
    T* out = d->begin();

    for (; a_first != a_last; ++a_first , ++b_first , ++c_first) {
        T a_b = func_a_b(*a_first, *b_first);
        *out = func_ab_c(a_b, *c_first);
        ++out;
    }
}

template <typename T, typename Func1, typename Func2>
void gpm_ternary_sub_range(const Func1& func_ab_c,
                           const Func2& func_a_b,
                           const gpm_array_2d_base<T>& a,
                           const gpm_array_2d_base<T>& b,
                           const gpm_array_2d_base<T>& c,
                           const int_extent_2d& extent,
                           gpm_array_2d_base<T>* d) {
    int_extent_2d::extent_type row_ext = extent.row_extent();
    int_extent_2d::extent_type col_ext = extent.col_extent();
    size_t len = col_ext.size();
    auto j = col_ext.lower();
    for (auto i = row_ext.lower(); i < row_ext.upper(); i++) {
        const T* a_first = &a(i, j);
        const T* a_last = a_first + len;
        const T* b_first = &b(i, j);
        const T* c_first = &c(i, j);
        T* out = &(d->at(i, j));

        for (; a_first != a_last; ++a_first , ++b_first , ++c_first) {
            T a_b = func_a_b(*a_first, *b_first);
            *out = func_ab_c(a_b, *c_first);
            ++out;
        }
    }
}

template <typename T, typename Func1, typename Func2>
void gpm_ternary_full_range(const Func1& func_ab_c,
                            const Func2& func_a_b,
                            const gpm_array_2d_base<T>& a,
                            const gpm_array_2d_base<T>& b,
                            T c,
                            gpm_array_2d_base<T>* d) {
    const T* a_first = a.begin();
    const T* a_last = a.end();
    const T* b_first = b.begin();
    T* out = d->begin();

    for (; a_first != a_last; ++a_first , ++b_first) {
        T a_b = func_a_b(*a_first, *b_first);
        *out = func_ab_c(a_b, c);
        ++out;
    }
}

template <typename T, typename Func1, typename Func2>
void gpm_ternary_sub_range(const Func1& func_ab_c,
                           const Func2& func_a_b,
                           const gpm_array_2d_base<T>& a,
                           const gpm_array_2d_base<T>& b,
                           T c,
                           const int_extent_2d& extent,
                           gpm_array_2d_base<T>* d) {
    int_extent_2d::extent_type row_ext = extent.row_extent();
    int_extent_2d::extent_type col_ext = extent.col_extent();
    size_t len = col_ext.size();
    auto j = col_ext.lower();
    for (auto i = row_ext.lower(); i < row_ext.upper(); i++) {
        const T* a_first = &a(i, j);
        const T* a_last = a_first + len;
        const T* b_first = &b(i, j);
        T* out = &(d->at(i, j));

        for (; a_first != a_last; ++a_first , ++b_first) {
            T a_b = func_a_b(*a_first, *b_first);
            *out = func_ab_c(a_b, c);
            ++out;
        }
    }
}


template <typename T, typename Func1, typename Func2>
void gpm_transform(const Func1& func_ab_c,
                   const Func2& func_a_b,
                   const gpm_array_2d_base<T>& a,
                   const gpm_array_2d_base<T>& b,
                   const gpm_array_2d_base<T>& c,
                   const int_extent_2d& extent,
                   gpm_array_2d_base<T>* d) {
    assert(a.extents().contains(extent));
    assert(b.extents().contains(extent));
    assert(c.extents().contains(extent));
    assert(d->extents().contains(extent));
    // Multiplies two grids, results in z
    if (a.extents() == extent && b.extents() == extent && c.extents() == extent && d->extents() == extent) {
        gpm_ternary_full_range(func_ab_c, func_a_b, a, b, c, d);
    }
    else {
        gpm_ternary_sub_range(func_ab_c, func_a_b, a, b, c, extent, d);
    }
}

template <typename T, typename Func1, typename Func2>
void gpm_transform(const Func1& func_ab_c,
                   const Func2& func_a_b,
                   const gpm_array_2d_base<T>& a,
                   const gpm_array_2d_base<T>& b,
                   const gpm_array_2d_base<T>& c,
                   gpm_array_2d_base<T>* d) {
    gpm_transform(func_ab_c, func_a_b, a, b, c, d->extents(), d);
}

template <typename T, typename Func1, typename Func2>
void gpm_transform(const Func1& func_ab_c,
                   const Func2& func_a_b,
                   const gpm_array_2d_base<T>& a,
                   const gpm_array_2d_base<T>& b,
                   T c,
                   const int_extent_2d& extent,
                   gpm_array_2d_base<T>* d) {
    assert(a.extents().contains(extent));
    assert(b.extents().contains(extent));
    assert(d->extents().contains(extent));
    // Multiplies two grids, results in z
    if (a.extents() == extent && b.extents() == extent && d->extents() == extent) {
        gpm_ternary_full_range(func_ab_c, func_a_b, a, b, c, d);
    }
    else {
        gpm_ternary_sub_range(func_ab_c, func_a_b, a, b, c, extent, d);
    }
}

template <typename T, typename Func1, typename Func2>
void gpm_transform(const Func1& func_ab_c,
                   const Func2& func_a_b,
                   const gpm_array_2d_base<T>& a,
                   const gpm_array_2d_base<T>& b,
                   T c,
                   gpm_array_2d_base<T>* d) {
    gpm_transform(func_ab_c, func_a_b, a, b, c, d->extents(), d);
}


template <typename T, typename Func>
inline void gpm_binary_full_range(const Func& func,
                                  const gpm_array_2d_base<T>& a,
                                  const gpm_array_2d_base<T>& b,
                                  gpm_array_2d_base<T>* c) {
    std::transform(a.begin(), a.end(), b.begin(), c->begin(), func);
}

template <typename T, typename Func>
inline void gpm_binary_sub_range(const Func& func,
                                 const gpm_array_2d_base<T>& a,
                                 const gpm_array_2d_base<T>& b,
                                 const int_extent_2d& extent,
                                 gpm_array_2d_base<T>* c) {
    int_extent_2d::extent_type row_ext = extent.row_extent();
    int_extent_2d::extent_type col_ext = extent.col_extent();
    size_t len = col_ext.size();
    auto j = col_ext.lower();
    for (auto i = row_ext.lower(); i < row_ext.upper(); i++) {
        const T* a_start(&a(i, j));
        const T* b_start(&b(i, j));
        T* c_start(&c->at(i, j));
        std::transform(a_start, a_start + len, b_start, c_start, func);
    }
}

template <typename T, typename Func>
inline void gpm_unary_full_range(const Func& func,
                                 const gpm_array_2d_base<T>& a,
                                 gpm_array_2d_base<T>* b) {
    std::transform(a.begin(), a.end(), b->begin(), func);
}

template <typename T, typename Func>
inline void gpm_unary_sub_range(const Func& func,
                                const gpm_array_2d_base<T>& a,
                                const int_extent_2d& extent,
                                gpm_array_2d_base<T>* b) {
    int_extent_2d::extent_type row_ext = extent.row_extent();
    int_extent_2d::extent_type col_ext = extent.col_extent();
    size_t len = col_ext.size();
    auto j = col_ext.lower();
    for (auto i = row_ext.lower(); i < row_ext.upper(); i++) {
        const T* a_start(&a(i, j));
        T* b_start(&b->at(i, j));
        std::transform(a_start, a_start + len, b_start, func);
    }
}


template <typename T, typename Func>
inline void gpm_transform(const Func& func,
                          const gpm_array_2d_base<T>& a,
                          const gpm_array_2d_base<T>& b,
                          const int_extent_2d& extent,
                          gpm_array_2d_base<T>* c) {
    assert(a.extents().contains(extent));
    assert(b.extents().contains(extent));
    assert(c->extents().contains(extent));
    // Multiplies two grids, results in z
    if (a.extents() == extent && b.extents() == extent && c->extents() == extent) {
        gpm_binary_full_range(func, a, b, c);
    }
    else {
        gpm_binary_sub_range(func, a, b, extent, c);
    }
}

template <typename T, typename Func>
void gpm_transform(const Func& func,
                   const gpm_array_2d_base<T>& a,
                   const gpm_array_2d_base<T>& b,
                   gpm_array_2d_base<T>* c) {
    gpm_transform(func, a, b, c->extents(), c);
}

template <typename T, typename Func>
inline void gpm_transform(const Func& func,
                          const gpm_array_2d_base<T>& a,
                          const int_extent_2d& extent,
                          gpm_array_2d_base<T>* b) {
    assert(a.extents().contains(extent));
    assert(b->extents().contains(extent));
    // Multiplies two grids, results in z
    if (a.extents() == extent && b->extents() == extent) {
        gpm_unary_full_range(func, a, b);
    }
    else {
        gpm_unary_sub_range(func, a, extent, b);
    }
}

template <typename T, typename Func>
inline void gpm_transform(const Func& func,
                          const gpm_array_2d_base<T>& a,
                          gpm_array_2d_base<T>* b) {
    gpm_transform(func, a, b->extents(), b);
}


template <typename T, typename Func>
inline void gpm_transform(const Func& func,
                          const gpm_array_2d_base<T>& a,
                          T b,
                          const int_extent_2d& extent,
                          gpm_array_2d_base<T>* c) {
    gpm_transform(std::bind(func, std::placeholders::_1, b), a, extent, c);
}

template <typename T, typename Func>
inline void gpm_transform(const Func& func,
                          const gpm_array_2d_base<T>& a,
                          T b,
                          gpm_array_2d_base<T>* c) {
    gpm_transform(func, a, b, c->extents(), c);
}

template <typename T, typename Func>
inline void gpm_transform(const Func& func,
                          T a,
                          const gpm_array_2d_base<T>& b,
                          const int_extent_2d& extent,
                          gpm_array_2d_base<T>* c) {
    gpm_transform(std::bind(func, a, std::placeholders::_1), b, extent, c);
}

template <typename T, typename Func>
inline void gpm_transform(const Func& func,
                          T a,
                          const gpm_array_2d_base<T>& b,
                          gpm_array_2d_base<T>* c) {
    gpm_transform(func, a, b, c->extents(), c);
}

template <typename T>
void gpm_add(const gpm_array_2d_base<T>& a,
             const gpm_array_2d_base<T>& b,
             gpm_array_2d_base<T>* c) {
    gpm_transform(std::plus<T>(), a, b, c->extents(), c);
}

template <typename T>
void gpm_add(T a,
             const gpm_array_2d_base<T>& b,
             gpm_array_2d_base<T>* c) {
    gpm_transform(std::plus<T>(), a, b, c->extents(), c);
}

template <typename T>
void gpm_add(const gpm_array_2d_base<T>& a,
             T b,
             gpm_array_2d_base<T>* c) {
    gpm_transform(std::plus<T>(), a, b, c->extents(), c);
}

template <typename T>
void gpm_subtract(const gpm_array_2d_base<T>& a,
                  const gpm_array_2d_base<T>& b,
                  gpm_array_2d_base<T>* c) {
    gpm_transform(std::minus<T>(), a, b, c->extents(), c);
}

template <typename T>
void gpm_subtract(T a,
                  const gpm_array_2d_base<T>& b,
                  gpm_array_2d_base<T>* c) {
    gpm_transform(std::minus<T>(), a, b, c->extents(), c);
}

template <typename T>
void gpm_subtract(const gpm_array_2d_base<T>& a,
                  T b,
                  gpm_array_2d_base<T>* c) {
    gpm_transform(std::minus<T>(), a, b, c->extents(), c);
}


template <typename T>
void gpm_multiply(const gpm_array_2d_base<T>& a,
                  const gpm_array_2d_base<T>& b,
                  gpm_array_2d_base<T>* c) {
    gpm_transform(std::multiplies<T>(), a, b, c->extents(), c);
}

template <typename T>
void gpm_multiply(T a,
                  const gpm_array_2d_base<T>& b,
                  gpm_array_2d_base<T>* c) {
    gpm_transform(std::multiplies<T>(), a, b, c->extents(), c);
}

template <typename T>
void gpm_multiply(const gpm_array_2d_base<T>& a,
                  T b,
                  gpm_array_2d_base<T>* c) {
    gpm_transform(std::multiplies<T>(), a, b, c->extents(), c);
}

template <typename T>
void gpm_multiply(const gpm_array_2d_base<T>& a,
                  const gpm_array_2d_base<T>& b,
                  const gpm_array_2d_base<T>& c,
                  gpm_array_2d_base<T>* d) {
    gpm_transform(std::multiplies<T>(), std::multiplies<T>(), a, b, c, d->extents(), d);
}

template <typename T>
void gpm_multiply(const gpm_array_2d_base<T>& a,
                  const gpm_array_2d_base<T>& b,
                  T c,
                  gpm_array_2d_base<T>* d) {
    gpm_transform(std::multiplies<T>(), std::multiplies<T>(), a, b, c, d->extents(), d);
}

template <typename T, typename Func>
void gpm_transform_add_to_output(const Func& func,
                                 const gpm_array_2d_base<T>& a,
                                 const gpm_array_2d_base<T>& b,
                                 gpm_array_2d_base<T>* c) {
    gpm_transform(std::plus<T>(), func, a, b, *c, c);
}

template <typename T, typename Func>
void gpm_transform_multiply_to_output(const Func& func,
                                      const gpm_array_2d_base<T>& a,
                                      const gpm_array_2d_base<T>& b,
                                      gpm_array_2d_base<T>* c) {
    gpm_transform(std::multiplies<T>(), func, a, b, *c, c);
}


}}}
#endif
