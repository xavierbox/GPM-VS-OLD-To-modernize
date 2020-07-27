#include "gpm_pars_methods.h"
#include "gpm_1d_interpolator.h"
#include "gpm_logger.h"
#include "libgp.h"
#include <algorithm>

namespace Slb { namespace Exploration { namespace Gpm {


void gpm_calc_sed_por_and_prs(pars* s_p, int ns, const Tools::gpm_logger& logger)
{
	struct pars& s(*s_p);
	find_permability_coeffs_1(s.diameter, s.por0, s.por1, s.perm0, s.perm1, &s.pfactor, &s.pexpon);
	for (int ised = 0; ised < ns; ised++) {
		logger.print(Tools::LOG_VERBOSE, "pfactor = %f, pexpon = %f\n", s.pfactor[ised], s.pexpon[ised]);
	}
}


}}}
