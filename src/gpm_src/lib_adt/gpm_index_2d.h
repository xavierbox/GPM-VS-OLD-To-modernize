#ifndef GPM_INDEX_2D_H
#define GPM_INDEX_2D_H

#include "gpm_array_common_defs.h"
#include <cmath>

namespace Slb { namespace Exploration { namespace Gpm {
class index_2d_int {
public:
	using index_type = gpm_array_index_defs::index_type;
	// Illegal index
    index_2d_int() : i(-1), j(-1)
    {
    }


    index_2d_int(index_type li, index_type lj) : i(li), j(lj)
    {
    }

    bool operator<(const index_2d_int& rhs) const
    {
        return i < rhs.i || (i==rhs.i && j < rhs.j);
    }

    bool operator==(const index_2d_int& rhs) const
    {
        return i == rhs.i && j == rhs.j;
    }
    bool operator!=(const index_2d_int& rhs)const
    {
        return !(*this==rhs);
    }
    index_type i;
	index_type j;
};

class index_2d_float {
public:
    // Illegal index
    index_2d_float() : i(-1), j(-1)
    {
    }

    float i;
    float j;
};

enum ClipPolicy { Floor = 0, Ceil = 1, Nearest = 2 };

inline index_2d_int::index_type gpm_clip_to_nearest(float val)
{
    return static_cast<index_2d_int::index_type>(val > 0 ? val + 0.5F : val - 0.5F);
}

inline index_2d_int gpm_clip_to(const index_2d_float& val, ClipPolicy pol)
{
	index_2d_int::index_type i;
	index_2d_int::index_type j;
    if (pol == Nearest) {
        i = gpm_clip_to_nearest(val.i);
        j = gpm_clip_to_nearest(val.j);
    }
    else if (pol == Ceil) {
        i = static_cast<int>(ceilf(val.i));
        j = static_cast<int>(ceilf(val.j));
    }
    else {
        i = static_cast<int>(floorf(val.i));
        j = static_cast<int>(floorf(val.j));
    }
    return index_2d_int(i, j);
}

}}}
#endif