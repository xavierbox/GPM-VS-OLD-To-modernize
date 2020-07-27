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
  ENUMKEYWORD SURF_DATA_MEMBERS {
    XCORNERS=0,
    YCORNERS,
    GRIDNAM,
    ZGRID,
	ZNULL,
    VERSION,
    NUM};

//}    

