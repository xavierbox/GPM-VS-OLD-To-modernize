// -- Schlumberger Private --

#include "gpm_vbl_vector_array_3d.h"
using namespace Slb::Exploration::Gpm;

int array_3d_vector_test(int argc, char* argv[]) {
    // 
    gpm_vbl_vector_array_3d<float> f1(1, 3, 5);
    gpm_vbl_vector_array_3d<int> i1;
    f1.at(0, 0, 0) = 12;
    f1(0, 0, 0) = 10;

    return 0;
}
