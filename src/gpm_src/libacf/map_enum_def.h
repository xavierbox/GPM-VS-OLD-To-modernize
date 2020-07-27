// -- Schlumberger Private --

#undef ENUMKEYWORD
#undef ENUMNAMESPACE

#ifdef MANAGED
#define ENUMKEYWORD public enum class
//#define ENUMNAMESPACE GPM_EnumShareManaged
#else
#define ENUMKEYWORD enum
//#define ENUMNAMESPACE GPM_EnumShare
#endif
// Need to use this for inclusion in c++ and c++/cli

//namespace ENUMNAMESPACE {
  ENUMKEYWORD PARM_MEMBERS {
    TITLE= 0,
    NODORDER,
    XMIN_OBSOLETE,
    XMAX_OBSOLETE,
    YMIN_OBSOLETE,
    YMAX_OBSOLETE,
    XCORNERS,
    YCORNERS,
    ZNULL,
    NUMROWS,		   
    NUMCOLS,
    GRIDNAM,
    ZGRID,
    TIMESET,
    AGESET,
    PROPNAM,
    ISNODAL,
    INDPROP,
    ELEMDEP,
    NUMELEM,
    ELEMX,
    ELEMY,
    ELEMVX,
    ELEMVY,
    PARM_VERSION,
	BASE_LAYER_MAP, // All the sediment ids that are part of the base layer
	BASE_ERODABILITY, // The erodability of the base
	SED_PROP_MAP, // All the sediments ids to sediment names SED1, sed2 etc
    PROP_DISPLAY_NAME, // Display names of the properties 
    INDPROP_STATS, // The min and max value for each timestep, row index equality with Timeset, column will min, max
    HISTORY, // Here we store history of who writes to the file
	GREF_START_INDEX, // The global start index in the zgrid for the incremntal file
	GREF_END_INDEX, // The global start index in the zgrid for the incremntal file
    NPAR};
  
//}    

