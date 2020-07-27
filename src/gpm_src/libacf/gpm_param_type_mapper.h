// -- Schlumberger Private --

#ifndef GPM_PARAM_TYPE_MAPPER_H
#define GPM_PARAM_TYPE_MAPPER_H
#include "base_param_holder.h"
#include "gpm_type_descriptor.h"
#include "gpm_vbl_array_2d.h"
#include "gpm_vbl_vector_array_3d.h"
#include <boost/any.hpp>
#include <memory>

namespace Slb { namespace Exploration { namespace Gpm {
// Remember that [] will create items when used, so check properly
// Class to hold data that is mapped in through the parser normally

class gpm_param_type_mapper: public BaseParamHolder<TypeDescr::id_type> {
public:
    // Some template magic
	template <ACL_PARSE_ARRAY_TYPES V, typename T>
	struct Array_Promotion {
		typedef T array_type;
	};

	template <class T>
	struct Array_Promotion<ARRAY_1d, T> {
		typedef std::vector<T> array_type;
	};

	template <class T>
	struct Array_Promotion<ARRAY_2d, T> {
		typedef gpm_vbl_array_2d<T> array_type;
	};

	template <class T>
	struct Array_Promotion<ARRAY_3d, T> {
		typedef gpm_vbl_vector_array_3d<T> array_type;
	};

	template <ACL_PARSE_POD_TYPES T, ACL_PARSE_ARRAY_TYPES V>
	struct Promotion {
		typedef typename TypeDescr::Pod_Promotion<T>::pod_type tmp_type;
		typedef typename Array_Promotion<V, tmp_type>::array_type return_type;
	};


    // These are the ones to use
	template <ACL_PARSE_POD_TYPES TVal>
	typename TypeDescr::Pod_Promotion<TVal>::pod_type get_value(TypeDescr::id_type key_val,
		typename TypeDescr::Pod_Promotion<TVal>::pod_type init_val) const {
		ACL_PARSE_POD_TYPES t1(TVal);
		ACL_PARSE_ARRAY_TYPES t2(POD_VAL);

		assert(t1 == _param_descrs.find(key_val)->second.pod_type);
		assert(t2 == _param_descrs.find(key_val)->second.num_dimensions);
		auto res = find_value_in_map(key_val, init_val);
		return res;
	}

	template <ACL_PARSE_POD_TYPES TVal, ACL_PARSE_ARRAY_TYPES AVal>
	std::shared_ptr<typename Promotion<TVal, AVal>::return_type> get_array_ptr(TypeDescr::id_type key_val) const {
		// just some tmp stuff
		// This is the check we want to make
		ACL_PARSE_POD_TYPES t1(TVal);
		ACL_PARSE_ARRAY_TYPES t2(AVal);

		assert(t1 == _param_descrs.find(key_val)->second.pod_type);
		assert(t2 == _param_descrs.find(key_val)->second.num_dimensions);
		typename Promotion<TVal, AVal>::return_type init_val;
		auto res = find_ptr_in_map(key_val, init_val);
		return res;
	}

	template <ACL_PARSE_ARRAY_TYPES AVal, typename T>
	std::shared_ptr<typename Array_Promotion<AVal, T>::return_type> get_array_ptr_by_type(TypeDescr::id_type key_val) const {
		// just some tmp stuff
		// This is the check we want to make
		ACL_PARSE_ARRAY_TYPES t2(AVal);
		assert(t2 == _param_descrs.find(key_val)->second.num_dimensions);
		T dummy;
		typename Array_Promotion<AVal, T>::return_type init_val;
		auto res = find_ptr_in_map(key_val, init_val);
		return res;
	}
	void initialize_parameters(const std::vector<TypeDescr>& descr);
	void add_id_holder(TypeDescr::id_type id);
private:

	void make_boost_types();
};


}}}

#endif // GPM_PARAM_TYPE_MAPPER_H
