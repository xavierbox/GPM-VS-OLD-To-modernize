// -- Schlumberger Private --

#include "acf_base_reader.h"
#include <boost/algorithm/string/case_conv.hpp>
#include <map>

namespace Slb { namespace Exploration { namespace Gpm { namespace {
const int warning_code = -1;
}

void
acf_base_reader::connect_string(const signal_string_type& subscriber) {
     m_string_sig = subscriber;
}

void
acf_base_reader::connect_int(const signal_int_type& subscriber) {
     m_int_sig = subscriber;
}

void
acf_base_reader::connect_float(const signal_float_type& subscriber) {
     m_float_sig = subscriber;
}

void
acf_base_reader::connect_double(const signal_double_type& subscriber) {
     m_double_sig = subscriber;
}

void
acf_base_reader::connect_string_array(const signal_string_array_type& subscriber) {
     m_string_array_sig = subscriber;
}

void
acf_base_reader::connect_int_array(const signal_int_array_type& subscriber) {
     m_int_array_sig = subscriber;
}

void
acf_base_reader::connect_float_array(const signal_float_array_type& subscriber) {
     m_float_array_sig = subscriber;
}

void
acf_base_reader::connect_double_array(const signal_double_array_type& subscriber) {
     m_double_array_sig = subscriber;
}

void acf_base_reader::connect_string_array2(const signal_string_array_type2& subscriber) {
     m_string_array_sig2 = subscriber;
}

void acf_base_reader::connect_int_array2(const signal_int_array_type2& subscriber) {
     m_int_array_sig2 = subscriber;
}

void acf_base_reader::connect_float_array2(const signal_float_array_type2& subscriber) {
     m_float_array_sig2 = subscriber;
}

void acf_base_reader::connect_double_array2(const signal_double_array_type2& subscriber) {
     m_double_array_sig2 = subscriber;
}

acf_base_reader::acf_base_reader() {
    m_error.clear();
    m_errno = 0;
}

void acf_base_reader::disconnect_slots() {
    m_string_sig=signal_string_type();
    m_int_sig=signal_int_type();
    m_float_sig=signal_float_type();
    m_double_sig=signal_double_type();
    m_string_array_sig=signal_string_array_type();
    m_int_array_sig=signal_int_array_type();
    m_float_array_sig=signal_float_array_type();
    m_double_array_sig=signal_double_array_type();

    m_string_array_sig2=signal_string_array_type2();
    m_int_array_sig2=signal_int_array_type2();
    m_float_array_sig2=signal_float_array_type2();
    m_double_array_sig2=signal_double_array_type2();
}

acf_base_reader::~acf_base_reader() {
    disconnect_slots();
}

std::string acf_base_reader::error() const {
    return m_error;
}

std::vector<acf_base_reader::key_type> acf_base_reader::has_missing_indexes() const {
    return m_counter.has_missing_indexes(0);
}

std::vector<std::pair<acf_base_reader::index_pair_type, acf_base_reader::interval_set_type>>
acf_base_reader::missing_indexes(const key_type& key) const {
    return m_counter.missing_indexes(key);
}

std::vector<acf_base_reader::min_max_type>
acf_base_reader::dimensions(const key_type& key) const {
    return m_counter.dimensions(key);
}

int acf_base_reader::error_number() const {
    return m_errno;
}

// If only warnings, this is what we get
int acf_base_reader::parse_warning_code() {
    return warning_code;
}


void acf_base_reader::set_parse_type_info(const std::vector<TypeDescr>& type_info) {
    initialize_internals(type_info);
}

void acf_base_reader::initialize_internals(const std::vector<TypeDescr>& type_info) {
    auto max_el = std::max_element(type_info.begin(), type_info.end(), [](const TypeDescr& a, const TypeDescr& b) { return a.id < b.id; });
    auto vector_size = max_el->id + 1;
    m_type_info = type_info;
    m_counter.clear();
    m_inumr.clear();
    m_idimr.clear();
    m_inumr.resize(vector_size);
    io_array_dim_type init_4int_array;
    init_4int_array.fill(0);
    m_idimr.resize(vector_size, init_4int_array);

    std::map<std::string, int> mapper;
    for (const auto& it: m_type_info) {
        mapper[it.name] = it.id;
    }
    m_counter.set_string_id_mapper(mapper);
}


void acf_base_reader::signal_integer(const acf_base_reader::key_type& key_name, int val, const std::vector<index_type>& indexes) {
    if (indexes.empty()) {
        m_int_sig(key_name, val);
    }
    else {
        m_counter.add_index(key_name, indexes);
        m_int_array_sig(key_name, indexes, val);
    }
}

void acf_base_reader::signal_float(const acf_base_reader::key_type& key_name, float val, const std::vector<index_type>& indexes) {
    if (indexes.empty()) {
        m_float_sig(key_name, val);
    }
    else {
        m_counter.add_index(key_name, indexes);
        m_float_array_sig(key_name, indexes, val);
    }
}


void acf_base_reader::signal_double(const acf_base_reader::key_type& key_name, double val, const std::vector<index_type>& indexes) {
    if (indexes.empty()) {
        m_double_sig(key_name, val);
    }
    else {
        m_counter.add_index(key_name, indexes);
        m_double_array_sig(key_name, indexes, val);
    }
}

void acf_base_reader::signal_string(const acf_base_reader::key_type& key_name, const std::string& val, const std::vector<index_type>& indexes) {
    if (indexes.empty()) {
        m_string_sig(key_name, val);
    }
    else {
        m_counter.add_index(key_name, indexes);
        m_string_array_sig(key_name, indexes, val);
    }
}


std::vector<TypeDescr> acf_base_reader::type_descriptors() const {
    return m_type_info;
}

//const std::map<int, std::vector<int > >& acf_parser::array_sizes() const
//{
//  return *m_arr_size;
//}

acf_base_reader::parameter_num_type acf_base_reader::num_read() const {
    return m_inumr;
}

acf_base_reader::array_dim_size_type acf_base_reader::array_sizes_read() const {
    return m_idimr;
}

void acf_base_reader::signal_integer_array(const key_type& key_name, const std::vector<index_type>& offset, const std::vector<size_type>& size, const std::vector<int>& values) {
    m_int_array_sig2(key_name, offset, size, values);
}

void acf_base_reader::signal_float_array(const key_type& key_name, const std::vector<index_type>& offset, const std::vector<size_type>& size, const std::vector<float>& values) {
    m_float_array_sig2(key_name, offset, size, values);
}

void acf_base_reader::signal_double_array(const key_type& key_name, const std::vector<index_type>& offset, const std::vector<size_type>& size, const std::vector<double>& values) {
    m_double_array_sig2(key_name, offset, size, values);
}

void acf_base_reader::signal_string_array(const key_type& key_name, const std::vector<index_type>& offset, const std::vector<size_type>& size, const std::vector<std::string>& values) {
    m_string_array_sig2(key_name, offset, size, values);
}
}}}
