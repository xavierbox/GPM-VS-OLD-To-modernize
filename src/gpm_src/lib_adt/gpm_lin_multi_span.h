#ifndef gpm_lin_multi_span_h
#define gpm_lin_multi_span_h

#include "gpm_lin_span.h"
#include <algorithm>
#include <numeric>
#include <cassert>


namespace Slb { namespace Exploration { namespace Gpm {
#ifdef WIN32
#define GPM_DO_INLINE __forceinline
#else
#define GPM_DO_INLINE inline
#endif
template <typename eT, int N>
struct lin_multi_span {
    std::size_t dims[N];
	std::size_t stride[N];
	lin_span<eT> holder;
    GPM_DO_INLINE lin_multi_span():holder()
    {
        std::fill(begin(dims), end(dims), 0);
		std::fill(begin(stride), end(stride), 0);
    };
    GPM_DO_INLINE lin_multi_span(eT* ptr, const std::size_t(&loc_dims)[N], const std::size_t(&loc_strides)[N])
    {
        assert(ptr != nullptr);
		std::copy(std::begin(loc_dims), std::end(loc_dims), std::begin(dims));
		std::copy(std::begin(loc_strides), std::end(loc_strides), std::begin(stride));
        auto nel = std::accumulate(std::begin(dims), std::end(dims), std::multiplies<std::size_t>());
		holder = lin_span<eT>(ptr, static_cast<int>(nel));
    };
	GPM_DO_INLINE lin_multi_span(eT* ptr, const std::array<std::size_t, N>& loc_dims, const std::array<size_t, N>& loc_strides)
	{
		assert(ptr != nullptr);
		std::copy(std::begin(loc_dims), std::end(loc_dims), std::begin(dims));
		std::copy(std::begin(loc_strides), std::end(loc_strides), std::begin(stride));
		auto nel = std::accumulate(std::begin(dims), std::end(dims), std::size_t(1), std::multiplies<std::size_t>());
		holder = lin_span<eT>(ptr, static_cast<int>(nel));
	};
    // indexes contains {i,j,k}
    GPM_DO_INLINE eT& operator[](const std::size_t(&indexes)[N])
    {
        auto adress = std::inner_product(std::begin(stride), std::end(stride), std::begin(indexes), std::size_t(0));
        return holder[static_cast<int>(adress)];
    }

    GPM_DO_INLINE const eT& operator[](const std::size_t(&indexes)[N]) const
    {
		auto adress = std::inner_product(std::begin(stride), std::end(stride), std::begin(indexes), std::size_t(0));
		return holder[static_cast<int>(adress)];
    }
	GPM_DO_INLINE eT& operator[](const std::array<std::size_t, N>& indexes)
	{
		auto adress = std::inner_product(std::begin(stride), std::end(stride), std::begin(indexes), std::size_t(0));
		return holder[static_cast<int>(adress)];
	}

	GPM_DO_INLINE const eT& operator[](const std::array<std::size_t, N>& indexes) const
	{
		auto adress = std::inner_product(std::begin(stride), std::end(stride), std::begin(indexes), std::size_t(0));
		return holder[static_cast<int>(adress)];
	}
    GPM_DO_INLINE eT* begin() const
    {
        return holder.mem;
    }

    GPM_DO_INLINE eT* end() const
    {
        return holder.end();
    }

    GPM_DO_INLINE int size() const
    {
        return holder.nel;
    }
};
}}}

#endif
