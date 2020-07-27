// -- Schlumberger Private --

#ifndef gpm_hdf5_reader_h
#define gpm_hdf5_reader_h

#include "acf_base_reader.h"

namespace Slb { namespace Exploration { namespace Gpm {

class gpm_h5cpp_wrapper;

class gpm_hdf5_reader : public acf_base_reader {
public:
    typedef acf_base_reader base;

    gpm_hdf5_reader();
    ~gpm_hdf5_reader();
    gpm_hdf5_reader(const gpm_hdf5_reader&) = delete;
    gpm_hdf5_reader& operator=(const gpm_hdf5_reader&) = delete;

    bool parse_text(const std::string& text_to_parse) override;
    bool parse_file(const std::string& file_name) override;
    static bool is_hdf5_file(const std::string& file_name);

private:

    int read_item(const TypeDescr& item_desc, io_array_dim_type& item_data);

    int readacf(const std::string& file_name, std::vector<TypeDescr>& type_info, parameter_num_type& inumr,
                array_dim_size_type& idimr, std::string& strerr);
    int read_ref_names(const key_type& key_id, io_array_dim_type& item_data,
                       const std::string& ref_dataset_name);

    static std::string get_dataset_name(const key_type& key_id, const std::string& name);

    template <typename T>
    static std::vector<size_type> get_size(const std::vector<T>& dimensions);

    template <typename T>
    static void assign_dimensions(io_array_dim_type& item_data, const std::vector<T>& dimensions);

    int read_scalar_attribute(const key_type& key_id, const ACL_PARSE_POD_TYPES& pod_type,
                              const std::string& name);
    int read_dataset(const key_type& key_id, const ACL_PARSE_POD_TYPES& pod_type, io_array_dim_type& item_data,
                     const std::string& name);
    int read_ref_values(const key_type& key_id, const TypeDescr& item_desc, io_array_dim_type& item_data,
                        const std::string& name);

    std::unique_ptr<gpm_h5cpp_wrapper> m_file_p;
};
}}}

#endif
