#include "gpm_test_parameters.h"
#include <boost/algorithm/string.hpp>
#include <algorithm>
#include <vector>
#include "gpm_logger.h"
#include "gpm_control_file_io.h"

namespace Slb { namespace Exploration { namespace Gpm {

std::string test_parms::find_option(std::vector<std::string>& args, const std::string& output_dir_text)
{
    std::string found;
    auto it = std::find_if(args.begin(), args.end(), [&output_dir_text](const std::string& item)
    {
        return boost::istarts_with(item, output_dir_text);
    });
    if (it != args.end()) {
        auto output_dir = *it;
        boost::replace_all(output_dir, output_dir_text, "");
        found = output_dir;
        args.erase(it);
    }
    return found;
}

void test_parms::packup_args(std::vector<std::string>& args)
{
    gp_exe_path = args[2];
    test_name = args[1];
    run_opti_command = find_option(args, "-run_opti");
    const auto tmp_input_directory = find_option(args, "-input_dir=");
    output_directory = find_option(args, "-output_dir=");
    data_directory = find_option(args, "-data_dir=");
    input_directory = tmp_input_directory+"/"+data_directory;
    const auto tmp_ctl_file = find_option(args, "-ctl_file=");
    ctl_file = input_directory+"/"+tmp_ctl_file;
    platform_suffix = find_option(args, "-plat_suffix=");
}

void test_parms::packup_args(std::vector<std::string>& args, const std::string& directory, const std::string& loc_ctl_file)
{
    gp_exe_path = args[1];
    test_name = args[0];
	auto it = std::find_if(args.begin(), args.end(), [](const std::string& item) {return boost::iends_with(item, "-run_opti");});
	if (it != args.end()) {
        run_opti_command = "-run_opti";
		args.erase(it);
	}
    auto input_dir_text = "-input_dir=";
    it = std::find_if(args.begin(), args.end(), [&input_dir_text](const std::string& item)
    {
        return boost::istarts_with(item, input_dir_text);
    });
    if (it != args.end()) {
        std::string input_dir = *it;
        boost::replace_all(input_dir, input_dir_text, "");
        const auto full_dir = input_dir + directory;
        input_directory = full_dir;
        ctl_file = full_dir + loc_ctl_file;
    }
    auto output_dir_text = "-output_dir=";
    it = std::find_if(args.begin(), args.end(), [&output_dir_text](const std::string& item)
    {
        return boost::istarts_with(item, output_dir_text);
    });
    if (it != args.end()) {
        std::string output_dir = *it;
        boost::replace_all(output_dir, output_dir_text, "");
        output_directory = output_dir;
    }
}

std::vector<std::string> test_parms::copy_args(int argc, char *argv[])
{
	std::vector<std::string> args;
    args.reserve(argc);
    for (int i = 0; i < argc; ++i)
	{
		args.emplace_back(argv[i]);
	}

	return args;
}

std::string test_parms::make_run_command() const
{
    const std::string gp_path = gp_exe_path;
    printf("%s\n", std::string("Input directory: " + input_directory).c_str());
    printf("%s\n", std::string("Controlfile: " + ctl_file).c_str());
    printf("%s\n", std::string("Output directory: " + output_directory).c_str());
    printf("%s\n", std::string("Outfile in controlfile: " + out_file_in_ctl_file).c_str());
    const std::string res_out_file = out_file;
    const std::string out_file_args = "-out_file " + res_out_file;
    printf("%s\n", std::string("Outfile used: " + out_file_args).c_str());
    const std::string run_opti = run_opti_command;
    const std::string output_dir_args = "-output_dir " + output_directory;
    const std::string log_file_name = output_directory + "/" + test_name + "_run_hdf5_log.txt";
    std::string sys_string = "cd " + input_directory + " && " + std::string(gp_path) + " " + ctl_file + " " +
        out_file_args + " " + run_opti + " " + output_dir_args + " -nocr -is_test -logfile " + log_file_name;
    return sys_string;
}

void test_parms::find_output_file_in_ctl_file(const std::function<std::string(const Tools::gpm_logger& log)>& func)
{
    std::string log_filename = output_directory + "/" + test_name + "_param_log.txt";
    const Tools::LogLevel level = Tools::LOG_NORMAL;
    const Tools::gpm_default_logger logger(level, true, log_filename.c_str());
    out_file_in_ctl_file = func(logger);
}
}}}
