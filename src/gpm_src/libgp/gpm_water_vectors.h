// -- Schlumberger Private --

#ifndef GPM_WATER_VECTORS_H
#define GPM_WATER_VECTORS_H

#include "gpm_2dvector_tie_array.h"

namespace Slb { namespace Exploration { namespace Gpm {

// All the ones below are just stepping stones to move to an array of vectors
// They ease the syntactical transition
class water_vel_vector: public gpm_2dvector_tie_array {
public:
    typedef gpm_2dvector_tie_array base_type;
    water_vel_vector();
    explicit water_vel_vector(const int_extent_2d& ext);
    ~water_vel_vector();
    float_2darray_base_type& get_qx();
    float_2darray_base_type& get_qy();
    const float_2darray_base_type& get_qx() const;
    const float_2darray_base_type& get_qy() const;
private:
    water_vel_vector(const water_vel_vector& rhs) = delete;
    water_vel_vector& operator=(const water_vel_vector& rhs) = delete;
    //gpm_vbl_array_2d<float> qx; // STEADY: Water veloc on x-ties or UNSTEADY: x velocity in cell
    //gpm_vbl_array_2d<float> qy; // STEADY: Water veloc on y-ties or UNSTEADY: y velocity in cell

};

class wave_dir_vector: public gpm_2dvector_tie_array {
public:
    typedef gpm_2dvector_tie_array base_type;
    wave_dir_vector();
    explicit wave_dir_vector(const int_extent_2d& ext);
    ~wave_dir_vector();
    float_2darray_base_type& get_qxa();
    float_2darray_base_type& get_qya();
    const float_2darray_base_type& get_qxa() const;
    const float_2darray_base_type& get_qya() const;
private:
    wave_dir_vector(const wave_dir_vector& rhs) = delete;
    wave_dir_vector& operator=(const wave_dir_vector& rhs) = delete;
    //gpm_vbl_array_2d<float> qxa;// Water direction unit vectors (old: acceleration due to waves)
    //gpm_vbl_array_2d<float> qya; 

};

class wave_vel_vector: public gpm_2dvector_tie_array {
public:
    typedef gpm_2dvector_tie_array base_type;
    wave_vel_vector();
    explicit wave_vel_vector(const int_extent_2d& ext);
    ~wave_vel_vector();
    float_2darray_base_type& get_qxw();
    float_2darray_base_type& get_qyw();
    const float_2darray_base_type& get_qxw() const;
    const float_2darray_base_type& get_qyw() const;
private:
    wave_vel_vector(const wave_vel_vector& rhs) = delete;
    wave_vel_vector& operator=(const wave_vel_vector& rhs) = delete;
    //gpm_vbl_array_2d<float> qxw;// Wave water effective veloc
    //gpm_vbl_array_2d<float> qyw; 

};

class water_depth_vector: public gpm_2dvector_tie_array {
public:
    typedef gpm_2dvector_tie_array base_type;
    water_depth_vector();
    explicit water_depth_vector(const int_extent_2d& ext);
    ~water_depth_vector();
    float_2darray_base_type& get_bxx();
    float_2darray_base_type& get_byy();
    const float_2darray_base_type& get_bxx() const;
    const float_2darray_base_type& get_byy() const;
private:
    water_depth_vector(const water_depth_vector& rhs) = delete;
    water_depth_vector& operator=(const water_depth_vector& rhs) = delete;
    //gpm_vbl_array_2d<float> bxx;// Water depth bet. nodes
    //gpm_vbl_array_2d<float> byy; 

};
}}}
#endif
