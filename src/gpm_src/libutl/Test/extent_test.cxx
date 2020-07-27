// -- Schlumberger Private --

#include "gpm_vbl_array_2d.h"
#include <algorithm>

using namespace Slb::Exploration::Gpm;



int iterator_test(int argc, char* argv[]) {
    std::vector<double> tmp;
    tmp.push_back(0);
    tmp.push_back(0);
    tmp.push_back(29);
    tmp.push_back(0);
    tmp.push_back(0);

    auto it = std::find_if(tmp.begin(), tmp.end(), [](const double& pt) { return pt > 10.0; });
    if (it != tmp.end()) {
        auto dist = std::distance(tmp.begin(), it);
        if (dist > 1) // Have some zeros we need to get rid of
        {
            tmp.erase(tmp.begin(), it - 1);
        }
        auto it_rev = std::find_if(tmp.rbegin(), tmp.rend(), [](const double& pt) { return pt > 10.0; });
        auto rev_dist = std::distance(tmp.rbegin(), it_rev);
        if (rev_dist > 1) {
            tmp.erase((it_rev - 1).base(), tmp.end());
        }
    }
    return 0;
}

int iterator_1_test(int argc, char* argv[]) {
    std::vector<double> tmp;
    tmp.push_back(0);

    auto it = std::find_if(tmp.begin(), tmp.end(), [](const double& pt) { return pt > 10.0; });
    if (it != tmp.end()) {
        auto dist = std::distance(tmp.begin(), it);
        if (dist > 1) // Have some zeros we need to get rid of
        {
            tmp.erase(tmp.begin(), it - 1);
        }
        auto it_rev = std::find_if(tmp.rbegin(), tmp.rend(), [](const double& pt) { return pt > 10.0; });
        auto rev_dist = std::distance(tmp.rbegin(), it_rev);
        if (rev_dist > 1) {
            tmp.erase((it_rev - 1).base(), tmp.end());
        }
    }
    return 0;
}
