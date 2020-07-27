#include "ssmstc_descr.h"


namespace Slb { namespace Exploration { namespace Gpm {

std::string parc_descr_holder::get_current_version() { return "1.2.4.0"; }

const std::vector<TypeDescr>& parc_descr_holder::get_parc_descr()
{
    static std::vector<TypeDescr> parc_descr =
    {
        make_typedescription<STRING>(MAPFILE, "MAPFILE"), // MAPFILE
        make_typedescription<STRING>(FLOWFILE, "FLOWFILE"), // FLOWFILE				    
        make_typedescription<STRING>(SEDFILE, "SEDFILE"), // SEDFILE				    
        make_typedescription<STRING>(CARBFILE, "CARBFILE"), // CARBFILE		
        make_typedescription<STRING>(OUTFILE, "OUTFILE"), // OUTFILE	
        make_typedescription<INTEGER>(SEDFLAG, "SEDFLAG"), // SEDFLAG: off, one, many		    
        make_typedescription<INTEGER>(SEAFLAG, "SEAFLAG"), // SEAFLAG: off, on			    
        make_typedescription<INTEGER>(FLOWFLAG, "FLOWFLAG"), // FLOWFLAG: off, grid, elem, extern, waves 
        make_typedescription<INTEGER>(ARROWS, "ARROWS"), // ARROWS: off, slop, wvel, wflow, wslop, se
        make_typedescription<INTEGER>(PARROWS, "PARROWS"), // PARROWS: off, vel, flow		    
        make_typedescription<INTEGER>(PORFLG, "PORFLAG"), // PORFLAG: off, on			    
        make_typedescription<INTEGER>(PRSFLG, "PRSFLAG"), // PRSFLAG: off, on			    
        make_typedescription<INTEGER>(COMPFLAG, "COMPFLAG"), // COMPFLAG: Compaction: off, load, load and
        make_typedescription<INTEGER>(CARBFLAG, "CARBFLAG"), // CARBFLAG: Carbonates: off, on	    
        make_typedescription<INTEGER>(PERMUN, "PERMUN"), // PERMUN: Permeability units		    
        make_typedescription<INTEGER>(CMPUN, "CMPUN"), // CMPUN: Compaction units		    
        make_typedescription<DOUBLE>(TIMBEG, "TIMBEG"), // TIMBEG				    
        make_typedescription<DOUBLE>(TIMEND, "TIMEND"), // TIMEND				    
        make_typedescription<DOUBLE>(TIMRUN, "TIMRUN"), // TIMRUN
        make_typedescription<DOUBLE>(TDISPLAY, "TDISPLAY"), // TDISPLAY				    
        make_typedescription<FLOAT>(ALTHICK, "ALTHICK"), // ALTHICK				    
        make_typedescription<FLOAT>(BASEROD, "BASEROD"), // BASEROD				    
        make_typedescription<FLOAT>(DTIME, "DTIME"), // DTIME				    
        make_typedescription<INTEGER>(ISAVE, "ISAVE"), // ISAVE				    
		make_typedescription<INTEGER>(SAVE_LAST_TIMESTEP_ONLY, "SAVE_LAST_TIMESTEP_ONLY"), // 				    
		make_typedescription<INTEGER>(DEPODEPTHFLAG, "DEPODEPTHFLAG"), // DEPODEPTHFLAG: off, sealevel, waterlevel 
        make_typedescription<INTEGER>(EROSIONFLAG, "EROSIONFLAG"), // EROSIONFLAG: off, erosion surface 
        make_typedescription<INTEGER>(SEACURVEEXTREMAFLAG, "SEACURVEEXTREMAFLAG"),
        make_typedescription<INTEGER>(POSTPROCESSFLAG, "POSTPROCESSFLAG"), // Shall we do processing
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
        make_typedescription<INTEGER>(RUN_OPTIMIZED_CODE, "RUN_OPTIMIZED_CODE"),
        make_typedescription<INTEGER>(DO_LOG_PROCESS_ENERGY, "DO_LOG_PROCESS_ENERGY"),
        make_typedescription<STRING>(ISOSTATIC_TECTONICS_PARMS, "ISOSTATIC_TECTONICS_PARMS"),
        make_typedescription<INTEGER>(DO_ISOSTATIC_TECTONICS, "DO_ISOSTATIC_TECTONICS"),
        make_typedescription<STRING>(FLOW_SIMULATOR_OUTPUT_PARMS, "FLOW_SIMULATOR_OUTPUT_PARMS"),
        make_typedescription<INTEGER>(DO_FLOW_SIMULATOR_OUTPUT, "DO_FLOW_SIMULATOR_OUTPUT"),
        make_typedescription<STRING>(PELAGIC_RAIN_PARMS, "PELAGIC_RAIN_PARMS"),
        make_typedescription<INTEGER>(DO_PELAGIC_RAIN, "DO_PELAGIC_RAIN"),
        make_typedescription<INTEGER>(DO_PLUGINS, "DO_PLUGINS"),
        make_typedescription<STRING>(PLUGINS_CTRL_PARMS, "PLUGINS_CTRL_PARMS"),
		make_typedescription<INTEGER>(DO_BATHYMETRY, "DO_BATHYMETRY"),
		make_typedescription<INTEGER>(DO_SAVE_LAST_TIMESTEP_ONLY, "DO_SAVE_LAST_TIMESTEP_ONLY"),
		make_typedescription<INTEGER>(DO_OUTPUT_INCREMENTAL_FILES, "DO_OUTPUT_INCREMENTAL_FILES"),
		make_typedescription<INTEGER>(DO_INCLUDE_ACTIVE_LAYER, "DO_INCLUDE_ACTIVE_LAYER"),
    };
    return parc_descr;
}


}}}
