// -- Schlumberger Private --

#include "gpm_source_indexes.h"
#include "gpm_vbl_array_2d.h"
#include "gpm_index_2d.h"
#include <vector>
#include <set>

namespace Slb { namespace Exploration { namespace Gpm {

std::pair<std::vector<index_2d_int>, std::set<int>>
gpm_find_sources(const int_2darray_base_type& src, const int_extent_2d& ext) {
    std::vector<index_2d_int> ret_val;
    std::set<int> ret_set;
    int_extent_2d intersect = src.extents() & ext;
    int_extent_2d::extent_type row_extent = intersect.row_extent();
    int_extent_2d::extent_type col_extent = intersect.col_extent();
    for (auto i = row_extent.lower(); i < row_extent.upper(); ++i) {
        for (auto j = col_extent.lower(); j < col_extent.upper(); ++j) {
            int src_id = src(i, j);
            if (src_id > 0) {
                ret_val.push_back(index_2d_int(i, j));
                ret_set.insert(src_id);
            }
        }
    }
    return make_pair(ret_val, ret_set);
}

std::vector<index_2d_int>
gpm_find_sinks(const int_2darray_base_type& src, const int_extent_2d& ext) {
    std::vector<index_2d_int> ret_val;
    int_extent_2d intersect = src.extents() & ext;
    std::vector<int_extent_2d> border_extents = intersect.border_extents(1, 1); // Find all eight borders
    for (std::vector<int_extent_2d>::const_iterator it = border_extents.begin(); it != border_extents.end(); ++it) {
        int_extent_2d loc(*it);
        int_extent_2d::extent_type row_extent = loc.row_extent();
        int_extent_2d::extent_type col_extent = loc.col_extent();
        for (auto i = row_extent.lower(); i < row_extent.upper(); ++i) {
            for (auto j = col_extent.lower(); j < col_extent.upper(); ++j) {
                ret_val.push_back(index_2d_int(i, j));
            }
        }
    }
    return ret_val;
}
}}}
