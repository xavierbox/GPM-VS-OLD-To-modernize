// -- Schlumberger Private --

#ifndef acf_base_writer_h
#define acf_base_writer_h

// A wrapper for the acf writer currently used, but where we use signals to call back to the struct

#include "gpm_int_extent_1d.h"
#include "acf_base_io_types.h"
#include "gpm_type_descriptor.h"
#include <array>
#include <functional>

namespace Slb { namespace Exploration { namespace Gpm {

class acf_base_writer:public acf_base_io_types {
public:
    typedef std::array<int_extent_1d, 3> io_array_dim_type;
    typedef std::vector<io_array_dim_type> array_dim_size_type;

    typedef std::function<std::string(const key_type& key)> signal_string_type;
    typedef std::function<int(const key_type& key)> signal_int_type;
    typedef std::function<float(const key_type& key)> signal_float_type;
    typedef std::function<double(const key_type& key)> signal_double_type;
    typedef std::function<std::string(const key_type& key, const std::vector<index_type>& indexes)> signal_string_array_type;
    typedef std::function<int(const key_type& key, const std::vector<index_type>& indexes)> signal_int_array_type;
    typedef std::function<float(const key_type& key, const std::vector<index_type>& indexes)> signal_float_array_type;
    typedef std::function<double(const key_type& key, const std::vector<index_type>& indexes)> signal_double_array_type;

    acf_base_writer();
    virtual ~acf_base_writer();

    virtual void write() = 0;
	virtual bool open_file(const std::string& file_name, bool create_new_file) = 0;
    virtual void set_write_info(const std::vector<TypeDescr>& type_info,
                                const parameter_num_type& inumr,
                                const array_dim_size_type& idimr) = 0;
	virtual void close_file() = 0;

    void disconnect_slots();
    std::vector<TypeDescr> type_descriptors() const;
    std::string error() const;
    int error_number() const;

    // Connect to the writer
    void connect_string(const signal_string_type& subscriber);
    void connect_int(const signal_int_type& subscriber);
    void connect_float(const signal_float_type& subscriber);
    void connect_double(const signal_double_type& subscriber);
    void connect_string_array(const signal_string_array_type& subscriber);
    void connect_int_array(const signal_int_array_type& subscriber);
    void connect_float_array(const signal_float_array_type& subscriber);
    void connect_double_array(const signal_double_array_type& subscriber);

protected:

    signal_string_type m_string_sig;
    signal_int_type m_int_sig;
    signal_float_type m_float_sig;
    signal_double_type m_double_sig;
    signal_string_array_type m_string_array_sig;
    signal_int_array_type m_int_array_sig;
    signal_float_array_type m_float_array_sig;
    signal_double_array_type m_double_array_sig;

    std::vector<TypeDescr> m_type_info;
	parameter_num_type m_inumr;
    array_dim_size_type m_idimr;
    std::string m_error;
    int m_errno;
    std::string m_file_name;
};
}}}

#endif
