#ifndef GPM_PARS_METHODS_H
#define GPM_PARS_METHODS_H

#include "sedstc.h"
#include "gpm_logger.h"
#include <map>

namespace Slb { namespace Exploration { namespace Gpm {

struct sediment_props {
    std::string id;
    std::string name;
    int sed_index;

};

std::map<std::string, int> gpm_sed_make_sediment_id_index(const pars& s);
std::vector<sediment_props> gpm_sed_make_sediment_props(const pars& s);
bool gpm_sed_has_all_ids(const pars& s, const std::vector<std::string>& ids);
void gpm_create_default_sediment_pack(pars* s_p);
int gpm_check_sediment_params(pars& s, const Tools::gpm_logger& logger);
void gpm_calc_sed_por_and_prs(pars* s_p, int ns, const Tools::gpm_logger& logger);
int gpm_sed_check_transport(pars* s_p, int ns, const Tools::gpm_logger& logger);

}}}

#endif
