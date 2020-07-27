// -- Schlumberger Private --

#ifndef GENERAL_PARAM_FILE_READER_H
#define GENERAL_PARAM_FILE_READER_H

#include "gpm_type_descriptor.h"
#include "acf_base_reader.h"
#include "general_param_mapper.h"
#include "gpm_logger.h"
#include <vector>
#include <string>
#include <memory>

namespace Slb { namespace Exploration { namespace Gpm { 

class general_param_file_reader {
public:
    general_param_file_reader(const std::string& param_file,
                              const std::string& param_file_descr,
                              const std::vector<TypeDescr>& type_info,
                              general_param_read_mapper* mapped_data,
                              const Tools::gpm_logger& logger);
    bool is_parsed() const;
    int read_return_value() const;
    std::shared_ptr<acf_base_reader> reader() const;
    ~general_param_file_reader();
private:
    general_param_file_reader(const general_param_file_reader& rhs);
    general_param_file_reader& operator=(const general_param_file_reader& rhs);
    std::shared_ptr<acf_base_reader> _reader;
    int _read_return_val;
    bool _parsed;
};

}}}

#endif
