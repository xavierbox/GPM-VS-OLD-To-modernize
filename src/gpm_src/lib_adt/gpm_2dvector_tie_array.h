// -- Schlumberger Private --

#ifndef GPM_2DVECTOR_TIE_ARRAY_H
#define GPM_2DVECTOR_TIE_ARRAY_H
#include "gpm_basic_defs.h"
#include "gpm_vbl_array_2d.h"
#include <functional>

namespace Slb { namespace Exploration { namespace Gpm {

class gpm_2dvector_tie_array {
public:
    typedef std::function<float (float)> filter_func_type;
    typedef std::function<float (float, float)> calc_func_type;
    gpm_2dvector_tie_array();
    explicit gpm_2dvector_tie_array(const int_extent_2d& ext);
    virtual ~gpm_2dvector_tie_array();
    void initialize(int nr, int nc);
    void initialize(const int_extent_2d& ext);
    void fill(float val);
    // Assumes that we have a subset of the core_extent
    // Will fill negative index if on border
    void fill(const int_extent_2d& ext, float val, bool border=true);
    void fill(const gpm_2dvector_tie_array& val);
    bool empty() const;
    // Simple accessors for now
    void set_value(int i, int j, float val_x, float val_y);
    void get_value(int i, int j, float* val_x, float* val_y) const;

    int_extent_2d core_extents() const;

    const float_2darray_base_type& get_vx() const;
    const float_2darray_base_type& get_vy() const;

    void add(const gpm_2dvector_tie_array& rhs);

    void compute_forward_average_both_dirs(float_2darray_base_type* vlx_p, float_2darray_base_type* vly_p) const;
    //Assume that we want to go from midpoint/tie to points
    //Means that this vector typically has 1 more point in one direction
    // And the arrays given as input are grid point values
    void compute_forward_average(float_2darray_base_type* vlx_p, float_2darray_base_type* vly_p) const;
    static void compute_forward_average_both_dirs(const float_2darray_base_type& vx, const float_2darray_base_type& vy,
                                                  float_2darray_base_type* vlx_p, float_2darray_base_type* vly_p);
    static void compute_forward_average(const float_2darray_base_type& vx, const float_2darray_base_type& vy,
                                        float_2darray_base_type* vlx_row_dir_p, float_2darray_base_type* vly_col_dir_p);
    static void compute_neigborhood(const float_2darray_base_type& vx, const float_2darray_base_type& vy,
                                    const calc_func_type& vlx_row_dir_func, const calc_func_type& vlx_col_dir_func,
                                    float_2darray_base_type* vlx_row_dir_p, float_2darray_base_type* vly_col_dir_p);
protected:
    float_2darray_type vx;
    float_2darray_type vy;
private :
    gpm_2dvector_tie_array(const gpm_2dvector_tie_array& rhs);
    gpm_2dvector_tie_array& operator=(const gpm_2dvector_tie_array& rhs);
};

}}}
#endif
