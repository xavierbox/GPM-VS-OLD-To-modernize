// -- Schlumberger Private --

#ifndef _mapstc_h_
#define _mapstc_h_

/* Used by: maplib, ge, ssm */
/* Grid parameter structure */
#include "gpm_vbl_vector_array_3d.h"
#include "gpm_vbl_array_3d.h"
#include "gpm_function_2d.h"

//typedef boost::multi_array<int, 2> int_2darray_type;
#include "gpm_basic_defs.h"

#include <string>
#include <vector>

namespace Slb { namespace Exploration { namespace Gpm {

// This is to hold the property description
// May contain other things like discrete/continous etc
struct property_description {
    std::string m_prop_id_;
    std::string m_prop_display_name_;
    bool m_is_nodal_;

    property_description(const std::string& id, const std::string& display_name, bool nodal) : m_prop_id_(id),
                                                                                               m_prop_display_name_(
                                                                                                   display_name),
                                                                                               m_is_nodal_(nodal)
    {
    }

    bool operator==(const property_description& rhs) const
    {
        return m_prop_id_ == rhs.m_prop_id_ && m_is_nodal_ == rhs.m_is_nodal_;
    }
};

struct parm {
    using index_type = gpm_array_common_defs<3>::index_type; // Just like the other arrays
    /* GLOBAL */
    parm(): nodorder(0), znull(-1.0e12f), base_erodability(0)
    {
    }

    std::string title;
    long nodorder;
    float znull;
    std::vector<float> xcorners;
    std::vector<float> ycorners;
    long numgrds() const { return static_cast<int>(zgrid.layers()); }
    long numrows() const { return static_cast<int>(zgrid.rows()); }
    long numcols() const { return static_cast<int>(zgrid.cols()); }
    long numtims() const { return static_cast<int>(indprop.rows()); }
    long numtops() const { return static_cast<int>(indprop.cols()); }
    std::vector<std::string> history; // These are history items that we add as we 
    /* LOCAL */
    std::vector<std::string> gridnam;

    /* ARRAYS */
    float_3darray_vec_type zgrid;

    std::vector<double> timeset, ageset; /* Set of times and ages*/
    //int numprops;
    std::vector<std::string> propnam;
    gpm_vbl_array_2d<std::string> prop_display_name; //first row is propnam, second row is display name of property
    std::vector<int> isnodal;
    gpm_vbl_vector_array_3d<function_2d> indprop; /* Properties */
    gpm_vbl_vector_array_3d<float> indprop_stats;
    // Layer is index equal with propnam, row is index equal with timeset, col 0 is min, col 1 is max

    // Sediment mappers
    std::vector<std::string> base_layer_map;
    gpm_vbl_array_2d<std::string> sediment_prop_map; // The first row is the sediment ids, second row is sed1, sed 2 etc
    float base_erodability; //How erodaible is the base layer
    /* INTERNAL */

    std::string version;
    // global index map for zgrid when doing local increments of hdf5
    // The span is closed [gref_start_index, gref_end_index] 
    index_type gref_start_index{0};
    index_type gref_end_index{-1};

    parm(const parm& rhs) = delete;
    parm& operator=(const parm& rhs) = delete;
};

typedef struct parm grid_parameter_type;

}}}

#endif
