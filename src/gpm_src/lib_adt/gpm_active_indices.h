#ifndef GPM_ACTIVE_IND_UTIL_H
#define GPM_ACTIVE_IND_UTIL_H
#include "gpm_vbl_array_2d.h"
#include "gpm_basic_defs.h"
#include <limits>
#include <algorithm>

namespace Slb { namespace Exploration { namespace Gpm {

struct active_indices_mgr {
	using index_type = bool_2darray_type::index_type;
	bool_2darray_type touched;
	index_type imin;
	index_type jmin;
	index_type imax;
	index_type jmax;

	active_indices_mgr()
    {
        imin = std::numeric_limits<index_type>::max();
        jmin = std::numeric_limits<index_type>::max();
        imax = std::numeric_limits<index_type>::min();
        jmax = std::numeric_limits<index_type>::min();
    }

    GPM_DO_INLINE void touch(index_type i, index_type j)
    {
        imin = std::min(imin, i);
        jmin = std::min(jmin, j);
        imax = std::max(imax, i);
        jmax = std::max(jmax, j);
        touched(i, j) = true;
    }

    void reset()
    {
        if (imin > imax || jmin > jmax) {
            touched.fill(false);
        }
        else {
            for (auto i = imin; i <= imax; i++) {
                for (auto j = jmin; j <= jmax; j++) {
                    touched(i, j) = false;
                }
            }
        }

        imin = std::numeric_limits<index_type>::max();
        jmin = std::numeric_limits<index_type>::max();
        imax = std::numeric_limits<index_type>::min();
        jmax = std::numeric_limits<index_type>::min();
    }
};

}}}

#endif
