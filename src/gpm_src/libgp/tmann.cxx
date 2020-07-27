#include <stdio.h>
#include <math.h>

#include "libgp.h"

int main(){

  float
    k1 = 10000.f,
    k2 = 10.f,
    k3 = 10.f;
  float d, r, s, vl, vr, v;
  float sx, sy, vsx, vsy, vx, vy;
  int iret, nd;

  printf("d = ");
  scanf("%f",&d);

  printf("r = ");
  scanf("%f",&r);

  nd = 0;
  while (nd!=1 && nd!=2){
    printf("Select 1 (for 1D) or 2 (for 2D) = ");
    scanf("%d",&nd);
  }

  if(nd==1){

    printf("s = ");
    scanf("%f",&s);
    
    printf("vl = ");
    scanf("%f",&vl);
    
    printf("vr = ");
    scanf("%f",&vr);
    
    printf("v = ");
    scanf("%f",&v);
    
    iret = mann(k1, k2, k3, s, r, d, vl, vr, &v);
    
    printf("v = %f\n", v);
  }

  else{

    printf("sx sy = ");
    scanf("%f %f",&sx, &sy);
    
    printf("vsx, vsy = ");
    scanf("%f %f",&vsx, &vsy);
    
    printf("vx, vy = ");
    scanf("%f %f",&vx, &vy);
    
    iret = mann2d(k1, k2, k3, sx, sy, r, d, vsx, vsy, &vx, &vy);
    
    printf("vx, vy = %f %f\n", vx, vy);

  }

  return iret;
}
