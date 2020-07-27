// -- Schlumberger Private --

#include "typedescr_to_funcmapper.h"
#include <algorithm>

namespace Slb { namespace Exploration { namespace Gpm {

bool TypeDescrToFunctionMapper::are_functions_consistent() const {
    auto res_ok = true;
    for (const auto& it : holder) {
        auto key = it.first;
        auto found = std::find_if(types.begin(), types.end(), [key](const TypeDescr& item) { return key == item.id; });
        if (found == types.end()) {
            res_ok = false;
        }
    }
    return res_ok;
}
}}}
