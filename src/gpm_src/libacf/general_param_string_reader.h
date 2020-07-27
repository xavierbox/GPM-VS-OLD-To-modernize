// -- Schlumberger Private --

#ifndef GENERAL_PARAM_STRING_READER_H
#define GENERAL_PARAM_STRING_READER_H

#include "gpm_type_descriptor.h"
#include "acf_base_reader.h"
#include "gpm_logger.h"
#include "general_param_mapper.h"
#include <vector>
#include <string>

namespace Slb { namespace Exploration { namespace Gpm { 

class general_param_string_reader {
public:
    general_param_string_reader(const std::string& param_string,
                                const std::string& param_string_descr,
                                const std::vector<TypeDescr>& type_info,
                                general_param_read_mapper* mapped_data,
                                const Tools::gpm_logger& logger);
    
    general_param_string_reader(const general_param_string_reader& rhs) = delete;
    general_param_string_reader& operator=(const general_param_string_reader& rhs) = delete;

    bool is_parsed() const;
    int read_return_value() const;
    std::shared_ptr<acf_base_reader> reader() const;
    ~general_param_string_reader();
private:
    std::shared_ptr<acf_base_reader> _reader;
    int _read_return_val;
    bool _parsed;
};

}}}

#endif
