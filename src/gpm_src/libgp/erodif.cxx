/*#*#erodif.c */
#include "mapstc.h"
#include "libgp.h"
#include "gpm_sediment_model.h"
#include "gpm_find_sediment_intervals.h"
#include "libutl/gpm_logger.h"
#include "libutl/gpm_logging_scope.h"
#include <numeric>


namespace Slb { namespace Exploration { namespace Gpm{
namespace 
{

void find_erodibility_factors(const gpm_sediment_model& the_funcs,	int index, sediment_mix_iterator& fraction)
{
	const auto& erod(the_funcs.erodability_funcs());
	for( int it = 0;  it < erod.size();++it){
		auto sedi = erod[it][index];	
		fraction[it] = sedi;
	}
}

void update_thickness(const gpm_sediment_model& the_funcs,  int i, int j,
		float distance, int index, sediment_mix_iterator& sedthck)
{
	// Here sedthich.size = the_funcs.sed_functions.size + 1
	float sum=0;
	int it;
	for( it = 0;  it < the_funcs.sed_functions.size();++it){
		float sedi = the_funcs.sed_functions[it][index](i,j);	
		sum += sedi;
		sedthck[it] += sedi*distance;
	}
}

void multiply_add_to_fractions(const sediment_mix_iterator& fraction,  
		float distance, sediment_mix_iterator& sedthck)
{
	// Here sedthich.size = the_funcs.sed_functions.size + 1
	for(auto it = 0;  it < fraction.size();++it){
		auto sedi = fraction[it];	
		sedthck[it] += sedi*distance;
	}
}
}
int erodif(const gpm_sediment_model& the_funcs, 
		int i, int j,
	   float ztop, float zbot, sediment_mix_iterator& sedthck,
	   float baserod, sediment_mix_iterator *serod,
	   const Slb::Exploration::Gpm::Tools::gpm_logger& logger)
{
  // Used in diffusion algorithm
  // Determines total thickness of each lith between 2 elevations
  // This simplified version only uses composition at ztop
  // baserod (in) = basement erodibility
  // serod (ou) = active layer erodibility for this point
  /* Should be greatly sped up by defining an ordered q.zgrid,
     and intlit[numtims][numlays][ns](pointers to lithology grids),
     and flit[numtims][numlays][ns](constant lithologies) */

  int l;
  float ftop=ztop; // Initialize to a value
  Tools::gpm_logging_scope tmp_logger(logger, "erodif");

  float z_thickness = ztop-zbot;
  if(z_thickness < -0.01) return -1;

  int numsol = the_funcs.size();
  if ( numsol == 0){
	  return -2; // Shouldn't happen
  }

  // Blank the thing
  std::fill(sedthck.begin(), sedthck.end(),0.0f);
  // Here we really should use base erodability, and the cutoff 1e-3
  if ( z_thickness < 1e-6){ // All 0
	  if (the_funcs.top_functions[0](i,j) < ztop)
	  {
		  std::fill(serod->begin(), serod->end(), baserod);
	  }
	  else
	  {
		  std::fill(serod->begin(), serod->end(), 1.0f);
	  }
	  return 0;
  }
  // Check if within sequence, go down from top */
  for(l=numsol-1;l>=0;l--){
     ftop = the_funcs.top_functions[l](i,j);
    if(ftop<ztop) break;
  }
  l++;				// This is the desired layer
  if(l>=numsol){ 
	  return 1;	// Off top of sequence
  }
  int lowest_index = l;  // for determining erodability
  sediment_mix_iterator sum_erodibility(sedthck.size(), 0.0f);
  if ( l == 0 || ftop <= zbot){ // Bottom or in one layer
	   update_thickness(the_funcs,  i, j, z_thickness, l, sedthck);
	  find_erodibility_factors(the_funcs, l, sum_erodibility);
  }
  else {
	  std::vector<float> tops;
	  for(int k= 0 ; k <= l;++k){
		  tops.push_back(the_funcs.top_functions[k](i,j));
	  }
	  lowest_index = tops.size();
	  sediment_mix_iterator fractions(sedthck.size(), 0.0f);
	  boost::icl::split_interval_map<float, int> intervals = find_sediment_intervals(zbot, ztop, tops);
	  int interval_distance = std::distance(intervals.begin(), intervals.end());
	  for(boost::icl::split_interval_map<float, int>::iterator tmp = intervals.begin(); tmp != intervals.end(); ++tmp){
		  int loc_index = (*tmp).second;
		  float loc_thickness = (*tmp).first.upper() - (*tmp).first.lower();
		  update_thickness(the_funcs,  i, j, loc_thickness, loc_index, sedthck);
		  find_erodibility_factors(the_funcs,  loc_index, fractions);
		  multiply_add_to_fractions(fractions, loc_thickness/z_thickness, sum_erodibility);
		  lowest_index = std::min(lowest_index, loc_index);		  
	  }
  }
  // Set serod
  // (set to 0 if in basement, 1 otherwise)
  // This should use some average if we partially cut into the base
  // Currently it doesn't
  if (lowest_index <=0)
  {
	  std::fill(serod->begin(), serod->end(), baserod);
  }
  else
  {
	  std::copy(sum_erodibility.begin(), sum_erodibility.end(), serod->begin());
	  //std::fill(serod->begin(), serod->end(), 1.0f);
  }

  return 0;
}

}}}
