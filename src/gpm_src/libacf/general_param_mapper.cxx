// -- Schlumberger Private --

#include "general_param_mapper.h"
#include "acf_base_reader.h"
#include "param_utils.h"
#include "gpm_logger.h"

namespace Slb { namespace Exploration { namespace Gpm {
    namespace {
    template <typename T>
    FunctionConstant<T> make_constant_func(T val) {
        FunctionConstant<T> tmpi([=]() { return val; });
        return tmpi;
    }

    template <typename T>
    Function1d<T> make_1d_func(std::shared_ptr<typename gpm_param_type_mapper::Array_Promotion<ARRAY_1d, T>::array_type> val) {
        Function1d<T> tmpi(val->size(), [=](FunctionBase::index_type i) { return val->at(i); });
        return tmpi;
    }

    template <typename T>
    Function2d<T> make_2d_func(std::shared_ptr<typename gpm_param_type_mapper::Array_Promotion<ARRAY_2d, T>::array_type> val) {
        Function2d<T> tmpi(val->rows(), val->cols(), [=](FunctionBase::index_type i, FunctionBase::index_type j) { return val->at(i, j); });
        return tmpi;
    }

    template <typename T>
    Function3d<T> make_3d_func(std::shared_ptr<typename gpm_param_type_mapper::Array_Promotion<ARRAY_3d, T>::array_type> val) {
        Function3d<T> tmpi(val->layers(), val->rows(), val->cols(), [=](FunctionBase::index_type i, FunctionBase::index_type j, FunctionBase::index_type k) { return val->at(i, j, k); });
        return tmpi;
    }
}

general_param_read_mapper::general_param_read_mapper() = default;


// Grid param mapper
general_param_read_mapper::general_param_read_mapper(acf_base_reader& parser) {
    attach_to_parser(parser);
}

void general_param_read_mapper::attach_to_parser(acf_base_reader& parser) {
    parser.connect_string(std::bind(&general_param_read_mapper::string_mapper, this, std::placeholders::_1, std::placeholders::_2));
    parser.connect_int(std::bind(&general_param_read_mapper::int_mapper, this, std::placeholders::_1, std::placeholders::_2));
    parser.connect_float(std::bind(&general_param_read_mapper::float_mapper, this, std::placeholders::_1, std::placeholders::_2));
    parser.connect_double(std::bind(&general_param_read_mapper::double_mapper, this, std::placeholders::_1, std::placeholders::_2));
    parser.connect_string_array(std::bind(&general_param_read_mapper::string_array_mapper, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    parser.connect_int_array(std::bind(&general_param_read_mapper::int_array_mapper, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    parser.connect_float_array(std::bind(&general_param_read_mapper::float_array_mapper, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    parser.connect_double_array(std::bind(&general_param_read_mapper::double_array_mapper, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	parser.connect_string_array2(std::bind(&general_param_read_mapper::string_array_mapper2, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
	parser.connect_int_array2(std::bind(&general_param_read_mapper::int_array_mapper2, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
	parser.connect_float_array2(std::bind(&general_param_read_mapper::float_array_mapper2, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
	parser.connect_double_array2(std::bind(&general_param_read_mapper::double_array_mapper2, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
	auto tmp = parser.type_descriptors();
	data_holder.initialize_parameters(tmp);
}


TypeDescrToFunctionMapper general_param_read_mapper::make_function_mapper() const {
    std::vector<TypeDescr> descrs;
    std::transform(data_holder._param_descrs.begin(), data_holder._param_descrs.end(), std::back_inserter(descrs), [](const std::pair<acf_base_writer::key_type, TypeDescr>& it) { return it.second; });
    TypeDescrToFunctionMapper res(descrs);
    std::vector<std::pair<key_type, FunctionConstant<int>>> int_consts;
    std::vector<std::pair<key_type, FunctionConstant<float>>> float_consts;
    std::vector<std::pair<key_type, FunctionConstant<double>>> double_consts;
    std::vector<std::pair<key_type, FunctionConstant<std::string>>> string_consts;

    std::vector<std::pair<key_type, Function1d<int>>> int_1ds;
    std::vector<std::pair<key_type, Function1d<float>>> float_1ds;
    std::vector<std::pair<key_type, Function1d<double>>> double_1ds;
    std::vector<std::pair<key_type, Function1d<std::string>>> string_1ds;

    std::vector<std::pair<key_type, Function2d<int>>> int_2ds;
    std::vector<std::pair<key_type, Function2d<float>>> float_2ds;
    std::vector<std::pair<key_type, Function2d<double>>> double_2ds;
    std::vector<std::pair<key_type, Function2d<std::string>>> string_2ds;

    std::vector<std::pair<key_type, Function3d<int>>> int_3ds;
    std::vector<std::pair<key_type, Function3d<float>>> float_3ds;
    std::vector<std::pair<key_type, Function3d<double>>> double_3ds;
    std::vector<std::pair<key_type, Function3d<std::string>>> string_3ds;

    for (const auto& it:data_holder._params) {
        auto key = it.first;
        auto val = it.second;
        const auto pod_type = data_holder._param_descrs.find(key)->second.pod_type;
        const auto dims = data_holder._param_descrs.find(key)->second.num_dimensions;
        switch (dims) {
        case POD_VAL:
            switch (pod_type) {
            case INTEGER:
                int_consts.emplace_back(key, make_constant_func(get_value<INTEGER>(key, int())));
                break;
            case FLOAT:
                float_consts.emplace_back(key, make_constant_func(get_value<FLOAT>(key, float())));
                break;
            case DOUBLE:
                double_consts.emplace_back(key, make_constant_func(get_value<DOUBLE>(key, double())));
                break;
            case STRING:
                string_consts.emplace_back(key, make_constant_func(get_value<STRING>(key, std::string())));
                break;
            default:
                break;
            }
            break;
        case ARRAY_1d:
            switch (pod_type) {
            case INTEGER:
                int_1ds.emplace_back(key, make_1d_func<int>(get_array_ptr<INTEGER, ARRAY_1d>(key)));
                break;
            case FLOAT:
                float_1ds.emplace_back(key, make_1d_func<float>(get_array_ptr<FLOAT, ARRAY_1d>(key)));
                break;
            case DOUBLE:
                double_1ds.emplace_back(key, make_1d_func<double>(get_array_ptr<DOUBLE, ARRAY_1d>(key)));
                break;
            case STRING:
                string_1ds.emplace_back(key, make_1d_func<std::string>(get_array_ptr<STRING, ARRAY_1d>(key)));
                break;
            default:
                break;
            }

            break;
        case ARRAY_2d:
            switch (pod_type) {
            case INTEGER:
                int_2ds.emplace_back(key, make_2d_func<int>(get_array_ptr<INTEGER, ARRAY_2d>(key)));
                break;
            case FLOAT:
                float_2ds.emplace_back(key, make_2d_func<float>(get_array_ptr<FLOAT, ARRAY_2d>(key)));
                break;
            case DOUBLE:
                double_2ds.emplace_back(key, make_2d_func<double>(get_array_ptr<DOUBLE, ARRAY_2d>(key)));
                break;
            case STRING:
                string_2ds.emplace_back(key, make_2d_func<std::string>(get_array_ptr<STRING, ARRAY_2d>(key)));
                break;
            default:
                break;
            }
            break;
        case ARRAY_3d:
            switch (pod_type) {
            case INTEGER:
                int_3ds.emplace_back(key, make_3d_func<int>(get_array_ptr<INTEGER, ARRAY_3d>(key)));
                break;
            case FLOAT:
                float_3ds.emplace_back(key, make_3d_func<float>(get_array_ptr<FLOAT, ARRAY_3d>(key)));
                break;
            case DOUBLE:
                double_3ds.emplace_back(key, make_3d_func<double>(get_array_ptr<DOUBLE, ARRAY_3d>(key)));
                break;
            case STRING:
                string_3ds.emplace_back(key, make_3d_func<std::string>(get_array_ptr<STRING, ARRAY_3d>(key)));
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
    }
    res.set_pod_functions(int_consts);
    res.set_pod_functions(float_consts);
    res.set_pod_functions(double_consts);
    res.set_pod_functions(string_consts);
    res.set_array1d_functions(int_1ds);
    res.set_array1d_functions(float_1ds);
    res.set_array1d_functions(double_1ds);
    res.set_array1d_functions(string_1ds);
    res.set_array2d_functions(int_2ds);
    res.set_array2d_functions(float_2ds);
    res.set_array2d_functions(double_2ds);
    res.set_array2d_functions(string_2ds);
    res.set_array3d_functions(int_3ds);
    res.set_array3d_functions(float_3ds);
    res.set_array3d_functions(double_3ds);
    res.set_array3d_functions(string_3ds);
    return res;
}

void general_param_read_mapper::string_mapper(const key_type& key, const std::string& val) {
    data_holder._params[key] = val;
}

void general_param_read_mapper::int_mapper(const key_type& key, int val) {
    data_holder._params[key] = val;
}

void general_param_read_mapper::float_mapper(const key_type& key, float val) {
    data_holder._params[key] = val;
}

void general_param_read_mapper::double_mapper(const key_type& key, double val) {
    data_holder._params[key] = val;
}

void general_param_read_mapper::string_array_mapper(const key_type& key, const std::vector<index_type>& indexes, const std::string& val) {
    array_mapper(key, indexes, val);
}

void general_param_read_mapper::int_array_mapper(const key_type& key, const std::vector<index_type>& indexes, int val) {
    array_mapper(key, indexes, val);
}

void general_param_read_mapper::float_array_mapper(const key_type& key, const std::vector<index_type>& indexes, float val) {
    /* ARRAYS */
    array_mapper(key, indexes, val);
}

void general_param_read_mapper::double_array_mapper(const key_type& key, const std::vector<index_type>& indexes, double val) {
    array_mapper(key, indexes, val);
}

void general_param_read_mapper::string_array_mapper2(const key_type& key, const std::vector<index_type>& offset, const std::vector<size_type>& size, const std::vector<std::string>& values) {
	array_mapper2(key, offset, size, values);
}

void general_param_read_mapper::int_array_mapper2(const key_type& key, const std::vector<index_type>& offset, const std::vector<size_type>& size, const std::vector<int>& values) {
	array_mapper2(key, offset, size, values);
}

void general_param_read_mapper::float_array_mapper2(const key_type& key, const std::vector<index_type>& offset, const std::vector<size_type>& size, const std::vector<float>& values) {
	/* ARRAYS */
	array_mapper2(key, offset, size, values);
}

void general_param_read_mapper::double_array_mapper2(const key_type& key, const std::vector<index_type>& offset, const std::vector<size_type>& size, const std::vector<double>& values) {
	array_mapper2(key, offset, size, values);
}


bool general_param_read_mapper::has_entry(acf_base_reader::key_type key_val) const {
    return data_holder.has_key(key_val);
}
bool general_param_read_mapper::all_data_types_valid(const Tools::gpm_logger& logger) const
{
    for(auto it:data_holder._params) {
        auto key = it.first;
        auto type = data_holder._param_storage_types.find(key);
        if (it.second.type() != (*type).second.type()) {
            logger.print(Tools::LOG_NORMAL, "The format of the file doesn't match %s\n", data_holder._param_descrs.find(key)->second.name.c_str());
            return false;
        }
    }
    return true;
}

}}}
