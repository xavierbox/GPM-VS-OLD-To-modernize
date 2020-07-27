// -- Schlumberger Private --

#ifndef _mapstc_descr_h_
#define _mapstc_descr_h_

/* Used by: maplib, ge, ssm */
/* Grid parameter structure */
#include "gpm_type_descriptor.h"
#include "mapstc.h"

#include <string>
#include <vector>
#include <map>

namespace Slb { namespace Exploration { namespace Gpm{

// Include the enum defs
#include "map_enum_def.h"

// This will end up 
class parm_type_descr_holder {
public:
    static const parameter_descr_type& get_parm_descr();
    static std::string get_current_version();
    static const TypeDescr& get_parm_description(PARM_MEMBERS id);
    static std::string make_json_history_item(const std::string& process, const std::string& git_hash);
    static std::string make_json_history_item(const std::string& process);
};


}}}

#endif
