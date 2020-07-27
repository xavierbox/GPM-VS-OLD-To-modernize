// -- Schlumberger Private --

#include "gpm_h5cpp_wrapper.h"
#include <memory>
#include <iterator>
using namespace H5;

namespace Slb { namespace Exploration { namespace Gpm {

const std::string gpm_h5cpp_wrapper::GRIDSDATASET = "GRID_REFERENCES";
const std::string gpm_h5cpp_wrapper::PROPERTIESDATASET = "PROPERTIES_REFERENCES";
const std::string gpm_h5cpp_wrapper::STATISTICSDATASET = "STATISTICS_REFERENCES";

gpm_h5cpp_wrapper::~gpm_h5cpp_wrapper() = default;

std::string gpm_h5cpp_wrapper::get_version() const
{
	const auto attrName = "VERSION";
	const auto attrExists = m_file.attrExists(attrName);
	if (attrExists) {
		const auto attribute = m_file.openAttribute(attrName);
		const auto version = read_string_attribute(attribute);
		return version;
	}

	return "1.0.0.0";
}

bool gpm_h5cpp_wrapper::open_file(const std::string& filename, bool createnew, bool readwrite)
{
    try {
		auto tmp = FileAccPropList(H5P_FILE_ACCESS);
		tmp.setLibverBounds(H5F_LIBVER_LATEST, H5F_LIBVER_LATEST);
		tmp.setCache(0, 0, 0, 0.0F);
        const auto flags = createnew ? H5F_ACC_TRUNC : (readwrite ? H5F_ACC_RDWR | H5F_ACC_SWMR_WRITE : H5F_ACC_RDONLY | H5F_ACC_SWMR_READ);
        m_file = H5File(filename, flags, FileCreatPropList::DEFAULT, tmp);
        return true;
    }
    catch (FileIException& error) {
        return false;
    }
}

bool gpm_h5cpp_wrapper::close_file()
{
    try {
		m_file= H5File();
        return true;
    }
    catch (FileIException& error) {
        return false;
    }
}

bool gpm_h5cpp_wrapper::dataset_exists(const std::string& name) const
{
    return m_file.nameExists(name);
}

bool gpm_h5cpp_wrapper::attribute_exists(const std::string& name) const
{
    return m_file.attrExists(name);
}

DataSet gpm_h5cpp_wrapper::get_dataset(const std::string& name) const
{
    return m_file.openDataSet(name);
}

DataSet gpm_h5cpp_wrapper::dereference_dataset(const DataSet& ref_dataset, const void* ref)
{
    DataSet dataset;
    dataset.dereference(ref_dataset, ref);
    return dataset;
}

DataSet gpm_h5cpp_wrapper::dereference_dataset(const void* ref) const
{
	DataSet dataset;
	dataset.dereference(m_file, ref);
	return dataset;
}

DataSet gpm_h5cpp_wrapper::get_or_create_ref_dataset(const std::string& name, const int& size) const
{
    if (!dataset_exists(name)) {
        const int rank = 1;
        const std::vector<hsize_t> initial_dims(rank, 0);
        const std::vector<hsize_t> maximum_dims(rank, H5S_UNLIMITED);
        const std::vector<hsize_t> chunk_dims(rank, size);
        return create_dataset(name, PredType::STD_REF_OBJ, initial_dims, maximum_dims, chunk_dims);
    }
    else {
        return get_dataset(name);
    }
}

DataSet gpm_h5cpp_wrapper::create_dataset(const std::string& name, const DataType& type,
                                              const std::vector<hsize_t>& initial_dims,
                                              const std::vector<hsize_t>& maximum_dims,
                                              const std::vector<hsize_t>& chunk_dims) const
{
    const DataSpace dataspace(initial_dims.size(), &(initial_dims[0]), &(maximum_dims[0]));
    DataSet dataset;
    if (!chunk_dims.empty()) {
        DSetCreatPropList prop;
        prop.setChunk(chunk_dims.size(), &(chunk_dims[0]));
        dataset = m_file.createDataSet(name, type, dataspace, prop);
        prop.close();
    }
    else {
        dataset = m_file.createDataSet(name, type, dataspace);
    }

    return dataset;
}

void gpm_h5cpp_wrapper::write_scalar_attribute(const DataType& type, const std::string& name, const std::string& value) const
{
	Attribute attribute;
	if (m_file.attrExists(name)) {
		attribute = m_file.openAttribute(name);
	}
	else {
		const DataSpace dataspace(H5S_SCALAR);
		attribute = m_file.createAttribute(name, type, dataspace);
	}
	attribute.write(type, value);
}

void gpm_h5cpp_wrapper::write_scalar_attribute(const DataType& type, const std::string& name, const void* value) const
{
	Attribute attribute;
    if (m_file.attrExists(name)) {
		attribute = m_file.openAttribute(name);
    }
	else {
		const DataSpace dataspace(H5S_SCALAR);
		attribute = m_file.createAttribute(name, type, dataspace);
	}
    attribute.write(type, value);
}

void gpm_h5cpp_wrapper::write_scalar_attribute(const DataSet& dataset, const DataType& type,
                                               const std::string& name, const std::string& value)
{
    const DataSpace dataspace(H5S_SCALAR);
    Attribute attribute;

    if (dataset.attrExists(name)) {
        attribute = dataset.openAttribute(name);
    }
    else {
        attribute = dataset.createAttribute(name, type, dataspace);
    }

    attribute.write(type, value);
}

void gpm_h5cpp_wrapper::write_values(const DataSet& dataset, const DataType& type, const void* data,
                                     const std::vector<hsize_t>& count, const std::vector<hsize_t>& dataset_size,
                                     const std::vector<hsize_t>& offset)
{
    DSetCreatPropList cparms = dataset.getCreatePlist();
    if (H5D_CHUNKED == cparms.getLayout()) {
		dataset.extend(&dataset_size[0]);
        const DataSpace dataspace = dataset.getSpace();
        const int rank = dataspace.getSimpleExtentNdims();
        const DataSpace memspace(rank, &count[0]);
        DataSpace filespace = dataset.getSpace();
        filespace.selectHyperslab(H5S_SELECT_SET, &count[0], &offset[0]);
        dataset.write(data, type, memspace, filespace);
    }
    else {
        dataset.write(data, type);
    }
    cparms.close();
}

void gpm_h5cpp_wrapper::write_references(const std::string& ref_dataset_name, const std::vector<std::string>& names,
                                         const DataType& type, int& count, const std::vector<hsize_t>& initial_dims,
                                         const std::vector<hsize_t>& maximum_dims,
                                         const std::vector<hsize_t>& chunk_dims,
	                                     hsize_t* prev_count_p) const
{
    DataSet ref_dataset = get_or_create_ref_dataset(ref_dataset_name, static_cast<int>(names.size()));
    DataSpace dataspace = ref_dataset.getSpace();

    hsize_t dims[1];
    int rank = dataspace.getSimpleExtentDims(dims);
    hsize_t current_count = dims[0];
    if (prev_count_p != nullptr) {
		*prev_count_p = current_count;
    }
    const std::string prefix = get_dataset_prefix(ref_dataset_name);

    const hsize_t size = current_count + names.size();
    std::vector<hobj_ref_t> reference_buffer(size, hobj_ref_t());
    if (current_count > 0) {
        ref_dataset.read(&reference_buffer[0], PredType::STD_REF_OBJ, H5S_ALL, H5S_ALL);
    }

    int number = ref_dataset_name == GRIDSDATASET ? current_count : 0;
    for (const auto& original_name : names) {
        const std::string unique_name = prefix + std::to_string(number++) + "_" + original_name;

        const DataSet dataset = create_dataset(unique_name, type, initial_dims, maximum_dims, chunk_dims);
        const StrType strType(0, original_name.length() + 1);
        write_scalar_attribute(dataset, strType, "NAME", original_name);

        hobj_ref_t ref;
        m_file.reference(&ref, unique_name, H5R_OBJECT);

        reference_buffer[current_count] = ref;

        ++current_count;
    }

    const hsize_t new_dims[1] = {static_cast<hsize_t>(reference_buffer.size())};
	ref_dataset.extend(new_dims);
    ref_dataset.write(&reference_buffer[0], PredType::STD_REF_OBJ);

    count = reference_buffer.size();
}

std::vector<std::string> gpm_h5cpp_wrapper::read_string_dataset(const DataSet& dataset, std::vector<hsize_t>& dimensions) const
{
	DataSpace dataspace = dataset.getSpace();

    const int rank = dataspace.getSimpleExtentNdims();
	hsize_t sdim[64];
	int ndims = dataspace.getSimpleExtentDims(sdim);
	std::copy(sdim, sdim + rank, std::back_inserter(dimensions));
    const uint64_t size = std::accumulate(sdim, sdim + rank, hsize_t(1), std::multiplies<hsize_t>());

	if (size > 0) {
		std::vector<char*> buffer(size, 0);
		const DataType dataType = dataset.getDataType();
		//	dataset.read((void*)&buffer[0], dataType);
		dataset.read(&buffer[0], dataType);
		auto res = convert_to_strings(buffer);
		// Now lets delete the things
		DataSet::vlenReclaim(&buffer[0], dataType, dataspace);
		return res;
	}
	return std::vector<std::string>();
}

std::vector<std::string> gpm_h5cpp_wrapper::read_string_dataset(const DataSet& dataset, const std::vector<hsize_t>& offset, const std::vector<hsize_t>& count) const
{	
	hobj_ref_t size = count[0] * count[1];

	if (size > 0) {
		DataSpace dataspace = dataset.getSpace();
		const int rank = dataspace.getSimpleExtentNdims();
		const DataSpace memspace(rank, &count[0]);
		dataspace.selectHyperslab(H5S_SELECT_SET, &count[0], &offset[0]);
		std::vector<char*> buffer(size, 0);
		const DataType dataType = dataset.getDataType();
		dataset.read(&buffer[0], dataType, memspace, dataspace);
		auto res = convert_to_strings(buffer);
		DataSet::vlenReclaim(&buffer[0], dataType, memspace);
		return res;
	}

	return std::vector<std::string>();
}

std::vector<hobj_ref_t> gpm_h5cpp_wrapper::get_references(const DataSet& ref_dataset)
{
	std::vector<hsize_t> dims;
	return get_references(ref_dataset, dims);
}

std::vector<hobj_ref_t> gpm_h5cpp_wrapper::get_references(const DataSet& ref_dataset, std::vector<hsize_t>& dimensions)
{
    DataSpace dataspace = ref_dataset.getSpace();

	int rank = dataspace.getSimpleExtentNdims();
	hsize_t sdim[64];
	dataspace.getSimpleExtentDims(sdim, NULL);
	std::copy(sdim, sdim + rank, std::back_inserter(dimensions));
	uint64_t size = std::accumulate(sdim, sdim + rank, hsize_t(1), std::multiplies<hsize_t>());
	
	std::vector<hobj_ref_t> buffer(size);
    
	ref_dataset.read(&buffer[0], PredType::STD_REF_OBJ);
	    
	return buffer;
}

std::vector<hobj_ref_t> gpm_h5cpp_wrapper::get_references(const DataSet& ref_dataset, const std::vector<hsize_t>& offset, const std::vector<hsize_t>& count)
{
	DataSpace dataspace = ref_dataset.getSpace();
	int rank = dataspace.getSimpleExtentNdims();
	const DataSpace memspace(rank, &count[0]);
	dataspace.selectHyperslab(H5S_SELECT_SET, &count[0], &offset[0]);
	hobj_ref_t size = count[0] * count[1];
	std::vector<hobj_ref_t> buffer(size);
	ref_dataset.read(&buffer[0], PredType::STD_REF_OBJ, memspace, dataspace);
	return buffer;
}

std::string gpm_h5cpp_wrapper::get_dataset_prefix(const std::string& ref_dataset_name)
{
    if (ref_dataset_name == GRIDSDATASET) {
        return "GREF_";
    }
    if (ref_dataset_name == PROPERTIESDATASET) {
        return "PREF_";
    }
    if (ref_dataset_name == STATISTICSDATASET) {
        return "STATREF_";
    }

    return "";
}

// This is dangerous, the vector of strings must not go out of scope or change after this call until the returned vector is finished
const std::vector<const char*> gpm_h5cpp_wrapper::convert_to_char_array(const std::vector<hsize_t>& dimensions,
                                                                        std::vector<std::string>& values)
{
    int index = 0;
    const int count = static_cast<int>(values.size());
    const int dimensions_count = static_cast<int>(dimensions.size());
    std::vector<const char*> char_values(count);
    for (int i = 0; i < dimensions[0]; i++) {
        index = i;
        if (dimensions_count > 1) {
            for (int j = 0; j < dimensions[1]; j++) {
                index = i + j * static_cast<int>(dimensions[0]);
                if (dimensions_count > 2) {
                    for (int k = 0; k < dimensions[2]; k++) {
                        index = i + i + j * static_cast<int>(dimensions[0]) + k * static_cast<int>(dimensions[0]) *
                            static_cast<int>(dimensions[1]);
                        char_values[index] = values[index].c_str();
                    }
                }
                else {
                    char_values[index] = values[index].c_str();
                }
            }
        }
        else {
            char_values[index] = values[index].c_str();
        }
    }

    return char_values;
}

std::vector<std::string> gpm_h5cpp_wrapper::read_ref_names(const DataSet& ref_dataset) const
{
    std::vector<hobj_ref_t> references = get_references(ref_dataset);
    std::vector<std::string> names;
    names.resize(references.size());

    int i = 0;
    for (std::vector<hobj_ref_t>::const_iterator iterator = references.begin(); iterator != references.end(); ++iterator) {
        const hobj_ref_t ref = *iterator;
        DataSet dataset = dereference_dataset(ref_dataset, &ref);
        names[i++] = read_string_attribute(dataset, "NAME");
    }

    return names;
}

std::vector<std::string> gpm_h5cpp_wrapper::convert_to_strings(const std::vector<char*>& values)
{
    std::vector<std::string> buffer;

    buffer.reserve(values.size());
    for (const auto& ptr:values) {
        buffer.emplace_back(ptr);
    }

    return buffer;
}

std::string gpm_h5cpp_wrapper::read_string_attribute(const DataSet& dataset, const std::string& name)
{
    const Attribute attribute = dataset.openAttribute(name);
    std::string line = read_string_attribute(attribute);

    return line;
}

std::string gpm_h5cpp_wrapper::read_string_attribute(const std::string& name) const
{
    const Attribute attribute = m_file.openAttribute(name);
    std::string line = read_string_attribute(attribute);

    return line;
}

std::string gpm_h5cpp_wrapper::read_string_attribute(const Attribute& attribute)
{
    const DataType type = attribute.getDataType();
	std::string line;
    attribute.read(type, line);
    return line;
}
}}}
