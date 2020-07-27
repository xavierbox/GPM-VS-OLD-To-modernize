// -- Schlumberger Private --

#ifndef TYPEDESCR_TO_FUNCMAPPER_H
#define TYPEDESCR_TO_FUNCMAPPER_H

#include "gpm_type_descriptor.h"
#include "acf_base_writer.h"
#include <memory>

namespace Slb { namespace Exploration { namespace Gpm {

class FunctionBase {
public:
	using size_type = std::size_t;
    using index_type = std::ptrdiff_t;
    virtual size_type size() const { return 1; }
    virtual std::vector<size_type> array_size() const { return std::vector<size_type>(); }

    virtual ~FunctionBase() {
    }
};

template <typename TVal>
class Function3d:public FunctionBase {
public:
    Function3d(size_type layers, size_type rows, size_type cols, const std::function<TVal (index_type, index_type, index_type)>& loc_func):
        read_func(loc_func), num_layers(layers), num_rows(rows), num_cols(cols) {
        _arr_size.push_back(layers);
        _arr_size.push_back(rows);
        _arr_size.push_back(cols);
    }

    std::function<TVal (index_type, index_type, index_type)> read_func;
	size_type num_layers;
	size_type num_rows;
	size_type num_cols;

	size_type size() const override {
        return num_cols * num_rows * num_layers;
    }

    std::vector<size_type> array_size() const override {
        return _arr_size;
    }

private:
    std::vector<size_type> _arr_size;
};

template <typename TVal>
class Function2d:public FunctionBase {
public:
    Function2d(size_type rows, size_type cols, const std::function<TVal (index_type, index_type)>& loc_func):
        read_func(loc_func), num_rows(rows), num_cols(cols) {
        _arr_size.push_back(rows);
        _arr_size.push_back(cols);
    }

    std::function<TVal (index_type, index_type)> read_func;
	size_type num_rows;
	size_type num_cols;

	size_type size() const override {
        return num_cols * num_rows;
    }

    std::vector<size_type> array_size() const override {
        return _arr_size;
    }

private:
    std::vector<size_type> _arr_size;
};

template <typename TVal>
class Function1d:public FunctionBase {
public:
    Function1d(size_type cols, const std::function<TVal (index_type)>& loc_func):
        read_func(loc_func), num_cols(cols) {
        _arr_size.push_back(cols);
    }

    std::function<TVal (index_type)> read_func;
	size_type num_cols;

	size_type size() const override {
        return num_cols;
    }

    std::vector<size_type> array_size() const override {
        return _arr_size;
    }

private:
    std::vector<size_type> _arr_size;
};

template <typename TVal>
class FunctionConstant:public FunctionBase {
public:
	explicit FunctionConstant(const std::function<TVal ()>& loc_func):
        read_func(loc_func) {
    }

    std::function<TVal()> read_func;

    size_type size() const override {
        return 1;
    }
};

class TypeDescrToFunctionMapper {
public:
    TypeDescrToFunctionMapper() {
    }

    // Must contain all the types that will be added
    explicit TypeDescrToFunctionMapper(const std::vector<TypeDescr>& vec) {
        types = vec;
    }

    // Use these if only one of this type
    template <typename T>
    void set_function(acf_base_writer::key_type key, const FunctionConstant<T>& it) {
        set_pod_functions(std::vector<std::pair<acf_base_writer::key_type, FunctionConstant<T>>>(1, std::make_pair(key, it)));
    }

    template <typename T>
    void set_function(acf_base_writer::key_type key, const Function1d<T>& it) {
        set_array1d_functions(std::vector<std::pair<acf_base_writer::key_type, Function1d<T>>>(1, std::make_pair(key, it)));
    }

    template <typename T>
    void set_function(acf_base_writer::key_type key, const Function2d<T>& it) {
        set_array2d_functions(std::vector<std::pair<acf_base_writer::key_type, Function2d<T>>>(1, std::make_pair(key, it)));
    }

    template <typename T>
    void set_function(acf_base_writer::key_type key, const Function3d<T>& it) {
        set_array3d_functions(std::vector<std::pair<acf_base_writer::key_type, Function3d<T>>>(1, std::make_pair(key, it)));
    }

    template <typename T>
    void set_pod_functions(const std::vector<std::pair<acf_base_writer::key_type, FunctionConstant<T>>>& it) {
        for (const auto& ind: it) {
            std::shared_ptr<FunctionBase> tmp = std::make_shared<FunctionConstant<T>>(ind.second);
            holder.insert(std::make_pair(ind.first, tmp));
        }
    }

    template <typename T>
    void set_array1d_functions(const std::vector<std::pair<acf_base_writer::key_type, Function1d<T>>>& it) {
        for (const auto& ind: it) {
            holder.insert(std::make_pair(ind.first, std::make_shared<Function1d<T>>(ind.second)));
        }
    }

    template <typename T>
    void set_array2d_functions(const std::vector<std::pair<acf_base_writer::key_type, Function2d<T>>>& it) {
        for (const auto& ind: it) {
            holder.insert(std::make_pair(ind.first, std::make_shared<Function2d<T>>(ind.second)));
        }
    }

    template <typename T>
    void set_array3d_functions(const std::vector<std::pair<acf_base_writer::key_type, Function3d<T>>>& it) {
        for (const auto& ind: it) {
            holder.insert(std::make_pair(ind.first, std::make_shared<Function3d<T>>(ind.second)));
        }
    }

    bool are_functions_consistent() const;

    std::map<acf_base_writer::key_type, std::shared_ptr<FunctionBase>> holder;
    std::vector<TypeDescr> types; // All the types in our descr
};

}}}

#endif
