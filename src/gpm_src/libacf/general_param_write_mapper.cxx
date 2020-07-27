// -- Schlumberger Private --

#include "general_param_write_mapper.h"
#include "acf_base_ascii_writer.h"
#include "acf_base_helper.h"
#include <boost/any.hpp>
#include <functional>
#include <cassert>

namespace Slb { namespace Exploration { namespace Gpm { namespace {

}

general_param_write_mapper::general_param_write_mapper() {
}

// Grid param mapper
void general_param_write_mapper::attach_to_writer(acf_base_writer* parser_p) {
    assert(parser_p != nullptr);
    parser_p->connect_string(std::bind(&general_param_write_mapper::string_mapper, this, std::placeholders::_1));
    parser_p->connect_int(std::bind(&general_param_write_mapper::int_mapper, this, std::placeholders::_1));
    parser_p->connect_float(std::bind(&general_param_write_mapper::float_mapper, this, std::placeholders::_1));
    parser_p->connect_double(std::bind(&general_param_write_mapper::double_mapper, this, std::placeholders::_1));
    parser_p->connect_string_array(std::bind(&general_param_write_mapper::string_array_mapper, this, std::placeholders::_1, std::placeholders::_2));
    parser_p->connect_int_array(std::bind(&general_param_write_mapper::int_array_mapper, this, std::placeholders::_1, std::placeholders::_2));
    parser_p->connect_float_array(std::bind(&general_param_write_mapper::float_array_mapper, this, std::placeholders::_1, std::placeholders::_2));
    parser_p->connect_double_array(std::bind(&general_param_write_mapper::double_array_mapper, this, std::placeholders::_1, std::placeholders::_2));
}

bool general_param_write_mapper::attach_to_writer(const std::string& file_name, const TypeDescrToFunctionMapper& proxy,
                                                  acf_base_writer* file_water_p) {
    const auto max_el = std::max_element(proxy.types.begin(), proxy.types.end(), [](const TypeDescr& a, const TypeDescr& b) { return a.id < b.id; });
    const auto vector_size = max_el->id + 1;
    // Now lets add in the potential ones that are not 
    // The arrays have to have an entry for any id
    acf_base_ascii_writer::array_dim_size_type dims(vector_size);
    acf_base_writer::parameter_num_type keys(vector_size);
    for (const auto& it : proxy.holder) {
        auto key = it.first;
        auto found = std::find_if(proxy.types.begin(), proxy.types.end(), [key](const TypeDescr& item) { return key == item.id; });
        if (found != proxy.types.end()) {
            const auto size = boost::any_cast<std::shared_ptr<FunctionBase>>(it.second)->size();
            auto array_size = boost::any_cast<std::shared_ptr<FunctionBase>>(it.second)->array_size();
            keys[found->id] = size;
            acf_base_helper::fill(&dims[found->id], 1);
            if (!array_size.empty()) {
                std::transform(array_size.begin(), array_size.end(), dims[found->id].begin() + dims[found->id].size() - array_size.size(), [](FunctionBase::size_type val){return acf_base_writer::array_dim_size_type::value_type::value_type(val);});
            }
        }
    }
    attach_to_writer(file_water_p);
	file_water_p->set_write_info(proxy.types, keys, dims);
    const auto res = file_water_p->open_file(file_name, true);
    func_holder = proxy;
    return res;
}

std::string general_param_write_mapper::string_mapper(const key_type& key) {
    const auto& tmp = dynamic_cast<FunctionConstant<std::string>*>(func_holder.holder[key].get());
    return tmp->read_func();
}

int general_param_write_mapper::int_mapper(const key_type& key) {
    const auto& tmp = dynamic_cast<FunctionConstant<int>*>(func_holder.holder[key].get());;
    return tmp->read_func();
}

float general_param_write_mapper::float_mapper(const key_type& key) {
    const auto& tmp = dynamic_cast<FunctionConstant<float>*>(func_holder.holder[key].get());;
    return tmp->read_func();
}

double general_param_write_mapper::double_mapper(const key_type& key) {
    const auto& tmp = dynamic_cast<FunctionConstant<double>*>(func_holder.holder[key].get());;
    return tmp->read_func();
}

std::string general_param_write_mapper::string_array_mapper(const key_type& key, const std::vector<index_type>& indexes) {
    const std::string tmp;
    return array_mapper(key, indexes, tmp);
}

int general_param_write_mapper::int_array_mapper(const key_type& key, const std::vector<index_type>& indexes) {
    return array_mapper(key, indexes, int(1));
}

float general_param_write_mapper::float_array_mapper(const key_type& key, const std::vector<index_type>& indexes) {
    /* ARRAYS */
    return array_mapper(key, indexes, float(0.0F));
}

double general_param_write_mapper::double_array_mapper(const key_type& key, const std::vector<index_type>& indexes) {
    return array_mapper(key, indexes, double(0.0));
}


}}}
