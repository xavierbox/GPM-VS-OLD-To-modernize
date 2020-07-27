// -- Schlumberger Private --

#include "libgp.h"
#include <vector>
#include <cmath>
using namespace Slb::Exploration::Gpm;

int gpm_porosity_test(int argc, char* argv[]) {
    std::vector<float> frac(4, 0);
    float diam_base[] = {1, .1f, .01f, .001f};
    float por_base[] = {.3f, .34f, .4f, .55f};
    std::vector<float> diam(diam_base, diam_base + 4);
    std::vector<float> por(por_base, por_base + 4);
    std::vector<float> res_por(4, 0);
    //SEDNAME =  'SAND1' 'SAND2' 'SILT' 'CLAY'
    //DIAMETER =  1 .1 .01 .001
    //GRAINDEN =  2.7 2.7 2.7 2.7
    //POR0 =  .3 .34 .4 .55

    for (int i = 0; i < 4; ++i) {
        std::fill(frac.begin(), frac.end(), 0.0F);
        frac[i] = 1;
        res_por[i]=littopor(frac, diam, por);
    }
    bool has_error = false;
    for (int i = 0; i < 4; ++i) {
        auto signed_diff = por[i] - res_por[i];
        float diff = std::abs(signed_diff);
        if (diff > 1e-5) {
            has_error = true;
            printf("por %d is calculated wrong", i);
        }
    }

    return has_error ? 1 : 0;
}

int gpm_porosity_test_full(int argc, char* argv[])
{
    const std::vector<float> diam = {1, .1f, .01f, .001f};
    const std::vector<float> por = {.3f, .34f, .4f, .55f};
    const std::vector<float> frac(4, 0.25);

    const auto res = littopor(frac, diam, por);
    const auto res_oracle = 0.234221950;
    const bool has_error = std::abs(res - res_oracle) > 1e-5;

    return has_error ? 1 : 0;
}

int gpm_permeability_test(int argc, char* argv[]) {
	std::vector<float> frac(4, 0);
	float mD_to_m2 = 0.9869f / 1E15f;
	float diam_base[] = { .001f, .0001f, .00001f, .000001f };
	float ipor_base[] = { .3f, .34f, .4f, .55f };
	float cpor_base[] = { .2f, .15f, .1f, .05f };
	float iper_base[] = { 100.0F*mD_to_m2, 10.0F*mD_to_m2, 0.1F*mD_to_m2, 0.01f*mD_to_m2 };
	float cper_base[] = { 10.0F*mD_to_m2, 1.0F*mD_to_m2, 0.1F*mD_to_m2, 0.001f*mD_to_m2 };
    float cperhtoperv_base[] = { 1.0F*mD_to_m2, 1.0F*mD_to_m2, 1.0F*mD_to_m2, 1.0F*mD_to_m2 };
	
	
	std::vector<float> diam(diam_base, diam_base + 4);
	std::vector<float> ipor(ipor_base, ipor_base + 4);
	std::vector<float> cpor(cpor_base, cpor_base + 4);
	std::vector<float> iper(iper_base, iper_base + 4);
	std::vector<float> cper(cper_base, cper_base + 4); 
    std::vector<float> cperhtoperv(cperhtoperv_base, cperhtoperv_base + 4);
    std::vector<float> res_per(4, 0);
	//SEDNAME =  'SAND1' 'SAND2' 'SILT' 'CLAY'
	//DIAMETER =  1 .1 .01 .001
	//GRAINDEN =  2.7 2.7 2.7 2.7
	//POR0 =  .3 .34 .4 .55

	for (int i = 0; i < 4; ++i) {
		std::fill(frac.begin(), frac.end(), 0.0F);
		frac[i] = 1;
		float pormix = cpor[i];
		res_per[i] = por_to_perm( pormix,  diam, frac,  ipor, cpor, iper, cper, cperhtoperv);
	}
	bool has_error = false;
	for (int i = 0; i < 4; ++i) {
		auto signed_diff = cper[i] - res_per[i];
		float diff = std::abs(signed_diff);
		if (diff > 1e-5) {
			has_error = true;
			printf("per %d is calculated wrong", i);
		}
	}

	return has_error ? 1 : 0;
}

