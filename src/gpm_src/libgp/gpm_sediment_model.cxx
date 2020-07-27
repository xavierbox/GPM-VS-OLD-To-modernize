// -- Schlumberger Private --

#include "gpm_vbl_vector_array_3d.h" // Need definition of 3d array first for the inlined header func to work properly
#include "gpm_sediment_model.h"
#include "gpm_parm_methods.h"
#include "libgp.h"
#include "mapstc.h"
#include "sedstc.h"
#include <algorithm>
#include <boost/core/null_deleter.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include "gpm_2darray_functions.h"

namespace Slb { namespace Exploration { namespace Gpm {

gpm_sediment_model::gpm_sediment_model(): _base_erod(0)
{
}

gpm_sediment_model::~gpm_sediment_model()
= default;

int
gpm_sediment_model::initialize(const parm& p, int top_index, const std::vector<int>& sed_indexes, int timestep,
                               int num_tops)
{
    top_functions.resize(num_tops);
    sed_functions.resize(sed_indexes.size());
    for (auto& sed_function : sed_functions) {
        sed_function.resize(num_tops);
    }
    const int iret = find_base_functions_for_timestep(p, top_index, sed_indexes, timestep, 0, num_tops - 1,
                                                      &top_functions, &sed_functions);
    if (iret == 0 ) {
        // If we only have one surface, we should be in good shape
        // Walk trough the arrays and clip
        // We need to assert that we have arrays underneath
        int_extent_2d ext2d = top_functions[0].extents();
        const int_extent_2d::extent_type z_exts(0, num_tops);
        const int_extent_2d::extent_type row_ext = ext2d.row_extent();
        const int_extent_2d::extent_type col_ext = ext2d.col_extent();
        _ordered_z.resize(z_exts, row_ext, col_ext);
        std::vector<float_2darray_base_type*> stack(num_tops);
        // do the copy
        for(auto l=0; l < num_tops;++l) {
            _ordered_z(l).fill(top_functions[l].const_array_holder());
            top_functions[l].set_array(
                std::shared_ptr<float_3darray_vec_base_type>(
                    &_ordered_z, boost::null_deleter()));
            top_functions[l].set_index(l);
            stack[l]=&_ordered_z(l);
        }
        order_surfaces_vertically(stack);
    }
    return iret;
}

std::ptrdiff_t gpm_sediment_model::erode_model(const float_2darray_base_type& erosion)
{
    const auto num_surfs = _ordered_z.layers();
	std::ptrdiff_t lower = num_surfs;
    if ( num_surfs > 0){
        auto& top( _ordered_z[num_surfs-1]);
        gpm_add(top, erosion, &top);
        std::vector<float_2darray_base_type*> stack(num_surfs);
        for(auto l=0; l < num_surfs;++l) {
            stack[l]=&_ordered_z(l);
        }
		lower = order_surfaces_vertically(stack);
    }
	return lower;
}

void gpm_sediment_model::assign_surfaces(std::vector<float_2darray_base_type*>* surfs_p)
{
    std::vector<float_2darray_base_type*>& surfs(*surfs_p);
    for (auto l = 0u; l < surfs.size(); ++l) {
        surfs[l]->fill(_ordered_z(l));
    }
}

std::ptrdiff_t gpm_sediment_model::order_surfaces_vertically(const std::vector<float_2darray_base_type*>& surfs)
{
    float_2darray_base_type& top(*surfs.back()); // the grid that has been eroded
    int_extent_2d::extent_type row_extent = top.row_extent();
    int_extent_2d::extent_type col_extent = top.col_extent();

    // MXG - Need to do some threading here.....
	std::ptrdiff_t lowest = surfs.size();
    for (auto i = row_extent.lower(); i < row_extent.upper(); i++) {
        for (auto j = col_extent.lower(); j < col_extent.upper(); j++) {
            float eroded_val = top(i, j);
            for (auto itop = boost::numeric_cast<std::ptrdiff_t>(surfs.size()) - 2; itop >= 0; --itop) {
                float_2darray_base_type& current_top(*surfs[itop]);
                if (current_top(i, j) > eroded_val) {
                    // We make thickness at leat the max diameter
                    current_top(i, j) = eroded_val;
					lowest = std::min(lowest, itop);
                }
                else if (current_top(i, j) < eroded_val) {
                    eroded_val = current_top(i, j);
                }
            }
        }
    }
	return lowest;
}

int gpm_sediment_model::initialize_erodability(const parm& p, const pars& s, const std::vector<double>& times, const std::vector<property_range_item>& sed_inprop_index)
{
    if (times.size() == top_functions.size()) {
        std::vector<double> diff;
        auto top = times.back();
        std::transform(times.begin(), times.end(), std::back_inserter(diff), [top](double val) { return top - val; });
        _erodability.resize(s.numseds);
        for (auto& iter:sed_inprop_index)
        {
            std::vector<function_2d> erosion_functions(p.numtops() - 1);
            auto iret = find_grid_functions_for_timestep(p, iter.indprop_index, p.numtims() - 1, 0, p.numtops() - 1, &erosion_functions);
            if (iret != 0)
            {
                return iret;
            }
            _erodability[iter.range_index].clear();
            const auto f1 = s.erode_funcs[iter.range_index];
			_erodability[iter.range_index].emplace_back(erosion_functions.front()); // Basement
            // Here is where we push back the erodability funcs when they are ready
			std::transform(diff.begin() + 1, diff.end(), erosion_functions.begin()+1, std::back_inserter(_erodability[iter.range_index]),
				[&f1](double diff, const function_2d& erodability)
			{
			    const auto age_coeff = f1->get(diff);
			    const auto erodability_func = erodability;
				return [=](index_type i, index_type j) {return age_coeff * erodability_func(i, j); };
			});
        }
    }
    return 0;
}

void gpm_sediment_model::set_erodability(const sed_order_layer_order_erod_type& times)
{
    std::transform(times.begin(), times.end(), std::inserter(_erodability, _erodability.begin()),
                   [](const vector_of_erodability_type& item) { return item; });
}

void gpm_sediment_model::set_base_erodability(float erod)
{
    _base_erod = erod;
}

int gpm_sediment_model::size() const
{
    return static_cast<int>(top_functions.size());
}

const gpm_sediment_model::sed_order_layer_order_erod_type&
gpm_sediment_model::erodability_funcs() const
{
    return _erodability;
}

// Surf_val has to be in layers
// Assume that layers are ordered
// Assume there are layers
// Find first surface >= surf_val
gpm_sediment_model::index_type gpm_sediment_model::find_layer_index(index_type i, index_type j, float surf_val)
{
    if (top_functions.empty()) {
        return -1; // Shouldn't happen
    }

    // Check if within sequence, go from bottom to top */
    const auto it = std::find_if_not(top_functions.begin(), top_functions.end(), [i, j, surf_val](const function_2d& func) {return func(i, j) < surf_val; });
    if (it == top_functions.end()) {
		return -2; // Wrong condition
    }
    const auto res = std::distance(top_functions.begin(), it);
    return res;
}

bool gpm_sediment_model::is_initialized() const
{
    return _ordered_z.has_data();
}
}}}
