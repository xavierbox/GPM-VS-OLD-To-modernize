// -- Schlumberger Private --

#include "acf_format_selector.h"
#include <boost/algorithm/string.hpp>
#include "general_param_mapper.h"
#include "general_param_write_mapper.h"
#include "acf_json_writer.h"
#include "acf_json_reader.h"
#include "gpm_hdf5_writer.h"
#include "gpm_hdf5_reader.h"

namespace Slb { namespace Exploration { namespace Gpm {

acf_format::acf_format(Formats from, Formats to) {
    setup_reader_writer(from, to);
}


std::shared_ptr<acf_base_reader> acf_format::find_reader(acf_format::Formats format_type) {
    if (format_type==Formats::JSON) {
        return std::make_shared<acf_json_reader>();
    }
    return std::make_shared<gpm_hdf5_reader>();
}

std::shared_ptr<acf_base_writer> acf_format::find_writer(acf_format::Formats format_type) {
    if (format_type== Formats::JSON) {
        return std::make_shared<acf_json_writer>();
    }
    return std::make_shared<gpm_hdf5_writer>();
}

std::shared_ptr<acf_base_reader> acf_format::find_reader_according_to_suffix(const std::string& file_name) {
    return find_reader(find_format_according_to_suffix(file_name));
}

std::shared_ptr<acf_base_writer> acf_format::find_writer_according_to_suffix(const std::string& file_name) {
    return find_writer(find_format_according_to_suffix(file_name));
}

acf_format::Formats acf_format::find_format_according_to_suffix(const std::string& file_name) {
    if (boost::iends_with(file_name, ".json")) {
        return Formats::JSON;
    }
    if (boost::iends_with(file_name, ".hdf5") || gpm_hdf5_reader::is_hdf5_file(file_name)) {
        return Formats::HDF5;
    }
    return Formats::JSON;
}

void acf_format::setup_reader_writer(Formats from, Formats to) {
    if (from == Formats::HDF5) {
        read_acf.reset(new gpm_hdf5_reader());
    }
    else {
        read_acf.reset(new acf_json_reader());
    }
   if (to == Formats::HDF5) {
        write_acf.reset(new gpm_hdf5_writer());
    }
    else {
        write_acf.reset(new acf_json_writer());
    }

}

void acf_format::setup_format(const std::vector<TypeDescr>& type_info) {
    read_acf->set_parse_type_info(type_info);
}

void acf_format::convert(const std::string& from_file, const std::string& to_file) {
    general_param_read_mapper reader;
    reader.attach_to_parser(*read_acf);
    read_acf->parse_file(from_file);
    auto func = reader.make_function_mapper(); {
        general_param_write_mapper writer;
        if (writer.attach_to_writer(to_file, func, write_acf.get())) {
            write_acf->write();
        }
        write_acf->disconnect_slots();
    }
}
}}}
