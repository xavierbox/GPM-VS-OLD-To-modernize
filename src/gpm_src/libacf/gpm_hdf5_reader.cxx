// -- Schlumberger Private --

#include "gpm_hdf5_reader.h"
#include <mapstc_descr.h>
#include "gpm_h5cpp_wrapper.h"

#include <boost/numeric/conversion/cast.hpp>

namespace Slb { namespace Exploration { namespace Gpm {
gpm_hdf5_reader::gpm_hdf5_reader()
{
    m_file_p.reset(new gpm_h5cpp_wrapper());
}

gpm_hdf5_reader::~gpm_hdf5_reader() = default;

bool gpm_hdf5_reader::parse_file(const std::string& file_name)
{
    m_errno = readacf(file_name, m_type_info, m_inumr, m_idimr, m_error);
    return m_errno == 0;
}
bool gpm_hdf5_reader::is_hdf5_file(const std::string& file_name)
{
    // Need to turn printing off
    // We'll handle exceptions
	H5E_auto_t func;
	void* client_data;
	H5::Exception::getAutoPrint(func, &client_data);
	H5::Exception::dontPrint();
	bool is_hdf5 = false;
    try {
		is_hdf5= H5::H5File::isHdf5(file_name);
	}
   catch(const H5::FileIException& exp) {
	   // Just set things up agin
	   is_hdf5 = false; // Not really needed
   }
   // set printing back on again
   H5::Exception::setAutoPrint(func, client_data);
   return is_hdf5;
}
int gpm_hdf5_reader::readacf(const std::string& file_name, std::vector<TypeDescr>& type_info, parameter_num_type& inumr,
                             array_dim_size_type& idimr, std::string& strerr)
{
    int iret = 0;

    try {
        if (is_hdf5_file(file_name) && m_file_p->open_file(file_name, false, false)) {
            for (const auto& item_desc : type_info) {
                io_array_dim_type item_data = idimr[item_desc.id];
                m_inumr[item_desc.id] = read_item(item_desc, item_data);
                m_idimr[item_desc.id] = item_data;
                if (iret != 0) {
                    break;
                }
            }
            m_file_p->close_file();
        }
        else {
            iret = -3;
            m_error = "Not a valid hdf5 file. Cannot open the file.";
        }
    }
    catch (H5::Exception& err) {
        iret = ERROR;
        m_error = err.getDetailMsg();
    }

    return iret;
}

bool gpm_hdf5_reader::parse_text(const std::string& text_to_parse)
{
    return false;
}

int gpm_hdf5_reader::read_item(const TypeDescr& item_desc, io_array_dim_type& item_data)
{
    const key_type key_id(item_desc.id);
    const auto name = item_desc.name;

    if (item_desc.num_dimensions <= 0) {
        return read_scalar_attribute(key_id, item_desc.pod_type, name);
    }
    const std::string dataset_name = get_dataset_name(key_id, name);
    switch (key_id) {
    case GRIDNAM:
    case PROPNAM: {
        return read_ref_names(key_id, item_data, dataset_name);
    }
    case ZGRID:
    case INDPROP:
    case INDPROP_STATS: {
        return read_ref_values(key_id, item_desc, item_data, dataset_name);
    }
    default: {
        return read_dataset(key_id, item_desc.pod_type, item_data, name);
    }
    }
}

std::string gpm_hdf5_reader::get_dataset_name(const key_type& key_id, const std::string& name)
{
    switch (key_id) {
    case ZGRID:
    case GRIDNAM: {
        return gpm_h5cpp_wrapper::GRIDSDATASET;
    }
    case PROPNAM:
    case INDPROP: {
        return gpm_h5cpp_wrapper::PROPERTIESDATASET;
    }
    case INDPROP_STATS: {
        return gpm_h5cpp_wrapper::STATISTICSDATASET;
    }
    default: {
        return name;
    }
    }
}

int gpm_hdf5_reader::read_ref_names(const key_type& key_id,
                                    io_array_dim_type& item_data,
                                    const std::string& ref_dataset_name)
{
    const H5::DataSet ref_dataset = m_file_p->get_dataset(ref_dataset_name);
    std::vector<std::string> names = m_file_p->read_ref_names(ref_dataset);

    const int count = static_cast<int>(names.size());
    const std::vector<index_type> offset(1, 0);
    const std::vector<size_type> size(1, count);
    const std::vector<hsize_t> dimensions(1, count);
    assign_dimensions(item_data, dimensions);

    signal_string_array(key_id, offset, size, names);
    return count;
}

int gpm_hdf5_reader::read_ref_values(const key_type& key_id,
                                     const TypeDescr& item_desc,
                                     io_array_dim_type& item_data,
                                     const std::string& dataset_name)
{
    if (!m_file_p->dataset_exists(dataset_name)) {
        return 0;
    }

    int num = 0;
    int k = 0;
    const H5::DataSet ref_dataset = m_file_p->get_dataset(dataset_name);
    std::vector<hobj_ref_t> references = m_file_p->get_references(ref_dataset);
    for (std::vector<hobj_ref_t>::const_iterator iterator = references.begin(); iterator != references.end(); ++iterator
    ) {
        const hobj_ref_t ref = *iterator;
        const H5::DataSet dataset = m_file_p->dereference_dataset(ref_dataset, &ref);
        std::vector<hsize_t> dimensions;
        if (item_desc.pod_type == STRING) {
            std::vector<std::string> values = m_file_p->read_string_dataset(dataset, dimensions);
            std::vector<index_type> offset(dimensions.size(), 0);
            std::vector<size_type> size = get_size(dimensions);
            if (key_id == INDPROP) {
                offset.push_back(k);
                size.push_back(1);
                dimensions.push_back(k + 1);
            }

            assign_dimensions(item_data, dimensions);
            signal_string_array(key_id, offset, size, values);
            num += static_cast<int>(values.size());
        }
        else {
            std::vector<float> values = m_file_p->read_dataset<float>(dataset, dimensions);
            std::vector<index_type> offset(dimensions.size(), 0);
            std::vector<size_type> size = get_size(dimensions);
            if (key_id == ZGRID || key_id == INDPROP_STATS) {
                offset.push_back(k);
                size.push_back(1);
                dimensions.push_back(k + 1);
            }

            assign_dimensions(item_data, dimensions);
            signal_float_array(key_id, offset, size, values);
            num += static_cast<int>(values.size());
        }
        k++;
    }

    return num;
}

template <typename S>
std::vector<acf_base_reader::size_type> gpm_hdf5_reader::get_size(const std::vector<S>& dimensions)
{
    std::vector<size_type> size;
    std::transform(dimensions.begin(), dimensions.end(), std::back_inserter(size),[](S val){return boost::numeric_cast<int>(val);});
    // Not understood why
    if (size.size() >= 2) {
        std::swap(size[0], size[1]);
    }

    return size;
}

int gpm_hdf5_reader::read_scalar_attribute(const key_type& key_id,
                                           const ACL_PARSE_POD_TYPES& pod_type,
                                           const std::string& name)
{
    if (!m_file_p->attribute_exists(name)) {
        return 0;
    }

    const std::vector<index_type> indexes;
    switch (pod_type) {
    case STRING: {
        const std::string value = m_file_p->read_string_attribute(name);
        base::signal_string(key_id, value, indexes);
        break;
    }
    case DOUBLE: {
        const double value = m_file_p->read_scalar_attribute<double>(name);
        base::signal_double(key_id, value, indexes);
        break;
    }
    case FLOAT: {
        const float value = m_file_p->read_scalar_attribute<float>(name);
        base::signal_float(key_id, value, indexes);
        break;
    }
    case INTEGER: {
        const int value = m_file_p->read_scalar_attribute<int>(name);
        base::signal_integer(key_id, value, indexes);
        break;
    }
    default:
        break;
    }

    return 1;
}

int gpm_hdf5_reader::read_dataset(const key_type& key_id, const ACL_PARSE_POD_TYPES& pod_type,
                                  io_array_dim_type& item_data, const std::string& name)
{
    if (!m_file_p->dataset_exists(name)) {
        return 0;
    }

    std::vector<hsize_t> dimensions;
    std::vector<int> indexes(4, 0);
    const H5::DataSet dataset = m_file_p->get_dataset(name);

    switch (pod_type) {
    case STRING: {
        const std::vector<std::string> values = m_file_p->read_string_dataset(dataset, dimensions);
        const std::vector<index_type> offset(dimensions.size(), 0);
        const std::vector<size_type> size = get_size(dimensions);
        assign_dimensions(item_data, dimensions);
        signal_string_array(key_id, offset, size, values);
        return values.size();
    }
    case DOUBLE: {
        std::vector<double> values = m_file_p->read_dataset<double>(dataset, dimensions);
        const std::vector<index_type> offset(dimensions.size(), 0);
        const std::vector<size_type> size = get_size(dimensions);
        assign_dimensions(item_data, dimensions);
        signal_double_array(key_id, offset, size, values);
        return values.size();
    }
    case FLOAT: {
        std::vector<float> values = m_file_p->read_dataset<float>(dataset, dimensions);
        const std::vector<index_type> offset(dimensions.size(), 0);
        const std::vector<size_type> size = get_size(dimensions);
        assign_dimensions(item_data, dimensions);
        signal_float_array(key_id, offset, size, values);
        return values.size();
    }
    case INTEGER: {
        std::vector<int> values = m_file_p->read_dataset<int>(dataset, dimensions);
        const std::vector<index_type> offset(dimensions.size(), 0);
        const std::vector<size_type> size = get_size(dimensions);
        assign_dimensions(item_data, dimensions);
        signal_integer_array(key_id, offset, size, values);
        return values.size();
    }
    default:
        break;
    }

    return 0;
}

template <typename T>
void gpm_hdf5_reader::assign_dimensions(io_array_dim_type& item_data, const std::vector<T>& dimensions)
{
    item_data.fill(1);
    std::vector<T> tmp(dimensions);

    if (dimensions.size() > 2) {
        // Reshuffle last dimension, simple rotate right
        // Why last index needs to be first is not understood
        std::rotate(tmp.rbegin(), tmp.rbegin() + 1, tmp.rend());
    }
    std::transform(tmp.rbegin(), tmp.rend(), item_data.rbegin(), [](T val)
    {
        return boost::numeric_cast<io_array_dim_type::value_type>(val);
    });
}
}}}
