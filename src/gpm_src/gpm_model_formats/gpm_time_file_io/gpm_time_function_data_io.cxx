// -- Schlumberger Private --

#include "gpm_time_function_data_io.h"
#include "gpm_time_function_data_stc.h"
#include "gpm_json_doc_mapper.h"
#include "gpm_time_file_io_sc.h"
#include "gpm_time_file_io_sc_old.h"

#include "gpm_version_number.h"
#include <boost/algorithm/string.hpp>
#include "gpm_json_keywords.h"
#include <rapidjson/pointer.h>


namespace Slb { namespace Exploration { namespace Gpm {namespace {
std::vector<std::string> get_string_array(const gpm_param_type_mapper& mapper, TIME_FUNCTION_DATA_MEMBERS id) {
    auto it = mapper.get_array_ptr<STRING, ARRAY_1d>(static_cast<gpm_param_type_mapper::key_type>(id));
    return it != nullptr ? *it : std::vector<std::string>();
}

}

gpm_time_function_data_io::gpm_time_function_data_io(): _parameters_ok(false), _parsed(false), _num_read(0) {

}

gpm_time_function_data_io::~gpm_time_function_data_io() = default;

int gpm_time_function_data_io::read_parameters(const std::string& param_file, const Tools::gpm_logger& logger) {
    _parsed = false;
	gpm_json_doc_mapper mapper(get_process_name(), gpm_time_file_io_schema_old, gpm_time_file_io_schema);
	const int ret_val = mapper.parse_file(param_file, logger);
	_parsed = ret_val == 0;
	if (_parsed) {
		_old_version = mapper.is_old_format();
		if (_old_version) {
			_mapper.initialize_parameters(gpm_time_function_data_descr_holder::gpm_time_function_data_descr);
			mapper.find_mapping(&_mapper);
		}
		else
		{
			_time_holder = get_holder(mapper);
		}
		_parameters_ok = are_parameters_ok(logger);
		return _parameters_ok ? 0 : 1;
	}
    return 1;
}

int gpm_time_function_data_io::read_parameters_from_string(const std::string& param_string, const Tools::gpm_logger& logger) {
    _parsed = false;
	gpm_json_doc_mapper mapper(get_process_name(), gpm_time_file_io_schema_old, gpm_time_file_io_schema);
	const int ret_val = mapper.parse_text(param_string, logger);
	_parsed = ret_val == 0;
	if (_parsed) {
		_old_version = mapper.is_old_format();
		if (_old_version) {
			_mapper.initialize_parameters(gpm_time_function_data_descr_holder::gpm_time_function_data_descr);
			mapper.find_mapping(&_mapper);
		}
		else
		{
			_time_holder = get_holder(mapper);
		}
		_parameters_ok = are_parameters_ok(logger);
		return _parameters_ok?0:1;
	}
    return 1;
}


std::string gpm_time_function_data_io::get_process_name() {
    return "Time function data";
}

bool gpm_time_function_data_io::are_parameters_ok(const Tools::gpm_logger& logger) const {
    // Here we check
    Tools::gpm_version_number found(version());
    if (!found.is_number_correct()) {
        logger.print(Tools::LOG_NORMAL, "Time functions file version number wrong\n");
        return false;
    }
	if (_old_version) {
		Tools::gpm_version_number supported(gpm_time_function_data_descr_holder::current_version);
		if (!found.version_compatible(supported)) {
			logger.print(Tools::LOG_IMPORTANT, "Time functions file versions not compatible\n");
			return false;
		}
	}

    return true;
}

// This is for unit testing really


std::vector<int> gpm_time_function_data_io::has_missing_indexes() const {
    return _missing_indexes;
}


bool gpm_time_function_data_io::has_sealevel_curve() {
	if (_old_version) {
		auto names = grid_names();
		return std::find_if(names.begin(), names.end(), [](const std::string& it) { return is_sealevel_curve_name(it); }) != names.end();
	}
	return _time_holder.sealevel_curve.has_data();
}

bool gpm_time_function_data_io::is_sealevel_curve_name(const std::string& name) {
    auto sea_name_prefix = gpm_time_function_data_descr_holder::time_function_1_index_name_prefix[TIME_FUNCTION_1_INDEX_NAMES::SEA_i];
    return boost::istarts_with(name, sea_name_prefix); // These should be 
}

bool gpm_time_function_data_io::do_output_relative_sealevel_rate() const {
    return (output_relative_sealevel_rate()>0);
}

std::vector<std::string> gpm_time_function_data_io::grid_names() const {
    return get_string_array(_mapper, TIME_FUNCTION_DATA_MEMBERS::GRIDNAM);
}

std::shared_ptr<float_3darray_vec_type> gpm_time_function_data_io::z_array() const {
    return _mapper.get_array_ptr<FLOAT, ARRAY_3d>(static_cast<int>(TIME_FUNCTION_DATA_MEMBERS::ZGRID));
}

std::string gpm_time_function_data_io::version() const {
	if (_old_version) {
		return _mapper.get_value<STRING>(static_cast<int>(TIME_FUNCTION_DATA_MEMBERS::VERSION), "1.0.0.0");
	}
	return _time_holder.version;
}

float gpm_time_function_data_io::no_val() const {
	if (_old_version) {
		return _mapper.get_value<FLOAT>(static_cast<int>(TIME_FUNCTION_DATA_MEMBERS::ZNULL), FLOAT_IO_NO_VALUE);
	}
	return -1e12F;
}

int gpm_time_function_data_io::output_relative_sealevel_rate() const {
	if (_old_version) {
		return _mapper.get_value<INTEGER>(static_cast<int>(TIME_FUNCTION_DATA_MEMBERS::OUT_RELATIVE_SEALEVEL_RATE), 0);
	}
	return _time_holder.output_relative_sealevel_rate ? 1 : 0;
}


float_2darray_type gpm_time_function_data_io::get_sealevel_array()const
{
	float_2darray_type res;
	if (_old_version)
	{
		auto names = grid_names();
		auto iter = std::find_if(names.begin(), names.end(), [](const std::string& it)
			{
				return Slb::Exploration::Gpm::gpm_time_function_data_io::is_sealevel_curve_name(it);
			});
		if (iter != names.end()) {
			const auto z_vals = z_array();
			const auto index = std::distance(names.begin(), iter);
			if (z_vals != nullptr && index < z_vals->layers() && z_vals->rows() == 2) {
				res = float_2darray_type((*z_vals)(index));
			}
		}
		return res;
	}
	return _time_holder.sealevel_curve;
}

std::shared_ptr<Slb::Exploration::Gpm::gpm_1d_interpolator>
gpm_time_function_data_io::get_sealevel_interpolator()const
{
	float_2darray_type res = get_sealevel_array();
	if (res.cols() == 0)
	{
		return std::shared_ptr<gpm_1d_interpolator>(new gpm_1d_constant(0));
	}
	if (res.cols() ==1 )
	{
		return std::shared_ptr<gpm_1d_interpolator>(new gpm_1d_constant(res(0, 0)));
	}
	return std::shared_ptr<gpm_1d_interpolator>(new gpm_1d_linear_array2d(res, no_val()));
}

gpm_time_function_data_io::time_data_holder gpm_time_function_data_io::get_holder(const gpm_json_doc_mapper& mapper) const
{
	time_data_holder res;
	const auto parameters = mapper.doc[gpm_json_keywords::PARAMETERS].GetObject();
	res.output_relative_sealevel_rate = parameters["OutputRelativeSealevelRate"].GetBool();
	res.version = mapper.doc[gpm_json_keywords::VERSION].GetString();
	res.schema_version = mapper.doc[gpm_json_keywords::SCHEMA_VERSION].GetString();
	if (parameters["SeaLevel"].IsNumber())
	{
		res.sealevel_curve = float_2darray_type(2, 1, 0);
		res.sealevel_curve(0, 0) = parameters["SeaLevel"].GetFloat();
	}
	else
	{
		// Get the real curve
		const auto amplitude_func_ptr = rapidjson::Pointer(parameters["SeaLevel"].Get<std::string>());
		const auto amplitude_func_arr = amplitude_func_ptr.Get(mapper.doc)->GetObject()[gpm_json_keywords::ARRAY_2D_VALUES].GetArray();
		gpm_json_doc_mapper::resize_rect_2darray(amplitude_func_arr, &res.sealevel_curve);
		gpm_json_doc_mapper::fill_rect_2darray(amplitude_func_arr, &res.sealevel_curve);
	}
	return res;
}

std::string gpm_time_function_data_descr_holder::current_version("1.0.0.0");
parameter_descr_type gpm_time_function_data_descr_holder::gpm_time_function_data_descr =
{
    make_typedescription<FLOAT>(static_cast<int>(TIME_FUNCTION_DATA_MEMBERS::XMIN), "XMIN"),
    make_typedescription<FLOAT>(static_cast<int>(TIME_FUNCTION_DATA_MEMBERS::XMAX), "XMAX"),
    make_typedescription<FLOAT>(static_cast<int>(TIME_FUNCTION_DATA_MEMBERS::ZNULL), "ZNULL"),
    make_typedescription<STRING>(static_cast<int>(TIME_FUNCTION_DATA_MEMBERS::GRIDNAM), "GRIDNAM", ARRAY_1d),
    make_typedescription<FLOAT>(static_cast<int>(TIME_FUNCTION_DATA_MEMBERS::ZGRID), "ZGRID", ARRAY_3d),
    make_typedescription<INTEGER>(static_cast<int>(TIME_FUNCTION_DATA_MEMBERS::OUT_RELATIVE_SEALEVEL_RATE),
                                  "OUT_RELATIVE_SEALEVEL_RATE"),
    make_typedescription<INTEGER>(static_cast<int>(TIME_FUNCTION_DATA_MEMBERS::OUT_DEPOSITIONAL_ENERGY),
                                  "OUT_DEPOSITIONAL_ENERGY"),
    make_typedescription<STRING>(static_cast<int>(TIME_FUNCTION_DATA_MEMBERS::VERSION), "VERSION")
};

std::map<TIME_FUNCTION_1_INDEX_NAMES, std::string> gpm_time_function_data_descr_holder::
    time_function_1_index_name_prefix =
    {
        {TIME_FUNCTION_1_INDEX_NAMES::SEA_i, "SEA"},
        {TIME_FUNCTION_1_INDEX_NAMES::PRECIP_i, "PRECIP"}
    };



std::string gpm_time_function_data_descr_holder::time_function_index_name(TIME_FUNCTION_1_INDEX_NAMES id, int source) {
    bool has_key = time_function_1_index_name_prefix.find(id) != time_function_1_index_name_prefix.end();
    std::string key = has_key ? time_function_1_index_name_prefix[id] : "NOMATCH";
    return key + std::to_string(source);
}


}}}
