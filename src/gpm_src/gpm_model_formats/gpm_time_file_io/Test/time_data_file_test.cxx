// -- Schlumberger Private --

#include <vector>

#include "gpm_time_function_data_io.h"

using namespace Slb::Exploration::Gpm;
std::vector<std::string> time_data_statement_list =
{ "{",
  "\"ZNULL\":-1E8,",
        "\"GRIDNAM\" : [\"SEA0\"],",
        "\"ZGRID\" : ["
            "[",
                "[181.0361,200,250,278.4459,285.1177,278.4459,250,220,200,189.0424,186.3736,200,235.7457,258.5793,267.7708,269.1052,250,220,200,193.0455,191.7111],",
                "[0,1000,2000,3000,4000,5000,6000,7000,8000,9000,10000,11000,12000,13000,14000,15000,16000,17000,18000,19000,20000]",
            "]",
        "]"
    "}"
};

int read_time_data_file_test(int argc, char* argv[]) {

    std::string buf;
    // Lets make the parser thing properly
    for (int i = 0; i < time_data_statement_list.size(); ++i) {
        buf += time_data_statement_list[i];
        buf += "\n";
    }
	Tools::gpm_default_logger my_logger(Tools::LOG_VERBOSE, true);
	gpm_time_function_data_io tester;
	auto iret = tester.read_parameters_from_string(buf, my_logger);

    if (iret == 0) {
        // Check he numbers 
        if (tester.grid_names().size() != 1) {
            iret = 1;
            printf("Gridname size is wrong\n");
        }
        auto xes = tester.get_sealevel_array();
        if (xes.rows() != 2) {
            iret = 1;
            printf("Row size is wrong");
        }
        if (xes.cols() != 21) {
            iret = 1;
            printf("Col size is wrong\n");
        }
    }
    else {
        printf("Parse errors\n");
    }
    return iret;
}

