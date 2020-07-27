// -- Schlumberger Private --

#ifndef GPM_PARM_HELPER_METHODS_H
#define GPM_PARM_HELPER_METHODS_H

#include "gpm_parm_methods.h"
#include "acf_base_reader.h"
#include "acf_base_writer.h"
#include "gpm_logger.h"
#include <vector>

// This is where we add the methods that operate on struct parm
// That way it may become a bit more clear that parm is a class that should be like that
// In essence there are two major data structures in parm
//  - zgrid, which keeps all the arrays currently in the model, in
//        essence it models a vector of 2d arrays that hold a model property for parm
//  - indprop,  which is a vector of 2d arrays where the size of the 2d array is the number of surfaces in the model (number of columns
//        and the number of rows are the number of ages or times in the mode
//        The items in a 2d array have pointers to an index in zgrid, or a constant or an empty value
//  - gridnam has as many name entires as zgrid has 2d arrays, so the sixe of gridman should be the same as layers of z grid
//  - propnam, similar to gridnam, but is for the properties in the model

// These methods are spread across different files to 
namespace Slb {namespace Exploration {namespace Gpm {

void adjust_dimensions(const parm& p, acf_base_writer::parameter_num_type& inumr, acf_base_writer::array_dim_size_type& idimr,
                       const Tools::gpm_logger& logger);
void adjust_incremental_dimensions(const acf_base_writer::parameter_num_type& inumr_prev,
                                   const acf_base_writer::array_dim_size_type& idimr_prev,
                                   acf_base_writer::parameter_num_type& inumr,
                                   acf_base_writer::array_dim_size_type& idimr,
                                   const Tools::gpm_logger& logger);

}}}
#endif
