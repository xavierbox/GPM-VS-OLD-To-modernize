// -- Schlumberger Private --

#include "gpm_2dvector_tie_array.h"
#include "gpm_extent_tie_helper.h"
#include "gpm_basic_defs.h"
#include "gpm_2darray_functions.h"

namespace Slb { namespace Exploration { namespace Gpm {

gpm_2dvector_tie_array::gpm_2dvector_tie_array()
= default;

gpm_2dvector_tie_array::gpm_2dvector_tie_array(const int_extent_2d& ext)
{
    initialize(ext);
}

gpm_2dvector_tie_array::~gpm_2dvector_tie_array()
= default;


void gpm_2dvector_tie_array::initialize(int nr, int nc)
{
    initialize(int_extent_2d(nr, nc));
}

void gpm_2dvector_tie_array::initialize(const int_extent_2d& ext)
{
    const gpm_extent_tie_helper extents(ext);
    vx = float_2darray_type(extents.full_extent_with_col_boundary_minus_1); // Tie Diffusion in x
    vx.set_core_size(extents.full_extent);
    vy = float_2darray_type(extents.full_extent_with_row_boundary_minus_1); // Tie Diffusion in y
    vy.set_core_size(extents.full_extent);
}

void gpm_2dvector_tie_array::fill(float val)
{
    vx.fill(val);
    vy.fill(val);
}

void gpm_2dvector_tie_array::fill(const int_extent_2d& ext, float val, bool border)
{
    assert(vx.core_extents().contains(ext));
    assert(vy.core_extents().contains(ext));
    if (border && vx.core_extents().col_extent().lower() == ext.col_extent().lower()) {
        // Need to add lower part of col
        const auto col_ext = vx.extents().col_extent().right_subtract(ext.col_extent()) | ext.col_extent();
        vx.fill(int_extent_2d(ext.row_extent(), col_ext), val);
    }
    else {
        vx.fill(ext, val);
    }
    if (border && vy.core_extents().row_extent().lower() == ext.row_extent().lower()) {
        // Need to add lower part of col
        const auto row_ext = vy.extents().row_extent().right_subtract(ext.row_extent()) | ext.row_extent();
        vy.fill(int_extent_2d(row_ext, ext.col_extent()), val);
    }
    else {
        vy.fill(ext, val);
    }
}

void gpm_2dvector_tie_array::fill(const gpm_2dvector_tie_array& val)
{
    assert(val.vx.extents() == vx.extents());
    vx.fill(val.vx);
    vy.fill(val.vy);
}


void gpm_2dvector_tie_array::compute_forward_average_both_dirs(float_2darray_base_type* vlx_p,
                                                               float_2darray_base_type* vly_p) const
{
    compute_forward_average_both_dirs(vx, vy, vlx_p, vly_p);
}

void gpm_2dvector_tie_array::compute_forward_average_both_dirs(const float_2darray_base_type& vx,
                                                               const float_2darray_base_type& vy,
                                                               float_2darray_base_type* vlx_p,
                                                               float_2darray_base_type* vly_p)
{
    float_2darray_base_type& vlx(*vlx_p);
    float_2darray_base_type& vly(*vly_p);
    assert(vlx.extents().contains(vly.extents()));
    assert(vx.extents().contains(vlx.extents()));
    assert(vy.extents().contains(vlx.extents()));
    int_extent_2d::extent_type row_extent = vlx.row_extent();
    int_extent_2d::extent_type col_extent = vlx.col_extent();
    for (auto i = row_extent.lower(); i < row_extent.upper() - 1; i++) {
        for (auto j = col_extent.lower(); j < col_extent.upper() - 1; j++) {
            vlx(i, j) = (vx(i, j) + vx(i + 1, j) + vx(i, j + 1) + vx(i + 1, j + 1)) * 0.25f;
            vly(i, j) = (vy(i, j) + vy(i + 1, j) + vy(i, j + 1) + vy(i + 1, j + 1)) * 0.25f;
        }
    }
}

void gpm_2dvector_tie_array::compute_forward_average(float_2darray_base_type* vlx_p,
                                                     float_2darray_base_type* vly_p) const
{
    compute_forward_average(vx, vy, vlx_p, vly_p);
}

void gpm_2dvector_tie_array::compute_forward_average(const float_2darray_base_type& vx,
                                                     const float_2darray_base_type& vy,
                                                     float_2darray_base_type* vlx_row_dir_p,
                                                     float_2darray_base_type* vly_col_dir_p)
{
    float_2darray_base_type& vlx(*vlx_row_dir_p);
    float_2darray_base_type& vly(*vly_col_dir_p);
    assert(vlx.extents().contains(vly.extents()));
    assert(vx.extents().contains(vlx.extents()));
    assert(vy.extents().contains(vlx.extents()));
    assert(vlx.extents() == vly.extents());
    int_extent_2d::extent_type row_extent = vlx.row_extent();
    int_extent_2d::extent_type col_extent = vlx.col_extent();
    const auto vx_row_off = vx.row_extent().lower() - vlx.row_extent().lower();
    const auto vx_col_off = vx.col_extent().lower() - vlx.col_extent().lower();
    const auto vy_row_off = vy.row_extent().lower() - vly.row_extent().lower();
    const auto vy_col_off = vy.col_extent().lower() - vly.col_extent().lower();
    for (auto i = row_extent.lower(), nr = row_extent.upper(); i < nr; i++) {
        for (auto j = col_extent.lower(), nc = col_extent.upper(); j < nc; j++) {
            vlx(i, j) = (vx(i, j) + vx(i + vx_row_off, j + vx_col_off)) * 0.5F;
            vly(i, j) = (vy(i, j) + vy(i + vy_row_off, j + vy_col_off)) * 0.5F;
        }
    }
}

void gpm_2dvector_tie_array::compute_neigborhood(const float_2darray_base_type& vx, const float_2darray_base_type& vy,
                                                 const calc_func_type& vlx_row_dir_calc,
                                                 const calc_func_type& vlx_col_dir_calc,
                                                 float_2darray_base_type* vlx_row_dir_p,
                                                 float_2darray_base_type* vly_col_dir_p)
{
    float_2darray_base_type& vlx(*vlx_row_dir_p);
    float_2darray_base_type& vly(*vly_col_dir_p);
    assert(vlx.extents().contains(vly.extents()));
    assert(vx.extents().contains(vlx.extents()));
    assert(vy.extents().contains(vlx.extents()));
    int_extent_2d::extent_type row_extent = vlx.row_extent();
    int_extent_2d::extent_type col_extent = vlx.col_extent();
    for (auto i = row_extent.lower(); i < row_extent.upper() - 1; i++) {
        for (auto j = col_extent.lower(); j < col_extent.upper() - 1; j++) {
            vlx(i, j) = vlx_row_dir_calc(vx(i, j), vx(i + 1, j));
            vly(i, j) = vlx_col_dir_calc(vy(i, j), vy(i, j + 1));
        }
        // Initialize unused row and col (may not be needed)
        vlx(i, col_extent.upper() - 1) = 0.;
        vly(i, col_extent.upper() - 1) = 0.;
    }
    for (auto j = col_extent.lower(); j < col_extent.upper() - 1; j++) {
        vlx(row_extent.upper() - 1, j) = 0.;
        vly(row_extent.upper() - 1, j) = 0.;
    }
}

bool gpm_2dvector_tie_array::empty() const
{
    // They should be in pairs
    return vx.empty();
}

void gpm_2dvector_tie_array::set_value(int i, int j, float val_x, float val_y)
{
    vx(i, j) = val_x;
    vy(i, j) = val_y;
}

void gpm_2dvector_tie_array::get_value(int i, int j, float* val_x, float* val_y) const
{
    *val_x = vx(i, j);
    *val_y = vy(i, j);
}

int_extent_2d gpm_2dvector_tie_array::core_extents() const
{
    assert(vx.core_extents() == vy.core_extents());
    return vx.core_extents();
}

const float_2darray_base_type& gpm_2dvector_tie_array::get_vx() const
{
    return vx;
}

const float_2darray_base_type& gpm_2dvector_tie_array::get_vy() const
{
    return vy;
}

void gpm_2dvector_tie_array::add(const gpm_2dvector_tie_array& rhs)
{
    gpm_add(vx, rhs.get_vx(), &vx);
    gpm_add(vy, rhs.get_vy(), &vy);
}

}}}
