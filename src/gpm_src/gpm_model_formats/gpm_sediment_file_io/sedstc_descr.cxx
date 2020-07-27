#include "sedstc_descr.h"

namespace Slb { namespace Exploration { namespace Gpm {

std::string pars_descr_holder::get_current_version() { return "1.1.3.0"; }

const std::vector<TypeDescr>& pars_descr_holder::get_pars_descr()
{
    static std::vector<TypeDescr> pars_descr =
    {
        make_typedescription<INTEGER>(NUMSEDSS, "NUMSEDS"),
        make_typedescription<STRING>(SEDNAME, "SEDNAME", ARRAY_1d),
        make_typedescription<STRING>(SEDCOMNT, "SEDCOMNT", ARRAY_1d),
        make_typedescription<STRING>(ID, "ID", ARRAY_1d),
        make_typedescription<FLOAT>(DIAMETER, "DIAMETER", ARRAY_1d),
        make_typedescription<FLOAT>(GRAINDEN, "GRAINDEN", ARRAY_1d),
        make_typedescription<FLOAT>(FALLVEL, "FALLVEL", ARRAY_1d),
        make_typedescription<FLOAT>(DIFFCF, "DIFFCF", ARRAY_1d),
        make_typedescription<FLOAT>(TRANSP, "TRANSP", ARRAY_1d),
        make_typedescription<FLOAT>(POR0, "POR0", ARRAY_1d),
        make_typedescription<FLOAT>(POR1, "POR1", ARRAY_1d),
        make_typedescription<FLOAT>(PERM0, "PERM0", ARRAY_1d),
        make_typedescription<FLOAT>(PERM1, "PERM1", ARRAY_1d),
        make_typedescription<FLOAT>(PERMANIS, "PERMANIS", ARRAY_1d),
        make_typedescription<FLOAT>(PFACTOR, "PFACTOR", ARRAY_1d),
        make_typedescription<FLOAT>(PEXPON, "PEXPON", ARRAY_1d),
        make_typedescription<FLOAT>(COMP, "COMP", ARRAY_1d),
        make_typedescription<FLOAT>(ERODABILITY_COEFF, "ERODABILITY_COEFF", ARRAY_1d),
        make_typedescription<FLOAT>(ERODABILITY_FUNC, "ERODABILITY_FUNC", ARRAY_3d),
        make_typedescription<STRING>(ERODABILITY_PROP, "ERODABILITY_PROP", ARRAY_1d),
        make_typedescription<FLOAT>(NO_VAL, "NO_VAL"),
        make_typedescription<STRING>(TRANSFORM_BY_TRANSPORT, "TRANSFORM_BY_TRANSPORT", ARRAY_1d),
        make_typedescription<INTEGER>(DO_INPLACE_TRANSFORM, "DO_INPLACE_TRANSFORM", ARRAY_1d),
        make_typedescription<FLOAT>(INPLACE_TRANSFORM_COEFF, "INPLACE_TRANSFORM_COEFF", ARRAY_1d),
        make_typedescription<STRING>(PARS_VERSION, "VERSION")
    };
    return pars_descr;
}
}}}
