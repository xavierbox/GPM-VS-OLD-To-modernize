#include "gpm_function_2d.h"
#include <numeric>

int function_2d_test(int argc, char* argv[]) {
    // 
    std::vector<Slb::Exploration::Gpm::function_2d> test(5);
    for (int i = 0; i < 5; ++i) {
        test[i].set_constant(static_cast<float>(i*i));
    }
    // Allocate outside loop
    std::vector<float> vals(test.size());
    std::vector<float> diff(test.size());
    int i = 0, j = 0;
    // In loop
    std::transform(test.begin(), test.end(), vals.begin(), [i, j](const Slb::Exploration::Gpm::function_2d& a) {return a(i, j); });
    vals[4] = 0.0F;
    std::adjacent_difference(vals.begin(), vals.end(), diff.begin());
    // Lets add the index as well
    float min_thickness = 0.1F;
    auto it = std::find_if(diff.rbegin(), diff.rend(), [&min_thickness](float a) { return a > min_thickness; });
    if (it != diff.rend()) {
        auto fwd = --(it.base());
        auto stack_index = std::distance(diff.begin(), fwd);
    }

    // Can do iota as well
    std::vector<std::pair<float, int>> val_indexes;
    int count = 0;
    for (float diff1 : diff) {
        val_indexes.emplace_back(diff1, count);
        //       val_indexes.push_back(std::pair<float, int>(diff1, count));
        ++count;
    }
    std::vector<std::pair<float, int>> filtered_indexes;
    std::copy_if(val_indexes.begin() + 1, val_indexes.end(), std::back_inserter(filtered_indexes), [&min_thickness](const std::pair<float, int>& a) { return a.first > min_thickness; });
    std::reverse(filtered_indexes.begin(), filtered_indexes.end());
    for (const auto& item : filtered_indexes)
    {
        auto stack_index_2 = item.second;
    }
    for (auto it1 = filtered_indexes.begin(); it1 != filtered_indexes.end(); ++it1)
    {
        auto stack_index_2 = (*it1).second;
    }

    return 0;
}

