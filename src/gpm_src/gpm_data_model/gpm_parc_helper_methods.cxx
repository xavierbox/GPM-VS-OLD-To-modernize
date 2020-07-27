#include "gpm_parc_helper_methods.h"
#include "parm_prop_descr.h"


namespace Slb { namespace Exploration { namespace Gpm {

void gpm_find_needed_output_properties(const parc& c, std::vector<property_description> * vals)
{
	if(c.do_top_slope){ // Slope
		vals->push_back(parm_descr_holder::make_property_description(TOP_SLOPE_X, "Top slope X", false));
		vals->push_back(parm_descr_holder::make_property_description(TOP_SLOPE_Y, "Top slope Y", false));
	}
	// Porosity
	if(c.depodepthflag > 0 ){
		vals->push_back(parm_descr_holder::make_property_description(DEPODEPTH, "Depth of deposition", false));
	}
	if (c.erosionflag){
		vals->push_back(parm_descr_holder::make_property_description(EROSION, "Erosion", false));
	}

	vals->push_back(parm_descr_holder::make_property_description(SEALEVEL, "Sea level", false));

	if (c.do_ogs_workflow) {
        vals->push_back(parm_descr_holder::make_property_description(OGS_PRESSURE, "Open GeoSys Pressure", false));
        vals->push_back(parm_descr_holder::make_property_description(OGS_VELOCITY_X, "Open GeoSys Velocity X Dir", false));
        vals->push_back(parm_descr_holder::make_property_description(OGS_VELOCITY_Y, "Open GeoSys Velocity Y Dir", false));
        vals->push_back(parm_descr_holder::make_property_description(OGS_VELOCITY_Z, "Open GeoSys Velocity Z Dir", false));
    }
}


}}}
