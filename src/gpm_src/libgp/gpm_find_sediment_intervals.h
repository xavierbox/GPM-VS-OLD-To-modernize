// -- Schlumberger Private --

#ifndef GPM_FIND_SEDIMENT_INTERVALS_H
#define GPM_FIND_SEDIMENT_INTERVALS_H

#include <boost/icl/split_interval_map.hpp>
#include <vector>

namespace Slb { namespace Exploration { namespace Gpm {

// Give a vector of tops, and a vertical interval defined by zbot and ztop
// Find the intervals where one wants to fetch sediments
// Return a interval map giving the intervals and the indexes to the top to which they use
// If there is a index of 0 and interval.lower() == ztop, then we are trying to fetch data outside the sediment range
// That is an error 
boost::icl::split_interval_map<float, int> find_sediment_intervals(float zbot, float ztop, const std::vector<float>& tops);

}}}
#endif
