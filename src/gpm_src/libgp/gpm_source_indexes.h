// -- Schlumberger Private --

#ifndef gpm_source_indexes_hh
#define gpm_source_indexes_hh

#include "gpm_basic_defs.h"
#include "gpm_index_2d.h"
#include <set>
#include <utility>
#include <vector>

namespace Slb { namespace Exploration { namespace Gpm {

std::pair<std::vector<index_2d_int>, std::set<int>> gpm_find_sources(const int_2darray_base_type& src, const int_extent_2d& ext);
std::vector<index_2d_int> gpm_find_sinks(const int_2darray_base_type& src, const int_extent_2d& ext);

}}}
#endif
