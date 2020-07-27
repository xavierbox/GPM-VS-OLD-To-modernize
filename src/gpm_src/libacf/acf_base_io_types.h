// -- Schlumberger Private --

#ifndef acf_base_io_types_h
#define acf_base_io_types_h

#include <array>
#include <vector>

namespace Slb { namespace Exploration { namespace Gpm {

struct acf_base_io_types {
    typedef int key_type;
	using size_type = int;// std::size_t;
	using index_type = int;// std::ptrdiff_t;
    typedef std::vector<std::size_t> parameter_num_type;

	typedef std::pair<index_type, index_type> index_pair_type;
	typedef std::pair<index_type, index_type> min_max_type;
};
}}}
#endif
