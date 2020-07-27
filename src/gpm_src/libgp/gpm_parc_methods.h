#ifndef GPM_PARC_METHODS_H
#define GPM_PARC_METHODS_H
#include "mapstc.h"
#include "ssmstc.h"

namespace Slb { namespace Exploration { namespace Gpm {

void gpm_find_needed_output_properties(const parc& c, std::vector<property_description>* vals);

}}}

#endif
