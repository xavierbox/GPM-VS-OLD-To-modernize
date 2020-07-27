// -- Schlumberger Private --

#ifndef gpm_hdf5_writer_h
#define gpm_hdf5_writer_h

#include "gpm_type_descriptor.h"
#include "acf_base_writer.h"
#include <memory>

namespace Slb { namespace Exploration { namespace Gpm {

class gpm_hdf5_writer_impl;

class gpm_hdf5_writer : public acf_base_writer {

public:
	bool open_file(const std::string& file_name, bool create_new_file) override;
    void set_write_info(const std::vector<TypeDescr>& type_info, const parameter_num_type& inumr, const array_dim_size_type& idimr) override;
    void write() override;
    void append(const std::vector<TypeDescr>& type_info, const parameter_num_type& inumr, const array_dim_size_type& idimr);
    void close_file() override;

    gpm_hdf5_writer();
    ~gpm_hdf5_writer();
    gpm_hdf5_writer(const gpm_hdf5_writer&)=delete;
    gpm_hdf5_writer& operator=(const gpm_hdf5_writer&)=delete;

private:

    bool check_file_access(const std::string& file_name, bool create_new);
    std::unique_ptr<gpm_hdf5_writer_impl> m_impl;
    
    int writeacf(const std::vector<TypeDescr>& type_info, const parameter_num_type& inumr, const array_dim_size_type& idimr);
    void set_signals_to_impl();
};
}}}

#endif
