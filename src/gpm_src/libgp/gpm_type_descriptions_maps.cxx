#include "mapstc_descr.h"
#include "sedstc_descr.h"
#include "ssmstc_descr.h"
#include "ssmstc.h"
#include <boost/lexical_cast.hpp>
#include <string>
#include <vector>
#include <map>

namespace Slb { namespace Exploration { namespace Gpm {

std::string pars_descr_holder::current_version("1.1.0.0");
std::vector<TypeDescr> pars_descr_holder::pars_descr =
{make_typedescription<INTEGER>(NUMSEDSS, "NUMSEDS"),
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
    make_typedescription<STRING>(PARS_VERSION, "VERSION")};


std::string parc_descr_holder::current_version("1.2.1.0");
std::vector<TypeDescr> parc_descr_holder::parc_descr =
{make_typedescription<STRING>(MAPFILE, "MAPFILE"), // MAPFILE
    make_typedescription<STRING>(FLOWFILE, "FLOWFILE"), // FLOWFILE				    
    make_typedescription<STRING>(SEDFILE, "SEDFILE"), // SEDFILE				    
    make_typedescription<STRING>(CARBFILE, "CARBFILE"), // CARBFILE		
    make_typedescription<STRING>(OUTFILE, "OUTFILE"), // OUTFILE	
    make_typedescription<INTEGER>(SEDFLAG, "SEDFLAG", 2), // SEDFLAG: off, one, many		    
    make_typedescription<INTEGER>(SEAFLAG, "SEAFLAG", 1), // SEAFLAG: off, on			    
    make_typedescription<INTEGER>(FLOWFLAG, "FLOWFLAG", 2), // FLOWFLAG: off, grid, elem, extern, waves 
    make_typedescription<INTEGER>(ARROWS, "ARROWS", 1), // ARROWS: off, slop, wvel, wflow, wslop, se
    make_typedescription<INTEGER>(PARROWS, "PARROWS"), // PARROWS: off, vel, flow		    
    make_typedescription<INTEGER>(PORFLG, "PORFLAG"), // PORFLAG: off, on			    
    make_typedescription<INTEGER>(PRSFLG, "PRSFLAG"), // PRSFLAG: off, on			    
    make_typedescription<INTEGER>(COMPFLAG, "COMPFLAG"), // COMPFLAG: Compaction: off, load, load and
    make_typedescription<INTEGER>(CARBFLAG, "CARBFLAG"), // CARBFLAG: Carbonates: off, on	    
    make_typedescription<INTEGER>(PERMUN, "PERMUN"), // PERMUN: Permeability units		    
    make_typedescription<INTEGER>(CMPUN, "CMPUN"), // CMPUN: Compaction units		    
    make_typedescription<DOUBLE>(TIMBEG, "TIMBEG"), // TIMBEG				    
    make_typedescription<DOUBLE>(TIMEND, "TIMEND", 1000.), // TIMEND				    
    make_typedescription<DOUBLE>(TIMRUN, "TIMRUN", 1000.), // TIMRUN
    make_typedescription<DOUBLE>(TDISPLAY, "TDISPLAY", 1000.), // TDISPLAY				    
    make_typedescription<FLOAT>(ALTHICK, "ALTHICK", 1.f), // ALTHICK				    
    make_typedescription<FLOAT>(BASEROD, "BASEROD", 1.f), // BASEROD				    
    make_typedescription<FLOAT>(DTIME, "DTIME", 1E-4f), // DTIME				    
    make_typedescription<INTEGER>(ISAVE, "ISAVE", 1), // ISAVE				    
    make_typedescription<INTEGER>(DEPODEPTHFLAG, "DEPODEPTHFLAG", 1), // DEPODEPTHFLAG: off, sealevel, waterlevel 
    make_typedescription<INTEGER>(EROSIONFLAG, "EROSIONFLAG", 1), // EROSIONFLAG: off, erosion surface 
    make_typedescription<INTEGER>(SEACURVEEXTREMAFLAG, "SEACURVEEXTREMAFLAG", 1),
    make_typedescription<INTEGER>(POSTPROCESSFLAG, "POSTPROCESSFLAG", 1), // Shall we do processing
    make_typedescription<STRING>(POSTPROCESSFILE, "POSTPROCESSFILE"), //Where to write the grid
    make_typedescription<STRING>(POSTPROCESSPARMS, "POSTPROCESSPARMS"), //Where to write the grid
    make_typedescription<STRING>(PARC_VERSION, "VERSION"),
    make_typedescription<STRING>(AREA_SEDIMENT_PARMS, "AREA_SEDIMENT_PARMS"),
    make_typedescription<INTEGER>(DO_AREA_SEDIMENT, "DO_AREA_SEDIMENT"),
    make_typedescription<STRING>(POINT_SEDIMENT_PARMS, "POINT_SEDIMENT_PARMS"),
    make_typedescription<INTEGER>(DO_POINT_SEDIMENT, "DO_POINT_SEDIMENT"),
    make_typedescription<STRING>(DIFFUSION_PARMS, "DIFFUSION_PARMS"),
    make_typedescription<INTEGER>(DO_DIFFUSION, "DO_DIFFUSION"),
    make_typedescription<STRING>(STEADY_FLOW_PARMS, "STEADY_FLOW_PARMS"),
    make_typedescription<INTEGER>(DO_STEADY_FLOW, "DO_STEADY_FLOW"),
    make_typedescription<STRING>(UNSTEADY_FLOW_PARMS, "UNSTEADY_FLOW_PARMS"),
    make_typedescription<INTEGER>(DO_UNSTEADY_FLOW, "DO_UNSTEADY_FLOW"),
    make_typedescription<STRING>(WAVE_PARMS, "WAVE_PARMS"),
    make_typedescription<INTEGER>(DO_WAVES, "DO_WAVES"),
    make_typedescription<STRING>(LITHO_CLASSIFICATION_PARMS, "LITHO_CLASSIFICATION_PARMS"),
    make_typedescription<INTEGER>(DO_LITHO_CLASSIFICATION, "DO_LITHO_CLASSIFICATION"),
    make_typedescription<STRING>(COMPACTION_PARMS, "COMPACTION_PARMS"),
    make_typedescription<INTEGER>(DO_COMPACTION, "DO_COMPACTION"),
    make_typedescription<STRING>(TECTONICS_PARMS, "TECTONICS_PARMS"),
    make_typedescription<INTEGER>(DO_TECTONICS, "DO_TECTONICS"),
    make_typedescription<STRING>(MODEL_SAMPLING_PARMS, "MODEL_SAMPLING_PARMS"),
    make_typedescription<INTEGER>(DO_MODEL_SAMPLING, "DO_MODEL_SAMPLING"),
    make_typedescription<STRING>(DIAGENESIS_PARMS, "DIAGENESIS_PARMS"),
    make_typedescription<INTEGER>(DO_DIAGENESIS, "DO_DIAGENESIS"),
    make_typedescription<INTEGER>(RUN_OPTIMIZED_CODE, "RUN_OPTIMIZED_CODE")};


std::string parm_descr_holder::current_version("1.1.0.0");
parameter_descr_type parm_descr_holder::parm_descr =
{make_typedescription<STRING>(TITLE, "TITLE"), // 0			   
    make_typedescription<INTEGER>(NODORDER, "NODORDER"),
    make_typedescription<FLOAT>(XMIN, "XMIN"),
    make_typedescription<FLOAT>(XMAX, "XMAX", 30.),
    make_typedescription<FLOAT>(YMIN, "YMIN"),
    make_typedescription<FLOAT>(YMAX, "YMAX", 30.),
    make_typedescription<FLOAT>(XCORNERS, "XCORNERS", ARRAY_1d, 30.), // The x coordinates of the corners of rectangle
    make_typedescription<FLOAT>(YCORNERS, "YCORNERS", ARRAY_1d, 30.), // the y coordinates of the corners
    make_typedescription<FLOAT>(ZNULL, "ZNULL", -1E8f),
    make_typedescription<INTEGER>(NUMGRDS, "NUMGRDS", 1),
    make_typedescription<INTEGER>(NUMROWS, "NUMROWS", 31),
    make_typedescription<INTEGER>(NUMCOLS, "NUMCOLS", 31),
    make_typedescription<STRING>(GRIDNAM, "GRIDNAM", ARRAY_1d),
    make_typedescription<FLOAT>(ZGRID, "ZGRID", ARRAY_3d, -1E8f),
    make_typedescription<INTEGER>(NUMTIMS, "NUMTIMS", 1),
    make_typedescription<INTEGER>(NUMTOPS, "NUMTOPS", 1),
    make_typedescription<DOUBLE>(TIMESET, "TIMESET", ARRAY_1d),
    make_typedescription<DOUBLE>(AGESET, "AGESET", ARRAY_1d),
    make_typedescription<STRING>(PROPNAM, "PROPNAM", ARRAY_1d), // PROPNAM		   	   
    make_typedescription<INTEGER>(ISNODAL, "ISNODAL", ARRAY_1d), // ISNODAL			   			       
    make_typedescription<STRING>(INDPROP, "INDPROP", ARRAY_3d),

    make_typedescription<FLOAT>(ELEMDEP, "ELEMDEP"),
    make_typedescription<INTEGER>(NUMELEM, "NUMELEM"),
    make_typedescription<FLOAT>(ELEMX, "ELEMX", ARRAY_1d),
    make_typedescription<FLOAT>(ELEMY, "ELEMY", ARRAY_1d),
    make_typedescription<FLOAT>(ELEMVX, "ELEMVX", ARRAY_1d),
    make_typedescription<FLOAT>(ELEMVY, "ELEMVY", ARRAY_1d),
    make_typedescription<STRING>(PARM_VERSION, "VERSION"),
    make_typedescription<STRING>(BASE_LAYER_MAP, "BASE_LAYER_MAP", ARRAY_1d), // The sediment ids that are part of the base layer
    make_typedescription<STRING>(SED_PROP_MAP, "SED_PROP_MAP", ARRAY_2d), // First row is ids, second row is sediment name, third row is sed1 sed2 etc
    make_typedescription<FLOAT>(BASE_ERODABILITY, "BASE_ERODABILITY")} // The erodability of the base sediments
;

std::map<PARM_GRID_MAP_NAMES, std::string> parm_descr_holder::grid_map_name =
{{TOP, "TOP"},
    {SV_X, "SVX"},
    {SV_Y, "SVY"},
    {PVX, "PVX"},
    {PVY, "PVY"},
    {PVZ, "PVZ"},
    {POR, "POR"},
    {PRS, "PRS"},
    {DEPODEPTH, "DEPODEPTH"},
    {EROSION, "EROSION"},
    {SRC, "SRC"},
    {TEC, "TEC"},
    {TOP_SLOPE_X, "TOP_SLOPE_X"},
    {TOP_SLOPE_Y, "TOP_SLOPE_Y"},
    {WATER_TOP_FLUID, "WATER_TOP_FLUID"},
    {WATER_TOP_FLUID_VEL_X, "WATER_TOP_FLUID_VEL_X"},
    {WATER_TOP_FLUID_VEL_Y, "WATER_TOP_FLUID_VEL_Y"},
    {WATER_TOP_TURB, "WATER_TOP_TURB"},
    {WATER_TOP_TURB_VEL_X, "WATER_TOP_TURB_VEL_X"},
    {WATER_TOP_TURB_VEL_Y, "WATER_TOP_TURB_VEL_Y"},
    {LITHOLOGY, "LITHOLOGY"},
    {WAVE_ENERGY_DISSIP, "WAVE_ENERGY_DISSIP"},
    {WAVE_TRAVEL_TIME, "WAVE_TRAVEL_TIME"},
    {WAVE_DIR_X, "WAVE_DIR_X"},
    {WAVE_DIR_Y, "WAVE_DIR_Y"},
    {WAVE_VEL_EFF_X, "WAVE_VEL_EFF_X"},
    {WAVE_VEL_EFF_Y, "WAVE_VEL_EFF_Y"},
    {WAVE_LONGSHORE_DIR_X, "WAVE_LONGSHORE_DIR_X"},
    {WAVE_LONGSHORE_DIR_Y, "WAVE_LONGSHORE_DIR_Y"},
    {WAVE_LONGSHORE_VEL_X, "WAVE_LONGSHORE_VEL_X"},
    {WAVE_LONGSHORE_VEL_Y, "WAVE_LONGSHORE_VEL_Y"},
    {SED_MOVE_X, "SED_MOVE_X"},
    {SED_MOVE_Y, "SED_MOVE_Y"},
    {SEALEVEL, "SEALEVEL"},
    {REL_SEALEVEL_CHANGE_RATE, "REL_SEALEVEL_CHANGE_RATE"}};

std::map<PARM_GRID_VECTOR_2D_NAMES, std::string> parm_descr_holder::grid_map_vector_2d_name =
{{SV, "SV"},
    {TOP_SLOPE, "TOP_SLOPE"},
    {WATER_TOP_FLUID_VEL, "WATER_TOP_FLUID_VEL"},
    {WATER_TOP_TURB_VEL, "WATER_TOP_TURB_VEL"},
    {WAVE_DIR, "WAVE_DIR"},
    {WAVE_LONGSHORE_VEL, "WAVE_LONGSHORE_VEL"},
    {WAVE_LONGSHORE_DIR, "WAVE_LONGSHORE_DIR"},
    {WAVE_VEL_EFF, "WAVE_VEL_EFF"},
    {SED_MOVE, "SED_MOVE"}};
std::map<PARM_GRID_VECTOR_2D_NAMES, std::pair<PARM_GRID_MAP_NAMES, PARM_GRID_MAP_NAMES>> parm_descr_holder::grid_map_vector_2d =
{{SV, std::make_pair(SV_X, SV_Y)},
    {TOP_SLOPE, std::make_pair(TOP_SLOPE_X, TOP_SLOPE_Y)},
    {WATER_TOP_FLUID_VEL, std::make_pair(WATER_TOP_FLUID_VEL_X, WATER_TOP_FLUID_VEL_Y)},
    {WATER_TOP_TURB_VEL, std::make_pair(WATER_TOP_TURB_VEL_X, WATER_TOP_TURB_VEL_Y)},
    {WAVE_DIR, std::make_pair(WAVE_DIR_X, WAVE_DIR_Y)},
    {WAVE_LONGSHORE_VEL, std::make_pair(WAVE_LONGSHORE_VEL_X, WAVE_LONGSHORE_VEL_Y)},
    {WAVE_LONGSHORE_DIR, std::make_pair(WAVE_LONGSHORE_DIR_X, WAVE_LONGSHORE_DIR_Y)},
    {WAVE_VEL_EFF, std::make_pair(WAVE_VEL_EFF_X, WAVE_VEL_EFF_Y)},
    {SED_MOVE, std::make_pair(SED_MOVE_X, SED_MOVE_Y)}};
std::vector<PARM_GRID_MAP_NAMES> parm_descr_holder::grid_map_time_indexes =
{(DEPODEPTH),
    (EROSION),
    (WATER_TOP_FLUID),
    (WATER_TOP_TURB),
    (LITHOLOGY),
    (WAVE_ENERGY_DISSIP),
    (WAVE_TRAVEL_TIME),
    (WAVE_DIR_X),
    (WAVE_DIR_Y),
    (WAVE_VEL_EFF_X),
    (WAVE_VEL_EFF_Y),
    (WAVE_LONGSHORE_DIR_X),
    (WAVE_LONGSHORE_DIR_X),
    (WAVE_LONGSHORE_VEL_X),
    (WAVE_LONGSHORE_VEL_X)};

std::map<PARM_GRID_MAP_1_INDEX_NAMES, std::string> parm_descr_holder::grid_map_1_index_name_prefix =
{{SED_j, "SED"},
    {INSITU_GROWTH_j, "INSITU_GROWTH"},
    {RES_TIME_j, "RES_TIME"}};


std::string parm_descr_holder::grid_map_index_name(PARM_GRID_MAP_1_INDEX_NAMES id, int sed) {
    bool has_key = grid_map_1_index_name_prefix.find(id) != grid_map_1_index_name_prefix.end();
    std::string key = has_key ? grid_map_1_index_name_prefix[id] : "NOMATCH";
    return key + boost::lexical_cast<std::string>(sed);
}
}}}
