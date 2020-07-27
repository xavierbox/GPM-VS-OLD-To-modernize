// -- Schlumberger Private --

#ifndef gpm_type_descriptor_h
#define gpm_type_descriptor_h

#include <vector>
#include <map>
#include <string>


namespace Slb { namespace Exploration { namespace Gpm{

// Get the POD defs in our own namespace
#include "acl_parse_enum_def.h"

	struct TypeDescr {
		// Here we map to the types at hand
		template <ACL_PARSE_POD_TYPES T>
		struct Pod_Promotion{
			typedef void pod_type;
		};
		TypeDescr(int m_id, const std::string& m_name, ACL_PARSE_POD_TYPES m_pod, ACL_PARSE_ARRAY_TYPES m_dims)
			:id(m_id),name(m_name), pod_type(m_pod), num_dimensions(m_dims){
		}
		TypeDescr(int m_id, const std::string& m_name, ACL_PARSE_POD_TYPES m_pod)
			:id(m_id),name(m_name), pod_type(m_pod), num_dimensions(POD_VAL){
		}

        using id_type = int;
		id_type                id; // The enum that we identify it by
		std::string            name; // The name for the parsing and identification
		ACL_PARSE_POD_TYPES    pod_type;
		ACL_PARSE_ARRAY_TYPES  num_dimensions; // POD, 1,2  or 3d array

		static bool compare_by_id(const TypeDescr& a, const TypeDescr& b)
		{
			return  a.id < b.id;
		}

		// Map id to index of array
		static std::map<int, int> make_index_mapper(const std::vector<TypeDescr>& arr){
			std::map<int, int> res;
			for(std::size_t i= 0;i < arr.size();++i){
				res[arr[i].id]=static_cast<int>(i);
			}    
			return res;
		}

		//Map by index
		static std::map<int, int> make_num_dimension_array(const std::vector<TypeDescr>& arr){
			std::map<int, int> res;
			for(const TypeDescr& it: arr){
				res[it.id]=it.num_dimensions;
			}
			return res;
		}
		static std::map<int, int>  make_pod_type_array(const std::vector<TypeDescr>& arr){
			std::map<int, int> res;
			for(const TypeDescr& it: arr){
				res[it.id]=it.pod_type;
			}
			return res;
		}
		static std::map<int, std::string> make_name_array(const std::vector<TypeDescr>& arr){
			std::map<int, std::string> res;
			for(const TypeDescr& it: arr){
				res[it.id]=it.name;
			}
			return res;
		}

	};

	// Need to be explicit outside class scope
	template<> struct TypeDescr::Pod_Promotion<INTEGER> {
		typedef int pod_type;
	};
	template<> struct TypeDescr::Pod_Promotion<FLOAT> {
		typedef float pod_type;
	};
	template<> struct TypeDescr::Pod_Promotion<DOUBLE> {
		typedef double pod_type;
	};
	template<> struct TypeDescr::Pod_Promotion<STRING> {
		typedef std::string pod_type;
	};

	// Typedefs to use for array sizes and parameter descriptions;
	typedef std::vector<TypeDescr> parameter_descr_type;
	//Support 3 dimensions
    //TODO Make this a regular array, so we can test on existence

	template <ACL_PARSE_POD_TYPES TVal> TypeDescr 
		make_typedescription(int m_id, const std::string& m_name, ACL_PARSE_ARRAY_TYPES m_dims)
	{
		return TypeDescr(m_id, m_name, TVal, m_dims);
	}

	template <ACL_PARSE_POD_TYPES TVal> TypeDescr 
		make_typedescription(int m_id, const std::string& m_name)
	{
		return TypeDescr(m_id, m_name, TVal);
	}

}}}


#endif
