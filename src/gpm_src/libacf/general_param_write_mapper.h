// -- Schlumberger Private --

#ifndef general_param_write_mapper_h
#define general_param_write_mapper_h

#include "typedescr_to_funcmapper.h"
#include <string>

namespace Slb { namespace Exploration { namespace Gpm {


class general_param_write_mapper {
public:
    template <ACL_PARSE_ARRAY_TYPES V, class T>
    struct Array_Promotion {
        typedef FunctionConstant<T> array_type;
    };

    template <class T>
    struct Array_Promotion<ARRAY_1d, T> {
        typedef Function1d<T> array_type;
    };

    template <class T>
    struct Array_Promotion<ARRAY_2d, T> {
        typedef Function2d<T> array_type;
    };

    template <class T>
    struct Array_Promotion<ARRAY_3d, T> {
        typedef Function3d<T> array_type;
    };

    template <ACL_PARSE_POD_TYPES T, ACL_PARSE_ARRAY_TYPES V>
    struct Promotion {
        typedef typename TypeDescr::Pod_Promotion<T>::pod_type tmp_type;
        typedef typename Array_Promotion<V, tmp_type>::array_type return_type;
    };

    // Do all the mapping needed
    general_param_write_mapper();
    bool attach_to_writer(const std::string& file_name, const TypeDescrToFunctionMapper& proxy,
                          acf_base_writer* file_water_p);
    // When we adjust, we need to also set the allocated array_sizes correct for now


private:
    typedef acf_base_writer::key_type key_type;
	typedef acf_base_writer::index_type index_type;
    void attach_to_writer(acf_base_writer* writer);
    std::string string_mapper(const key_type& key);
    int int_mapper(const key_type& key);
    float float_mapper(const key_type& key);
    double double_mapper(const key_type& key);
    std::string string_array_mapper(const key_type& key, const std::vector<index_type>& indexes);
    int int_array_mapper(const key_type& key, const std::vector<index_type>& indexes);
    float float_array_mapper(const key_type& key, const std::vector<index_type>& indexes);
    double double_array_mapper(const key_type& key, const std::vector<index_type>& indexes);

    template <typename T>
    T array_mapper(const key_type& key, const std::vector<index_type>& indexes, const T& val) {
        if (indexes.size() == 1) {
            auto tmp = dynamic_cast<typename Array_Promotion<ARRAY_1d, T>::array_type*>(func_holder.holder[key].get());
            return tmp->read_func(indexes[0]);
        }
        else if (indexes.size() == 2) {
            auto tmp = dynamic_cast<typename Array_Promotion<ARRAY_2d, T>::array_type*>(func_holder.holder[key].get());
            return tmp->read_func(indexes[0], indexes[1]);
        }
        else if (indexes.size() == 3) {
            auto tmp = dynamic_cast<typename Array_Promotion<ARRAY_3d, T>::array_type*>(func_holder.holder[key].get());
            return tmp->read_func(indexes[0], indexes[1], indexes[2]);
        }
        return val;
    }

    TypeDescrToFunctionMapper func_holder;
};

}}}
#endif
