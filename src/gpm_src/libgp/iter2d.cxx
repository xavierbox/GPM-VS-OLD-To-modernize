// iter2d.cxx/
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "libgp.h"
#include "libutl.h"

int iter2d(
	int nr, int nc,	// Number of rows and columns
	float **z,		// Topography at nodes 0 nr-1 x 0 nc-1 */
	float **h,		// Water depth 0 nr-1 x 0 nc-1 */
	float **qx,		// Previous vel. bet. nodes,  0 nr-1 x -1 nc-1
	float **qy,		// Previous vel. bet. nodes, -1 nr-1 x  0 nc-1
	float **qxb,            // Auxiliary new cell velocity
	float **qyb,
	float **bx,		// Depth bet. nodes,  0 nr-1 x -1 nc-1
	float **by,		// Depth bet. nodes, -1 nr-1 x  0 nc-1
	float deltx,		// Cell x-dimension (meters)
	float delty,		// Cell y-dimension (meters)
	float vrou,		// Viscosity/roughness coeff. (not used)
	float dt,		// Time inc. in seconds
	float slevnow,		// Sea level now
	float **srcgrid,	// Source grid
				//   >=0 on edge: no flow boundary
				//   >0: source
	float **fsrc,	        // Flow at source
	int ipycnal)		// Pycnal flag (-1: hypo, 0: iso, 1: hyper)
				//    depth h must already be prepared for each type of flow
{
  // Iterates steady flow calculation, and inputs fluid, self contained
  // Returns 0 if OK, 1 if possible instability
  int
    i,j,
    ierr;
  float
//    dh,		// Water level drop bet. nodes
//    b0,		// Depth at back node */
//    b1,		// Depth at forward node */
    bu,		// Temporary depth
    //qa,	// Adjusted veloc. bet. nodes */
    //dq,	// Change in veloc with time */
    //qlim,
    dtc,
    //hmax,
    dht;	// Water depth change this time
  //float vl, vr; // Velocities left and right of current tie

  // Manning coefficients
  float
    k1 = 10000.f,
    k2 = 10.f,
    k3 = 10.f;
  int iret;

  float qxnew, qynew;
  float vfact = 0.1f; // 0.1 Velocity adjustment factor, time (0: no change)
  float vfacs = 0.1f; // Velocity adjustment factor, space (0: no filter)

  float
    dqx, dqy,
    //dqmin = 0.0f,
    //dqmax = 0.0f, // Velocity changes
    //dhtmin = 0.0f,
    //dhtmax = 0.0f, // Depth changes
    densrat = 0.1f; // Sea density - river density
                    //   Test value: 1.0, actual: 0.01
  // int imin=-1, jmin=-1, imax=-1,  jmax=-1;

  float sum; // Auxiliary variables for spatial filtering
  int num;

  float **zh = 0; // Water surface elevation

  float celldep, sx, sy, vsx, vsy;
  float deltxy = (float)sqrt(deltx*delty);

  ierr = 0;
  //dtc = dt/vrou;
  dtc = dt;
  //hmax = 0.1f;			// Stability limit for h

  zh = matrix(0,nr-1,0,nc-1); // Allocate water surface elevation

  // Set zh
  if(ipycnal<=0){ // Hypopycnal or isopycnal
    for (i=0; i<nr; i++) {
      for (j=0; j<nc; j++) {
	if(z[i][j]+h[i][j]>slevnow) zh[i][j] = z[i][j] + h[i][j];
	else zh[i][j] = slevnow + h[i][j]*densrat;
      }
    }
  }
  else{ // Hyperpycnal
    for (i=0; i<nr; i++) {
      for (j=0; j<nc; j++) {
	if(z[i][j]+h[i][j]>slevnow) zh[i][j] = z[i][j] + h[i][j];
	else zh[i][j] = slevnow - (slevnow - (z[i][j]+h[i][j]))*densrat;
      }
    }
  }

  // Note: X and Y slopes have opposite signs than x & y derivatives

  // Find tie depths
  for (i=0; i<nr; i++) {
    bx[i][-1] = h[i][0]/2.0f;
    for (j=0; j<nc-1; j++) {
      //if(h[i][j]>0 && h[i][j+1]>0)
	bx[i][j] = (h[i][j]+h[i][j+1])/2.0f; // Could try harmonic avg.
      //else // limit if dry node
	//bx[i][j] = 0;
    }
    bx[i][nc-1] = h[i][nc-1]/2.0f;
  }
  for (j=0; j<nc; j++) {
    by[-1][j] = h[0][j]/2.0f;
    for (i=0; i<nr-1; i++) {
      //if(h[i][j]>0 && h[i+1][j+1]>0)
	by[i][j] = (h[i][j]+h[i+1][j])/2.0f;
      //else
	//by[i][j] = 0;
    }
    by[nr-1][j] = h[nr-1][j]/2.0f;
  }

  // Loop per cell
  for (i=0; i<nr-1; i++) {
    for (j=0; j<nc-1; j++) {
      // Find center-cell depth from surrounding nodes
      celldep = (h[i][j] + h[i+1][j] + h[i][j+1] + h[i+1][j+1])/4.0f;
      if(celldep<=0){
	qxb[i][j] = 0;
	qyb[i][j] = 0;
      }
      else{
	// Find center-cell slope
	sx = ((zh[i][j]-zh[i][j+1]) + (zh[i+1][j]-zh[i+1][j+1]))/2.0f/deltx;
	sy = ((zh[i+1][j]-zh[i][j]) + (zh[i+1][j+1]-zh[i][j+1]))/2.0f/delty;
	// Find surrounding velocity
	vsx = (qx[i][j] + qx[i+1][j])/2.0f;
	vsy = (qy[i][j] + qx[i][j+1])/2.0f;
	qxnew = qxb[i][j];
	qynew = qyb[i][j];
	// Solve
	iret = mann2d(k1, k2, k3, sx, sy, celldep, deltxy, vsx, vsy,
	  &(qxnew), &(qynew) );
	qxb[i][j] = qxb[i][j]+(qxnew-qxb[i][j])*vfact;
	qyb[i][j] = qyb[i][j]+(qynew-qyb[i][j])*vfact;
	//qxb[i][j] = qxnew;
	//qyb[i][j] = qynew;
      }
    }
  }

  // Filter velocities spatially
  for (i=0; i<nr; i++) {
    for (j=0; j<nc; j++) {
      sum = 0;
      num = 0;
      if(i>0   ){ sum += qxb[i-1][j]; num++;}
      if(i<nr-1){ sum += qxb[i+1][j]; num++;}
      if(j>-1  ){ sum += qxb[i][j-1]; num++;}
      if(j<nc-1){ sum += qxb[i][j+1]; num++;}
      qx[i][j] = qxb[i][j] + (sum/num-qxb[i][j])*0.5f*vfacs;
    }
  }
  for (i=0; i<nr; i++) {
    for (j=0; j<nc; j++) {
      sum = 0;
      num = 0;
      if(i>-1  ){ sum += qyb[i-1][j]; num++;}
      if(i<nr-1){ sum += qyb[i+1][j]; num++;}
      if(j>0   ){ sum += qyb[i][j-1]; num++;}
      if(j<nc-1){ sum += qyb[i][j+1]; num++;}
      qy[i][j] = qyb[i][j] + (sum/num-qyb[i][j])*0.5f*vfacs;
    }
  }

  // Copy
  for (i=0; i<nr; i++) {
    for (j=0; j<nc; j++) {
      qxb[i][j] = qx[i][j];
    }
  }
  for (i=0; i<nr; i++) {
    for (j=0; j<nc; j++) {
      qyb[i][j] = qy[i][j];
    }
  }

  // Calculate tie velocities
  for (j=0; j<nc-1; j++) {
    qx[0][j] = qxb[0][j]; // i = 0;
    for (i=1; i<nr-1; i++) {
      qx[i][j] = (qxb[i][j] + qxb[i-1][j])/2.0f; // Should use time filter
    }
    qx[nr-1][j] = qxb[nr-2][j]; // i = nr-1;
  }
  for (i=0; i<nr-1; i++) {
    qy[i][0] = qyb[i][0]; // j = 0;
    for (j=1; j<nc-1; j++) {
      qy[i][j] = (qyb[i][j] + qyb[i][j-1])/2.0f;
    }
    qy[i][nc-1] = qyb[i][nc-2]; // j = nc-1;
  }

  /*
  // Do edge ties, taking into account no-flow boundaries
  for (i=0; i<nr; i++) {
    // j = 0;
    if (srcgrid[i][0]>=0) qx[i][-1] = 0; // No-flow boundary
    else if(qx[i][0]>0)   qx[i][-1] = 0; // No inflow
    else                  qx[i][-1] = qx[i][0]; // Outflow
    // j = nc-1;
    if (srcgrid[i][nc-1]>=0) qx[i][nc-1] = 0; // No-flow boundary
    else if(qx[i][nc-2]<0)   qx[i][nc-1] = 0; // No inflow
    else                     qx[i][nc-1] = qx[i][nc-2]; // Outflow
  }
  for (j=0; j<nc; j++) {
    //i = -1;
    if (srcgrid[0][j]>=0) qy[-1][j] = 0; // No-flow boundary
    else if(qy[0][j]<0)   qy[-1][j] = 0; // No inflow
    else                  qy[-1][j] = qy[0][j]; // Outflow
    //i = nr-1;
    if (srcgrid[nr-1][j]>=0) qy[nr-1][j] = 0; // No-flow boundary
    else if(qy[nr-1][j]<0)   qy[nr-1][j] = 0; // No inflow
    else                     qy[nr-1][j] = qy[nr-2][j]; // Outflow
  }
  */

  // Limit flow from dry nodes
  for (i=0; i<nr; i++) {
    for (j=0; j<nc-1; j++) {
      if( (qx[i][j]>0 && h[i][j  ]<=0) ||
	  (qx[i][j]<0 && h[i][j+1]<=0) ){
	dqx = -qx[i][j];
	qx[i][j] = 0.0f;
      }
    }
  }
  for (i=0; i<nr-1; i++) {
    for (j=0; j<nc; j++) {
      if( (qy[i][j]>0 && h[i+1][j]<=0) ||
	  (qy[i][j]<0 && h[i  ][j]<=0) ){
	dqy = -qy[i][j];
	qy[i][j] = 0.0f;
      }
    }
  }

/*
  // Filter velocities spatially
  for (i=0; i<nr; i++) {
    for (j=-1; j<nc; j++) {
      sum = 0;
      num = 0;
      if(i>0   ){ sum += qxb[i-1][j]; num++;}
      if(i<nr-1){ sum += qxb[i+1][j]; num++;}
      if(j>-1  ){ sum += qxb[i][j-1]; num++;}
      if(j<nc-1){ sum += qxb[i][j+1]; num++;}
      qx[i][j] = qxb[i][j] + (sum/num-qxb[i][j])*0.5f*vfacs;
    }
  }
  for (i=-1; i<nr; i++) {
    for (j=0; j<nc; j++) {
      sum = 0;
      num = 0;
      if(i>-1  ){ sum += qyb[i-1][j]; num++;}
      if(i<nr-1){ sum += qyb[i+1][j]; num++;}
      if(j>0   ){ sum += qyb[i][j-1]; num++;}
      if(j<nc-1){ sum += qyb[i][j+1]; num++;}
      qy[i][j] = qyb[i][j] + (sum/num-qyb[i][j])*0.5f*vfacs;
    }
  }

  // Copy
  for (i=0; i<nr; i++) {
    for (j=-1; j<nc; j++) {
      qxb[i][j] = qx[i][j];
    }
  }
  for (i=-1; i<nr; i++) {
    for (j=0; j<nc; j++) {
      qyb[i][j] = qy[i][j];
    }
  }
*/
  // Update water surface by applying qx and qy (should use conductivities)
  for (i=0;i<nr;i++) {
    for (j=0;j<nc;j++) {
      // Net inflow rate
      dht = ( qx[i][j-1]*bx[i][j-1]-qx[i][j]*bx[i][j])*delty +
	    (-qy[i-1][j]*by[i-1][j]+qy[i][j]*by[i][j])*deltx;
      // Add source flow rate
      dht += fsrc[i][j];
      // Net depth change
      dht = dht/(deltx*delty)*dtc;

      bu = h[i][j];//-z[i][j];
      if(bu>0.01 && bu+dht<0.5f*bu && ierr<1) ierr = 1; // Rate of decrease too high
      if(bu>0.1 && bu+dht<0) ierr = 2; // Absolute decrease too high

      h[i][j] += dht; // Do it

      // Handle outflow boundaries
      if((i==0||i==nr-1||j==0||j==nc-1) &&
	  srcgrid[i][j]<0) h[i][j] = 0;
      // Limit depth (if limit too small, spurious flow occurs,
      //   if too big, no flow in sahllow water and low slope)
      if(h[i][j]<1E-8) h[i][j] = 0;

      //if(dht>dhtmax) dhtmax = dht;
      //if(dht<dhtmin) dhtmin = dht;
      /* printf("%d %d %f %f %f %f    %f\n",i,j,qx[i][j-1],qx[i][j],
	     qy[i-1][j],qy[i][j],h[i][j]); */
    }
  }
  // Find general maximum abs value depth change dhtmax
  //if(-dhtmin>dhtmax) dhtmax = -dhtmin;

  // Check stability by measuring velocity change

  //if(ierr>0) printf("iter: Unstable, decrease dt\n");
//#ifdef _DEBUG
//  printf("dqmax = %f dhtmax = %f: %d %d\n",dqmax,dhtmax, imax, jmax);
//#endif

  // Free zh
  free_matrix(zh,0,nr-1,0,nc-1);

  return ierr;
}
