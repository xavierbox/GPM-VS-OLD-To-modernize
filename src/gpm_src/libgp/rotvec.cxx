#include "libgp.h"
#include "gpm_vbl_array_2d.h"

namespace Slb { namespace Exploration { namespace Gpm{


int rotvec (int nrows, int ncols, float_2darray_base_type* vecx_p, float_2darray_base_type* vecy_p,
	    int iflag)
{
	// Rotates vectors 90 degrees clockwise or counterclockwise
	// vectors assumed node centered
	// can be cell centered by using nrows and ncols of cells
	// If original length is 0, it stays 0
	// nrows, ncols: number of rows and columns of nodes or cells
	// iflag: 0=clockwise, 1=counterclockwise


	if(nrows<2 || ncols<2) return -1;

	float_2darray_base_type& vecx(*vecx_p);
	float_2darray_base_type& vecy(*vecy_p);
	if(!iflag){ // clockwise
		for(int i=0; i<nrows; i++){
			for(int j=0; j<ncols; j++){
				float fhold = vecx(i,j);
				vecx(i,j) = vecy(i,j);
				vecy(i,j) = -fhold;
			}
		}
	}
	else { // counterclockwise
		for(int i=0; i<nrows; i++){
			for(int j=0; j<ncols; j++){
				float fhold = vecx(i,j);
				vecx(i,j) = -vecy(i,j);
				vecy(i,j) = fhold;
			}
		}
	}

	return 0;
}
}}}
