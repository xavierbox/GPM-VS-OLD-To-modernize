// -- Schlumberger Private --

#undef ENUMKEYWORD
#undef ENUMNAMESPACE

#ifdef MANAGED
#define ENUMKEYWORD public enum class
//#define ENUMNAMESPACE GPM_EnumShareManaged
#else
#define ENUMKEYWORD enum class
//#define ENUMNAMESPACE GPM_EnumShare
#endif
// Need to use this for inclusion in c++ and c++/cli

//namespace ENUMNAMESPACE {
ENUMKEYWORD TIME_FUNCTION_DATA_MEMBERS{
	XMIN=0,
	XMAX,
	ZNULL,
	GRIDNAM,
	ZGRID,
	OUT_RELATIVE_SEALEVEL_RATE,
	OUT_DEPOSITIONAL_ENERGY,
	VERSION,
	NTIME_FUNCTION_DATA_MEMBERS };

 ENUMKEYWORD TIME_FUNCTION_1_INDEX_NAMES {
	 SEA_i =0,   // Sea level SEAi
	 PRECIP_i,
	 NTIME_FUNCTION_1_INDEX_NAMES };

