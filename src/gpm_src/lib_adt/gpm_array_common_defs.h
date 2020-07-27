// -- Schlumberger Private --
#ifndef gpm_array_common_defs_h_
#define gpm_array_common_defs_h_

#include "gpm_int_extent_1d.h"


namespace Slb { namespace Exploration { namespace Gpm {
class gpm_array_index_defs {
public:
	using size_type = std::size_t;
	using index_type = std::ptrdiff_t;
	using extent_type = int_extent_1d;
	// First index, second index
	struct index_2d {
		index_type i{}; // Row
		index_type j{}; // Col
	};
	// First index, second index, third index
	struct index_3d {
		index_type i{};
		index_type j{};
		index_type k{};
	};
};

template <int N>
class gpm_array_common_defs: public gpm_array_index_defs {
public:
	using dimension_type = std::array<size_type, N>;
    using stride_type = std::array<size_type, N>;
};
}}}

#endif