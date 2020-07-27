// -- Schlumberger Private --

#ifndef param_wrapper_helper_h
#define param_wrapper_helper_h
#include "gpm_basic_defs.h"
#include "gpm_type_descriptor.h"
#include <vector>
#include <map>
#include <string>
#include <utility>
#include "acf_base_io_types.h"

namespace Slb { namespace Exploration { namespace Gpm {

template <class T, class Id>
class param_data_accessor {
public:
    typedef std::string T::* PSTRINGM;
    typedef int T::* PINTM;
    typedef long T::* PLONGM;
    typedef float T::* PFLOATM;

    explicit param_data_accessor(T* item = nullptr): m_item(item) {
    }

    void set_item(T* item) {
        m_item = item;
    }

    void set_accessor(const Id& id, PINTM data) {
        m_int_data_map[id] = data;
    }

    void set_accessor(const Id& id, PLONGM data) {
        m_long_data_map[id] = data;
    }

    void set_accessor(const Id& id, PFLOATM data) {
        m_float_data_map[id] = data;
    }

    void set_accessor(const std::string& id, PSTRINGM data) {
        m_string_data_map[id] = data;
    }

    bool has_int_accessor(const Id& id) {
        return m_int_data_map.find(id) != m_int_data_map.end();
    }

    bool has_long_accessor(const Id& id) {
        return m_long_data_map.find(id) != m_long_data_map.end();
    }

    bool has_float_accessor(const Id& id) {
        return m_float_data_map.find(id) != m_float_data_map.end();
    }

    bool has_string_accessor(const Id& id) {
        return m_string_data_map.find(id) != m_string_data_map.end();
    }

    int get_int(const Id& id) {
        PINTM pm = m_int_data_map[id];
        return m_item ->* pm;
    }

    long get_long(const Id& id) {
        PLONGM pm = m_long_data_map[id];
        return m_item ->* pm;
    }

    float get_float(const Id& id) {
        PFLOATM pm = m_float_data_map[id];
        return m_item ->* pm;
    }

    std::string get_string(const Id& id) {
        PSTRINGM pm = m_string_data_map[id];
        return m_item ->* pm;
    }

    void set_data(const Id& id, int data) {
        PINTM pm = m_int_data_map[id];
        m_item ->* pm = data;
    }

    void set_data(const Id& id, long data) {
        PLONGM pm = m_long_data_map[id];
        m_item ->* pm = data;
    }

    void set_data(const Id& id, float data) {
        PFLOATM pm = m_float_data_map[id];
        m_item ->* pm = data;
    }

    void set_data(const Id& id, const std::string& data) {
        PSTRINGM pm = m_string_data_map[id];
        m_item ->* pm = data;
    }

private:
    T* m_item;
    std::map<Id, PINTM> m_int_data_map;
    std::map<Id, PLONGM> m_long_data_map;
    std::map<Id, PFLOATM> m_float_data_map;
    std::map<Id, PSTRINGM> m_string_data_map;

};

// This helper class should also be moved to own class
class param_wrapper_helper:acf_base_io_types {
public:
    typedef std::map<key_type, std::vector<index_type>> array_size_mapper_type;
    // This will hold min and max index as we parse the arrays
    typedef std::map<key_type, std::vector<index_pair_type>> array_size_index_type;
    // This will hold the variables set, and how many times
    typedef std::map<key_type, size_type> variable_count_type;
    typedef std::map<key_type, int> variable_type_type;
    // These two should have been a bijective map, perhaps later
    typedef std::map<key_type, std::string> variable_id_string_type;
    typedef std::map<std::string, key_type> variable_string_id_type;
    // Lets do some accounting 
    array_size_index_type array_indexes;
    variable_count_type variable_counter;
    variable_type_type variable_type;
    //    array_size_mapper_type array_sizes;

    variable_id_string_type id_string_map;
    variable_string_id_type string_id_map;

    static const int char_type_num = 2;
    static const int float_type_num = 1;
    static const int int_type_num = 0;


    template <typename T1, typename T2>
    static void initialize_arrays(T1& arr, T2 val) {
        if (!arr.empty()) {
            std::fill(arr.begin(), arr.end(), val);
        }
    }


    void init(const std::vector<TypeDescr>& type_info);
    void clear();

    void add_array_indexes(key_type id, const std::vector<index_type>& inds);
    void add_variable_count(key_type id);
    void add_variable_count(key_type id, index_type count);

    static void calc_array_indexes(array_size_index_type& data, int id, const std::vector<index_type>& inds);
    static array_size_mapper_type setup_array_sizes(const std::vector<TypeDescr>& type_info, const array_size_mapper_type& arr_size);
    static array_size_index_type setup_array_indexes(const std::vector<TypeDescr>& type_info, const array_size_mapper_type& arr_size);
    static variable_count_type setup_variable_count(const std::vector<TypeDescr>& type_info);
    static variable_type_type setup_variable_type(const std::vector<TypeDescr>& type_info);
    static variable_id_string_type setup_id_string_map(const std::vector<TypeDescr>& type_info);
    static variable_string_id_type setup_string_id_map(const std::vector<TypeDescr>& type_info);
};

}}}
#endif
