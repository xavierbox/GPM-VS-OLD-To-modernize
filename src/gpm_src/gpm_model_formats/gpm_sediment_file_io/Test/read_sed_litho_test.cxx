// -- Schlumberger Private --

#include "sedstc.h"
#include "gpm_logger.h"
#include "gpm_sediment_file_io.h"

using namespace Slb::Exploration::Gpm;
std::string input = R"({
"NUMSEDS" : 4,
"ID" : ["SAND1","SAND2","SILT","CLAY"],
"SEDNAME" : ["SAND1","SAND2","SILT","CLAY"],
"POR0" : [0.30,0.34,0.40,0.55],
"PERM0" : [100,10,0.1, 0.01],
"POR1" : [0.20, 0.24, 0.30, 0.45],
"PERM1" : [10, 1, 0.01, 0.001],
"PERMANIS" : [1, 1, 1, 1],
"COMP" : [5000, 5000, 500, 500],
"GRAINDEN" : [0.7, 2.7, 2.7, 2.7],
"DIAMETER" : [1,  0.1,   0.01,  0.001],
"VERSION":"1.0.0.0"
}
)";

int read_sed_litho_test(int /*argc*/, char* /*argv*/ []) {
    int res = -1;

    struct pars test;
    gpm_sediment_file_io io;
    Tools::gpm_default_logger logger(Tools::LOG_DEBUG, true);
    auto iret = io.read_parameters_from_string(input, logger);
    std::vector<int> wrong_indexes = io.has_missing_indexes();
    if (iret == 0 && wrong_indexes.empty()) { // Could have tested that 
        printf("Passed");
        res = 0;
    }
    else {
        // Now we really should check the content
        printf("failed");
        res = 1;
    }
    return res;
}
