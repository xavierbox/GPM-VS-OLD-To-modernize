// -- Schlumberger Private --

#ifndef GPM_SURFACE_DATA_STC_H
#define GPM_SURFACE_DATA_STC_H

#include "gpm_type_descriptor.h"
#include <vector>

namespace Slb { namespace Exploration { namespace Gpm {
// Include the enum def
#include "gpm_surface_data_enum_def.h"

// Our stuff
class gpm_surface_data_descr_holder {
public:
    static std::vector<TypeDescr> gpm_surface_data_descr;
    static std::string current_version;
    static std::string get_json_schema();
};


}}}


#endif
