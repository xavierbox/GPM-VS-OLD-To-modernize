// -- Schlumberger Private --

#ifndef general_param_mapper_h
#define general_param_mapper_h

#include "gpm_param_type_mapper.h"
#include "acf_base_reader.h"
#include "base_param_holder.h"
#include "typedescr_to_funcmapper.h"  // In case we want to make a writer of what has been read in
#include "gpm_logger.h"
#include "param_utils.h"

namespace Slb { namespace Exploration { namespace Gpm {

class general_param_read_mapper:acf_base_io_types {
public:
    // Do all the mapping needed

    explicit general_param_read_mapper(acf_base_reader& parser);
    general_param_read_mapper();
    void attach_to_parser(acf_base_reader& parser);
    // When we adjust, we need to also set the allocated array_sizes correct for now

	template <ACL_PARSE_POD_TYPES TVal>
	typename TypeDescr::Pod_Promotion<TVal>::pod_type get_value(acf_base_reader::key_type key_val,
		typename TypeDescr::Pod_Promotion<TVal>::pod_type init_val) const {
		return data_holder.get_value<TVal>(key_val, init_val);
	}

	template <ACL_PARSE_POD_TYPES TVal, ACL_PARSE_ARRAY_TYPES AVal>
	std::shared_ptr<typename gpm_param_type_mapper::Promotion<TVal, AVal>::return_type> get_array_ptr(acf_base_reader::key_type key_val) const {
		return data_holder.get_array_ptr<TVal, AVal>(key_val);
	}
    bool has_entry(acf_base_reader::key_type key_val) const;

    bool all_data_types_valid(const Tools::gpm_logger& logger) const;


    TypeDescrToFunctionMapper make_function_mapper() const;
private:
    void string_mapper(const key_type& key, const std::string& val);
    void int_mapper(const key_type& key, int val);
    void float_mapper(const key_type& key, float val);
    void double_mapper(const key_type& key, double val);
    void string_array_mapper(const key_type& key, const std::vector<index_type>& indexes, const std::string& val);
    void int_array_mapper(const key_type& key, const std::vector<index_type>& indexes, int val);
    void float_array_mapper(const key_type& key, const std::vector<index_type>& indexes, float val);
    void double_array_mapper(const key_type& key, const std::vector<index_type>& indexes, double val);
	void string_array_mapper2(const key_type& key, const std::vector<index_type>& offset, const std::vector<size_type>& size, const std::vector<std::string>& values);
	void int_array_mapper2(const key_type& key, const std::vector<index_type>& offset, const std::vector<size_type>& size, const std::vector<int>& values);
	void float_array_mapper2(const key_type& key, const std::vector<index_type>& offset, const std::vector<size_type>& size, const std::vector<float>& values);
	void double_array_mapper2(const key_type& key, const std::vector<index_type>& offset, const std::vector<size_type>& size, const std::vector<double>& values);


	template <typename T>
    void array_mapper(const key_type& key, const std::vector<index_type>& indexes, const T& val) {
        if (indexes.size() == 1) {
            if (!data_holder.has_key(key)) {
				data_holder.add_id_holder(key);
            }
//			auto tmp = data_holder.get_array_ptr_by_type<ARRAY_1d, T>(key);
			auto tmp = boost::any_cast<std::shared_ptr<typename gpm_param_type_mapper::Array_Promotion<ARRAY_1d, T>::array_type>>(data_holder._params[key]);
            param_helper::add_val_to_array(*tmp, indexes[0], val);
        }
        else if (indexes.size() == 2) {
            if (!data_holder.has_key(key)) {
				data_holder.add_id_holder(key);
            }
			//auto tmp = data_holder.get_array_ptr<ARRAY_2d, T>(key);
            auto tmp = boost::any_cast<std::shared_ptr<typename gpm_param_type_mapper::Array_Promotion<ARRAY_2d, T>::array_type>>(data_holder._params[key]);
            param_helper::add_val_to_array(*tmp, indexes, val);
        }
        else if (indexes.size() == 3) {
            if (!data_holder.has_key(key) ) {
				data_holder.add_id_holder(key);
            }
            // TODO why doesn't this work JT
			//auto tmp = data_holder.get_array_ptr_by_type<ARRAY_3d, T>(key);
            auto tmp = boost::any_cast<std::shared_ptr<typename gpm_param_type_mapper::Array_Promotion<ARRAY_3d, T>::array_type>>(data_holder._params[key]);
            param_helper::add_val_to_array(*tmp, indexes, val);
        }

    }

	template <typename T>
	void array_mapper2(const key_type& key, const std::vector<index_type>& offset, const std::vector<size_type>& size, const std::vector<T>& values) {
		std::vector<index_type> indexes(offset.rbegin(), offset.rend());
		std::vector<index_type> offset_lij(offset.rbegin(), offset.rend());
		std::vector<size_type> size_lij(size.rbegin(), size.rend());
		int k = 0;
		if (size.size() == 1)
		{
			for (int i = 0; i < size[0];++i) {
				array_mapper(key, indexes, values[k]);
				++k;
				++indexes[0];
			}
		}
		else if (size.size() == 2) {
			for (int i = 0; i < size_lij[0];++i)
			{
				indexes[1] = offset_lij[1];
				for (int j = 0; j < size_lij[1];++j)
				{
					array_mapper(key, indexes, values[k]);
					++k;
					++indexes[1];
				}
				++indexes[0];
			}
		}
		else if (size.size() == 3) {
			for (int l = 0; l < size_lij[0];++l)
			{
				indexes[1] = offset_lij[1];
				for (int i = 0; i < size_lij[1];++i)
				{
					indexes[2] = offset_lij[2];
					for (int j = 0; j < size_lij[2];++j)
					{
						array_mapper(key, indexes, values[k]);
						++k;
						++indexes[2];
					}
					++indexes[1];
				}
			}
		}

	}

    gpm_param_type_mapper data_holder;
};


}}}
#endif
