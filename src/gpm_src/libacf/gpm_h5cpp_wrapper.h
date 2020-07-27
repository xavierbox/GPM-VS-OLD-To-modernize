// -- Schlumberger Private --

#ifndef gpm_hdf5_api_h
#define gpm_hdf5_api_h

#include "H5Cpp.h"
#include "gpm_lin_span.h"
#include <numeric>
#include <vector>

namespace Slb { namespace Exploration { namespace Gpm {
/*
 * Class to encapsulate access to H5Cpp libraries API
 */
class gpm_h5cpp_wrapper {

public:
    static const std::string GRIDSDATASET;
    static const std::string PROPERTIESDATASET;
    static const std::string STATISTICSDATASET;

    ~gpm_h5cpp_wrapper();

    /*
     * Reads file format version
     */
    std::string get_version() const;

    /*
     * Opens or creates a file. When createnew=true a new file will be created or an existing file will be overwritten.
     */
    bool open_file(const std::string& filename, bool createnew, bool readwrite = true);
    /*
     * Closes a file and releases a file ptr.
     */
    bool close_file();
    /*
     * Looks for a dataset with specified name. When isn't found, returns nullptr.
     */
    H5::DataSet get_dataset(const std::string& name) const;
    /*
     * Returns dereferenced dataset. Looks for ref_dataset dataset, reads values and dereference dataset by index.
     */
    static H5::DataSet dereference_dataset(const H5::DataSet& ref_dataset, const void* ref);

	H5::DataSet dereference_dataset(const void* ref) const;
    /*
     * Creates dataset with rank=1 to store references. The dataset will be chunked and the chunk size will be 10.
     */
    H5::DataSet get_or_create_ref_dataset(const std::string& name, const int& size) const;
    /*
     * Creates dataset with given parameters. Leave chunk_dims empty to create not chunked dataset.
     */
    H5::DataSet create_dataset(const std::string& name, const H5::DataType& type,
                               const std::vector<hsize_t>& initial_dims, const std::vector<hsize_t>& maximum_dims,
                               const std::vector<hsize_t>& chunk_dims) const;
    void write_scalar_attribute(const H5::DataType& type, const std::string& name, const std::string& value) const;
    /*
     * Writes scalar value as an attribute into the file. Attribute will be creates if it doesn't exist.
     */
    void write_scalar_attribute(const H5::DataType& type, const std::string& name, const void* value) const;
    /*
     * Writes scalar value as an attribute into the dataset. Attribute will be creates if it doesn't exist.
     */
    static void write_scalar_attribute(const H5::DataSet& dataset, const H5::DataType& type, const std::string& name,
                                const std::string& value);
    /*
     * Writes values into dataset.
     */
    static void write_values(const H5::DataSet& dataset, const H5::DataType& type, const void* data,
                      const std::vector<hsize_t>& count = std::vector<hsize_t>(0),
                      const std::vector<hsize_t>& dataset_size = std::vector<hsize_t>(0),
                      const std::vector<hsize_t>& offset = std::vector<hsize_t>(0));
    /*
     * Creates referenced datasets by names and writes references into a references dataset.
     */
    void write_references(const std::string& ref_dataset_name, const std::vector<std::string>& names,
                          const H5::DataType& type, int& count, const std::vector<hsize_t>& initial_dims,
                          const std::vector<hsize_t>& maximum_dims, const std::vector<hsize_t>& chunk_dims,
		                  hsize_t* prev_count_p=nullptr) const;

    /*
     * Reads dataset 
     */
	std::vector<std::string> read_string_dataset(const H5::DataSet& dataset, std::vector<hsize_t>& dimensions) const;
	std::vector<std::string> read_string_dataset(const H5::DataSet& dataset, const std::vector<hsize_t>& offset, const std::vector<hsize_t>& count) const;

