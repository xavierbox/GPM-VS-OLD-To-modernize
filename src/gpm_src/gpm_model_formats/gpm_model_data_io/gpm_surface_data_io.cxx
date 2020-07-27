// -- Schlumberger Private --

#include "gpm_surface_data_io.h"
#include "gpm_surface_data_stc.h"

#include "general_param_mapper.h"
#include "gpm_format_helper.h"
#include "gpm_version_number.h"
#include "general_param_file_reader.h"
#include "general_param_string_reader.h"
#include "general_param_write_mapper.h"
#include "acf_format_selector.h"

namespace Slb { namespace Exploration { namespace Gpm {namespace {
std::vector<std::string> get_string_array(const general_param_read_mapper& mapper, SURF_DATA_MEMBERS id) {
    auto it = mapper.get_array_ptr<STRING, ARRAY_1d>(id);
    return it != nullptr ? *it : std::vector<std::string>();
}

std::vector<float> get_float_array(const general_param_read_mapper& mapper, SURF_DATA_MEMBERS id) {
    auto it = mapper.get_array_ptr<FLOAT, ARRAY_1d>(id);
    return it != nullptr ? *it : std::vector<float>();
}
}

gpm_surface_data_io::gpm_surface_data_io(): _parameters_ok(false), _parsed(false), _num_read(0) {
}

gpm_surface_data_io::~gpm_surface_data_io() {
}

int gpm_surface_data_io::check_parsed_state(acf_base_reader& sediment_param_parser, const std::string& param_file, const Tools::gpm_logger& logger) {
    int iret = sediment_param_parser.error_number();
    auto inumrc = sediment_param_parser.num_read();
    if (iret != 0) {
        if (iret == acf_base_reader::parse_warning_code()) {
            logger.print(Tools::LOG_NORMAL, "\nSurface data file contains warnings:\n %s\n", sediment_param_parser.error().c_str());
        }
        else {
            logger.print(Tools::LOG_IMPORTANT, "Surface data file not properly parsed: %s\n '%s'", param_file.c_str(), sediment_param_parser.error().c_str());
            return iret;
        }
    }
    _parsed = true;
    _parameters_ok = are_parameters_ok(logger);
    _missing_indexes = sediment_param_parser.has_missing_indexes();
    return _parameters_ok ? 0 : 1;
}

int gpm_surface_data_io::read_parameters(const std::string& param_file, const Tools::gpm_logger& logger) {
    _parsed = false;
    if (!param_file.empty()) {
        general_param_file_reader reader(param_file, get_process_name(), gpm_surface_data_descr_holder::gpm_surface_data_descr, &_mapper, logger);
        return check_parsed_state(*reader.reader(), param_file, logger);
    }
    return 1;
}

int gpm_surface_data_io::read_parameters_from_string(const std::string& param_string, const Tools::gpm_logger& logger) {
    _parsed = false;
    if (!param_string.empty()) {
        // Process the thing
        general_param_string_reader reader(param_string, get_process_name(), gpm_surface_data_descr_holder::gpm_surface_data_descr, &_mapper, logger);
        return check_parsed_state(*reader.reader(), "Surface data file", logger);
    }
    return 1;
}

int gpm_surface_data_io::write_file(const std::string& /*param_file*/,
                                    const std::vector<std::pair<std::string, float_2darray_base_type*>>& /*surfaces*/,
                                    const std::vector<float>& /*x_corners*/,
                                    const std::vector<float>& /*y_corners*/,
                                    const Tools::gpm_logger& /*logger*/) {
    TypeDescrToFunctionMapper proxy(gpm_surface_data_descr_holder::gpm_surface_data_descr);
    return 0;
}

std::string gpm_surface_data_io::get_process_name() {
    return "Surface data";
}

bool gpm_surface_data_io::are_parameters_ok(const Tools::gpm_logger& logger) const {
    // Here we check
    Tools::gpm_version_number found(version());
    if (!found.is_number_correct()) {
        logger.print(Tools::LOG_NORMAL, "Surface data file version number wrong\n");
        return false;
    }
    Tools::gpm_version_number supported(gpm_surface_data_descr_holder::current_version);
    if (!found.version_compatible(supported)) {
        logger.print(Tools::LOG_IMPORTANT, "Surface data file versions not compatible\n");
        return false;
    }

    return true;
}

// This is for unit testing really

std::vector<float> gpm_surface_data_io::x_corners() const {
    return get_float_array(_mapper, XCORNERS);
}

std::vector<float> gpm_surface_data_io::y_corners() const {
    return get_float_array(_mapper, YCORNERS);
}


std::vector<std::string> gpm_surface_data_io::grid_names() const {
    return get_string_array(_mapper, GRIDNAM);
}

bool gpm_surface_data_io::get_surface_data(const std::string& id, float_2darray_type* res) const {
    bool found = false;
    auto names = grid_names();
    auto it = std::find(names.begin(), names.end(), id);
    if (it != names.end()) {
        auto dist = std::distance(names.begin(), it);
        auto z_arr = z_array();
        if (z_arr != nullptr && dist < z_arr->layers()) {
            const float_2darray_base_type& grd((*z_arr)(static_cast<int>(dist)));
            res->resize(grd.extents());
            res->fill(grd);
            found = true;
        }
    }
    return found;
}

std::shared_ptr<float_3darray_vec_type> gpm_surface_data_io::z_array() const {
    return _mapper.get_array_ptr<FLOAT, ARRAY_3d>(ZGRID);
}

std::string gpm_surface_data_io::version() const { return _mapper.get_value<STRING>(VERSION, "1.0.0.0"); }
float gpm_surface_data_io::no_val() const { return _mapper.get_value<FLOAT>(ZNULL, FLOAT_IO_NO_VALUE); }


parameter_descr_type gpm_surface_data_descr_holder::gpm_surface_data_descr =
{
    make_typedescription<FLOAT>(SURF_DATA_MEMBERS::ZGRID, "ZGRID", ARRAY_3d),
    make_typedescription<FLOAT>(SURF_DATA_MEMBERS::ZNULL, "ZNULL", POD_VAL),
    make_typedescription<FLOAT>(SURF_DATA_MEMBERS::XCORNERS, "XCORNERS", ARRAY_1d),
    make_typedescription<FLOAT>(SURF_DATA_MEMBERS::YCORNERS, "YCORNERS", ARRAY_1d),
    make_typedescription<STRING>(SURF_DATA_MEMBERS::GRIDNAM, "GRIDNAM", ARRAY_1d),
    make_typedescription<STRING>(SURF_DATA_MEMBERS::VERSION, "VERSION", POD_VAL)
};

std::string gpm_surface_data_descr_holder::current_version = "1.0.0.0";

std::string gpm_surface_data_descr_holder::get_json_schema()
{
	static std::string schema(
		R"(
{
	"$schema": "http://json-schema.org/draft-07/schema#",
	"$id": "http://example.com/product.schema.json",
	"title": "GPM sediment input",
	"description": "Data input to GPM",
	"type": "object",
	"required": [],
	"properties": {
		"XCORNERS": {
			"description": "",
			"type": "array",
			"items": {
				"type": "number"
			}
		},
		"YCORNERS": {
			"description": "",
			"type": "array",
			"items": {
				"type": "number"
			}
		},
		"ZNULL": {
			"description": "",
			"type": "number"
		},
		"GRIDNAM": {
			"description": "",
			"type": "array",
			"items": {
				"type": "string"
			}
		},
		"ZGRID": {
			"description": "flow file",
			"type": "array",
			"items": {
				"type": "array",
				"items": {
					"type": "array",
					"items": {
						"type": "number"
					}
				}
			}
		},
		"VERSION": {
			"description": "Describes the version of the input format",
			"type": "string",
			"pattern": "^[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+$"
		}
	}
}
)");
	return schema;
}

gpm_surface_data_output_io::gpm_surface_data_output_io() {
}

gpm_surface_data_output_io::~gpm_surface_data_output_io() {
}

int gpm_surface_data_output_io::write_file(const std::string& param_file, const std::vector<std::pair<std::string, float_2darray_base_type*>>& surfaces, const std::vector<float>& x_corners, const std::vector<float>& y_corners, float no_val, const Tools::gpm_logger& /*logger*/) {
    TypeDescrToFunctionMapper proxy(gpm_surface_data_descr_holder::gpm_surface_data_descr);

    Function1d<std::string> gridnam(surfaces.size(), [&surfaces](FunctionBase::index_type i) { return surfaces[i].first; });
    Function1d<float> xcorners(x_corners.size(), [&x_corners](FunctionBase::index_type i) { return x_corners[i]; });
    Function1d<float> ycorners(y_corners.size(), [&y_corners](FunctionBase::index_type i) { return y_corners[i]; });
    const auto& first(*surfaces.front().second);
    Function3d<float> zgrid(surfaces.size(), first.rows(), first.cols(), [&surfaces](FunctionBase::index_type i, FunctionBase::index_type j, FunctionBase::index_type k) { return surfaces[i].second->at(j, k); });
    FunctionConstant<float> nval([=]() { return no_val; });
    FunctionConstant<std::string> vers([=]() { return gpm_surface_data_descr_holder::current_version; });
    proxy.set_function(ZNULL, nval);
    proxy.set_function(VERSION, vers);
    std::vector<std::pair<acf_base_writer::key_type, Function1d<float>>> corners;
    corners.push_back(std::make_pair(XCORNERS, xcorners));
    corners.push_back(std::make_pair(YCORNERS, ycorners));
    proxy.set_array1d_functions(corners);
    proxy.set_function(GRIDNAM, gridnam);
    proxy.set_function(ZGRID, zgrid);

    general_param_write_mapper write_mapper;
    auto writer_acf = acf_format::find_writer_according_to_suffix(param_file);
    // have to make sure that the typedescriptor has id and index as the same, otherwise it blows up
    write_mapper.attach_to_writer(param_file, proxy, writer_acf.get());
    writer_acf->write();
    _err_num = writer_acf->error_number();
    _error = writer_acf->error();
    writer_acf->close_file();
    writer_acf->disconnect_slots(); // So we don't end up in a dependency thing

    return _err_num;
}

int gpm_surface_data_output_io::error_number() const {
    return _err_num;
}

std::string gpm_surface_data_output_io::error() const {
    return _error;
}
}}}
