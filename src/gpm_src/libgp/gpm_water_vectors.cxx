// -- Schlumberger Private --

#include "gpm_water_vectors.h"
#include "gpm_basic_defs.h"

namespace Slb { namespace Exploration { namespace Gpm {

////
water_vel_vector::water_vel_vector() = default;

water_vel_vector::water_vel_vector(const int_extent_2d& ext): base_type(ext) {
}

water_vel_vector::~water_vel_vector() = default;

float_2darray_base_type& water_vel_vector::get_qx() {
    return vx;
}

float_2darray_base_type& water_vel_vector::get_qy() {
    return vy;
}

const float_2darray_base_type& water_vel_vector::get_qx() const {
    return vx;
}

const float_2darray_base_type& water_vel_vector::get_qy() const {
    return vy;
}

////
wave_dir_vector::wave_dir_vector() = default;

wave_dir_vector::wave_dir_vector(const int_extent_2d& ext): base_type(ext) {
}

wave_dir_vector::~wave_dir_vector() = default;

float_2darray_base_type& wave_dir_vector::get_qxa() {
    return vx;
}

float_2darray_base_type& wave_dir_vector::get_qya() {
    return vy;
}

const float_2darray_base_type& wave_dir_vector::get_qxa() const {
    return vx;
}

const float_2darray_base_type& wave_dir_vector::get_qya() const {
    return vy;
}

////
wave_vel_vector::wave_vel_vector() = default;

wave_vel_vector::wave_vel_vector(const int_extent_2d& ext): base_type(ext) {
}

wave_vel_vector::~wave_vel_vector() = default;

float_2darray_base_type& wave_vel_vector::get_qxw() {
    return vx;
}

float_2darray_base_type& wave_vel_vector::get_qyw() {
    return vy;
}

const float_2darray_base_type& wave_vel_vector::get_qxw() const {
    return vx;
}

const float_2darray_base_type& wave_vel_vector::get_qyw() const {
    return vy;
}

////
water_depth_vector::water_depth_vector() = default;

water_depth_vector::water_depth_vector(const int_extent_2d& ext): base_type(ext) {
}

water_depth_vector::~water_depth_vector() = default;

float_2darray_base_type& water_depth_vector::get_bxx() {
    return vx;
}

float_2darray_base_type& water_depth_vector::get_byy() {
    return vy;
}

const float_2darray_base_type& water_depth_vector::get_bxx() const {
    return vx;
}

const float_2darray_base_type& water_depth_vector::get_byy() const {
    return vy;
}

}}}
