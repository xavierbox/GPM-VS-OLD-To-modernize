// -- Schlumberger Private --

#include "gpm_control_file_io.h"
#include "acf_json_reader.h"
#include "gpm_logger.h"
#include <vector>
#include <string>
#include <iostream>

using namespace Slb::Exploration::Gpm;
std::vector<std::string> control_statement_list =
{ "{\"MAPFILE\":  \"singlecol.map\",",
"\"FLOWFILE\" :  \"singlecol.flw\",",
"\"SEDFILE\" :  \"singlecol.sed\",",
"\"CARBFILE\" :  \"\",",
"\"OUTFILE\" :  \"singlecol.out\",",
"\"SEDFLAG\" :  2,",
"\"SEAFLAG\" :  1,",
"\"FLOWFLAG\" :  0,",
"\"ARROWS\" :  0,",
"\"PARROWS\" :  0,",
"\"PORFLAG\" :  0,",
"\"PRSFLAG\" :  0,",
"\"COMPFLAG\" :  0,",
"\"CARBFLAG\" :  0,",
"\"PERMUN\" :  0,",
"\"CMPUN\" :  0,",
"\"TIMBEG\" :  0,",
"\"TIMEND\" :  1000,",
"\"TDISPLAY\" :  100,",
"\"ALTHICK\" :  1,",
"\"BASEROD\" :  1,",
"\"DTIME\" :  0.05,",
"\"VERSION\" : \"1.0.0.0\",",
"\"ISAVE\" :  0}" };

std::vector<std::string> control_statement_list_iso =
{ "{\"MAPFILE\":  \"singlecol.map\",",
"\"FLOWFILE\" :  \"singlecol.flw\",",
"\"SEDFILE\" :  \"singlecol.sed\",",
"\"CARBFILE\" :  \"\",",
"\"OUTFILE\" :  \"singlecol.out\",",
"\"SEDFLAG\" :  2,",
"\"SEAFLAG\" :  1,",
"\"FLOWFLAG\" :  0,",
"\"ARROWS\" :  0,",
"\"PARROWS\" :  0,",
"\"PORFLAG\" :  0,",
"\"PRSFLAG\" :  0,",
"\"COMPFLAG\" :  0,",
"\"CARBFLAG\" :  0,",
"\"PERMUN\" :  0,",
"\"CMPUN\" :  0,",
"\"TIMBEG\" :  0,",
"\"TIMEND\" :  1000,",
"\"TDISPLAY\" :  100,",
"\"ALTHICK\" :  1,",
"\"BASEROD\" :  1,",
"\"DTIME\" :  0.05,",
"\"ISAVE\" :  0,",
"\"VERSION\" : \"1.0.0.0\",",
"\"DO_ISOSTATIC_TECTONICS\" :  1," ,
"\"ISOSTATIC_TECTONICS_PARMS\" :  \"test_parms\"}" };

std::vector<std::string> control_statement_list_sim =
{ "{\"MAPFILE\":  \"singlecol.map\",",
"\"FLOWFILE\" :  \"singlecol.flw\",",
"\"SEDFILE\" :  \"singlecol.sed\",",
"\"CARBFILE\" :  \"\",",
"\"OUTFILE\" :  \"singlecol.out\",",
"\"SEDFLAG\" :  2,",
"\"SEAFLAG\" :  1,",
"\"FLOWFLAG\" :  0,",
"\"ARROWS\" :  0,",
"\"PARROWS\" :  0,",
"\"PORFLAG\" :  0,",
"\"PRSFLAG\" :  0,",
"\"COMPFLAG\" :  0,",
"\"CARBFLAG\" :  0,",
"\"PERMUN\" :  0,",
"\"CMPUN\" :  0,",
"\"TIMBEG\" :  0,",
"\"TIMEND\" :  1000,",
"\"TDISPLAY\" :  100,",
"\"ALTHICK\" :  1,",
"\"BASEROD\" :  1,",
"\"DTIME\" :  0.05,",
"\"ISAVE\" :  0,",
"\"VERSION\" : \"1.0.0.0\",",
"\"DO_FLOW_SIMULATOR_OUTPUT\" :  1," ,
"\"FLOW_SIMULATOR_OUTPUT_PARMS\" :  \"test_sim_parms\"}" };

int read_control_file_test(int /*argc*/, char*[] /*argv[]*/) {

    std::string buf;
    // Lets make the parser thing properly
    for (const auto& it : control_statement_list) {
        buf += it;
        buf += "\n";
    }
    struct parc c;
    gpm_control_file_io ctl_io;
    const Tools::gpm_default_logger logger(Tools::LOG_DEBUG, true);
    const auto iret = ctl_io.read_parameters_from_string(buf, logger, false);

    if (iret != 0) {

        printf("Parse errors\n");
    }
    return iret;
}

int read_control_file_test_iso(int /*argc*/, char* /*argv*/[]) {

	std::string buf;
	// Lets make the parser thing properly
	for (const auto& it : control_statement_list_iso) {
		buf += it;
		buf += "\n";
	}
	struct parc c;
	gpm_control_file_io ctl_io;
	const Tools::gpm_default_logger logger(Tools::LOG_DEBUG, true);
	auto iret = ctl_io.read_parameters_from_string(buf, logger, false);

	if (iret == 0) {
		// Check he numbers 
		{
			parc ctl_parms;
			ctl_io.get_parms(false,false, &ctl_parms);
			if (!ctl_parms.do_isostatic_tectonics)
			{
				std::cout << "Hasn't read the bool iso flag "<<std::endl;
				iret = 1;
			}
			if (ctl_parms.isostatic_tectonics_parms!="test_parms")
			{
				std::cout << "Hasn't read the iso parameters " << std::endl;
				iret += 1;
			}
		}
	}
	else {
		std::cout << "Parse errors " << std::endl;
	}
	return iret;
}

int read_control_file_test_sim(int /*argc*/, char*[] /*argv[]*/) {

    std::string buf;
    // Lets make the parser thing properly
    for (const auto& it : control_statement_list_sim) {
        buf += it;
        buf += "\n";
    }
    struct parc c;
    gpm_control_file_io ctl_io;
    const Tools::gpm_default_logger logger(Tools::LOG_DEBUG, true);
    int iret = ctl_io.read_parameters_from_string(buf, logger, false);

    if (iret == 0) {
        // Check he numbers 
        {
            // Array sizes should be 0 as well;
            parc ctl_parms;
            ctl_io.get_parms(false, false, &ctl_parms);
            if (!ctl_parms.do_flow_simulator_output)
            {
				std::cout << "Hasn't read the bool simulator flag" << std::endl;
                iret = 1;
            }
            if (ctl_parms.flow_simulator_output_parms != "test_sim_parms")
            {
				std::cout << "Hasn't read the sim parameters" << std::endl;
                iret += 1;
            }
        }
    }
    else {
		std::cout << "Parse errors" << std::endl;
    }
    return iret;
}
