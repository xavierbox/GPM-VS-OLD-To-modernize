// -- Schlumberger Private --

#include "gpm_parm_methods.h"
#include "mapstc.h"
#include <boost/numeric/conversion/cast.hpp>
#include <boost/algorithm/string.hpp>
#include <algorithm>

namespace Slb { namespace Exploration { namespace Gpm {
namespace {
std::vector<int> find_indexes_starting_with(const std::string& name, const std::vector<std::string>& vec) {
    std::vector<int> res;
    for (auto i = 0u; i < vec.size(); ++i) {
        if (boost::istarts_with(vec[i], name)) {
            res.push_back(i);
        }
    }
    return res;
}

} // Anonym namespace

bool has_property(const parm& p, const std::string& name) {
    return std::find(p.propnam.begin(), p.propnam.end(), name) != p.propnam.end();
}

std::map<std::string, int> get_property_index_map(const parm& p) {
    std::map<std::string, int> res;
    int i = 0;
    for (auto it = p.propnam.begin(); it != p.propnam.end(); ++it , ++i) {
        res.insert(std::make_pair(*it, i));
    }
    return res;
}

// Returns -1 if not found
int get_property_index(const parm& p, const std::string& name) {
    const auto it = std::find(p.propnam.begin(), p.propnam.end(), name);
    const int res = (it != p.propnam.end()) ? boost::numeric_cast<int>(it - p.propnam.begin()) : -1;
    return res;
}

std::vector<int> find_properties_with_name_prefix(const parm& p, const std::string& name) {
    return find_indexes_starting_with(name, p.propnam);
}


int find_or_add_property(parm* p,
                         const std::string& name, const std::string& display_name, int is_nodal, int* indprop, bool* isnew) {
    // Finds name in list of properties and returns position in indprop
    // If not found, and iappend is 0, sets indprop to -1,
    //               and iappend is 1, appends name, increments p->numprop, and returns position
    // p = data structure
    // Note: ityp, ndim, idim can be all NULL if iappend is 0
    // ityp = entire set of types for p
    // ndim = entire set of dimension numbers for p
    // idim = entire set of dimensions for p
    // name = name of property (in capitals, e.g.: "SVX")
    // iappend = append flag
    //         0: do not append name if not found
    //         1: append name if not found
    // indprop = returned index
    // isnew = New name return flag
    //         0: name was not appended
    //         1: name was appended

    *indprop = -1;
    *isnew = false;

    for (auto iprop = 0u; iprop < p->propnam.size(); iprop++) {
        if (p->propnam[iprop] == name) {
            *indprop = iprop;
            break;
        }
    }
    if (*indprop < 0) { // if not found
        //    for(i=0;i<GPM::PROPNAM-1;i++) icum += ndim[i]; // Find location of PROPNAM dimensions

        *isnew = true;
        *indprop = static_cast<int>(p->propnam.size());
        //idimnew[1] = arr_size[PROPNAM][1]; // Number of characters in PROPNAM
        p->propnam.push_back(name);

        // Update ISNODAL
        p->isnodal.resize(p->propnam.size());
        p->isnodal.back() = is_nodal;

        // Update PROP_DISPLAY_NAME
        p->prop_display_name.resize(2, p->propnam.size());
        p->prop_display_name(0,p->propnam.size()-1)= name;
        p->prop_display_name(1,p->propnam.size()-1)= display_name;
        // Add the property to indprop as well
        p->indprop.resize(p->propnam.size(), p->indprop.rows(), p->indprop.cols());
    }

    return 0;
}

}}}
