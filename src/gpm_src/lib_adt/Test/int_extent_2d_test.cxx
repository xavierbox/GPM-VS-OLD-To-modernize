// -- Schlumberger Private --

#include "gpm_int_extent_2d.h"
#include "gpm_basic_defs.h"
#include <algorithm>
#include "gpm_index_2d.h"
using namespace Slb::Exploration::Gpm;

int int_extent_2d_test(int argc, char* argv[]) {
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
    const bool isOk = std::equal(ret_val.begin(), ret_val.end(), oracle_val);

    int_extent_2d test2(10, 9);
    std::vector<int_extent_2d> border2_extents = test2.border_extents(1, 1);
    return isOk ? 0 : 1;
}

int int_extent_2d_difference_test(int argc, char* argv[]) {
    // 
    const int_extent_2d test(-1, 3, 0, 3);
    const int_extent_2d core(0, 3, 0, 3);
    auto t2 = test.right_subtract(core);
    const auto isOk = true;
    return isOk ? 0 : 1;
}

bool do_border_test() {
    int_extent_2d first(0, 20, -1, 21);
    const int row_border = 1;
    const int col_border = 1;
    const std::vector<int_extent_2d> border_extents = first.border_extents(row_border, col_border);
    const int_extent_2d inner_area = first.inner_extent(row_border, col_border);
    bool isOk = border_extents.size() == 8;
    isOk = isOk && inner_area == int_extent_2d(1, 19, 0, 20);
    return isOk;
}

bool do_radius_test()
{
    const int_extent_2d first(0, 20, 0, 18);
    const int row_origin = 17;
    const int col_origin = 16;
    const std::vector<int_extent_2d> border_extents = first.radius_extent(row_origin, col_origin, 3);
    const bool isOk = border_extents.size() == 2;
    return isOk;

}

bool do_union_test() {
    const int_extent_2d uni_test = int_extent_2d(2, 2) | int_extent_2d::make_index(4, 4);
    const bool isOk = uni_test.row_extent().lower() == 0 && uni_test.row_extent().upper() == 5;
    return isOk;
}

bool do_containment_test() {
    const int_extent_2d loc_ext(4, 4);
    const bool first_ok = loc_ext.contains(int_extent_2d(2, 2));
    const bool second_ok = !loc_ext.contains(int_extent_2d::make_index(4, 4));
    return first_ok && second_ok;
}

// Here is the extent test start
int extent_test(int argc, char* argv[]) {
    const bool t1 = do_union_test();
    const bool t2 = do_containment_test();
    const bool t3 = do_border_test();
    const bool t4 = do_radius_test();
    return t1 && t2 && t3 &&t4 ? 0 : 1;
}
