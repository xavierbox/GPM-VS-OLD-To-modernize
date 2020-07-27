#include "libgp.h"
#include "gpm_vbl_array_2d.h"

namespace Slb { namespace Exploration { namespace Gpm{

int antigradnc (int nrows, int ncols, const float_2darray_base_type& gradx, const float_2darray_base_type& grady, float_2darray_base_type& z)
{
	// Constructs a first approximation of z
	int i, j;

	// First element
	z(0,0) = 0;

	// First row
	for(j=1; j<ncols; j++){
		z(0,j) = z(0,j-1) + gradx(0,j);
	}

	// All others
	for(i=1; i<nrows; i++){
		z(i,0) = z(i-1,0) - grady(i,0);
		for(j=1; j<ncols; j++){
			z(i,j) = (z(i-1,j)-grady(i,j) + z(i,j-1)+gradx(i,j))/2.0f;
		}
	}

	return 0;
}

int antigradn (int nrows, int ncols, float_2darray_base_type* gradx_p, float_2darray_base_type* grady_p, float_2darray_base_type* z_p)
{
	// Calculates a grid z from gradients
	// Gradients are node centered, nrows by ncols
	// z is node centered, nrows by ncols
	// Alters gradx and grady
	// Output grid starts with z = 0 in lower left corner
	// Output grid may not honor gradients exactly
	/* nrows (in) = number of rows of nodes (minimum 2)
	ncols (in) = number of columns of nodes (minimum 2)
	gradx (in), grady(in) = gradient grids in vertical units per cell side
	z (ou) = output grid (must be allocated by caller)
	Return value = 0: OK
	-1: Error
	*/

	float diff;
	int i, j, k;
	int niter = 5000;
	int count;

	if(nrows<2 || ncols<2) return -1;

	float_2darray_base_type& gradx(*gradx_p);
	float_2darray_base_type& grady(*grady_p);
	float_2darray_base_type& z(*z_p);
	// Allocate auxiliary grid z1;
	float_2darray_type z1(nrows,ncols);

	// Calculate tie gradients from node-center gradients
	// Ties are inner (longitudinally one less than nodes)
	int iret = nod2tie(gradx_p, grady_p);
	if (iret!=0) return iret;

	// Assign initial node values
	antigradnc(nrows, ncols, gradx, grady, z);
	for(i=0; i<nrows; i++){
		for(j=0; j<ncols; j++){
			z1(i,j) = z(i,j);
		}
	}

	// Iterate
	for(k=0; k<niter; k++){
		for(i=0; i<nrows; i++){
			for(j=0; j<ncols; j++){
				diff = 0.0f;
				count = 0;
				if(i<nrows-1) {diff += (z(i+1,j  ) + grady(i  ,j  ) - z(i,j)); count++;}
				if(i>0      ) {diff += (z(i-1,j  ) - grady(i-1,j  ) - z(i,j)); count++;}
				if(j<ncols-1) {diff += (z(i  ,j+1) - gradx(i  ,j  ) - z(i,j)); count++;}
				if(j>0      ) {diff += (z(i  ,j-1) + gradx(i  ,j-1) - z(i,j)); count++;}
				z1(i,j) += diff/static_cast<float>(count);
			}
		}
		for(i=0; i<nrows; i++){
			for(j=0; j<ncols; j++){
				z(i,j) = z1(i,j);
			}
		}
	}

	// Free
	return 0;
}


}}}
