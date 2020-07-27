// -- Schlumberger Private --

#include "param_wrapper_helper.h"
#include <algorithm>
#include <limits>

namespace Slb { namespace Exploration { namespace Gpm {

void
param_wrapper_helper::init(const std::vector<TypeDescr>& type_info) {
    //array_sizes = setup_array_sizes(type_info, arr_size);
    variable_counter = setup_variable_count(type_info);
    variable_type = setup_variable_type(type_info);
    array_indexes.clear();// = setup_array_indexes(type_info, arr_size);
    id_string_map = setup_id_string_map(type_info);
    string_id_map = setup_string_id_map(type_info);
}

void
param_wrapper_helper::clear() {
    //array_sizes.clear();
    variable_counter.clear();
    array_indexes.clear();

}

void
param_wrapper_helper::add_array_indexes(key_type id, const std::vector<index_type>& inds) {
    calc_array_indexes(array_indexes, id, inds);
}

void
param_wrapper_helper::add_variable_count(key_type id) {
    variable_counter[id] += 1;
}

void param_wrapper_helper::add_variable_count(key_type id, index_type count) {
    variable_counter[id] += count;
}

// Wrapper helper
param_wrapper_helper::array_size_mapper_type
param_wrapper_helper::setup_array_sizes(const std::vector<TypeDescr>& type_info, const array_size_mapper_type& arr_size) {
    array_size_mapper_type array_sizes;
    std::size_t npar = type_info.size();
    for (std::size_t i = 0; i < npar; ++i) {
        if (type_info[i].num_dimensions > 0) {
            // we have an array
            const TypeDescr& obj(type_info[i]);
            int num = obj.num_dimensions;
            int id = obj.id;
            array_sizes[id] = arr_size.at(obj.id);
        }
    }
    return array_sizes;
}


// Wrapper helper
param_wrapper_helper::variable_count_type
param_wrapper_helper::setup_variable_count(const std::vector<TypeDescr>& type_info) {
    variable_count_type vari_count;
    std::size_t npar = type_info.size();
    for (std::size_t i = 0; i < npar; ++i) {
        //       if (ndim[i] == 0 ){
        // we have an variable
        int id = type_info[i].id;
        vari_count[id] = 0;
        //       }
    }
    return vari_count;
}

param_wrapper_helper::variable_type_type
param_wrapper_helper::setup_variable_type(const std::vector<TypeDescr>& type_info) {
    variable_type_type vari_type;
    std::size_t npar = type_info.size();
    for (std::size_t i = 0; i < npar; ++i) {
        //       if (ndim[i] == 0 ){
        // we have an variable
        key_type id = type_info[i].id;
        vari_type[id] = type_info[i].pod_type;
        //       }
    }
    return vari_type;
}

void
param_wrapper_helper::calc_array_indexes(array_size_index_type& data, key_type id, const std::vector<index_type>& inds) {
    std::vector<index_pair_type>& tmp(data[id]);
    // Remnant of the old array counting system
    std::size_t ind_min = std::min(tmp.size(), inds.size());
    for (std::size_t i = 0; i < ind_min; ++i) {
        tmp[i].first = std::min(tmp[i].first, inds[i]);
        tmp[i].second = std::max(tmp[i].second, inds[i]);
    }
}

param_wrapper_helper::array_size_index_type
param_wrapper_helper::setup_array_indexes(const std::vector<TypeDescr>& type_info, const array_size_mapper_type& arr_size) {
    array_size_index_type array_sizes;
    int count = 0;
    std::size_t npar = type_info.size();
    for (std::size_t i = 0; i < npar; ++i) {
        if (type_info[i].num_dimensions > 0) {
            // we have an array
            const TypeDescr& obj(type_info[i]);
            int num = obj.num_dimensions;
            int id = obj.id;
            std::vector<index_pair_type> tmp;
            for (int j = 0; j < num; ++j) {
                std::pair<int, int> old(std::numeric_limits<int>::max(), -std::numeric_limits<int>::max());
                tmp.push_back(old);
                ++count;
            }
            array_sizes[id] = tmp;
        }
    }
    return array_sizes;
}

param_wrapper_helper::variable_id_string_type
param_wrapper_helper::setup_id_string_map(const std::vector<TypeDescr>& type_info) {
    variable_id_string_type map;
    std::size_t npar = type_info.size();
    for (std::size_t i = 0; i < npar; ++i) {
        TypeDescr loc_type(type_info[i]);
        map[loc_type.id] = loc_type.name;
    }
    return map;
}

param_wrapper_helper::variable_string_id_type
param_wrapper_helper::setup_string_id_map(const std::vector<TypeDescr>& type_info) {
    array_size_index_type array_sizes;
    std::size_t npar = type_info.size();
    variable_string_id_type map;
    for (std::size_t i = 0; i < npar; ++i) {
        TypeDescr loc_type(type_info[i]);
        map[loc_type.name] = loc_type.id;
    }
    return map;
}

}}}

