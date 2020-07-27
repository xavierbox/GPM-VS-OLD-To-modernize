// -- Schlumberger Private --

#ifndef GPM_SEDIMENT_MODEL_H
#define GPM_SEDIMENT_MODEL_H

#include "gpm_parm_methods.h"
#include "gpm_function_2d.h"
#include "gpm_vbl_vector_array_3d.h"
#include "sedstc.h"
#include "mapstc.h"
#include <vector>

namespace Slb { namespace Exploration { namespace Gpm {

// The sediment model for a given timestep
class gpm_sediment_model {
public:
	using index_type =  function_2d::index_type;
    typedef std::vector<function_2d> vector_of_function2d_type;
	using func_2d = std::function<float(index_type, index_type)>; // Try first with scaling 
	using vector_of_erodability_type = std::vector<func_2d>;
    // First index sediment, second index layer 
	using sed_order_layer_order_erod_type = std::vector<vector_of_erodability_type>;
	using sed_order_layer_order_type = std::vector<vector_of_function2d_type>;
    vector_of_function2d_type top_functions;
	sed_order_layer_order_type sed_functions;
    // Initialize the vectors with this timestep and top area
    // Additional should typically be given as negative, since these are removed from the end
    gpm_sediment_model();
    gpm_sediment_model(const gpm_sediment_model& rhs) = delete;
    gpm_sediment_model& operator=(const gpm_sediment_model& rhs) = delete;
    ~gpm_sediment_model();

    int initialize(const parm& p, int top_index, const std::vector<int>& sed_indexes, int timestep, int num_tops);
    std::ptrdiff_t erode_model(const float_2darray_base_type& erosion);
    void assign_surfaces(std::vector<float_2darray_base_type*>* surfs_p);
    int initialize_erodability(const parm& p, const pars& s, const std::vector<double>& times, const std::vector<property_range_item>& sed_inprop_index);
    void set_erodability(const sed_order_layer_order_erod_type& times);
    void set_base_erodability(float erod);
    int size() const;
    const sed_order_layer_order_erod_type& erodability_funcs() const;
    gpm_sediment_model::index_type find_layer_index(index_type i, index_type j, float surf_val);

    bool is_initialized() const;
    // Assumes that they are ordered from bottom to top
    // index i will have lower z values than index i + 1
    static std::ptrdiff_t order_surfaces_vertically(const std::vector<float_2darray_base_type*>& surfs);

private:
    float_3darray_vec_type _ordered_z;
	sed_order_layer_order_erod_type _erodability; // For each current surface in the timestack
    float _base_erod;
};
}}}
#endif
