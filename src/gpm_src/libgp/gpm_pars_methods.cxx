#include "gpm_pars_methods.h"
#include "gpm_logger.h"
#include "libgp.h"


namespace Slb { namespace Exploration { namespace Gpm {

std::map<std::string, int> gpm_sed_make_sediment_id_index(const pars& s)
{
    std::map<std::string, int> sediment_id_index;
    if (!s.sed_id.empty()) {
        for (int i = 0; i < s.sed_id.size(); ++i) {
            sediment_id_index[s.sed_id[i]] = i;
        }
    }
    else {
        for (int i = 0; i < s.diameter.size(); ++i) {
            sediment_id_index[std::to_string(i + 1)] = i;
        }
    }
    return sediment_id_index;
}

std::vector<sediment_props> gpm_sed_make_sediment_props(const pars& s)
{
	std::vector<sediment_props> sediment_id_index;
	if (!s.sed_id.empty()) {
		for (int i = 0; i < s.sed_id.size(); ++i) {
			sediment_props it{ s.sed_id[i] , s.sedname[i],i };
			sediment_id_index.push_back(it);
		}
	}
	else {
		for (int i = 0; i < s.diameter.size(); ++i) {
			sediment_props it{ std::to_string(i + 1) , s.sedname[i],i };
			sediment_id_index.push_back(it);
		}
	}
	return sediment_id_index;
}

bool gpm_sed_has_all_ids(const pars& s, const std::vector<std::string>& ids)
{
	for (const auto& it : ids) {
		if (std::find(s.sed_id.begin(), s.sed_id.end(), it) == s.sed_id.end()) {
			return false;
		}
	}
	return true;
}
void gpm_create_default_sediment_pack(pars* s_p)
{
	s_p->numseds = 4;
	s_p->sed_id = { "1","2","3","4" };
	s_p->sedname = { "Sand (coarse)","Sand (fine)","Silt","Clay" };
	s_p->diameter = { 1, 0.1F, 0.0099999998F, 0.001F };
	s_p->grainden = {2.7F, 2.7F, 2.7F, 2.7F};
	s_p->transp = { 0.80000001F, 1.6F, 3.2F, 6.4000001F };
	s_p->por0 = { 0.30000001F, 0.34F, 0.40000001F, 0.55000001F };
	s_p->por1 = { 0.2F, 0.15000001F, 0.1F, 0.050000001F};
	s_p->perm0 = { 100, 10, 0.1F, 0.0099999998F };
	s_p->perm1 = { 10, 1, 0.1F, 0.001F };
	s_p->permanis = { 1, 1, 1, 10 };
	s_p->comp = { 5000, 5000, 500, 50 };
	s_p->erodability = { 1, 1, 1, 1 };
	s_p->erode_funcs = { std::make_shared<gpm_1d_constant>(1.0f),std::make_shared<gpm_1d_constant>(1.0f),std::make_shared<gpm_1d_constant>(1.0f),std::make_shared<gpm_1d_constant>(1.0f) };
	s_p->transform_by_transport = { "", "", "", "" };
	s_p->do_inplace_transform = { 0, 0, 0, 0 };
	s_p->inplace_transform_coeff = { 0, 0, 0, 0 };
	s_p->version = "1.1.3.0";
}

int gpm_check_sediment_params(pars& s, const Tools::gpm_logger& logger)
{
	auto num_seds = s.numseds;
	if (s.por0.size() == s.por1.size() && s.perm0.size() == s.perm1.size() && s.por0.size() == s.perm1.size() && s.por0.size() == num_seds) {
		if (std::equal(s.por0.begin(), s.por0.end(), s.por1.begin())) {
			logger.print(Tools::LOG_IMPORTANT, "Initial and current porosity can not be equal\n");
			return 1;
		}
		return 0;
	}
	logger.print(Tools::LOG_IMPORTANT, "Porosity and permeability arrays need to be equal length\n");
	return 1;
}
void gpm_calc_sed_por_and_prs(pars* s_p, int ns, const Tools::gpm_logger& logger)
{
	struct pars& s(*s_p);
	find_permability_coeffs_1(s.diameter, s.por0, s.por1, s.perm0, s.perm1, &s.pfactor, &s.pexpon);
	for (int ised = 0; ised < ns; ised++) {
		logger.print(Tools::LOG_VERBOSE, "pfactor = %f, pexpon = %f\n", s.pfactor[ised], s.pexpon[ised]);
	}
}

int gpm_sed_check_transport(pars* s_p, int ns, const Tools::gpm_logger& logger)
{
	struct pars& s(*s_p);
	int ret = 0;
	if ((s.transp.size() < ns) || std::all_of(s.transp.begin(), s.transp.end(), [](float val) {return val <= 0.0F; })) {
		s.transp.resize(ns);
		ret = std::any_of(s.diameter.begin(), s.diameter.end(), [](float val) {return val > 20; }) ? 2 : 0;
		for (auto ised = 0; ised < ns; ised++) {
			if (s.diameter[ised] > 20) {
				logger.print(Tools::LOG_IMPORTANT, "Sediment diameter %d = %f is too large, negative transportability\n", ised, s.diameter[ised]);
			}
			s.transp[ised] = static_cast<float>(-log(s.diameter[ised] / 20.0) / 20.0);
		}
	}
	return ret;
}

}}}
