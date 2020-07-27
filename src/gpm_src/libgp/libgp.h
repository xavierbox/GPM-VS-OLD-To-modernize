// -- Schlumberger Private --

#ifndef _libgp_h_
#define _libgp_h_

#include "gpm_int_extent_1d.h"
#include "gpm_basic_defs.h"
#include <vector>


namespace Slb { namespace Exploration { namespace Gpm {

void gradn_cells(const gpm_array_2d_base<float>& z, float deltx, float delty, gpm_array_2d_base<float>* gradx_p,
                 gpm_array_2d_base<float>* grady_p);

void normvec(gpm_array_2d_base<float>* vecx_p, gpm_array_2d_base<float>* vecy_p);

void grid_filter(float_2darray_base_type& qx, const float_2darray_base_type& qxb, float vfacs);
void grid_filter(const int_extent_2d& ext, float_2darray_base_type& qx, const float_2darray_base_type& qxb,
                 float vfacs);

void nod2tie(gpm_array_2d_base<float>* gradx_p, gpm_array_2d_base<float>* grady_p);

void cell2nod(const gpm_array_2d_base<float>& cellprop_p, gpm_array_2d_base<float>* nodeprop_p);

// Following in littopet.cxx
bool find_fractions(int_extent_1d::index_type i, int_extent_1d::index_type j, const sediment_func_array_type& flith, std::vector<float>* frac_p);
void lit_2_properties(const std::vector<float>& diameters,
                      const std::vector<float>& init_por0,
                      const std::vector<float>& init_por1,
                      const sediment_func_array_type& flith, float_2darray_base_type* por_p,
                      const std::vector<float>& init_perm0, const std::vector<float>& init_perm1,
                      const std::vector<float>& init_permm, float_2darray_base_type* perh_p,
                      float_2darray_base_type* perv_p);

void lit_2_compaction_properties(const sediment_func_array_type& flith, const std::vector<float>& init_density,
                                 const std::vector<float>& init_comp, float_2darray_base_type* cmp_p,
                                 float_2darray_base_type* dmt_p);
void lit_2_perm_coeff(const sediment_func_array_type& flith, const std::vector<float>& diam,
                      const std::vector<float>& por0, const std::vector<float>& por1, const std::vector<float>& perm0,
                      const std::vector<float>& perm1, const
                      std::vector<float>& perhtoperv, float_2darray_base_type* perm_factor_p,
                      float_2darray_base_type* perm_exp_p, float_2darray_base_type* m_diameter_p,
                      float_2darray_base_type
                      * perh_to_perv_p);

void lit_2_porosity(const std::vector<float>& diameters,
                    const std::vector<float>& init_porosity,
                    const sediment_func_array_type& flith, float_2darray_base_type* por_p);

void find_permability_coeffs_1(const std::vector<float>& diameters,
                               const std::vector<float>& por_init, const std::vector<float>& por_curr,
                               const std::vector<float>& perm_init, const std::vector<float>& perm_curr,
                               std::vector<float>* factors_p, std::vector<float>* exponents_p);
void find_permability_coeffs_2_harmonic_mean(const std::vector<float>& diameters, const std::vector<float>& fractions,
                               const std::vector<float>& perm_factors, const std::vector<float>& perm_exp,
                               const std::vector<float>& perhtoperv,
                               float& ediam, float& efact, float& eexpo, float& eperhtoperv);
void find_permability_coeffs_2_arithmetic_mean(const std::vector<float>& diameters, const std::vector<float>& fractions,
    const std::vector<float>& perm_factors, const std::vector<float>& perm_exp,
    const std::vector<float>& perhtoperv,
    float& ediam, float& efact, float& eexpo, float& eperhtoperv);

float por_to_perm(float por_mix, const std::vector<float>& diameters, const std::vector<float>& fractions,
                  const std::vector<float>& por_init, const std::vector<float>& por_curr,
                  const std::vector<float>& perm_init, const std::vector<float>& perm_curr,
                  const std::vector<float>& perh_to_perv);
float por_to_perm_with_coeffs(float por_mix, float ediam, float efact, float eexpo);
float perh_to_perv(float perh, const std::vector<float>& fractions, const std::vector<float>& init_perm);
float perh_to_perv(float perh, float coef_per);

float littopor(const std::vector<float>& frac, const std::vector<float>& diam, const std::vector<float>& por0);
void portoper(int nlith, float* frac, const std::vector<float>& diam, float por, float* permix);
void pprtopar(float por0, float por1, float per0, float per1, float diameter, float* factor, float* expon);
float perfmpor(float por, float factor, float expon);
float arithmetic_mean(const std::vector<float>& fractions, const std::vector<float>& value);
float harmonic_mean(const std::vector<float>& fractions, const std::vector<float>& value);
void per_exp_fact_coeff( const std::vector<float>& diameters, const std::vector<float>& fractions,
                        const std::vector<float>& por_init, const std::vector<float>& por_comp,
                        const std::vector<float>& perm_init, const std::vector<float>& perm_comp, float* perfact,
                        float* perexp);


}}}
#endif
