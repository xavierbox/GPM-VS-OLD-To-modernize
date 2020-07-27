#ifndef GPM_PARS_HELPER_METHODS_H
#define GPM_PARS_HELPER_METHODS_H

#include "sedstc.h"
#include "gpm_logger.h"

namespace Slb { namespace Exploration { namespace Gpm {
void gpm_calc_sed_por_and_prs(pars* s_p, int ns, const Tools::gpm_logger& logger);

}}}

#endif
