#include "gpm_program_options.h"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <fstream>

namespace Slb { namespace Exploration { namespace Gpm {
using namespace Tools;

program_option_params::program_option_params(): write_every_cycle(false),
                                                write_vtk_file(false),
                                                read_vtu_file(false), read_vtu_file_in_binmode(true),
                                                write_vtu_file(false), write_vtu_file_in_binmode(true), write_vtu_origional_files(true),
                                                write_ogs_project_file(false), read_ogs_properties(false),
                                                runtime_limit(std::numeric_limits<float>::infinity()), log_level(LOG_NORMAL),
                                                use_runtime_limit(false), is_test(boost::indeterminate),
                                                do_cpu_monitoring_(false), number_of_running_threads_(-1),
                                                // set default threads
                                                fluid_sim_(false), avx_detection_(false), tbb_grainsize_(-1)
{
    control_file_suffixes = {".ctl", ".ctl.json"};
}

void program_option_params::set_control_file_suffix(std::vector<std::string>& suffixes)
{
    control_file_suffixes=suffixes;
}
void program_option_params::set_options(std::vector<std::string>& program_options)
{
    if (!program_options.empty()) {
        auto it = std::find(program_options.begin(), program_options.end(), "-logfile");
        if (it != program_options.end()) {
            const auto file_it = it + 1;
            if (file_it != program_options.end()) {
                log_file_name = *file_it;
                program_options.erase(file_it);
            }
            program_options.erase(it);
        }
        auto it2 = std::find(program_options.begin(), program_options.end(), "-loglevel");
        std::string log_file_level;
#ifdef _DEBUG
	log_file_level="VERBOSE";
#endif
        if (it2 != program_options.end()) {
            const auto file_it = it2 + 1;
            if (file_it != program_options.end()) {
                log_file_level = *file_it;
                program_options.erase(file_it);
            }
            program_options.erase(it2);
        }
        boost::to_upper(log_file_level);
        log_level = LOG_NORMAL;

        if (log_file_level == "VERBOSE") {
            log_level = Tools::LOG_VERBOSE;
        }
        else if (log_file_level == "DEBUG") {
            log_level = Tools::LOG_DEBUG;
        }
        else if (log_file_level == "TRACE") {
            log_level = Tools::LOG_TRACE;
        }

        // Skip flags, if any are found before a file name
        it = std::find(program_options.begin(), program_options.end(), "-writecycles");
        if (it != program_options.end()) {
            write_every_cycle = true;
            program_options.erase(it);
        }
        it = std::find(program_options.begin(), program_options.end(), "-is_test");
        if (it != program_options.end()) {
            is_test = true;
            program_options.erase(it);
        }

        it = std::find(program_options.begin(), program_options.end(), "-write_vtk");
        if (it != program_options.end()) {
            write_vtk_file = true;
			program_options.erase(it);
        }
		it = std::find(program_options.begin(), program_options.end(), "-read_vtu_ascii");
		if (it != program_options.end()) {
			read_vtu_file = true;
			read_vtu_file_in_binmode = false;
			const auto file_it = it + 1;
			if (file_it != program_options.end()) {
				vtu_prop_file_name = *file_it;
				program_options.erase(file_it);
			}
			program_options.erase(it);
		}
        // full path to ogs binary exe
        it = std::find(program_options.begin(), program_options.end(), "-ogs_binary_exe");
        if (it != program_options.end()) {
            const auto file_it = it + 1;
            if (file_it != program_options.end()) {
                ogs_binary_exe = *file_it;
                program_options.erase(file_it);
            }
            std::ifstream ogsbin(ogs_binary_exe);
            std::cout << "OGS Binary: " << ogs_binary_exe;
            if (ogsbin.fail()) {
                std::cout << "  --> NOT PRESENT (Workflow not enabled)" << std::endl;
                read_ogs_properties = false;
            }
            else {
                std::cout << "  --> PRESENT  (Workflow enabled)" << std::endl;
                read_ogs_properties = true;
                write_vtu_file = true;
                write_ogs_project_file = true;
            }
            program_options.erase(it);
        }
        
        it = std::find(program_options.begin(), program_options.end(), "-read_vtu_binary");
		if (it != program_options.end()) {
			read_vtu_file = true;
			read_vtu_file_in_binmode = true;
			const auto file_it = it + 1;
			if (file_it != program_options.end()) {
				vtu_prop_file_name = *file_it;
				program_options.erase(file_it);
			}
			program_options.erase(it);
		}
        it = std::find(program_options.begin(), program_options.end(), "-read_ogs_properties");
        if (it != program_options.end()) {
            read_ogs_properties = true;
            program_options.erase(it);
        }
        it = std::find(program_options.begin(), program_options.end(), "-write_ogs_project");
		if (it != program_options.end()) {
			write_vtu_file = true;
			write_vtu_file_in_binmode = false;
			write_ogs_project_file = true;
			program_options.erase(it);
		}

		it = std::find(program_options.begin(), program_options.end(), "-write_vtu_ascii");
        if (it != program_options.end()) {
            write_vtu_file = true;
            write_vtu_file_in_binmode = false;
            program_options.erase(it);
        }
        it = std::find(program_options.begin(), program_options.end(), "-write_vtu_binary");
        if (it != program_options.end()) {
            write_vtu_file = true;
            write_vtu_file_in_binmode = true;
            program_options.erase(it);
        }
        it = std::find(program_options.begin(), program_options.end(), "-exclude_vtu_origional_files");
        if (it != program_options.end()) {
            write_vtu_origional_files = false;
            program_options.erase(it);
        }
        it = std::find(program_options.begin(), program_options.end(), "-output_last_timestep_only");
		if (it != program_options.end()) {
			output_only_last_timestep = true;
			program_options.erase(it);
		}
		it = std::find(program_options.begin(), program_options.end(), "-output_incremental_files");
		if (it != program_options.end()) {
			output_incremental_files = true;
			program_options.erase(it);
		}
        // Runtime limit in minutes
        it = std::find(program_options.begin(), program_options.end(), "-runtime_limit");
        if (it != program_options.end()) {
            if (it + 1 != program_options.end()) {
                const std::string dur(*(it + 1));
                program_options.erase(it + 1);
                runtime_limit = boost::lexical_cast<double>(dur) * 60; // Get it to seconds
                use_runtime_limit = true;
            }
            program_options.erase(it);
            // Turn the thing into a number
        }
        // Redirect the output to this file
        it = std::find(program_options.begin(), program_options.end(), "-out_file");
        if (it != program_options.end()) {
            if (it + 1 != program_options.end()) {
                const std::string new_out_file(*(it + 1));
                program_options.erase(it + 1);
                out_file_name = new_out_file;
            }
            program_options.erase(it);
        }

                // Redirect the output to this file
        it = std::find(program_options.begin(), program_options.end(), "-output_dir");
        if (it != program_options.end()) {
            if (it + 1 != program_options.end()) {
                const std::string new_output_dir(*(it + 1));
                program_options.erase(it + 1);
                output_directory = new_output_dir;
            }
            program_options.erase(it);
        }
        // number of runtime threads
        it = std::find(program_options.begin(), program_options.end(), "-nthreads");
        if (it != program_options.end()) {
            if (it + 1 != program_options.end()) {
                const std::string nthreads(*(it + 1));
                program_options.erase(it + 1);
                number_of_running_threads_ = boost::lexical_cast<int>(nthreads);
            }
            program_options.erase(it);
        }

        // size of grainsize used with tbb for loops
        it = std::find(program_options.begin(), program_options.end(), "-grainsize");
        if (it != program_options.end()) {
            if (it + 1 != program_options.end()) {
                const std::string grainsize(*(it + 1));
                program_options.erase(it + 1);
                tbb_grainsize_ = boost::lexical_cast<int>(grainsize);
    
            }
            program_options.erase(it);
        }

        it = std::find(program_options.begin(), program_options.end(), "-cpumon");
        if (it != program_options.end()) {
            do_cpu_monitoring_ = true;
            program_options.erase(it);
        }

        it = std::find(program_options.begin(), program_options.end(), "-fluid_sim");
        if (it != program_options.end()) {
            fluid_sim_ = true;
            program_options.erase(it);
        }

        it = std::find(program_options.begin(), program_options.end(), "-avx_detection");
        if (it != program_options.end()) {
            avx_detection_ = true;
            program_options.erase(it);
        }

        if (!program_options.empty()) {
            // Skip flags, if any are found before a file name
            unsigned iarg = 0;
            while (iarg < program_options.size()) {
                auto prog_arg = program_options[iarg];
                if (std::any_of(control_file_suffixes.begin(), control_file_suffixes.end(),[prog_arg](const std::string& val){return boost::iends_with(prog_arg, val) ;})) {
                    control_file_name = prog_arg;
                    break;
                }
                iarg++;
            }
        }
    }
}

void program_option_params::print_usage(const gpm_logger& logger) const
{
    if (write_vtk_file) {
        logger.print(LOG_NORMAL, "-write_vtk used, writing vtk files\n");
    }
    if (write_vtu_file) {
        logger.print(LOG_NORMAL, "-write_vtu_ascii or -write_vtu_binary used, writing vtu files\n");
    }
    if (write_every_cycle) {
        logger.print(LOG_NORMAL, "-writecycles used, writing results for every cycle\n");
    }
    if (use_runtime_limit) {
        logger.print(LOG_NORMAL, "-runtime_limit used, stopping after %f minutes used\n", runtime_limit_minutes());
    }
}

double program_option_params::runtime_limit_minutes() const
{
    return runtime_limit / 60.0;
}

}}}
