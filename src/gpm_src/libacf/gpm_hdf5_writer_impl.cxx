// -- Schlumberger Private --

#include "gpm_hdf5_writer_impl.h"
#include "acf_base_writer.h"
#include "gpm_h5cpp_wrapper.h"
#include <mapstc_descr.h>
#include "H5Cpp.h"
#include <numeric>
#include <algorithm>
#include <boost/range/irange.hpp>
#include <boost/range/combine.hpp>

namespace {
using namespace Slb::Exploration::Gpm;

template <typename T>
std::vector<T> get_array(const acf_base_writer::key_type& key_id,
                         const acf_base_writer::io_array_dim_type& extents,
                         const std::vector<hsize_t>& dimensions,
                         std::function<T(const acf_base_writer::key_type& key,
                                         const std::vector<acf_base_writer::index_type>& indexes)>& signal)
{
    const auto dimensions_count = static_cast<int>(dimensions.size());
    std::vector<acf_base_writer::index_type> indexes(dimensions_count);

    hsize_t length = std::accumulate(dimensions.begin(), dimensions.end(),hsize_t(1), std::multiplies<hsize_t>());

    const auto columns = dimensions[0];
    hsize_t index = 0;
	acf_base_writer::io_array_dim_type::value_type::index_type col_ind;
	hsize_t col;
    std::vector<T> vector_values(length);
	for (const auto col_tup : boost::combine(boost::irange(columns), extents[2])) {
		boost::tie(col, col_ind) = col_tup;
        if (dimensions_count > 1) {
            const auto rows = dimensions[1];
			acf_base_writer::io_array_dim_type::value_type::index_type row_ind;
			hsize_t row;
			for (const auto row_tup : boost::combine(boost::irange(rows), extents[1])) {
				boost::tie(row, row_ind) = row_tup;
                if (dimensions_count > 2) {
                    auto layers = dimensions[2];
					acf_base_writer::io_array_dim_type::value_type::index_type layer_ind;
					hsize_t layer;
					for (const auto layer_tup : boost::combine(boost::irange(layers), extents[0])) {
						boost::tie(layer, layer_ind) = layer_tup;
						indexes[0] = layer_ind;
                        indexes[1] = row_ind;
                        indexes[2] = col_ind;
                        T val = signal(key_id, indexes);


                        index = col + row * columns + layer * columns * rows;
                        vector_values[index] = val;
                    }
                }
                else {
                    indexes[0] = row_ind;
                    indexes[1] = col_ind;
                    T val = signal(key_id, indexes);

                    index = col + row * columns;
                    vector_values[index] = val;
                }
            }
        }
        else {
            indexes[0] = col_ind;
            T val = signal(key_id, indexes);

            index = col;
            vector_values[index] = val;
        }
    }

    return vector_values;
}

template <typename T>
std::vector<T> get_layer_from_3D_array(const int& key_id, 
	                                   const acf_base_writer::io_array_dim_type::value_type::index_type& k,
                                       const acf_base_writer::io_array_dim_type::value_type& row_extent,
	                                   const acf_base_writer::io_array_dim_type::value_type& col_extent,
                                       const std::vector<hsize_t>& dimensions,
                                       std::function<T(const acf_base_writer::key_type& key,
                                                       const std::vector<acf_base_writer::index_type>& indexes)>& signal)
{
    std::vector<acf_base_writer::index_type> indexes(3);
    std::vector<T> vector_values(dimensions[0] * dimensions[1]);

    const hsize_t columns = dimensions[0];
    const hsize_t rows = dimensions[1];

	acf_base_writer::io_array_dim_type::value_type::index_type row_ind;
	hsize_t row;
    for (const auto row_tup : boost::combine(boost::irange(rows), row_extent)) {
		boost::tie(row, row_ind) = row_tup;
        for (const auto col_tup : boost::combine(boost::irange(columns), col_extent)) {
			acf_base_writer::io_array_dim_type::value_type::index_type col_ind;
			hsize_t col;
			boost::tie(col, col_ind) = col_tup;
            indexes[0] = k;
            indexes[1] = row_ind;
            indexes[2] = col_ind;

            T val = signal(key_id, indexes);
            vector_values[col + row * columns] = val;
        }
    }

    return vector_values;
}

std::vector<hsize_t> get_dimensions(const int& rank, const acf_base_writer::io_array_dim_type& item_data)
{
    std::vector<hsize_t> dimensions(rank);
    // Should just copy from end to beginning
    // Column comes in at the end, ends up first in output
    std::transform(item_data.rbegin(), item_data.rbegin()+rank, dimensions.begin(), [](const acf_base_writer::io_array_dim_type::value_type& val){return val.size();});

    return dimensions;
}
}

