// -- Schlumberger Private --

#ifndef GPM_INDPROP_INDEXES_HELPER_H
#define GPM_INDPROP_INDEXES_HELPER_H

#include "gpm_indprop_indexes.h"
#include "parm_prop_descr.h"
#include <map>

namespace Slb { namespace Exploration { namespace Gpm {
// Free function so we can move the class out to the data_model
void gpm_set_indprop_indexes(const std::map<PARM_GRID_MAP_NAMES, std::string>& prop_mapper, const std::vector<std::string>& index_names, indprop_indexes* holder_p);

}}}
#endif
