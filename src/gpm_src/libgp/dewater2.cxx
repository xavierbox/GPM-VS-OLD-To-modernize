#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "libgp.h"

int dewater2(float sig1, float vol1a, float vol1b,
	     float coma, float comb, float por1a, float por1b,
	     float dvolin, int niter,
	     float *por2a, float *por2b, float *sig2){
  /*
  Input values
  stot:  Total overburden stress
  sig1:  Grain stress before
  vol1a: Volume A before
  vol1b: Volume B before
  coma:  Compaction modulus A
  comb:  Compaction modulus B
  por1a: Porosity A before
  por1b: Porosity B before
  dvol:  Volume coming in
  niter: number of iterations: recommended: 3

  Output values:
  por2a: Porosity A after
  por2b: Porosity B after
  sig2:  Grain stress after
  */

  float dera, derb;
  float dvola, dvolb; // pore volume removed from each
  float sig2a, sig2b;
  float fact;
  int iter;
  float dvol; // Volume going out
  float epspor = 1E-5f;


  dvol = -dvolin;

  if(niter<=0) return -2;

  if(dvol > vol1a*por1a + vol1b*por1b) {
    printf("\nToo much volume, dvol=%f\n",dvol);
    return -1; // Too much volume removed
  }

  if(vol1a*por1a<=0 && vol1b*por1b <=0) {
    printf("Negative volume: %f %f  or por %f %f\n",vol1a, vol1b, por1a, por1b);
    return -1;
  }

  if( (dvol>=0 && dvol<vol1a*por1a*epspor && dvol<vol1b*por1b*epspor) ||
    (dvol<=0 && -dvol<vol1a*por1a*epspor && -dvol<vol1b*por1b*epspor) ){
    // No change
    *por2a = por1a;
    *por2b = por1b;
    *sig2 = sig1;
    return 0;
  }

  if(vol1a*por1a<=0){ // b only
    dvolb = dvol;
    //*por2b = 1.0f/(1.0f+(vol1b*(1.0f-por1b))/(vol1b*por1b-dvolb));
    //*por2b = 1.0f - vol1b/(vol1b-dvolb)*(1.0f-por1b);
    *por2b = (vol1b*por1b-dvolb)/(vol1b-dvolb);
    *sig2 = sig1 + comb*((float)log(por1b)-(float)log(*por2b));
    return 0;
  }
  if(vol1b*por1b<=0){ // a only
    dvola = dvol;
    //*por2a = 1.0f/(1.0f+(vol1a*(1.0f-por1a))/(vol1a*por1a-dvola));
    //*por2a = 1.0f - vol1a/(vol1a-dvola)*(1.0f-por1a);
    *por2a = (vol1a*por1a-dvola)/(vol1a-dvola);
    *sig2 = sig1 + coma*((float)log(por1a)-(float)log(*por2a));
    return 0;
  }

  // Temporary solution: give in proportion to derivative of pore volume
  // with respect to grain stress

  dera = vol1a*por1a/coma;
  derb = vol1b*por1b/comb;

  // Calculate volume changes
  dvola = dvol*dera/(dera+derb);
  dvolb = dvol*derb/(dera+derb);

  for(iter=0; iter<niter; iter++){
    if(dvola>=vol1a*por1a || dvolb>=vol1b*por1b){
      printf("\nToo much volume, iterating: dvola=%f dvolb=%f\n",dvola,dvolb);
      printf(" por1a=%f, por1b=%f, vol1a=%f, vol1b=%f\n", por1a, por1b, vol1a, vol1b);
      printf(" coma=%f, comb=%f, dvolin=%f, sig1=%f\n", coma, comb, dvolin, sig1);
      return -1;
    }
    
    // New porosities
    //*por2a = 1.0f/(1.0f+(vol1a*(1.0f-por1a))/(vol1a*por1a-dvola));
    //*por2a = 1.0f - vol1a/(vol1a-dvola)*(1.0f-por1a);
    *por2a = (vol1a*por1a-dvola)/(vol1a-dvola);
    //*por2a = por1a-dvola/vol1a; // Wrong
    //*por2b = 1.0f/(1.0f+(vol1b*(1.0f-por1b))/(vol1b*por1b-dvolb));
    //*por2b = 1.0f - vol1b/(vol1b-dvolb)*(1.0f-por1b);
    *por2b = (vol1b*por1b-dvolb)/(vol1b-dvolb);
    //*por2b = por1b-dvolb/vol1b; // Wrong
    
    // Calculate sigmas
    sig2a = sig1 + coma*((float)log(por1a)-(float)log(*por2a));
    sig2b = sig1 + comb*((float)log(por1b)-(float)log(*por2b));
    
    *sig2 = (sig2a+sig2b)/2.0f;
    //printf("%d: sig2=%f\n",iter,*sig2);
    
    // To solve recursively, continue as follows:
    // Back calculate *por2a and *por2b from *sig2
    // Back calculate dvola and dvolb
    // Adjust dvola and dvolb to add to dvol
    // Go back to "New porosities"
    
    // Back calculate *por2a and *por2b from *sig2
    fact = (float)exp((sig1-(*sig2))/coma);
    *por2a = por1a*fact;
    fact = (float)exp((sig1-(*sig2))/comb);
    *por2b = por1b*fact;
    
    // Back calculate dvola and dvolb
    //dvola = vol1a*(por1a - (1.0f-por1a)/(1.0f/(*por2a)-1.0f));
    //dvola = vol1a*(1.0f-(1.0f-por1a)/(1.0f-(*por2a)));
    dvola = vol1a*(por1a- *por2a)/(1.0f- *por2a);
    //dvolb = vol1b*(por1b - (1.0f-por1b)/(1.0f/(*por2b)-1.0f));
    //dvolb = vol1b*(1.0f-(1.0f-por1b)/(1.0f-(*por2b)));
    dvolb = vol1b*(por1b- *por2b)/(1.0f- *por2b);

    if(dvola+dvolb==0){
      // No change
      *por2a = por1a;
      *por2b = por1b;
      *sig2 = sig1;
      return 0;
    }
    
    // Adjust dvola and dvolb to add to dvol
    fact = dvol/(dvola+dvolb);
    dvola *= fact;
    dvolb *= fact;
    
    // Go back to "New porosities"
  }

  return 0;
}
