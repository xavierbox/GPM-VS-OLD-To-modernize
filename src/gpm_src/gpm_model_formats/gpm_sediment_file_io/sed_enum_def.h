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
// Need to use this for inclusion in c++ and c++

//namespace ENUMNAMESPACE{
  ENUMKEYWORD PARS_MEMBERS {
    NUMSEDSS= 0,
    SEDNAME,
    SEDCOMNT,
    DIAMETER,
    GRAINDEN,
    FALLVEL,
    DIFFCF,
    TRANSP,
    POR0,
    POR1,
    PERM0,
    PERM1,
    PERMANIS,
    PFACTOR,
    PEXPON,
    COMP,
	ID, // the id of each record
    PARS_VERSION,
	ERODABILITY_COEFF,  // Erodability function for each sediment
	ERODABILITY_FUNC,   // Time functions for change in erodability 
	ERODABILITY_PROP,   // Property of erodability change, either constant or index to ERODABILITY_FUNC
	NO_VAL, // No value in the ERODABILITY_FUNC
	TRANSFORM_BY_TRANSPORT, // If we have transport of sediment, transform to other sediment. ID of sediment if transform should happen
    DO_INPLACE_TRANSFORM, // DO_INPLACE_TRANSFORM_OPTIONS: If this is >0, transform sediments according to halflife coefficient. Deposting layer or exposed layer
    INPLACE_TRANSFORM_COEFF, // Half life coefficient for decay transformation of sediments
    NPARS};
  // DO_INPLACE_TRANSFORM_EXPOSED is really above sea level, BELOW is below sea level
  ENUMKEYWORD DO_INPLACE_TRANSFORM_OPTIONS{ DO_INPLACE_TRANSFORM_OFF = 0, DO_INPLACE_TRANSFORM_DEPOSITING=1, DO_INPLACE_TRANSFORM_EXPOSED=2,  DO_INPLACE_TRANSFORM_EXPOSED_BELOW=4 };

//}


