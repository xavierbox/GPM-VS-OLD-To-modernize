#ifndef GPM_PROGRAM_OPTIONS_H
#define GPM_PROGRAM_OPTIONS_H

#include "gpm_logger.h"
#include <boost/logic/tribool.hpp>
#include <string>
#include <vector>

namespace Slb { namespace Exploration { namespace Gpm{

class program_option_params{
public:
	bool write_every_cycle;
	bool write_vtk_file;
    bool read_vtu_file;
    bool read_vtu_file_in_binmode;
	bool write_vtu_file;
    bool write_vtu_file_in_binmode;
    bool write_vtu_origional_files;
    bool read_ogs_properties;
    bool write_ogs_project_file;
    std::string ogs_binary_exe;
	double runtime_limit;
	std::string control_file_name;
	std::string out_file_name;
    std::string output_directory; // prefix all output with this director
    std::string log_file_name;
	std::string vtu_prop_file_name;
	Tools::LogLevel log_level;
	bool use_runtime_limit;
	bool output_only_last_timestep{};
	bool output_incremental_files{};
	boost::tribool is_test;
    bool do_cpu_monitoring_;        // used to closely monitor cpu time usage
    int  number_of_running_threads_; // if defaulted and tbb enabled the decision of how many threads to use is set by the task scheduler (max equal to number of cores)
    bool fluid_sim_;
    bool avx_detection_;
    int tbb_grainsize_;

    std::vector<std::string> control_file_suffixes;
	program_option_params();
    void set_control_file_suffix(std::vector<std::string>& suffixes);
    void set_options(std::vector<std::string>& program_options);
	void print_usage(const Tools::gpm_logger& logger) const;
	double runtime_limit_minutes() const;
};
}}}
#endif