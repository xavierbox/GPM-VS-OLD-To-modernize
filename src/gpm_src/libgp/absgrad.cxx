#include "libgp.h"
#include "gpm_vbl_array_2d.h"
#include <cmath>

namespace Slb { namespace Exploration { namespace Gpm{

int absgrad(int nrows, int ncols, const float_2darray_base_type& z,
	  float deltx, float delty, float_2darray_base_type* grad_p)
{
  // Finds the absolute value of the gradient of z
  // Input and result are node-centered

	float dx, dy;

	if(nrows<2 || ncols<2) return -1;

	float_2darray_base_type& grad(*grad_p);
	for(int i=0; i<nrows; i++){
		for(int j=0; j<ncols; j++){
			// dx
			if(j==0) dx = (z(i,j+1)-z(i,j))/deltx;
			else if(j==ncols-1) dx = (z(i,j)-z(i,j-1))/deltx;
			else dx = (z(i,j+1)-z(i,j-1)) / (2.0f*deltx);
			// dy
			if(i==0) dy = (z(i,j)-z(i+1,j))/delty;
			else if(i==nrows-1) dy = (z(i-1,j)-z(i,j))/delty;
			else dy = (z(i-1,j) - z(i+1,j)) / (2.0f*delty);
			grad(i,j) = static_cast<float>(sqrt(dx*dx+dy*dy));
		}
	}

	return 0;
}

}}}
