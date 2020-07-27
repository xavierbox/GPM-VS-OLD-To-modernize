// -- Schlumberger Private --

#ifndef GPM_TIME_FUNCTION_DATA_STC_H
#define GPM_TIME_FUNCTION_DATA_STC_H

#include "gpm_type_descriptor.h"
#include <vector>

namespace Slb { namespace Exploration { namespace Gpm {
// Include the enum def
#include "gpm_time_function_data_enum_def.h"

// Our stuff
class gpm_time_function_data_descr_holder {
public:
    static std::vector<TypeDescr> gpm_time_function_data_descr;
    static std::string current_version;
    static std::map<TIME_FUNCTION_1_INDEX_NAMES, std::string> time_function_1_index_name_prefix;
    static std::string time_function_index_name(TIME_FUNCTION_1_INDEX_NAMES id, int source);

};


}}}


#endif
