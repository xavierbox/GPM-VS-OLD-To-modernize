// -- Schlumberger Private --

#include "gpm_int_extent_2d.h"
#include "gpm_index_2d.h"
#include "gpm_basic_defs.h"
#include <algorithm>

using namespace Slb::Exploration::Gpm;

//TODO fill in real test later
int int_extent_1d_test(int argc, char* argv[]) {
    // 
    int_extent_2d test(3, 3);
    std::vector<int_extent_2d> border_extents = test.border_extents(1, 1);
    std::vector<index_2d_int> ret_val;
    index_2d_int oracle_val[] = {index_2d_int(0, 0),index_2d_int(0, 1),index_2d_int(0, 2),
        index_2d_int(1, 0),index_2d_int(1, 2),
        index_2d_int(2, 0),index_2d_int(2, 1),index_2d_int(2, 2)};
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
    std::sort(ret_val.begin(), ret_val.end());
    bool isOk = std::equal(ret_val.begin(), ret_val.end(), oracle_val);

    int_extent_2d test2(10, 9);
    std::vector<int_extent_2d> border2_extents = test2.border_extents(1, 1);
    return isOk ? 0 : 1;
}

int int_extent_1d_iterator_test(int argc, char* argv[]) {
    // 
    int_extent_1d test(-1, 3);
    int i = 0;
    bool isOk = true;
    for (auto it : test) {
        isOk = isOk && it == i-1;
        ++i;
    }
    return i==test.size() && isOk? 0 : 1;
}