// -- Schlumberger Private --

#ifndef acf_base_reader_h
#define acf_base_reader_h

// A wrapper for the acf parser currently used, but where we use signals to call back to the struct
#include "acf_base_io_types.h"
#include "index_counter.h"
#include "gpm_type_descriptor.h"
#include <array>
#include <functional>

namespace Slb { namespace Exploration { namespace Gpm {

class acf_base_reader:public acf_base_io_types {
public:
    // Perhaps we should do it all by array?
	typedef std::array<int, 3> io_array_dim_type;
	typedef std::vector<io_array_dim_type> array_dim_size_type;

    typedef std::function<void(const key_type& key, const std::string& val)> signal_string_type;
    typedef std::function<void(const key_type& key, int val)> signal_int_type;
    typedef std::function<void(const key_type& key, float val)> signal_float_type;
    typedef std::function<void(const key_type& key, double val)> signal_double_type;
    typedef std::function<void(const key_type& key, const std::vector<index_type>& indexes, const std::string& val)> signal_string_array_type;
    typedef std::function<void(const key_type& key, const std::vector<index_type>& indexes, int val)> signal_int_array_type;
    typedef std::function<void(const key_type& key, const std::vector<index_type>& indexes, float val)> signal_float_array_type;
    typedef std::function<void(const key_type& key, const std::vector<index_type>& indexes, double val)> signal_double_array_type;

    typedef std::function<void(const key_type& key, const std::vector<index_type>& offset, const std::vector<size_type>& size, const std::vector<std::string>& values)> signal_string_array_type2;
    typedef std::function<void(const key_type& key, const std::vector<index_type>& offset, const std::vector<size_type>& size, const std::vector<int>& values)> signal_int_array_type2;
    typedef std::function<void(const key_type& key, const std::vector<index_type>& offset, const std::vector<size_type>& size, const std::vector<float>& values)> signal_float_array_type2;
    typedef std::function<void(const key_type& key, const std::vector<index_type>& offset, const std::vector<size_type>& size, const std::vector<double>& values)> signal_double_array_type2;

    acf_base_reader();
    void disconnect_slots();
    virtual ~acf_base_reader();
    void set_parse_type_info(const std::vector<TypeDescr>& type_info);

    // Returns the state of the stream
    virtual bool parse_text(const std::string& text_to_parse) =0;
    virtual bool parse_file(const std::string& file) =0;

    // Connect to the parser
    void connect_string(const signal_string_type& subscriber);
    void connect_int(const signal_int_type& subscriber);
    void connect_float(const signal_float_type& subscriber);
    void connect_double(const signal_double_type& subscriber);
    void connect_string_array(const signal_string_array_type& subscriber);
    void connect_int_array(const signal_int_array_type& subscriber);
    void connect_float_array(const signal_float_array_type& subscriber);
    void connect_double_array(const signal_double_array_type& subscriber);

    void connect_string_array2(const signal_string_array_type2& subscriber);
    void connect_int_array2(const signal_int_array_type2& subscriber);
    void connect_float_array2(const signal_float_array_type2& subscriber);
    void connect_double_array2(const signal_double_array_type2& subscriber);

    std::string error() const;

    typedef std::pair<index_type, index_type> index_pair_type;
    typedef std::pair<index_type, index_type> min_max_type;
    typedef boost::icl::interval_set<index_type> interval_set_type ;

    std::vector<key_type> has_missing_indexes() const;
    std::vector<std::pair<index_pair_type, interval_set_type>> missing_indexes(const key_type& key) const;
    std::vector<min_max_type> dimensions(const key_type& key) const;

    int error_number() const;
    static int parse_warning_code();
    // Now for finding out what is read
    std::vector<TypeDescr> type_descriptors() const;
    //const std::map<int, std::vector<int > >& array_sizes() const;
    parameter_num_type num_read() const;
    array_dim_size_type array_sizes_read() const;
protected:
    void signal_float(const acf_base_reader::key_type& key_name, float val, const std::vector<index_type>& indexes);
    void signal_double(const acf_base_reader::key_type& key_name, double val, const std::vector<index_type>& indexes);
    void signal_integer(const acf_base_reader::key_type& key_name, int val, const std::vector<index_type>& indexes);
    void signal_string(const acf_base_reader::key_type& key_name, const std::string& val, const std::vector<index_type>& indexes);

    void signal_integer_array(const key_type& key_name, const std::vector<index_type>& offset, const std::vector<size_type>& size, const std::vector<int>& values);
    void signal_float_array(const key_type& key_name, const std::vector<index_type>& offset, const std::vector<size_type>& size, const std::vector<float>& values);
    void signal_double_array(const key_type& key_name, const std::vector<index_type>& offset, const std::vector<size_type>& size, const std::vector<double>& values);
    void signal_string_array(const key_type& key_name, const std::vector<index_type>& offset, const std::vector<size_type>& size, const std::vector<std::string>& values);

    void initialize_internals(const std::vector<TypeDescr>& type_info);

    signal_string_type m_string_sig;
    signal_int_type m_int_sig;
    signal_float_type m_float_sig;
    signal_double_type m_double_sig;
    signal_string_array_type m_string_array_sig;
    signal_int_array_type m_int_array_sig;
    signal_float_array_type m_float_array_sig;
    signal_double_array_type m_double_array_sig;

    signal_string_array_type2 m_string_array_sig2;
    signal_int_array_type2 m_int_array_sig2;
    signal_float_array_type2 m_float_array_sig2;
    signal_double_array_type2 m_double_array_sig2;

    index_counter m_counter;

    // Parse info

    std::vector<TypeDescr> m_type_info;
    //std::map<int, std::vector<int > >* m_arr_size;
    // For the reader the data can be kept and returned when asked for
    parameter_num_type m_inumr;
    array_dim_size_type m_idimr;

    std::string m_error;
    int m_errno;
};
}}}
#endif
