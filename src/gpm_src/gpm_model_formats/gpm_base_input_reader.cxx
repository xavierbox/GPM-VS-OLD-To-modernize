#include "gpm_base_input_reader.h"
#include "gpm_codes.h"
#include "gpm_control_file_io.h"
#include "parm_prop_descr.h"
#include <fstream>
#include "acf_base_reader.h"
#include "gpm_parm_methods.h"
#include "gpm_pars_methods.h"
#include "gpm_pars_helper_methods.h"
#include "grid_param_mapper.h"
#include "acf_base_helper.h"
#include "gpm_time_function_data_io.h"
#include "gpm_sediment_file_io.h"
#include "gpm_model_definition.h"

using namespace Slb::Exploration::Gpm::Tools;

namespace Slb { namespace Exploration { namespace Gpm {

int gpm_base_input_reader::initialize(const program_option_params& params,
                                     const gpm_logger& logger)
{
    if (!params.control_file_name.empty()) {
        // Skip flags, if any are found before a file name
        if (!std::ifstream(params.control_file_name, std::ios_base::in)) {
            logger.print(LOG_IMPORTANT, "Control file not found:'%s'\n", params.control_file_name.c_str());
            return GPM_CTL_FILE_MISSING;
        }
    }
    else {
        return GPM_CTL_FILE_MISSING;
    }

    logger.print(LOG_NORMAL, "Reading CTL file %s ...", params.control_file_name.c_str());
    // Number read
    gpm_control_file_io ctl_io;
    auto iret = ctl_io.read_parameters(params.control_file_name, logger, true);
    if (iret != 0) {
        return iret;
    }
    ctl_io.get_parms(params, &c);
    if (c.timbeg >= c.timend || c.timend - c.timbeg < c.tdisplay) {
        logger.print(
            LOG_NORMAL,
            "ERROR: The start time must be smaller than the end time of the simulation. Delta between start and end time shouldn’t be less than display interval.\n");
        return 2;
    }

    logger.print(LOG_NORMAL, "Done\n");


    logger.print(LOG_NORMAL, "Reading MAP file %s ...", c.mapfile.c_str());
    acf_base_reader::array_dim_size_type in_idimr;

    iret = gpm_read_map_file(c.mapfile, "MAP", p, inumr, in_idimr, logger);
    if (iret != 0) {
        return iret;
    }
    idimr = acf_base_helper::convert(in_idimr);
    p.version = parm_descr_holder::get_current_version();
    if (inumr[PARM_MEMBERS::BASE_ERODABILITY] == 0) {
        p.base_erodability = 1.0; // Simple copy for now
        logger.print(LOG_IMPORTANT, "Base erodability not set. Will be set to 1.0\n");
        inumr[PARM_MEMBERS::BASE_ERODABILITY] = 1;
        acf_base_helper::fill(&idimr[PARM_MEMBERS::BASE_ERODABILITY], 1);
    }
    logger.print(LOG_NORMAL, "Done\n");
    // Assign variables

    // Initialize variables (should go in prepin)
    // Find numtims (number of preexisting times)
    iret = gpm_map_struct_property_check(&p, c.timbeg, logger);
    if (iret != 0) {
        return iret;
    }
    iret = gpm_check_propnam_index_items(p, parm_descr_holder::get_grid_map_1_index_name_prefixes_as_strings(), logger); // So we don't need to check again, hopefully
    if (iret != 0) {
        return iret;
    }

    //**** FLW file *****/
    logger.print(LOG_NORMAL, "Reading FLW file %s ...", c.flowfile.c_str());

    gpm_time_function_data_io curve_pars;
    iret = curve_pars.read_parameters(c.flowfile, logger);
    if (iret != 0) {
        return iret;
    }
    if (c.seaflag) {
        if (!curve_pars.has_sealevel_curve()) {
            logger.print(LOG_NORMAL, "ERROR: Sea flag set but no sea level in flow file\n");
            return -1;
        }
    }
    // Setup the curves so we are ready to go
    sea = curve_pars.get_sealevel_interpolator();
	do_output_relative_sealevel_rate = curve_pars.do_output_relative_sealevel_rate();
    logger.print(LOG_NORMAL, "Done\n");

    //**** SED file *****/

    logger.print(LOG_NORMAL, "Reading SED file %s ...", c.sedfile.c_str());

    gpm_sediment_file_io sed_pars;
    iret = sed_pars.read_parameters(c.sedfile, logger);
    if (iret != 0) {
        return iret;
    }
    sed_pars.get_parms(&s);
    logger.print(LOG_NORMAL, "Done\n");

    iret = gpm_check_sediment_params(s, logger);
    if (iret != 0) {
        return iret;
    }
    const auto num_sediments_defined = s.numseds;
    sediment_id_index = gpm_sed_make_sediment_id_index(s);
    sediment_id_parm_index = gpm_find_sediment_id_propnam_map(p);
    // Check consistency
    if (sediment_id_parm_index.size() != sediment_id_index.size()) {
        logger.print(LOG_NORMAL, "Number of sediments defined in model doesn't match number of sediments in file\n");
        return 2;
    }
    std::vector<std::string> parm_sed_ids;
    std::transform(sediment_id_parm_index.begin(), sediment_id_parm_index.end(), std::back_inserter(parm_sed_ids),
                   [](const std::pair<std::string, std::pair<std::string, int>>& val) { return val.first; });
    if (!gpm_sed_has_all_ids(s, parm_sed_ids)) {
        logger.print(LOG_NORMAL, "Sediments ids in model doesn't match sediment ids in file\n");
        return 2;
    }
    for (const auto& it : sediment_id_parm_index) {
        if (it.second.second < 0) {
            logger.print(LOG_NORMAL, "Sediments ids in model doesn't have property in indprop\n");
            return 2;
        }
    }
    // Perm0 and 1 needs to be there
    // por0 and 1 needs to be there
    // Calculate por/per parameters
    gpm_calc_sed_por_and_prs(&s, num_sediments_defined, logger);

    // Calculate sediment transport coefficient if not provided
    iret = gpm_sed_check_transport(&s, num_sediments_defined, logger);
    if (iret != 0) {
        return iret;
    }

    // Lets do the model as well
	const int_extent_2d model_extent(p.numrows(), p.numcols());
	model_def.set_model_size(model_extent);
	model_def.set_world_coordinates(p.xcorners, p.ycorners);

    return iret;
}
}}}
