#include <stdio.h>
#include <math.h>
#include "libgp.h"

//int littopor (int nlith, float *frac, float *diam, float *por0, float *por, float *per);
 
int main(){
  int nlith, iret;
  float val1 = 1.0f, val2 = 1.0f, val3 = 1.0f;
  float frac[100], por[100], diam[100], portot, pertot;

  // Input
  nlith = 0;
  while(val1>0){
    printf("Enter frac(f), por(f), diam(mm) (x to exit):");
    iret = scanf("%f %f %f", &val1, &val2, &val3);
    if(iret<3) break;
    if(val1<=0|| val2<=0) break;

    frac[nlith] = val1;
    por[nlith] = val2;
    diam[nlith] = val3;
    nlith++;
    if(nlith>=100) break;
    printf("\n");
  }
  if(nlith<=0){
    printf("Nothing to process\n");
    return 1;
  }

  // Processing
  iret = littopor(nlith, frac, diam, por, &portot, &pertot);

  // Output
  printf("Total por = %f, perm = %f\n",portot, pertot);

  return 0;
}
