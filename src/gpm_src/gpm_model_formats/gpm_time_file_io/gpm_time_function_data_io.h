// -- Schlumberger Private --

#ifndef gpm_time_function_data_io_h
#define gpm_time_function_data_io_h


#include "gpm_param_type_mapper.h"
#include "gpm_json_doc_mapper.h"
#include "gpm_1d_interpolator.h"

#include "gpm_logger.h"
#include "gpm_basic_defs.h"
#include <string>
#include <vector>

namespace Slb { namespace Exploration { namespace Gpm {
	// New stuff

	
class gpm_time_function_data_io {
public:
    gpm_time_function_data_io();
    virtual ~gpm_time_function_data_io();
    int read_parameters(const std::string& param_file, const Tools::gpm_logger& logger);
    static std::string get_process_name();
    bool are_parameters_ok(const Tools::gpm_logger& logger) const;
    int read_parameters_from_string(const std::string& param_string, const Tools::gpm_logger& logger);
    std::vector<int> has_missing_indexes() const;
    bool has_sealevel_curve();
    bool do_output_relative_sealevel_rate() const;
    std::string version() const;
    float no_val() const;
    int output_relative_sealevel_rate() const;
    float_2darray_type get_sealevel_array() const;
    std::shared_ptr<gpm_1d_interpolator> get_sealevel_interpolator() const;

	// Make private later
	std::vector<std::string> grid_names() const;


private:
	struct time_data_holder
	{
		std::string version;
		std::string schema_version;
		bool output_relative_sealevel_rate{};
		float_2darray_type sealevel_curve;
	};
	time_data_holder get_holder(const gpm_json_doc_mapper& mapper) const;

	static bool is_sealevel_curve_name(const std::string& name);
	std::shared_ptr<float_3darray_vec_type> z_array() const;

	bool _parameters_ok;
    bool _parsed;
	bool _old_version{};
    int _num_read;
    std::vector<int> _missing_indexes;
	gpm_param_type_mapper _mapper;
	time_data_holder _time_holder;
};

}}}
#endif
