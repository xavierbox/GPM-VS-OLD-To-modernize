#include <math.h>
#include <stdio.h>

#include "libutl.h"
#include "libgeom.h" // roundiv(...)
#include "libfil.h" // sprintf8(...)
#include "mapstc.h"
#include "var3d.h"
#include "qparm.h"
#include "libdyn.h"

int indtoint(struct parm *p, int iprop, int **igrid){
  // Finds grid pointer matrix for property iprop as integers
  // **igrid: 2D int matrix allocated by caller
  // Places grid number in each entry of igrid, and -1 if value

    int itop, itim, iret, ival;

    for(itim=0; itim<p->numtims; itim++){
        for(itop=0; itop<p->numtops; itop++){
            iret = 0;
            if(p->indprop[iprop][itim][itop].is_index()){
                ival = p->indprop[iprop][itim][itop].index_value();
//                iret = sscanf(p->indprop[iprop][itim][itop]+1,"%d",&ival);
// Just fake it for now JT
                if ( iret == 0) igrid[itim][itop] = ival;
//                if(iret==1) igrid[itim][itop] = ival;
                else return -1; // Invalid entry in indprop
            }
            //if(p->indprop[iprop][itim][itop][0] == '#'){
            //    iret = sscanf(p->indprop[iprop][itim][itop]+1,"%d",&ival);
            //    if(iret==1) igrid[itim][itop] = ival;
            //    else return -1; // Invalid entry in indprop
            //}
            else //if(p->indprop[iprop][itim][itop][0] == '+' ||
                //   p->indprop[iprop][itim][itop][0] == '-') {
                igrid[itim][itop] = -1; // Flag that property is not a grid
            //}
        }
    }

    return 0;
}
