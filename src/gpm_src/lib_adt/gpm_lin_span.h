#ifndef gpm_lin_span_h
#define gpm_lin_span_h
#include <cstddef>
#include <cassert>


namespace Slb { namespace Exploration { namespace Gpm {
#ifdef WIN32
#define GPM_DO_INLINE __forceinline
#else
#define GPM_DO_INLINE inline
#endif
template <typename eT>
struct lin_span {
	using size_type = std::size_t;
    eT* mem;
	size_type nel;
    GPM_DO_INLINE lin_span()
    {
        mem = nullptr;
        nel = 0;
    };
    GPM_DO_INLINE lin_span(eT* ptr, size_type num)
    {
        assert(ptr != nullptr && num > 0);
        mem = ptr;
        nel = num;
    };
    GPM_DO_INLINE eT& operator[](const size_type i)
    {
        assert(i >= 0 && i < nel);
        return mem[i];
    }

    GPM_DO_INLINE const eT& operator[](const size_type i) const
    {
        assert(i >= 0 && i < nel);
        return mem[i];
    }

    GPM_DO_INLINE eT* begin() const
    {
        return mem;
    }

    GPM_DO_INLINE eT* end() const
    {
        return mem + nel;
    }

    GPM_DO_INLINE size_type size() const
    {
        return nel;
    }
};
}}}

#endif
