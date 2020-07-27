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
  ENUMKEYWORD ACL_PARSE_STATES {
    OK= 0,
    WARNING=-1,
    ERROR=-2,
    FILE_NOT_FOUND=-3,
    FILE_NOT_OPENED=-4,
    UNKNOWN_KEYWORD=-5,
    WRONG_FORMAT=-6,
    FILE_NOT_CLOSED=-7,
    FILE_NAME_EMPTY=-8,
    WRONG_DATA_CONTENT=-9,
    WRONG_SCHEMA_CONTENT=-10,
    WRONG_DATA_BY_SCHEMA=-11
  };
	// See also in gpm_type_description.h
	ENUMKEYWORD ACL_PARSE_POD_TYPES {INTEGER=0, FLOAT=1, STRING=2, DOUBLE=3};
	ENUMKEYWORD ACL_PARSE_ARRAY_TYPES {POD_VAL=0, ARRAY_1d=1, ARRAY_2d=2, ARRAY_3d=3}; 

    const float FLOAT_IO_NO_VALUE = -1e12f;

//}    

