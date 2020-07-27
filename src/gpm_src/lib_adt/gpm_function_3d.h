// -- Schlumberger Private --

#ifndef _gpm_function_3d_h_
#define _gpm_function_3d_h_



//typedef boost::multi_array<int, 2> int_2darray_type;
#include "gpm_basic_defs.h"

namespace Slb { namespace Exploration { namespace Gpm {
	// This class is used as a public interface to extract a 2d array of a 3d field
	// The field is defined in x,y,time or (i,j,time)
	// Let the extent of the vals be filled in
	// the definition may vary quite a bit
	class function_3d{
	public:
		virtual ~function_3d()
	    = default;

	    // Not sure we need this one really
		// int_extent_2d model_xy_extent()=0;
		virtual void find_at(float t, float_2darray_base_type* vals)=0;
		virtual void integrate_span(double t0, double t1, float_2darray_base_type* vals)=0;
	};
}}}

#endif
