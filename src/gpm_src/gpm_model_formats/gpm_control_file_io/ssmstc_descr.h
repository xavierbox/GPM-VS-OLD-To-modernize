// -- Schlumberger Private --

#ifndef _ssmstc_descr_h_
#define _ssmstc_descr_h_

#include "gpm_type_descriptor.h"
#include <string>
#include <vector>

namespace Slb { namespace Exploration { namespace Gpm {
// Include the enum in the GPM namespace
#include "ssm_enum_def.h"

class parc_descr_holder {
public:
    static const std::vector<TypeDescr>& get_parc_descr();
    static std::string get_current_version();
};
}}}
#endif


