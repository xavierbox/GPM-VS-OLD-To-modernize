// -- Schlumberger Private --

#include "gpm_vbl_array_3d.h"
#include <vector>
#include <boost/range/iterator_range.hpp>
using namespace Slb::Exploration::Gpm;

int array_3d_test(int argc, char* argv[]) {
    // 
    gpm_vbl_array_3d<float> f1(1, 3, 5);
    gpm_vbl_array_3d<int> i1;
    f1.at(0, 0, 0) = 12;
    f1(0, 0, 0) = 10;
    boost::iterator_range<gpm_vbl_array_3d<float>::iterator> it(f1.begin(), f1.end());
    gpm_vbl_array_3d<float>::col_iterator mt_typ(f1(0, 0));
    std::vector<float> init_4float_array(4, 0);
    boost::iterator_range<std::vector<float>::iterator> test(init_4float_array);
    boost::iterator_range<std::vector<float>::const_iterator> test1(init_4float_array);
    boost::iterator_range<float*> t1(&init_4float_array[0], &init_4float_array[0] + init_4float_array.size());
    auto tt = f1(0, 0);
    tt[1] = 12;
    t1[0] = 23;
    test[0] = 12;
    mt_typ[0] = 23;
    //std::vector<float> container(10,2);
    //gpm_vbl_array_1d<float> f2(10,2);
    //gpm_vbl_array_1d<float> c2(container);
    //gpm_vbl_array_1d<float> a2(&(container[0]), container.size());
    //bool is1=f2==c2;
    //bool is2=f2==a2;
    return 0;
}
