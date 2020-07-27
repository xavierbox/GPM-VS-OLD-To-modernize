#include "gpm_2dvector_tie_array.h"
#include <numeric>
#include <cmath>

bool test_diff(float a, float b, float eps=1e-6f)
{
    return std::abs(a - b) < eps;
}

int gpm_2dvector_tie_origin_test(int argc, char* argv[]) {
    Slb::Exploration::Gpm::gpm_2dvector_tie_array tmp(Slb::Exploration::Gpm::int_extent_2d(2,3));
    tmp.fill(Slb::Exploration::Gpm::int_extent_2d(1, 2),2.0F);
    const auto ret1 = (2.0F - tmp.get_vx()(0, -1)) + (2.0F-tmp.get_vx()(0, 0)) + (2.0F-tmp.get_vx()(0, 1));
    const auto ret2 = 2.0F - tmp.get_vy()(-1, 0) + 2.0F - tmp.get_vy()(-1,1) + 2.0F - tmp.get_vy()(0, 0) + 2.0F - tmp.get_vy()(0, 1) ;
    const auto ret3 = test_diff(ret1,0.0F) && test_diff(ret2, 0.0F);
    return ret3 ? 0:1;
}

int gpm_2dvector_tie_off_origin_test(int argc, char* argv[]) {
    Slb::Exploration::Gpm::gpm_2dvector_tie_array tmp(Slb::Exploration::Gpm::int_extent_2d(2, 3));
    tmp.fill(Slb::Exploration::Gpm::int_extent_2d(Slb::Exploration::Gpm::int_extent_1d(1,2), Slb::Exploration::Gpm::int_extent_1d(2, 3)), 2.0F);
    const auto ret1 = (2.0F - tmp.get_vx()(1, 2)) ;
    auto sum_vx = std::accumulate(tmp.get_vx().begin(), tmp.get_vx().end(), 0.0F);
    const auto ret2 = 2.0F - tmp.get_vy()(1,2) ;
    auto sum_vy = std::accumulate(tmp.get_vy().begin(), tmp.get_vy().end(), 0.0F);
    auto ret3 = test_diff(ret1, 0.0F) && test_diff(ret2, 0.0F) && test_diff(sum_vx, 2.0F) && test_diff(sum_vy, 2.0F);
    return ret3 ? 0 : 1;
}
