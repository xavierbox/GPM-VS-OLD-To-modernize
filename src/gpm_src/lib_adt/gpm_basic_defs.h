// -- Schlumberger Private --

#ifndef gpm_basic_defs_h
#define gpm_basic_defs_h

#include "gpm_lin_span.h"
#include <vector>
#include <cassert>


namespace Slb { namespace Exploration { namespace Gpm{

    //typedef std::pair<int,int> index2_int_type;

	// Some forward declarations
	template <typename T> class gpm_vbl_array_2d;
	template <typename T> class gpm_array_2d_base;
	template <typename T> class gpm_vbl_array_3d;
	template <typename T> class gpm_array_3d_base;
	template <typename T> class gpm_vbl_vector_array_3d;
	template <typename T> class gpm_array_3d_vec_base;
	class function_2d;
	class int_extent_2d;
	class int_extent_1d;

    typedef gpm_vbl_array_2d<float> float_2darray_type;
    typedef gpm_array_2d_base<float> float_2darray_base_type;
    typedef gpm_vbl_array_2d<double> double_2darray_type;
    typedef gpm_array_2d_base<double> double_2darray_base_type;
    typedef gpm_vbl_array_2d<int>    int_2darray_type;
    typedef gpm_array_2d_base<int>   int_2darray_base_type;
    typedef gpm_vbl_array_2d<bool> bool_2darray_type;
    typedef gpm_array_2d_base<bool> bool_2darray_base_type;
    typedef gpm_vbl_array_3d<float> float_3darray_type;
    typedef gpm_array_3d_base<float> float_3darray_base_type;
    typedef gpm_vbl_vector_array_3d<float> float_3darray_vec_type;
    typedef gpm_array_3d_vec_base<float> float_3darray_vec_base_type;
    typedef gpm_vbl_vector_array_3d<double> double_3darray_vec_type;
    typedef gpm_array_3d_vec_base<double> double_3darray_vec_base_type;
    
	// This one will become static_vector
	//typedef fixed_4float_array_type sediment_mix_type;
    //typedef std_alias::array<gpm_array_2d_base<float>*, num_sediments> sediment_array_type;
    typedef std::vector<function_2d> sediment_func_array_type;

	//typedef boost::iterator_range<sediment_mix_type::iterator> sediment_mix_iterator;
	//typedef boost::iterator_range<sediment_mix_type::const_iterator> sediment_mix_const_iterator;
	//typedef float_3darray_base_type sediment_layer_base_type;
	//typedef float_3darray_type  sediment_layer_type;

	enum FlowType {hypopycnal=-1, isopycnal, hyperpycnal};

	template <class T> class scaled_distance_function{
	public:
		scaled_distance_function():_val(T(0)){}
		explicit scaled_distance_function(const T& val):_val(val){}
//		T operator()(const T& a)const {return _val;}
		T operator()(const T& a, const T& b)const {return (b-a)*_val;}
	private:
		T _val;
	};
	template <class T> class constant_value_function{
	public:
		constant_value_function():_val(T(0)){}
		explicit constant_value_function(const T& val):_val(val){}
		//		T operator()(const T& a)const {return _val;}
		T operator()(const T& a)const {return _val;}
	private:
		T _val;
	};

}}}



#endif
