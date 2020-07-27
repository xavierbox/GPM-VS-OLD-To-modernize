// -- Schlumberger Private --

#include "gpm_diffusion_vector.h"
#include <algorithm>
#define NOMINMAX
#if defined(USE_TBB)
#include "tbb/tbb.h"
using namespace tbb;
#endif


namespace Slb { namespace Exploration { namespace Gpm {

// These should really be sets that keep the stuff and make them easier to maintain
// Future improvement. Allocate the following dynamically.


diffusion_vector::diffusion_vector() = default;

diffusion_vector::~diffusion_vector() = default;

float_2darray_base_type& diffusion_vector::get_cdx() {
    return vx;
}

float_2darray_base_type& diffusion_vector::get_cdy() {
    return vy;
}

const float_2darray_base_type& diffusion_vector::get_cdx() const {
    return vx;
}

const float_2darray_base_type& diffusion_vector::get_cdy() const {
    return vy;
}

float diffusion_vector::find_max_velocity(const int_extent_2d& extents)
{
    float max_sum_diff = 0;

    // MXG imp step 3
    for (auto i = extents.row_extent().lower(), nr = extents.row_extent().upper(); i < nr; i++) {
        for (auto j = extents.col_extent().lower(), nc = extents.col_extent().upper(); j < nc; j++) {
            float sum = vx(i, j) + vy(i, j);
            max_sum_diff = std::max(max_sum_diff, sum);
        }
    }
    return max_sum_diff;
}

float diffusion_vector::calculate_tie_harmonic_average_diffusion_serial(const float_2darray_base_type& coeff, const float_2darray_base_type& za, float dx2, float dy2, float transportability)
{
    const auto dx2_reciproc = 1.0F / dx2;
    const auto dy2_reciproc = 1.0F / dy2;
    // Make extent 1 less on the extent we add to
    const auto& extents = za.extents();
    const int_extent_1d& row_extent(za.row_extent());
    const int_extent_1d& col_extent(za.col_extent());
    const int_extent_1d mod_row_extent(row_extent.lower(), row_extent.upper() - 1);
    const int_extent_1d mod_col_extent(col_extent.lower(), col_extent.upper() - 1);
    calculate_tie_harmonic_diffusion_coeffs(coeff, row_extent, mod_col_extent, 0, 1, dx2_reciproc, vx);
    calculate_tie_harmonic_diffusion_coeffs(coeff, mod_row_extent,col_extent , 1, 0, dy2_reciproc, vy);
    add_height_difference(row_extent, mod_col_extent, 0, 1, za, vx);
    add_height_difference(mod_row_extent, col_extent, 1, 0, za, vy);
    const float max_sum_diff = find_max_velocity(extents);
    const auto transp = transportability > 0.0F ? transportability : 1.0F;
    const auto max_time_step = 1.0F / (2.0F * max_sum_diff * transp);
    return max_time_step;
}

float diffusion_vector::calculate_tie_harmonic_average_diffusion_TBB(const float_2darray_base_type& coeff, const float_2darray_base_type& za, float dx2, float dy2, float transportability)
{
    const auto dx2_reciproc = 1.0F / dx2;
    const auto dy2_reciproc = 1.0F / dy2;
    // Make extent 1 less on the extent we add to
    const auto& extents = za.extents();
    const int_extent_1d& row_extent(za.row_extent());
    const int_extent_1d& col_extent(za.col_extent());
    const int_extent_1d mod_row_extent(row_extent.lower(), row_extent.upper() - 1);
    const int_extent_1d mod_col_extent(col_extent.lower(), col_extent.upper() - 1);
#if defined(USE_TBB)
    tbb::parallel_invoke(
        [&]() {
        parallel_for(blocked_range<int>(row_extent.lower(), row_extent.upper()), [&](const blocked_range<int>& row) {
            calculate_tie_harmonic_diffusion_coeffs(coeff, int_extent_1d(row.begin(), row.end()), mod_col_extent, 0, 1, dx2_reciproc, vx);
        });},
        [&]() {parallel_for(blocked_range<int>(mod_row_extent.lower(), mod_row_extent.upper()), [&](const blocked_range<int>& row) {
            calculate_tie_harmonic_diffusion_coeffs(coeff, int_extent_1d(row.begin(), row.end()), col_extent, 1, 0, dy2_reciproc, vy);
        });});
    tbb::parallel_invoke(
        [&]() {    parallel_for(blocked_range<int>(row_extent.lower(), row_extent.upper()), [&](const blocked_range<int>& row) {
        add_height_difference(int_extent_1d(row.begin(), row.end()), mod_col_extent, 0, 1, za, vx);
    });},
        [&]() { parallel_for(blocked_range<int>(mod_row_extent.lower(), mod_row_extent.upper()), [&](const blocked_range<int>& row) {
        add_height_difference(int_extent_1d(row.begin(), row.end()), col_extent, 1, 0, za, vy);
    });});
#endif
    const float max_sum_diff = find_max_velocity(extents);
    const auto transp = transportability > 0.0F ? transportability : 1.0F;
    const auto max_time_step = 1.0F / (2.0F * max_sum_diff * transp);
    return max_time_step;
}
float diffusion_vector::calculate_tie_harmonic_average_diffusion(const float_2darray_base_type& coeff, const float_2darray_base_type& za, float dx2, float dy2, float transportability)
{
#if defined(USE_TBB)
    return calculate_tie_harmonic_average_diffusion_TBB(coeff, za, dx2, dy2, transportability);
#else
    return calculate_tie_harmonic_average_diffusion_serial(coeff, za, dx2, dy2, transportability);
#endif
}
float diffusion_vector::calculate_tie_average_diffusion_TBB(const float_2darray_base_type& coeff, const float_2darray_base_type& za, float factor, float dx2, float dy2, float transportability)
{
    const auto& extents= za.extents();
    const auto dx2_reciproc = 1.0F / dx2;
    const auto dy2_reciproc = 1.0F / dy2;
    const int_extent_1d mod_row_extent(extents.row_extent().lower(), extents.row_extent().upper() - 1);
    const int_extent_1d mod_col_extent(extents.col_extent().lower(), extents.col_extent().upper() - 1);
    const int_extent_1d& row_extent(extents.row_extent());
    const int_extent_1d& col_extent(extents.col_extent());

#if defined(USE_TBB)
    tbb::parallel_invoke(
        [&]()
        {
            parallel_for(blocked_range<int>(row_extent.lower(), row_extent.upper()), [&](const blocked_range<int>& row)
            {
                calculate_tie_diffusion_coeffs(coeff, int_extent_1d(row.begin(), row.end()), mod_col_extent, 0, 1,
                                               factor, dx2_reciproc, vx);
            });
        },
        [&]()
        {
            parallel_for(blocked_range<int>(mod_row_extent.lower(), mod_row_extent.upper()),
                         [&](const blocked_range<int>& row)
                         {
                             calculate_tie_diffusion_coeffs(coeff, int_extent_1d(row.begin(), row.end()), col_extent, 1,
                                                            0, factor, dy2_reciproc, vy);
                         }
            );
        });
    // MXG imp step 3
    tbb::parallel_invoke(
        [&]()
        {
            parallel_for(blocked_range<int>(row_extent.lower(), row_extent.upper()), [&](const blocked_range<int>& row)
            {
                add_height_difference(int_extent_1d(row.begin(), row.end()), mod_col_extent, 0, 1, za, vx);
            });
        },
        [&]()
        {
            parallel_for(blocked_range<int>(mod_row_extent.lower(), mod_row_extent.upper()),
                         [&](const blocked_range<int>& row)
                         {
                             add_height_difference(int_extent_1d(row.begin(), row.end()), col_extent, 1, 0, za, vy);
                         });
        }
    );
#endif
    const float max_sum_diff = find_max_velocity(extents);
    const auto transp = transportability > 0.0F ? transportability : 1.0F;
    const auto max_time_step = 1.0F / (2.0F * max_sum_diff * transp);
    return max_time_step;
}

float diffusion_vector::calculate_tie_average_diffusion_serial(const float_2darray_base_type& coeff, const float_2darray_base_type& za, float factor, float dx2, float dy2, float transportability)
{
    const auto extents = za.extents();
    const auto dx2_reciproc = 1.0F / dx2;
    const auto dy2_reciproc = 1.0F / dy2;
    const int_extent_1d mod_row_extent(extents.row_extent().lower(), extents.row_extent().upper() - 1);
    const int_extent_1d mod_col_extent(extents.col_extent().lower(), extents.col_extent().upper() - 1);
    const int_extent_1d& row_extent(extents.row_extent());
    const int_extent_1d& col_extent(extents.col_extent());
    calculate_tie_diffusion_coeffs(coeff, row_extent, mod_col_extent, 0, 1, factor, dx2_reciproc, vx);
    calculate_tie_diffusion_coeffs(coeff, mod_row_extent, col_extent, 1, 0, factor, dy2_reciproc, vy);
    add_height_difference(row_extent, mod_col_extent, 0, 1, za, vx);
    add_height_difference(mod_row_extent, col_extent, 1, 0, za, vy);
    const float max_sum_diff = find_max_velocity(extents);
    const auto transp = transportability > 0.0F ? transportability : 1.0F;
    const auto max_time_step = 1.0F / (2.0F * max_sum_diff * transp);
    return max_time_step;
}
float diffusion_vector::calculate_tie_average_diffusion(const float_2darray_base_type& coeff, const float_2darray_base_type& za, float factor, float dx2, float dy2, float transportability)
{
#if defined(USE_TBB)
    return calculate_tie_average_diffusion_TBB(coeff, za, factor, dx2, dy2, transportability);
#else
    return calculate_tie_average_diffusion_serial(coeff, za, factor, dx2, dy2, transportability);
#endif
}
void diffusion_vector::calculate_tie_harmonic_diffusion_coeffs(const float_2darray_base_type& coeff, const int_extent_1d& row_extent, const int_extent_1d& col_extent, int row_offset,
                                                               int col_offset,
                                                               float d2_reciproc, float_2darray_base_type& cdx)
{
    // Find tie diffusion coefficients (cdx and cdy)
    // by harmonic averaging
    for (auto i = row_extent.lower(), nr = row_extent.upper(); i < nr; ++i) {
        for (auto j = col_extent.lower(), nc = col_extent.upper(); j < nc; ++j) {
            const auto cdn_ij = coeff(i, j);
            const auto cdn_ijp1 = coeff(i + row_offset, j + col_offset);
            const auto loc_cdx = (cdn_ij * cdn_ijp1) / (cdn_ij + cdn_ijp1) * 2.F;
            cdx(i, j) = loc_cdx * d2_reciproc;
        }
    }
}

void diffusion_vector::calculate_tie_diffusion_coeffs(const float_2darray_base_type& coeff, const int_extent_1d& row_extent, const int_extent_1d& col_extent, int row_offset,
                                                      int col_offset, float factor,
                                                      float d2_reciproc, float_2darray_base_type& cdx)
{
    for (auto i = row_extent.lower(), nr = row_extent.upper(); i < nr; ++i) {
        for (auto j = col_extent.lower(), nc = col_extent.upper(); j < nc; ++j) {
            const auto diss_avg = (coeff(i, j) + coeff(i + row_offset, j + col_offset)) * 0.5F;
            const auto delta = diss_avg * factor * d2_reciproc;
            cdx(i, j) = delta;
        }
        }
}

void diffusion_vector::add_height_difference(const int_extent_1d& row_extent, const int_extent_1d& col_extent, int row_offset,
                                             int col_offset,
                                             const float_2darray_base_type& za, float_2darray_base_type& cdx)
{
    // Find tie diffusion coefficients (cdx and cdy)
    // by harmonic averaging
    for (auto i = row_extent.lower(), nr = row_extent.upper(); i < nr; ++i) {
        for (auto j = col_extent.lower(), nc = col_extent.upper(); j < nc; ++j) {
            const auto dz = (za(i, j) - za(i + row_offset, j + col_offset));
            cdx(i, j) *= dz;
        }
    }
}
}}}