    template <typename T>
    std::vector<T> read_dataset(const H5::DataSet& dataset, std::vector<hsize_t>& dimensions)
    {
        H5::DataSpace dataspace = dataset.getSpace();

        int rank = dataspace.getSimpleExtentNdims();
        hsize_t sdim[64];
        int ndims = dataspace.getSimpleExtentDims(sdim, NULL);
        std::copy(sdim, sdim+rank, std::back_inserter(dimensions));
        uint64_t size = std::accumulate(sdim, sdim + rank, hsize_t(1), std::multiplies<hsize_t>());

		if (size > 0) {
			std::vector<T> buffer(size);
			H5::DataType dataType = dataset.getDataType();
			dataset.read(&buffer[0], dataType);
			return buffer;
		}
		return std::vector<T>();
    }	

	template <typename T>
	std::vector<T> read_dataset(const H5::DataSet& dataset, const std::vector<hsize_t>& offset, const std::vector<hsize_t>& count)
	{		
		uint64_t size = count[0] * count[1];
		if (size > 0) {
			H5::DataSpace dataspace = dataset.getSpace();
            const int rank = dataspace.getSimpleExtentNdims();
			const H5::DataSpace memspace(rank, &count[0]);
			dataspace.selectHyperslab(H5S_SELECT_SET, &count[0], &offset[0]);
			std::vector<T> buffer(size);
			H5::DataType dataType = dataset.getDataType();
			dataset.read(&buffer[0], dataType, memspace, dataspace);
			return buffer;
		}

		return std::vector<T>();
	}

    template <typename T>
    static void read_dataset(const H5::DataSet& dataset, lin_span<T>* data)
    {
        H5::DataType dataType = dataset.getDataType();
        dataset.read(data->begin(), dataType);
    }

    template <typename T>
    std::vector<T> read_attribute(const H5::Attribute& attribute, std::vector<hsize_t>& dimensions)
    {
        H5::DataType type = attribute.getDataType();
        H5::DataSpace dataspace = attribute.getSpace();

        int rank = dataspace.getSimpleExtentNdims();
        hsize_t sdim[64];
        int ndims = dataspace.getSimpleExtentDims(sdim, nullptr);
        std::copy(sdim, sdim+rank, std::back_inserter(dimensions));
        hsize_t size = std::accumulate(sdim, sdim + rank, hsize_t(1), std::multiplies<hsize_t>());

        std::vector<T> buffer(size);

        attribute.read(type, &(buffer[0]));

        return buffer;
    }

    static std::string read_string_attribute(const H5::DataSet& dataset, const std::string& name);
    std::string read_string_attribute(const std::string& name) const;
    static std::string read_string_attribute(const H5::Attribute& attribute);

    template <typename T>
    T read_scalar_attribute(const std::string& name)
    {
        std::vector<hsize_t> dims(1,1);
        return read_attribute<T>(name, dims).front();
    }

    template <typename T>
    std::vector<T> read_attribute(const std::string& name, std::vector<hsize_t>& dimensions)
    {
        H5::Attribute attribute = m_file.openAttribute(name);
        std::vector<T> buffer = read_attribute<T>(attribute, dimensions);

        return buffer;
    }

    template <typename T>
    std::vector<T> read_attribute(const H5::DataSet& dataset, const std::string& name, std::vector<hsize_t>& dimensions)
    {
        H5::Attribute attribute = dataset.openAttribute(name);
        std::vector<T> buffer = read_attribute<T>(attribute, dimensions);

        return buffer;
    }

    std::vector<std::string> read_ref_names(const H5::DataSet& ref_dataset) const;

    static const std::vector<const char*> convert_to_char_array(const std::vector<hsize_t>& dimensions,
                                                         std::vector<std::string>& values);
    static std::vector<hobj_ref_t> get_references(const H5::DataSet& ref_dataset);
	static std::vector<hobj_ref_t> get_references(const H5::DataSet& ref_dataset, std::vector<hsize_t>& dimensions);
	static std::vector<hobj_ref_t> get_references(const H5::DataSet& ref_dataset, const std::vector<hsize_t>& offset, const std::vector<hsize_t>& count);
    bool dataset_exists(const std::string& name) const;
    bool attribute_exists(const std::string& name) const;

private:
    typedef int key_type;
    static std::vector<std::string> convert_to_strings(const std::vector<char*>& values);

    H5::H5File m_file;

    static std::string get_dataset_prefix(const std::string& ref_dataset_name);
};
}}}

#endif