namespace Slb { namespace Exploration { namespace Gpm {

gpm_hdf5_writer_impl::gpm_hdf5_writer_impl() 
{
}

gpm_hdf5_writer_impl::~gpm_hdf5_writer_impl() = default;

bool gpm_hdf5_writer_impl::close_file() 
{
    return m_file_p.close_file();
}

bool gpm_hdf5_writer_impl::open_file(const std::string& file_name, bool createnew) 
{
    return m_file_p.open_file(file_name, createnew);
}

void gpm_hdf5_writer_impl::write(const TypeDescr& item_desc, const acf_base_writer::io_array_dim_type& item_data)
{
    key_type key(item_desc.id);
    const std::string name(item_desc.name);

    if (item_desc.num_dimensions <= 0) {
        write_scalar_attribute(key, item_desc.pod_type, name);
    }
    else {
        const int rank = item_desc.num_dimensions;
        std::vector<hsize_t> dimensions = get_dimensions(rank, item_data);

        switch (key) {
        case ZGRID:
        case GRIDNAM: {
            write_grids(key, item_data, dimensions);
            break;
        }
        case INDPROP:
        case PROPNAM: {
            write_properties(key, item_data, dimensions);
            break;
        }
        case INDPROP_STATS: {
            write_statistics(key, item_data, dimensions);
            break;
        }
        default: {
            write_array(key, item_desc.pod_type, name, dimensions, item_data);
        }
        }
    }
}

void gpm_hdf5_writer_impl::set_signals(signals* signals)
{
    m_string_sig = signals->string_sig;
    m_int_sig = signals->int_sig;
    m_float_sig = signals->float_sig;
    m_double_sig = signals->double_sig;
    m_string_array_sig = signals->string_array_sig;
    m_int_array_sig = signals->int_array_sig;
    m_float_array_sig = signals->float_array_sig;
    m_double_array_sig = signals->double_array_sig;
}

//private

void gpm_hdf5_writer_impl::write_scalar_attribute(const key_type& key_id, const ACL_PARSE_POD_TYPES& pod_type,
                                                  const std::string& name) const
{
    if (pod_type == STRING) {
        const std::string value = (*m_string_sig)(key_id);
        const H5::StrType dataType(0, value.length() + 1);
        m_file_p.write_scalar_attribute(dataType, name, value.c_str());
    }
    else if (pod_type == FLOAT) {
        float value = (*m_float_sig)(key_id);
        m_file_p.write_scalar_attribute(H5::PredType::NATIVE_FLOAT, name, &value);
    }
    else if (pod_type == DOUBLE) {
        double value = (*m_double_sig)(key_id);
        m_file_p.write_scalar_attribute(H5::PredType::NATIVE_DOUBLE, name, &value);
    }
    else if (pod_type == INTEGER) {
        int value = (*m_int_sig)(key_id);
        m_file_p.write_scalar_attribute(H5::PredType::STD_I32LE, name, &value);
    }
}

void gpm_hdf5_writer_impl::write_grids(const key_type& key_id, const acf_base_writer::io_array_dim_type& read_extents,
                                       std::vector<hsize_t>& dimensions)
{
    if (key_id == GRIDNAM) {
        write_grid_names(key_id, read_extents, dimensions);
    }
    else {
        const hsize_t numK = dimensions[2];
        const H5::DataSet ref_dataset = m_file_p.get_dataset(gpm_h5cpp_wrapper::GRIDSDATASET);
        auto references = m_file_p.get_references(ref_dataset);
        const auto first_grid_index = _prev_grid_nums; // Set at write_grid_names
		acf_base_writer::io_array_dim_type::value_type::index_type layer_ind;
		hsize_t layer;
		for (const auto layer_tup : boost::combine(boost::irange(numK), read_extents[0])) {
			boost::tie(layer, layer_ind) = layer_tup;
            std::vector<float> data = get_layer_from_3D_array(key_id, layer_ind, read_extents[1], read_extents[2], dimensions, *m_float_array_sig);
            const H5::DataSet dataset = m_file_p.dereference_dataset(ref_dataset, &references[layer + first_grid_index]);
            m_file_p.write_values(dataset, H5::PredType::NATIVE_FLOAT, data.data());
            data.clear();
        }
    }
}

void gpm_hdf5_writer_impl::write_property_names(const key_type& key_id, const acf_base_writer::io_array_dim_type& read_extents,
                                                const std::vector<hsize_t>& dimensions)
{
    int count = 0;
    //1. get properties names
    const std::vector<std::string> names = get_array<std::string>(key_id, read_extents, dimensions, *(m_string_array_sig));

    //2. get INDPROP's datasets dimensions
    const std::vector<hsize_t> dataset_initial_dims(2, 0);
    const std::vector<hsize_t> dataset_maximum_dims(2, H5S_UNLIMITED);
    const std::vector<hsize_t> dataset_chunk_dims(2, 100);

    //3. create ref datasets
    const H5::StrType strType(0, H5T_VARIABLE);
    m_file_p.write_references(gpm_h5cpp_wrapper::PROPERTIESDATASET, names, strType, count, dataset_initial_dims,
                               dataset_maximum_dims, dataset_chunk_dims);

    //4. create statistics ref datasets
    //get dimensions of datasets with values
    std::vector<hsize_t> dims(1, 1);
    //std::vector<int> num_timesteps = m_file_p.read_attribute<int>(parm_descr_holder::get_parm_description(NUMTIMS).name, dims);

    std::vector<hsize_t> stat_dataset_initial_dims(2, 2);
    stat_dataset_initial_dims[0] = 0;
    std::vector<hsize_t> stat_dataset_chunk_dims(2, 2);
    stat_dataset_chunk_dims[0] = 1;
    std::vector<hsize_t> stat_dataset_maximum_dims(2, 2);
    stat_dataset_maximum_dims[0] = H5S_UNLIMITED;

    m_file_p.write_references(gpm_h5cpp_wrapper::STATISTICSDATASET, names, H5::PredType::NATIVE_FLOAT, count,
                               stat_dataset_initial_dims, stat_dataset_maximum_dims, stat_dataset_chunk_dims);
}

void gpm_hdf5_writer_impl::write_property_values(const key_type& key_id,
                                                 const acf_base_writer::io_array_dim_type& read_extents,
                                                 const std::vector<hsize_t>& dimensions) const
{
    const std::vector<hsize_t>::size_type row_dim = dimensions[1];
    const std::vector<hsize_t>::size_type col_dim = dimensions[0];
    std::vector<hsize_t> count = {row_dim, col_dim};

    std::vector<hsize_t> dims(1, 1);
    //std::vector<int> num_rows = m_file_p.read_attribute<int>(parm_descr_holder::get_parm_description(NUMTIMS).name,
    //                                                          dims);
    //std::vector<int> num_cols = m_file_p.read_attribute<int>(parm_descr_holder::get_parm_description(NUMTOPS).name,
    //                                                          dims);

    //if (num_rows.empty()) {
    //    m_file_p.write_scalar_attribute(H5::PredType::STD_I32LE, parm_descr_holder::get_parm_description(NUMTIMS).name,
    //                                     &row_dim);
    //}

    //if (num_cols.empty()) {
    //    m_file_p.write_scalar_attribute(H5::PredType::STD_I32LE, parm_descr_holder::get_parm_description(NUMTOPS).name,
    //                                     &col_dim);
    //}

    const H5::DataSet ref_dataset = m_file_p.get_dataset(gpm_h5cpp_wrapper::PROPERTIESDATASET);
    auto references = m_file_p.get_references(ref_dataset);
	acf_base_writer::io_array_dim_type::value_type::index_type layer_ind;
	hsize_t layer;
	for (const auto layer_tup : boost::combine(boost::irange(dimensions[2]), read_extents[0])) {
		boost::tie(layer, layer_ind) = layer_tup;
		H5::DataSet dataset = m_file_p.dereference_dataset(ref_dataset, &references[layer]);
        H5::DataSpace dataspace = dataset.getSpace();

        std::vector<hsize_t> current_dimensions(2, 0);
        dataspace.getSimpleExtentDims(&(current_dimensions[0]));

        const H5::StrType strType(0, H5T_VARIABLE);
        std::vector<std::string> new_values = get_layer_from_3D_array<std::string>(key_id, layer_ind, read_extents[1], read_extents[2], dimensions, *(m_string_array_sig));
        const std::vector<hsize_t> offset = { current_dimensions[0], 0 };
        const std::vector<hsize_t> dataset_size = { current_dimensions[0] + count[0], count[1] };
        std::vector<const char*> data = gpm_h5cpp_wrapper::convert_to_char_array(count, new_values);
        gpm_h5cpp_wrapper::write_values(dataset, strType, data.data(), count, dataset_size, offset);

        if (current_dimensions[0] > 0) { //need to enter empty strings to previous rows
            std::vector<hsize_t> repl_count = { current_dimensions[0], count[1] - current_dimensions[1] };
            std::vector<std::string> repl_vals(repl_count[0] * repl_count[1], "");
            const std::vector<hsize_t> repl_offset = { 0, current_dimensions[1] };
            std::vector<const char*> repl_data = gpm_h5cpp_wrapper::convert_to_char_array(repl_count, repl_vals);
            gpm_h5cpp_wrapper::write_values(dataset, strType, repl_data.data(), repl_count, dataset_size, repl_offset);
        }

        data.clear();
    }
}

void gpm_hdf5_writer_impl::write_properties(key_type& key_id, const acf_base_writer::io_array_dim_type& read_extents, 
                                            const std::vector<hsize_t>& dimensions)
{
    if (key_id == PROPNAM) {
        write_property_names(key_id, read_extents, dimensions);
    }
    else {
        write_property_values(key_id, read_extents, dimensions);
    }
}

void gpm_hdf5_writer_impl::write_statistics(key_type& key_id, const acf_base_writer::io_array_dim_type& read_extents,
                                            const std::vector<hsize_t>& dimensions) const
{
    const std::vector<hsize_t>::size_type layers = dimensions[2];
    const H5::DataSet ref_dataset = m_file_p.get_dataset(gpm_h5cpp_wrapper::STATISTICSDATASET);
    auto references = m_file_p.get_references(ref_dataset);

    std::vector<hsize_t> count(2, 0);
    count[0] = read_extents[1].upper() - read_extents[1].lower(); //rows
    count[1] = read_extents[2].upper() - read_extents[2].lower(); //cols

	acf_base_writer::io_array_dim_type::value_type::index_type layer_ind;
	hsize_t layer;
	for (const auto layer_tup : boost::combine(boost::irange(layers), read_extents[0])) {
		boost::tie(layer, layer_ind) = layer_tup;
        std::vector<float> data = get_layer_from_3D_array(key_id, layer_ind, read_extents[1], read_extents[2], dimensions, *m_float_array_sig);
        const H5::DataSet dataset = m_file_p.dereference_dataset(ref_dataset, &references[layer]);

        std::vector<hsize_t> current_dimensions(2, 0);
        H5::DataSpace dataspace = dataset.getSpace();
        dataspace.getSimpleExtentDims(&current_dimensions[0]);
        
        std::vector<hsize_t> offset(2, 0);
        offset[0] = current_dimensions[0];
        offset[1] = 0;

        std::vector<hsize_t> dataset_size(2, 0);
        dataset_size[0] = current_dimensions[0] + count[0];
        dataset_size[1] = current_dimensions[1];

        m_file_p.write_values(dataset, H5::PredType::NATIVE_FLOAT, data.data(), count, dataset_size, offset);
        data.clear();
    }
}

void gpm_hdf5_writer_impl::write_grid_names(const key_type& key_id,
                                            const acf_base_writer::io_array_dim_type& read_extents,
                                            const std::vector<hsize_t>& dimensions)
{
    int count = 0;
    //1. get names 
    const std::vector<std::string> names = get_array<std::string>(key_id, read_extents, dimensions, *m_string_array_sig);

    //2. get dimensions of datasets with values
    std::vector<hsize_t> dims(1, 1);
    std::vector<int> num_rows = m_file_p.read_attribute<int>(parm_type_descr_holder::get_parm_description(NUMROWS).name,
                                                              dims);
    std::vector<int> num_cols = m_file_p.read_attribute<int>(parm_type_descr_holder::get_parm_description(NUMCOLS).name,
                                                              dims);

    std::vector<hsize_t> data_initial_dims(2, 1);
    data_initial_dims[0] = num_rows[0];
    data_initial_dims[1] = num_cols[0];

    std::vector<hsize_t> data_maximum_dims(2, 1);
    data_maximum_dims[0] = num_rows[0];
    data_maximum_dims[1] = num_cols[0];

    const std::vector<hsize_t> data_chunk_dims; // won't be chunked

    //3. create and reference datasets
	hsize_t prev_num = 0;
    m_file_p.write_references(gpm_h5cpp_wrapper::GRIDSDATASET, names, H5::PredType::NATIVE_FLOAT, count,
                               data_initial_dims, data_maximum_dims, data_chunk_dims, &prev_num);
	_prev_grid_nums = prev_num;
}

void gpm_hdf5_writer_impl::write_array(const key_type& key_id, const ACL_PARSE_POD_TYPES pod_type,
                                       const std::string& name, const std::vector<hsize_t>& dimensions,
                                       const acf_base_writer::io_array_dim_type& item_data)
{
    if (!m_file_p.dataset_exists(name)) {
        create_dataset(key_id, pod_type, name, dimensions, item_data);
        return;
    }
    else {

        if (key_id == TIMESET || key_id == AGESET) {
            H5::DataSet dataset = m_file_p.get_dataset(name);
            std::vector<hsize_t> current_dimensions(1, 0);
            H5::DataSpace dataspace = dataset.getSpace();
            int ndims = dataspace.getSimpleExtentDims(&current_dimensions[0]);
            std::vector<hsize_t> dataset_size(1, 0);
            dataset_size[0] = current_dimensions[0] + dimensions[0];
            const std::vector<hsize_t> offset(1, 0);
            std::vector<double> prev_values = m_file_p.read_dataset<double>(dataset, current_dimensions);
            std::vector<double> new_values = get_array<double>(key_id, item_data, dimensions, *m_double_array_sig);

            std::vector<double> values;
            values.insert(values.end(), prev_values.begin(), prev_values.end());
            values.insert(values.end(), new_values.begin(), new_values.end());

            m_file_p.write_values(dataset, dataset.getDataType(), values.data(), dataset_size, dataset_size, offset);
            if (key_id == TIMESET) {
                auto count = dataset_size[0];
                m_file_p.write_scalar_attribute(H5::PredType::STD_I32LE, "NUMTIMS", &(count));
            }
        }
    }
}

void gpm_hdf5_writer_impl::write_numeric_array(const H5::DataSet& dataset, const H5::DataType& type,
                                               const std::vector<hsize_t>& dimensions, const bool& chunking,
                                               const void* data)
{
    const std::vector<hsize_t> offset(dimensions.size(), 0);

    if (!chunking) {
        m_file_p.write_values(dataset, type, data);
    }
    else {
        m_file_p.write_values(dataset, type, data, dimensions, dimensions, offset);
    }
}

void gpm_hdf5_writer_impl::write_string_array(const H5::DataSet& dataset, const std::vector<hsize_t>& dimensions,
                                              const bool& chunking, const void* data) const
{
    const std::vector<hsize_t> offset(dimensions.size(), 0);
    const H5::StrType str_type(0, H5T_VARIABLE);
    if (!chunking) {
        m_file_p.write_values(dataset, str_type, data);
    }
    else {
        m_file_p.write_values(dataset, str_type, data, dimensions, dimensions, offset);
    }
}

void gpm_hdf5_writer_impl::create_dataset(const key_type& key_id, const ACL_PARSE_POD_TYPES pod_type,
                                          const std::string& name, const std::vector<hsize_t>& dimensions,
                                          const acf_base_writer::io_array_dim_type& item_data)
{
    std::vector<hsize_t> initial_dims = dimensions;
    if (initial_dims.size() > 1) {
        initial_dims[0] = dimensions[1];
        initial_dims[1] = dimensions[0];
    }
    const std::vector<hsize_t> maximum_dims = get_max_dimentions(key_id, initial_dims);
    const std::vector<hsize_t> chunks = get_chunks_dimentions(key_id, initial_dims);

    if (pod_type == STRING) {
        std::vector<std::string> vector_values = get_array<std::string>(key_id, item_data, dimensions, *m_string_array_sig);
        std::vector<const char*> data = m_file_p.convert_to_char_array(dimensions, vector_values);
        const H5::StrType strType(0, H5T_VARIABLE);
        const H5::DataSet dataset = m_file_p.create_dataset(name, strType, initial_dims, maximum_dims, chunks);
        write_string_array(dataset, initial_dims, !chunks.empty(), data.data());
        data.clear();
        vector_values.clear();
    }
    else {
        if (pod_type == FLOAT) {
            std::vector<float> data = get_array<float>(key_id, item_data, dimensions, *m_float_array_sig);
            const H5::DataSet dataset = m_file_p.create_dataset(name, H5::PredType::NATIVE_FLOAT, initial_dims,
                                                                 maximum_dims, chunks);
            write_numeric_array(dataset, H5::PredType::NATIVE_FLOAT, initial_dims, !chunks.empty(), data.data());
            data.clear();
        }
        else if (pod_type == DOUBLE) {
            std::vector<double> data = get_array<double>(key_id, item_data, dimensions, *m_double_array_sig);
            const H5::DataSet dataset = m_file_p.create_dataset(name, H5::PredType::NATIVE_DOUBLE, initial_dims,
                                                                 maximum_dims, chunks);
            write_numeric_array(dataset, H5::PredType::NATIVE_DOUBLE, initial_dims, !chunks.empty(), data.data());
            data.clear();
        }
        else if (pod_type == INTEGER) {
            std::vector<int> data = get_array<int>(key_id, item_data, dimensions, *m_int_array_sig);
            const H5::DataSet dataset = m_file_p.create_dataset(name, H5::PredType::STD_I32LE, initial_dims,
                                                                 maximum_dims, chunks);
            write_numeric_array(dataset, H5::PredType::STD_I32LE, initial_dims, !chunks.empty(), data.data());
            data.clear();
        }
    }
}

std::vector<hsize_t> gpm_hdf5_writer_impl::get_max_dimentions(const key_type& key_id,
                                                              const std::vector<hsize_t>& initial_dims)
{
    if (is_extendable(key_id)) {
        std::vector<hsize_t> dataset_maximum_dims(initial_dims.size(), H5S_UNLIMITED);
        return dataset_maximum_dims;
    }

    return initial_dims;
}

bool gpm_hdf5_writer_impl::is_extendable(const key_type& key_id)
{
    return key_id == PROPNAM || key_id == GRIDNAM || key_id == TIMESET || key_id == AGESET || key_id == INDPROP ||
        key_id == INDPROP_STATS || key_id == SED_PROP_MAP || key_id == BASE_LAYER_MAP || key_id == PROP_DISPLAY_NAME || key_id == ISNODAL;
}

std::vector<hsize_t> gpm_hdf5_writer_impl::get_chunks_dimentions(const key_type& key_id,
                                                                 const std::vector<hsize_t>& initial_dims)
{
    if (is_extendable(key_id)) {
        std::vector<hsize_t> dataset_chunk_dims(initial_dims);
        return dataset_chunk_dims;
    }
    else {
        std::vector<hsize_t> dataset_chunk_dims;
        return dataset_chunk_dims;
    }
}
}}}
