#include <math.h>

int invvec (int nrows, int ncols, float **vecx, float **vecy){
  // Inverts vector lengths (from vecx to deltx/vecx)
  int i,j;
  float vlen2;

  if(nrows<2 || ncols<2) return -1;

  for(i=0; i<nrows-1; i++){
    for(j=0; j<ncols-1; j++){
      vlen2 = vecx[i][j]*vecx[i][j]+vecy[i][j]*vecy[i][j];
      if(vlen2>0){
	vecx[i][j] /= vlen2;
	vecy[i][j] /= vlen2;
      }
    }
  }

  return 0;
}
