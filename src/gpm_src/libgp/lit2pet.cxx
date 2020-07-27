// -- Schlumberger Private --

#include "libgp.h"
#include "gpm_vbl_array_2d.h"
#include "gpm_function_2d.h"
#include <cmath>
#include <numeric>


namespace Slb { namespace Exploration { namespace Gpm { namespace {
bool correct_fractions(int num_zero, float sum_frac, std::vector<float>* frac, float diff = 1e-2)
{
    bool has_poro = true;
    if (num_zero == frac->size() || sum_frac < diff) {
        has_poro = false;;
    }
    else {
        if (std::abs(sum_frac - 1.0F) > diff) {
            std::for_each(frac->begin(), frac->end(), [&sum_frac](float& item) { item = item / sum_frac; });
        }
    }
    return has_poro;
}
}
bool find_fractions(int_extent_1d::index_type i, int_extent_1d::index_type j, const sediment_func_array_type& flith, std::vector<float>* frac_p)
{
    auto& frac(*frac_p);
    float sumfrac = 0;
    int num_zero = 0;
    for (int lith = 0; lith < flith.size(); ++lith) {
        frac[lith] = flith[lith].at(i, j);
        if (frac[lith] <= 0) {
            frac[lith] = 0;
            ++num_zero;
        }
        if (frac[lith] > 1.0F) frac[lith] = 1.0F;
        sumfrac += frac[lith];
    }
    return correct_fractions(num_zero, sumfrac, frac_p);
}


void lit_2_properties(const std::vector<float>& diameters,
                    const std::vector<float>& init_por0,
	                const std::vector<float>& init_por1,
                    const sediment_func_array_type& flith, float_2darray_base_type* por_p, 
	const std::vector<float>& init_perm0, const std::vector<float>& init_perm1, const std::vector<float>& init_permm, float_2darray_base_type* perh_p, float_2darray_base_type* perv_p) {
 

    float_2darray_base_type& por(*por_p);
	float_2darray_base_type& perh(*perh_p);
	float_2darray_base_type& perv(*perv_p);
    std::vector<float> frac(flith.size(), 0);


    const auto row_extent = por.row_extent();
    const auto col_extent = por.col_extent(); 
	

    // Porosity calculation
    for (auto i = row_extent.lower(), nr = row_extent.upper(); i < nr; i++) {
        for (auto j = col_extent.lower(), nc = col_extent.upper(); j < nc; j++) {
            if (!find_fractions(i,j, flith, &frac)){
                por(i, j) = 1.0F;  // Is this right? 
            }
            else {
				por(i, j) = littopor(frac, diameters, init_por0);
            }
			float pormix = por(i, j);
			//perh(i, j) = por_to_perm(pormix, diameters, frac, init_por0, init_por1, init_perm0,  init_perm1, init_permm);
            perh(i, j) = arithmetic_mean(frac, init_perm0);
			perv(i, j) = perh_to_perv(perh(i, j), frac, init_permm);
        }
    }
}

void lit_2_perm_coeff(const sediment_func_array_type& flith, const std::vector<float>& diam,
                      const std::vector<float>& por0, const std::vector<float>& por1, const std::vector<float>& perm0,
                      const std::vector<float>& perm1, const std::vector<float>& perhtoperv,
                      float_2darray_base_type* perm_factor_p, float_2darray_base_type* perm_exp_p, float_2darray_base_type* m_diameter_p, float_2darray_base_type
                      * perh_to_perv_p)
{
    float_2darray_base_type& perm_factor(*perm_factor_p);
    float_2darray_base_type& perm_exp(*perm_exp_p);
    float_2darray_base_type& mdiam(*m_diameter_p);
    float_2darray_base_type& mperhtoperv(*perh_to_perv_p);

    const int size = static_cast<int>(flith.size());
    std::vector<float> frac(size, 0);
    const auto row_extent = perm_factor.row_extent();
    const auto col_extent = perm_factor.col_extent();
    for (auto i = row_extent.lower(), nr = row_extent.upper(); i < nr; i++) {
        for (auto j = col_extent.lower(), nc = col_extent.upper(); j < nc; j++) {

            const bool has_fractions = find_fractions(i, j, flith, &frac);
            float sumwei = has_fractions ? std::accumulate(frac.begin(), frac.end(), 0.0F) : 0.0F;
            if (!find_fractions(i, j, flith, &frac)) {
                if (perm_factor.has_data()) perm_factor(i, j) = 0.0F;
                if (perm_exp.has_data()) perm_exp(i, j) = 0.0F;//""
            }
            else {
   
                //std::vector<float> perfac_5;
                //std::vector<float> perexp_5;
                float perfact;
                float perexp;
                float ediam;
                float eperhtoperv;
                //find_permability_coeffs_1(diam, por0, por1, perm0, perm1, &perfac_5, &perexp_5);
                //find_permability_coeffs_2_arithmetic_mean(diam, frac, perfac_5, perexp_5, perhtoperv, ediam, perfact, perexp, eperhtoperv);
                per_exp_fact_coeff( diam, frac, por0, por1, perm0, perm1, &perfact, &perexp);
    
                mdiam(i, j) = harmonic_mean(frac,diam);
                perm_factor(i, j) = perfact;
                perm_exp(i, j) = perexp;
                mperhtoperv(i, j) = arithmetic_mean(frac, perhtoperv);
            }
        }
    }
}


void lit_2_compaction_properties (const sediment_func_array_type& flith, const std::vector<float>& init_density,
                                  const std::vector<float>& init_comp, float_2darray_base_type* cmp_p, float_2darray_base_type* dmt_p)
{

    float_2darray_base_type& cmp(*cmp_p);
    float_2darray_base_type& dmt(*dmt_p);
    const int size = static_cast<int>(flith.size());
    std::vector<float> frac(size, 0);
    // Should check that fractions add to 1
    const auto row_extent = cmp.row_extent();
    const auto col_extent = dmt.col_extent();


    // Other properties
    for (auto i = row_extent.lower(), nr = row_extent.upper(); i < nr; i++) {
        for (auto j = col_extent.lower(), nc = col_extent.upper(); j < nc; j++) {

            const bool has_fractions = find_fractions(i, j, flith, &frac);
            float sumwei = has_fractions ? std::accumulate(frac.begin(), frac.end(), 0.0F) : 0.0F;
			if (!find_fractions(i, j, flith, &frac)) {
				if (cmp.has_data()) cmp(i, j) = 1000.0F; //Do we need and else to allocate memory?
				if (dmt.has_data()) dmt(i, j) = 2.67f;
			}
			else {
				auto plus_func = std::plus<float>();
				auto divide_func = std::divides<float>();
				if (cmp.has_data()) {
					//float sumcmp = std::inner_product(frac.begin(), frac.end(), cmp.begin(), 0.0F, plus_func, divide_func);
                    const float sumcmp = std::inner_product(init_comp.begin(), init_comp.end(), frac.begin(), 0.0F);
					cmp(i, j) = sumcmp;
				}
				else {
					//to create grid for this property. do we really need to output this value outside the class?
					//calculate the value
				}

				if (dmt.has_data()) {
					//float sumdmt = std::inner_product(frac.begin(), frac.end(), dmt.begin(), 0.0F, plus_func, divide_func);
                    const float sumdmt = std::inner_product(init_density.begin(), init_density.end(), frac.begin(), 0.0F);
					dmt(i, j) = sumdmt;
				}
				else {
					//Again, to create grid for this property. do we really need to output this value outside the class?
					//calculate the value
				}
			}
          }
       }

  }


float littopor(const std::vector<float>& frac, const std::vector<float>& diam, const std::vector<float>& por)
{
    // Converts sediment mixture of sediments to porosity, using:
    // frac = grain volume fraction of each of types
    // diam = grain diameter (MUST be in decreasing order)
    // por = initial porosity of each pure component.
    // OUTPUT:
    // pormix = Porosity of the mixture

    // Method: start with coarse fraction,
    // then successively "mix in" the finer fractions

    float psp = por[0] / (1.0F - por[0]) * frac[0]; // Pore space relative to all grain volume
    float fracnum = frac[0] / (diam[0] * diam[0] * diam[0]); // Number fraction (fraction of number of particles)

    for (int ilith = 1; ilith < frac.size(); ilith++) {
        // pore space (psp...) is relative to total grain volume
        // Calculate available pore space
        // (as if every particle in the mixture needed at least
        // the pore space used by each particle to be added)
        // First, find pore space per numberfraction (pspnf)
        const float pspnf = (por[ilith] / (1.0F - por[ilith])) *
            (diam[ilith] * diam[ilith] * diam[ilith]);
        // Multiply by already-processed numberfraction to get least pore space
        const float pspleast = pspnf * fracnum;
        // Find available, as existing minus least
        const float pspavail = psp - pspleast;
        // Does next sediment fraction with its pores fit in pores of previous?
        if (frac[ilith] / (1.0F - por[ilith]) < pspavail) {
            // Yes
            psp -= frac[ilith];
        }
        else {
            // No
            // First fill pores
            const float fracused = pspavail * (1.0F - por[ilith]);
            psp -= fracused;
            const float fracremain = frac[ilith] - fracused;
            // Then aggregate remaining (1 - total volume / grain volume)
            psp += fracremain * por[ilith] / (1.0F - por[ilith]);
        }
        // Update number fraction
        fracnum += frac[ilith] /
            (diam[ilith] * diam[ilith] * diam[ilith]); // Sum of processed number-fractions
    }

    const auto pormix = psp / (1.0F + psp);
    return pormix;
}

/*
int littoss (int nlith, float por, float *frac, float *diam, float *ssmix){
  // Calculates specific surface of a mixture of nlith sediments
  // nlith (in) = Number of components
  // por (in) = porosity of the mixture
  // frac (in) = Fraction (volume) of each component
  // diam (in) = diameter of each component in mm
  // ssmix (out) = specific surface of the mixture (units 1/m)
  // METHOD: Calculate grain volume per cubic m, then surface per cu m.
  // assuming spheres

  float grainvol; // Grain volume
  int ilith;

  //grainvol = 1.0F - por;
  *ssmix = 0;
  for(ilith=0; ilith<nlith; ilith++){
    *ssmix += (1.0F - por) * frac[ilith] * 3.0f * diam[ilith]/(2000.0F);
  }

  return 0;
}
*/

// Find the permeability coefficients needed
void find_permability_coeffs_1(const std::vector<float>& diameters, 
    const std::vector<float>& por_init, const std::vector<float>& por_curr, 
    const std::vector<float>& perm_init, const std::vector<float>& perm_curr,
    std::vector<float>* factors_p, std::vector<float>* exponents_p)
  
{
    std::vector<float>& perfac_5(*factors_p);
    std::vector<float>& perexp_5(*exponents_p);
    const auto nlith = diameters.size();
    for (int ilith = 0; ilith < nlith; ++ilith) {
        float x0;
        float x1;
        pprtopar(por_init[ilith], por_curr[ilith], perm_init[ilith], perm_curr[ilith], diameters[ilith], &x0, &x1);
        perfac_5.push_back(x0);
        perexp_5.push_back(x1);
    }
}

void find_permability_coeffs_2_harmonic_mean(const std::vector<float>& diameters, const std::vector<float>& fractions,
                               const std::vector<float>& perm_factors, const std::vector<float>& perm_exp, const std::vector<float>& perhtoperv,
                               float& ediam, float& efact, float& eexpo, float& eperhtoperv)
{
    const auto plus_func = std::plus<float>();

    const float sum = std::inner_product(fractions.begin(), fractions.end(), diameters.begin(), 0.0F, plus_func, [](float frac, float diam) {return diam != 0.0 ? frac / diam : 0.0F; });
    const float fact = std::inner_product(fractions.begin(), fractions.end(), perm_factors.begin(), 0.0F, plus_func,[](float frac, float permf) {return permf != 0.0 ? frac / permf : 0.0F; });
    const float expo = std::inner_product(fractions.begin(), fractions.end(), perm_exp.begin(), 0.0F, plus_func, [](float frac, float permex) {return permex != 0.0 ? frac / permex : 0.0F; });
    const float fperhtoperv = std::inner_product(fractions.begin(), fractions.end(),perhtoperv.begin(), 0.0F, plus_func, [](float frac, float perhtv) {return perhtv != 0.0 ? frac / perhtv : 0.0F; });

    ediam = sum != 0.0F ? 1.0F / sum : 0.0F;
    efact = fact != 0.0F ? 1.0F / fact : 0.0F;
    eexpo = expo != 0.0F ? 1.0F / expo : 0.0F;
    eperhtoperv = fperhtoperv != 0.0F ? 1.0 / fperhtoperv : 0.0F;
}

void find_permability_coeffs_2_arithmetic_mean(const std::vector<float>& diameters, const std::vector<float>& fractions,
    const std::vector<float>& perm_factors, const std::vector<float>& perm_exp, const std::vector<float>& perhtoperv,
    float& ediam, float& efact, float& eexpo, float& eperhtoperv)
{
    auto plus_func = std::plus<float>();

    ediam = std::inner_product(fractions.begin(), fractions.end(), diameters.begin(),0.0F);
    efact = std::inner_product(fractions.begin(), fractions.end(), perm_factors.begin(), 0.0F);
    eexpo = std::inner_product(fractions.begin(), fractions.end(), perm_exp.begin(), 0.0F);
    eperhtoperv = std::inner_product(fractions.begin(), fractions.end(), perhtoperv.begin(), 0.0F);

}

float por_to_perm_with_coeffs(const float por_mix, const float ediam, const float efact, const float eexpo)
{
    const float permix = efact*pow(por_mix, eexpo)*ediam*ediam;
	return permix;
}



float por_to_perm(float por_mix, const std::vector<float>& diameters, const std::vector<float>& fractions, const std::vector<float>& por_init, const std::vector<float>& por_curr, const std::vector<float>& perm_init, const std::vector<float>& perm_curr, const
                  std::vector<float>& perh_to_perv)
{
    // assert. por_init and por_1 can not be equal on the same indexes
    // !std::equal(por_init.begin(), por_init.end(), por_curr.begin())
    std::vector<float> perfac_5;
    std::vector<float> perexp_5;
    float perfact;
    float perexp;
    float ediam;
    float eperhtoperv;
    find_permability_coeffs_1(diameters, por_init, por_curr, perm_init, perm_curr, &perfac_5, &perexp_5);
    find_permability_coeffs_2_arithmetic_mean(diameters, fractions, perfac_5, perexp_5, perh_to_perv, ediam, perfact, perexp, eperhtoperv);
    const float permix = por_to_perm_with_coeffs(por_mix, ediam, perfact, perexp);
    return permix;
}

float perh_to_perv (float perh, const std::vector<float>& fractions, const std::vector<float>& init_perm)
{
	//auto plus_func = std::plus<float>();
	//auto divide_func = std::divides<float>();
	//float coef_per = std::inner_product(fractions.begin(), fractions.end(), init_perm.begin(), 0.0F, plus_func, divide_func);
    const float coef_per = std::inner_product(fractions.begin(), fractions.end(), init_perm.begin(), 0.0F);
	//coef_per = 1.0F / coef_per;
    const float perv = perh*coef_per;
	return perv;
}

float perh_to_perv(const float perh, const float coef_per)
{
    const float perv = perh*coef_per;
    return perv;
}
void portoper(int nlith, float* frac, const std::vector<float>& diam, float por, float* permix) {
    // Calculates permeability from porosity, fractions and diameters
    // nlith = number of components
    // frac = grain volume fraction of each of (nlith-1) types
    // diam = grain diameter (MUST be in decreasing order)
    // por = Porosity of the mixture
    // OUTPUT:
    // permix = Permeability of the mixture
    // Method:
    // Find equivalent diameter by weighted harmonic averaging
    // Apply Kozeny Carman for spheres (see "The Rock Physics Handbook");

    const float fact = 15.0f;

    float sum = 0; // Sum of squares and cubes of diameters
    for (int ilith = 0; ilith < nlith; ilith++) {
        sum += frac[ilith] / diam[ilith];
    }
    const float ediam = 1.0F / sum; // Equivalent diameter (in mm)

    //*per = fact*por*por*por*ediam*ediam*1E6f; // Bourbie et al. (1987)
    *permix = fact * por * por * por * ediam * ediam / (1.0F - por * por) * 1E6f;
}

void pprtopar(float por0, float por1, float per0, float per1, float diameter, float* factor, float* expon) {
    // PURPOSE: Finds parameters (factor and expon) from por and per at 2 different times
    // ARGUMENTS:
    // por0, por1 (in): Porosity at two different times
    // per0, per1 (in): permeability at two different times
    // factor (ou): factor
    // expon (ou): exponent
    // RETURN: 0: OK, other: error
    // METHOD: solve the fol. system for factor and expon
    // per0 = factor*pow(por0, expon);
    // per1 = factor*pow(por1, expon);


    // Should check for por or per 0

    // Convert to x0 + a0*x1 = b0
    //            x0 + a1*x1 = b1
    if (per0 > 0.0 && per1 > 0.0F && por0 > 0.0 && por1 > 0.0)
    {
        const float b0 = log(per0);
        const float b1 = log(per1);
        const float a0 = log(por0);
        const float a1 = log(por1);
        const float c = 2 * log(diameter);
        if ((a0 - a1) == 0.0F) {
            const float x1 = 0.0F;
            const float x0 = b0 - c;
            *factor = exp(x0);
            *expon = x1;
        }
        else {
            const float x1 = (b0 - b1) / (a0 - a1);
            const float x0 = b0 - a0 * x1 - c;
            *factor = exp(x0);
            *expon = x1;
        }
    }
    else {
        // returning 0, 
        *factor = 0.0F;
        *expon = 0.0F;
    }
}

float perfmpor(float por, float factor, float expon) {
    // Permeability from porosity (and parameters factor and expon)
    if (por > 0 && por < 1.0F) return static_cast<float>(factor * pow(por, expon));
    return 0;
}



void lit_2_porosity(const std::vector<float>& diameters,
    const std::vector<float>& init_porosity,
    const sediment_func_array_type& flith, float_2darray_base_type* por_p) {
    // Finds depositional petrophysical properties from lithology
    // INPUT:
    // s = sediment property structure
    // nr, nc = number of rows and columns
    // flith = (input) pointer to pointers of lithology fractions,
    //          excluding last, wich is calculated from remaining
    // OUTPUT: Grids must be allocated by caller
    // por = (in/ou) pointer to porosity grid

    float_2darray_base_type& por(*por_p);
    std::vector<float> frac(flith.size(), 0);

    // Should check that fractions add to 1
    const auto row_extent = por.row_extent();
    const auto col_extent = por.col_extent();

    // Porosity calculation
    for (auto i = row_extent.lower(), nr = row_extent.upper(); i < nr; i++) {
        for (auto j = col_extent.lower(), nc = col_extent.upper(); j < nc; j++) {
            if (!find_fractions(i, j, flith, &frac)) {
                por(i, j) = 0.0F;  // Is this right
            }
            else {
                por(i, j) = littopor(frac, diameters, init_porosity);
            }
        }
    }
}

float perm_from_lith_coeff(float por_mix, const std::vector<float>& diameters, const std::vector<float>& fractions,
                           const std::vector<float>& por_init, const std::vector<float>& por_comp,
                           const std::vector<float>& perm_init, const std::vector<float>& perm_comp, const
                           std::vector<float>& perh_to_perv)
{
    float perexp;
    float perfact;
    const float ediam = harmonic_mean(fractions, diameters);
    per_exp_fact_coeff(diameters, fractions, por_init, por_comp, perm_init, perm_comp, &perfact, &perexp);
    const float permix = por_to_perm_with_coeffs(por_mix, ediam, perfact, perexp);
    return permix;
}

void per_exp_fact_coeff(const std::vector<float>& diameters, const std::vector<float>& fractions,
                        const std::vector<float>& por_init, const std::vector<float>& por_comp,
                        const std::vector<float>& perm_init, const std::vector<float>& perm_comp, float* perfact,
                        float* perexp)
{
    float ipormix = littopor(fractions, diameters, por_init);
    if (ipormix < 0.01f) ipormix = 0.01f;
    float cpormix = littopor(fractions, diameters, por_comp);
    if (cpormix < 0.01f) cpormix = 0.01f;
    const float ipermix = arithmetic_mean(fractions, perm_init);
    const float cpermix = arithmetic_mean(fractions, perm_comp);
    const float ediam = harmonic_mean(fractions, diameters);
    float pfact;
    float pexp;
    pprtopar( ipormix, cpormix, ipermix, cpermix, ediam, &pfact, &pexp);
    *perexp = pexp;
    *perfact = pfact;
}

float arithmetic_mean(const std::vector<float>& fractions, const std::vector<float>& value)
{
    return std::inner_product(fractions.begin(), fractions.end(), value.begin(), 0.0F);
}

float harmonic_mean(const std::vector<float>& fractions, const std::vector<float>& value)
{
    const auto plus_func = std::plus<float>();
    const float sum = std::inner_product(fractions.begin(), fractions.end(), value.begin(), 0.0F, plus_func,
                                   [](float frac, float val) { return val != 0.0 ? frac / val : 0.0F; });
    const float h_mean = sum != 0.0F ? 1.0F / sum : 0.0F;
    return h_mean;
}

}}}

