// -- Schlumberger Private --

#ifndef acf_base_ascii_writer_h
#define acf_base_ascii_writer_h

// A wrapper for the acf writer currently used, but where we use signals to call back to the struct

#include "gpm_type_descriptor.h"
#include <fstream>

#include "acf_base_writer.h"

namespace Slb { namespace Exploration { namespace Gpm {

class acf_base_ascii_writer : public acf_base_writer {

public:
    acf_base_ascii_writer();

    ~acf_base_ascii_writer() override;
    bool open_file(const std::string& file_name, bool create_new) override;

    void set_write_info(const std::vector<TypeDescr>& type_info,
                        const parameter_num_type& inumr,
                        const array_dim_size_type& idimr) override;
    void close_file() override;

protected:
    std::ofstream m_filin;

};
}}}

#endif
