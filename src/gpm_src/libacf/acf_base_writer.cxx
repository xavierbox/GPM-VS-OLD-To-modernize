// -- Schlumberger Private --

#include "acf_base_writer.h"

namespace Slb { namespace Exploration { namespace Gpm {

acf_base_writer::acf_base_writer(): m_errno(0) {
}

acf_base_writer::~acf_base_writer() {
    disconnect_slots();
}

void acf_base_writer::disconnect_slots() {
    m_string_sig=signal_string_type();
    m_int_sig=signal_int_type();
    m_float_sig=signal_float_type();
    m_double_sig=signal_double_type();
    m_string_array_sig=signal_string_array_type();
    m_int_array_sig=signal_int_array_type();
    m_float_array_sig=signal_float_array_type();
    m_double_array_sig=signal_double_array_type();
}

std::vector<TypeDescr> acf_base_writer::type_descriptors() const {
    return m_type_info;
}

std::string acf_base_writer::error() const {
    return m_error;
}

int acf_base_writer::error_number() const {
    return m_errno;
}

void acf_base_writer::connect_string(const signal_string_type& subscriber) {
     m_string_sig = subscriber;
}

void acf_base_writer::connect_int(const signal_int_type& subscriber) {
     m_int_sig = subscriber;
}

void acf_base_writer::connect_float(const signal_float_type& subscriber) {
     m_float_sig = subscriber;
}

void acf_base_writer::connect_double(const signal_double_type& subscriber) {
     m_double_sig = subscriber;
}

void acf_base_writer::connect_string_array(const signal_string_array_type& subscriber) {
     m_string_array_sig = subscriber;
}

void acf_base_writer::connect_int_array(const signal_int_array_type& subscriber) {
     m_int_array_sig = subscriber;
}

void acf_base_writer::connect_float_array(const signal_float_array_type& subscriber) {
     m_float_array_sig = subscriber;
}

void acf_base_writer::connect_double_array(const signal_double_array_type& subscriber) {
     m_double_array_sig = subscriber;
}
}}}
