// -- Schlumberger Private --

#ifndef gpm_hdf5_writer_impl_h
#define gpm_hdf5_writer_impl_h

#include "gpm_type_descriptor.h"
#include "gpm_h5cpp_wrapper.h"
#include "acf_base_writer.h"
#include <functional>
#include <memory>


namespace Slb { namespace Exploration { namespace Gpm {
struct signals {
	using index_type = acf_base_writer::index_type;
    std::function<std::string(const int& key)>* string_sig;
    std::function<int(const int& key)>* int_sig;
    std::function<float(const int& key)>* float_sig;
    std::function<double(const int& key)>* double_sig;
    std::function<std::string(const int& key, const std::vector<index_type>& indexes)>* string_array_sig;
    std::function<int(const int& key, const std::vector<index_type>& indexes)>* int_array_sig;
    std::function<float(const int& key, const std::vector<index_type>& indexes)>* float_array_sig;
    std::function<double(const int& key, const std::vector<index_type>& indexes)>* double_array_sig;
};

class gpm_h5cpp_wrapper;

class gpm_hdf5_writer_impl {

public:
    typedef int key_type;
	using index_type = acf_base_writer::index_type;
    typedef std::function<std::string(const key_type& key)> signal_string_type;
    typedef std::function<int(const key_type& key)> signal_int_type;
    typedef std::function<float(const key_type& key)> signal_float_type;
    typedef std::function<double(const key_type& key)> signal_double_type;
    typedef std::function<std::string(const key_type& key, const std::vector<index_type>& indexes)> signal_string_array_type;
    typedef std::function<int(const key_type& key, const std::vector<index_type>& indexes)> signal_int_array_type;
    typedef std::function<float(const key_type& key, const std::vector<index_type>& indexes)> signal_float_array_type;
    typedef std::function<double(const key_type& key, const std::vector<index_type>& indexes)> signal_double_array_type;

    gpm_hdf5_writer_impl();
	gpm_hdf5_writer_impl(const gpm_hdf5_writer_impl& rhs)=delete;
	gpm_hdf5_writer_impl& operator=(const gpm_hdf5_writer_impl& rhs) = delete;
    ~gpm_hdf5_writer_impl();

    void write(const TypeDescr& item_desc, const acf_base_writer::io_array_dim_type& item_data);
    bool open_file(const std::string& file_name, bool createnew) ;
    bool close_file() ;

    void set_signals(signals* signals);

private:
    gpm_h5cpp_wrapper m_file_p;
	index_type _prev_grid_nums{};

    void write_scalar_attribute(const key_type& key_id, const ACL_PARSE_POD_TYPES& pod_type,
                                const std::string& name) const;
    void write_array(const key_type& key_id, const ACL_PARSE_POD_TYPES pod_type, const std::string& name,
                     const std::vector<hsize_t>& dimensions, const acf_base_writer::io_array_dim_type& item_data);
    void write_grids(const key_type& key_id, const acf_base_writer::io_array_dim_type& read_extents, std::vector<hsize_t>& dimensions);
    void write_properties(key_type& key_id, const acf_base_writer::io_array_dim_type& read_extents, const std::vector<hsize_t>& dimensions);
    void write_statistics(key_type& key_id, const acf_base_writer::io_array_dim_type& read_extents, const std::vector<hsize_t>& dimensions) const;
    void write_property_names(const key_type& key_id,
                              const acf_base_writer::io_array_dim_type& read_extents, const std::vector<hsize_t>& dimensions);
    void write_property_values(const key_type& key_id,
                               const acf_base_writer::io_array_dim_type& read_extents, const std::vector<hsize_t>& dimensions) const;
    void write_grid_names(const key_type& key_id, const acf_base_writer::io_array_dim_type& read_extents, const std::vector<hsize_t>& dimensions);
    void create_dataset(const key_type& key_id, const ACL_PARSE_POD_TYPES pod_type, const std::string& name,
                        const std::vector<hsize_t>& dimensions, const acf_base_writer::io_array_dim_type& item_data);
    void write_numeric_array(const H5::DataSet& dataset, const H5::DataType& type,
                             const std::vector<hsize_t>& dimensions, const bool& chunking, const void* data);
    void write_string_array(const H5::DataSet& dataset, const std::vector<hsize_t>& dimensions, const bool& chunking,
                            const void* data) const;

    static std::vector<hsize_t> get_max_dimentions(const key_type& key_id, const std::vector<hsize_t>& initial_dims);
    static std::vector<hsize_t> get_chunks_dimentions(const key_type& key_id, const std::vector<hsize_t>& initial_dims);
    static bool is_extendable(const key_type& key_id);

    signal_string_type* m_string_sig{};
    signal_int_type* m_int_sig{};
    signal_float_type* m_float_sig{};
    signal_double_type* m_double_sig{};
    signal_string_array_type* m_string_array_sig{};
    signal_int_array_type* m_int_array_sig{};
    signal_float_array_type* m_float_array_sig{};
    signal_double_array_type* m_double_array_sig{};
};
}}}

#endif
