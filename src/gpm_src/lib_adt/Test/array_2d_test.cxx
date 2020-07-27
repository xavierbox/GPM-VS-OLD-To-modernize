// -- Schlumberger Private --

#include "gpm_basic_defs.h"
#include "gpm_vbl_array_2d.h"
#include <algorithm>
#include "gpm_lin_multi_span.h"

using namespace Slb::Exploration::Gpm;

int array_2d_test(int argc, char* argv[]) {
    // 
    float_2darray_type f1(2,3);
    int_2darray_type i1;
    int i =0;
    for(auto& val:f1) {
        val = static_cast<float>(i);
        ++i;
    }
    int tass=0;
    //std::vector<float> container(10,2);
    //gpm_vbl_array_2d<float> f2(10,2);
    //gpm_vbl_array_1d<float> c2(container);
    //gpm_vbl_array_1d<float> a2(&(container[0]), container.size());
    //bool is1=f2==c2;
    //bool is2=f2==a2;
    return 0;
}

int array_2d_multi_span_test(int argc, char* argv[]) {
	// 
	float_2darray_type f1(2, 3);
	int_2darray_type i1;
	int i = 0;
	for (auto& val : f1) {
		val = static_cast<float>(i);
		++i;
	}
	int tass = 0;
	lin_multi_span<float, 2> holder(f1.begin(), f1.dimensions(), f1.strides());
	auto item = holder[{1, 2}];
	holder[std::array<size_t,2>{1, 2}] = 10;
	int ii = 0;
	//std::vector<float> container(10,2);
	//gpm_vbl_array_2d<float> f2(10,2);
	//gpm_vbl_array_1d<float> c2(container);
	//gpm_vbl_array_1d<float> a2(&(container[0]), container.size());
	//bool is1=f2==c2;
	//bool is2=f2==a2;
	return 0;
}
