#include <stdio.h>
#include <math.h>

#include "libgp.h"

int main(){
  float 
    sig1 = 1703.8046f, //1E3,
    vol1a = 1322835.875f, //1E5f,
    vol1b = 1952520.0f, //2E5f,
    coma = 510.2040f, //1.0E5f,
    comb = 5000.0f, //2.0E5f,
    por1a = 0,//0.00147f,//0.1f,
    por1b = 0.2548f,//0.2f,
    dvol = -0.293589f,//-1.0E3f,

    por2a,
    por2b,
    sig2;

  int iret;

  iret = dewater2(sig1, vol1a, vol1b, coma, comb, por1a, por1b, dvol, 3,
    &por2a, &por2b, &sig2);

  printf("iret=%d\n",iret);
  printf("por2a=%f\n",por2a);
  printf("por2b=%f\n",por2b);
  printf("sig2 =%f\n",sig2);

  // Calculate new volume
  float rdvol, oldvol, newvol;
  oldvol = vol1a + vol1b;
  newvol = vol1a*(1.0f-por1a)/(1.0f-por2a) +
           vol1b*(1.0f-por1b)/(1.0f-por2b);
  rdvol = newvol - oldvol;

  printf("oldvol=%f, newvol=%f, rdvol=%f, dvol=%f\n",
	  oldvol, newvol, rdvol, dvol);

  return 0;
}
