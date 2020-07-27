// -- Schlumberger Private --

#ifndef _sedstc_h_
#define _sedstc_h_

#include "gpm_1d_interpolator.h"
#include <vector>
#include <string>

namespace Slb { namespace Exploration { namespace Gpm{

struct pars{
  int numseds;
  std::string version;
  std::vector<std::string>
    sedname,
    sedcomnt,
	sed_id; // Something to identify each item, since they may get sorted
  std::vector<float>
     diameter,
    grainden,
    fallvel,
    diffcf,
    transp,
    por0,
    por1,
    perm0,
    perm1,
    permanis,
    pfactor, // Permeability factor in equation: perm = factor * pow(por, pexpon)
    pexpon, // Permeability exponent
    comp,
	erodability; 
  std::vector<std::shared_ptr<gpm_1d_interpolator>> erode_funcs;// Erodability for each sediment
  std::vector<std::string> transform_by_transport;
    std::vector<int> do_inplace_transform;
    std::vector<float> inplace_transform_coeff;
    pars():numseds(0){} // All the rest is default constructed
};
typedef struct pars sed_parameter_type ;

}}}


#endif
