#include "libutl.h"
#include "libgp.h"

int antigradnc (int nrows, int ncols, float **gradx, float **grady, float **z);

int antigradn (int nrows, int ncols, float **gradx, float **grady, float **z){
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

  float **z1, diff;
  int i, j, k;
  int niter = 10; //00; //10000;
  int iret;

  if(nrows<2 || ncols<2) return -1;

  // Allocate auxiliary grid z1;
  z1 = matrix(0,nrows-1,0,ncols-1);

  // Calculate tie gradients from node-center gradients
  // Ties are inner (longitudinally one less than nodes)
  iret = nod2tie(nrows, ncols, gradx, grady);
  if (iret!=0) return iret;

  // Assign initial node values
  antigradnc(nrows, ncols, gradx, grady, z);
  for(i=0; i<nrows; i++){
    for(j=0; j<ncols; j++){
      z1[i][j] = z[i][j];
    }
  }

  // Iterate
  for(k=0; k<niter; k++){
    for(i=0; i<nrows; i++){
      for(j=0; j<ncols; j++){
	diff = 0.0f;
	if(i<nrows-1) diff += (z[i+1][j  ] + grady[i  ][j  ] - z[i][j]) /4.0f;
	if(i>0      ) diff += (z[i-1][j  ] - grady[i-1][j  ] - z[i][j]) /4.0f;
	if(j<ncols-1) diff += (z[i  ][j+1] - gradx[i  ][j  ] - z[i][j]) /4.0f;
	if(j>0      ) diff += (z[i  ][j-1] + gradx[i  ][j-1] - z[i][j]) /4.0f;
	z1[i][j] += diff;
      }
    }
    for(i=0; i<nrows; i++){
      for(j=0; j<ncols; j++){
	z[i][j] = z1[i][j];
      }
    }
  }

  // Free
  free_matrix(z1,0,nrows-1,0,ncols-1);

  return 0;
}

int antigradnc (int nrows, int ncols, float **gradx, float **grady, float **z){
  // Constructs a first approximation of z
  int i, j;

  // First element
  z[0][0] = 0;

  // First row
  for(j=1; j<ncols; j++){
     z[0][j] = z[0][j-1] + gradx[0][j];
  }

  // All others
  for(i=1; i<nrows; i++){
    z[i][0] = z[i-1][0] - grady[i][0];
    for(j=1; j<ncols; j++){
      z[i][j] = (z[i-1][j]-grady[i][j] + z[i][j-1]+gradx[i][j])/2.0f;
    }
  }

  return 0;
}
