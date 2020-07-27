// -- Schlumberger Private --

#ifndef BASE_PARAM_HOLDER_H
#define BASE_PARAM_HOLDER_H

#include "gpm_type_descriptor.h"
#include <boost/any.hpp>
#include <map>
#include <memory>
#include <cassert>

namespace Slb { namespace Exploration { namespace Gpm{
// Remember that [] will create items when used, so check properly
	// Class to hold data that is mapped in through the parser normally
template <class TKey> 
class BaseParamHolder
	{
	public:
		typedef TKey key_type;
		// Boost any will have an entry if the data has been read
		std::map<TKey, boost::any> _params;
		// All valid keys will have a description
		std::map<TKey, TypeDescr> _param_descrs;
        std::map<TKey, boost::any> _param_storage_types;
		bool is_value_type(TKey key_val) const
		{
			assert(_param_descrs.count(key_val)!= 0);
			auto res = _param_descrs.find(key_val)->second.num_dimensions == POD_VAL;
			return res;
		}

		bool is_array_type(TKey key_val)const
		{
			return !is_value_type(key_val);
		}

        bool has_key(TKey key_val) const
		{
            return _params.count(key_val) != 0;
		}
 
    template <class TVal> TVal find_value_in_map(TKey key_val, TVal init_val) const
		{
			assert(is_value_type(key_val));
			TVal res = init_val;
			if (_params.count(key_val)!= 0){
				res = boost::any_cast<TVal>(_params.find(key_val)->second);
			}
			return res;
		}
		template <class TVal> std::shared_ptr<TVal> find_ptr_in_map(TKey key_val, const TVal& init_val) const
		{
			assert(is_array_type(key_val));
			std::shared_ptr<TVal> res=std::shared_ptr<TVal>();
			if (_params.count(key_val)!= 0){
				res = boost::any_cast<std::shared_ptr<TVal> >(_params.find(key_val)->second);
			}
			return res;
		}
};


}}}

#endif