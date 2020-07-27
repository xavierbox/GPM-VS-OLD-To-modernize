// -- Schlumberger Private --

#include "gpm_find_sediment_intervals.h"

namespace Slb { namespace Exploration { namespace Gpm {

boost::icl::split_interval_map<float, int> find_sediment_intervals(float zbot, float ztop, const std::vector<float>& tops) {
    boost::icl::split_interval_map<float, int> intervals;
    int size = static_cast<int>(tops.size());
    int outer_interval_index = size; // One for -inf, and one for starting at index 1
    auto boundary = boost::icl::interval<float>::left_open(zbot, ztop);
    intervals += std::make_pair(boundary, outer_interval_index);
    for (int i = 1; i < tops.size(); ++i) {
        auto interval = boost::icl::interval<float>::left_open(tops[i - 1], tops[i]);
        if (boost::icl::intersects(boundary, interval)) {
            intervals += std::make_pair(interval, i);
        }
    }
    // Now lets split it with the last
    std::vector<boost::icl::interval_map<float, int>::iterator> out_of_bounds;
    for (boost::icl::interval_map<float, int>::iterator tmp = intervals.begin();
         tmp != intervals.end(); ++tmp) {
        (*tmp).second -= outer_interval_index;
        if ((*tmp).second < 0) {
            out_of_bounds.push_back(tmp);
        }
    }
    for (std::vector<boost::icl::interval_map<float, int>::iterator>::iterator tmp = out_of_bounds.begin();
         tmp != out_of_bounds.end(); ++tmp) {
        intervals.erase(*tmp);
    }
    return intervals;
}

}}}
