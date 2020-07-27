#ifndef GPM_TEST_PARAMETERS_H
#define GPM_TEST_PARAMETERS_H
#include <string>
#include <vector>
#include "gpm_logger.h"
#include <functional>

namespace Slb { namespace Exploration { namespace Gpm {

class test_parms {
public:
    std::string gp_exe_path;
    std::string test_name;
    std::string ctl_file;
    std::string out_file;
    std::string out_file_in_ctl_file;
    std::string input_directory;
    std::string output_directory;
    std::string data_directory;
    std::string platform_suffix;
    std::string run_opti_command;


    static std::string find_option(std::vector<std::string>& args,
                            const std::string& output_dir_text);
    void packup_args(std::vector<std::string>& args);
    void packup_args(std::vector<std::string>& args, const std::string& directory, const std::string& loc_ctl_file);
    static std::vector<std::string> copy_args(int argc, char * argv[]);
    std::string make_run_command() const;
    typedef std::function<std::string(const Tools::gpm_logger& log)> outfile_function_type;
    void find_output_file_in_ctl_file(const outfile_function_type& func);
};

}}}
#endif