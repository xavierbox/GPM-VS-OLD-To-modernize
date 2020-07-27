// -- Schlumberger Private --

#include "general_param_string_reader.h"
#include "acf_format_selector.h"
#include "general_param_mapper.h"
#include "gpm_logger.h"

namespace Slb { namespace Exploration { namespace Gpm {

general_param_string_reader::general_param_string_reader(const std::string& param_string,
                                                         const std::string& param_string_descr,
                                                         const std::vector<TypeDescr>& type_info,
                                                         general_param_read_mapper* mapped_data,
                                                         const Tools::gpm_logger& logger) {
    if (!param_string.empty()) {
        // Process the thing
        _reader = acf_format::find_reader(acf_format::Formats::JSON);
        _reader->set_parse_type_info(type_info);
        mapped_data->attach_to_parser(*_reader);
        _reader->parse_text(param_string);
        _read_return_val = _reader->error_number();
        auto inumrc = _reader->num_read();
        if (_read_return_val != 0) {
            logger.print(Tools::LOG_IMPORTANT, "%s parameter string not properly parsed:'%s'\n", param_string_descr.c_str(), param_string.c_str());
        }
        else {
            _parsed = true;
            if (!mapped_data->all_data_types_valid(logger)) {
                _read_return_val = WRONG_FORMAT;
            }
        }
    }
    else {
        _read_return_val = ERROR;
    }
}

bool general_param_string_reader::is_parsed() const {
    return _parsed;
}

int general_param_string_reader::read_return_value() const {
    return _read_return_val;
}

std::shared_ptr<acf_base_reader> general_param_string_reader::reader() const {
    return _reader;
}

general_param_string_reader::~general_param_string_reader() {

}

}}}
