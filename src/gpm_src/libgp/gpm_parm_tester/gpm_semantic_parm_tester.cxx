// -- Schlumberger Private --

#include "gpm_semantic_parm_tester.h"
#include "mapstc.h"
#include "mapstc_descr.h"

#include "gpm_hdf5_reader.h"
#include "gpm_hdf5_writer.h"
#include "grid_param_mapper.h"
#include "gpm_parm_methods.h"
#include "gpm_version_number.h"

#include "gpm_2darray_functions.h"
#include "gpm_vbl_array_2d.h"

#include <boost/algorithm/string.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/math/special_functions/relative_difference.hpp>

#include <string>
#include <vector>
#include <functional>
#include <numeric>
#include <iostream>
#include "gpm_format_helper.h"


using namespace Slb::Exploration::Gpm;
namespace  bmi = boost::multi_index;

namespace {

using index_list_type = boost::multi_index_container<
    std::string,
    bmi::indexed_by<
        bmi::sequenced<bmi::tag<struct ordered_by_insertion> >,
        bmi::ordered_non_unique<
            bmi::tag<struct ordered_by_id>,
            bmi::identity<std::string>
            >
        >
    >;

float rel_vs_abs_limit(5e-3f);

// return value = 2 = rel_diff
// return value = 3 => abs_diff
template <typename T>
int check_diff(T v1, T v2, T rel_diff, T abs_diff, T* res_diff)
{
    int diff_found = 0;
    *res_diff = 0;
    T diff = boost::math::relative_difference(v1, v2);
    if (rel_diff < diff) {
		T a_diff = std::abs(v1 - v2);
		if (abs_diff < a_diff) {
			*res_diff = diff;
			diff_found = 2;
		}
    }
    return diff_found;
}

template <typename T>
int compare_scalar(const std::string& name, const T& expected, const T& actual, std::function<int(T, T, T*)> differ,
                   std::function<void(const std::string&, T, T, T, int)> printer)
{
    T diff; // A bit on the wrong side JT
    const int is_diff = differ(expected, actual, &diff);
    if (is_diff != 0) {
        printer(name, expected, actual, diff, is_diff);
        return 1;
    }
    return 0;
}

void print_diff_with_strings(const std::string& name, const std::string& expected, const std::string& actual)
{
    printf("%s : expected value %s actual value %s.\n", name.c_str(), expected.c_str(), actual.c_str());
}

int compare_exact_scalar(const std::string& name, const std::string& expected, const std::string& actual)
{
    if (expected != actual) {
        print_diff_with_strings(name, expected, actual);
        return 1;
    }

    return 0;
}

template <typename T>
void print_diff(const T& actual, const T& expected, const std::string& name, int& numfound)
{
    const std::string exp_1 = std::to_string(expected);
    const std::string act_1 = std::to_string(actual);
    print_diff_with_strings(name, exp_1, act_1);
    ++numfound;
}

int verify_string_array_with_col_order(const std::string& name,
	const gpm_vbl_array_2d<std::string>& actual_results,
	const gpm_vbl_array_2d<std::string>& expected_results,
	const std::vector<std::pair<int, int>>& order)
{
	int numfound = 0;
	if (expected_results.size() == actual_results.size() && order.size() == actual_results.col_extent().size()) {
		if (actual_results.extents() == expected_results.extents()) {
			for (auto j = order.begin(); j != order.end(); ++j) {
				if (actual_results(0, j->first) != expected_results(0, j->second)) {
					const auto expected = actual_results(0, j->first);
					const auto actual = expected_results(0, j->second);

					numfound++;
				}
				else {
					const auto expected = actual_results(1, j->first);
					const auto actual = expected_results(1, j->second);
                    if (expected != actual) {
                        numfound++;

                    }
                }
            }
            printf("%s : Number of differences found: %d\n", name.c_str(), numfound);
        }
        else {
            printf("%s : Extents differ: expected: (((%td,%td),(%td,%td)), test: (((%td,%td),(%td,%td))\n",
                   name.c_str(),
                   actual_results.row_extent().lower(), actual_results.row_extent().upper(),
                   actual_results.col_extent().lower(), actual_results.col_extent().upper(),
                   expected_results.row_extent().lower(), expected_results.row_extent().upper(),
                   expected_results.col_extent().lower(), expected_results.col_extent().upper());
            ++numfound;
        }
    }
    else {
        const size_t actual_size = expected_results.size();
        const size_t expected_size = actual_results.size();
        print_diff(actual_size, expected_size, name, numfound);
    }

    return numfound;
}


int verify_string_array_2d(const std::string& name, 
                           const gpm_vbl_array_2d<std::string>& expected_results,
                           const gpm_vbl_array_2d<std::string>& test_results)
{
    int numfound = 0;
    if (test_results.size() == expected_results.size()) {
        if (expected_results.extents() == test_results.extents()) {
            for (auto i = test_results.row_extent().lower(); i < test_results.row_extent().upper(); i++) {
                for (auto j = test_results.col_extent().lower(); j < test_results.col_extent().upper(); j++) {
                    const auto expected = expected_results(i, j);
                    const auto actual = test_results(i, j);
                    if (expected != actual) {
                        numfound++;

                    }
                }
            }
            printf("%s : Number of differences found: %d\n", name.c_str(), numfound);
        }
        else {
            printf("%s : Extents differ: expected: (((%td,%td),(%td,%td)), test: (((%td,%td),(%td,%td))\n",
                   name.c_str(),
                   expected_results.row_extent().lower(), expected_results.row_extent().upper(),
                   expected_results.col_extent().lower(), expected_results.col_extent().upper(),
                   test_results.row_extent().lower(), test_results.row_extent().upper(),
                   test_results.col_extent().lower(), test_results.col_extent().upper());
            ++numfound;
        }
    }
    else {
        const size_t actual_size = test_results.size();
        const size_t expected_size = expected_results.size();
        print_diff(actual_size, expected_size, name, numfound);
    }

    return numfound;
}


class test_utils {
public:
    float rel_diff_limit{5e-4f};
    float abs_diff_limit{5e-5f};

    static parameter_descr_type get_descriptor()
    {
        return parm_descr_holder::get_parm_descr();
    }

    template <class T>
    static int check_expected_property(const parm& expected_parm, int expeced_index, const parm& actual_parm,
                                       int actual_index,
                                       std::function<int(T, T, T*)> differ,
                                       std::function<void(const std::string&, T, T, T, int)> printer);;
    template <class T>
    static int  check_expected_property_for_mass_balance(const parm& expected_parm, const parm& actual_parm,
                                                        std::function<int(T, T, T*)> differ,
                                                        std::function<void(const std::string&, T, T, T, int)> printer);;


    int compare(const parm& expected, const parm& actual)
    {
        int diff_count = 0;
        const std::function<int(float, float, float*)> float_differ = [&](float v1, float v2, float* diff)
        {
            return check_diff<float>(v1, v2, rel_diff_limit, abs_diff_limit, diff);
        };
        const std::function<void(const std::string&, float, float, float, int)> float_print = [
            ](const std::string& name, float expected_val, float actual_val, float diff, int is_diff)
        {
            printf("%s: expected: %g actual: %g. %sdiff: %g .\n", name.c_str(), expected_val, actual_val,
                   is_diff > 1 ? is_diff == 2 ? "rel_" : "abs_" : "", diff);
        };
        const std::function<int(int, int, int*)> int_differ = [](int v1, int v2, int* diff)
        {
            *diff = v1 - v2;
            return *diff != 0 ? 1 : 0;
        };
        const std::function<void(const std::string&, int, int, int, int)> int_print = [
            ](const std::string& name, int expected_val, int actual_val, int diff, int is_diff)
        {
            printf("%s: expected: %d actual: %d. diff: %d .\n", name.c_str(), expected_val, actual_val, diff);
        };

        diff_count += compare_scalar(std::string("nodorder"), static_cast<int>(expected.nodorder),
                                     static_cast<int>(actual.nodorder), int_differ, int_print);
        diff_count += compare_scalar(std::string("znull"), expected.znull, actual.znull, float_differ, float_print);
        Tools::gpm_version_number exp_ver(expected.version);
        const Tools::gpm_version_number act_ver(actual.version);
        if (!exp_ver.version_compatible(act_ver)) {
            diff_count += compare_exact_scalar(std::string("version"), expected.version, actual.version);
        }
        // This is where we should not depend on order

        const std::function<int(double, double, double*)> double_differ = [&](double v1, double v2, double* diff)
        {
            return check_diff<double>(v1, v2, rel_diff_limit, abs_diff_limit, diff);
        };
        const std::function<void(const std::string&, double, double, double, int)> double_print = [
            ](const std::string& name, double expected_val, double actual_val, double diff, int is_diff)
        {
            printf("%s: expected: %g actual: %g. %sdiff: %g .\n", name.c_str(), expected_val, actual_val,
                   is_diff > 1 ? is_diff == 2 ? "rel_" : "abs_" : "", diff);
        };

        diff_count += verify_array_1d("xcorners", expected.xcorners, actual.xcorners, float_differ, float_print);

        diff_count += verify_array_1d("ycorners", expected.ycorners, actual.ycorners, float_differ, float_print);

        diff_count += verify_array_1d("isnodal", expected.isnodal, actual.isnodal, int_differ, int_print);

        const std::function<int(std::string, std::string, std::string*)> string_differ = [
            ](const std::string& v1, const std::string& v2, const std::string* diff)
        {
            return v1 != v2 ? 1 : 0;
        };
        const std::function<void(const std::string&, std::string, std::string, std::string, int)> string_print = [
            ](const std::string& name, const std::string& expected_val, const std::string& actual_val,
              const std::string& diff, int is_diff)
        {
            print_diff_with_strings(name, expected_val, actual_val);
        };
        diff_count += verify_array_1d("base_layer_map", expected.base_layer_map, actual.base_layer_map, string_differ,
                                      string_print);
        diff_count += verify_string_array_2d("sediment_prop_map", actual.sediment_prop_map, expected.sediment_prop_map);
        diff_count += check_string_array("propnam", expected.propnam, actual.propnam);

        // If we want to do age matches as well, we need to do some age and time diffs
        // These will affect time, age and grid names

        diff_count += verify_array_1d("ageset", expected.ageset, actual.ageset, double_differ, double_print);
        diff_count += verify_array_1d("timeset", expected.timeset, actual.timeset, double_differ, double_print);


        diff_count += check_string_array("gridnam", expected.gridnam, actual.gridnam);
        index_list_type exp_prop_names{expected.propnam.begin(), expected.propnam.end()};
        index_list_type act_prop_names{actual.propnam.begin(), actual.propnam.end()};
        std::vector<std::string> prop_intersection;
        auto& expect_it = exp_prop_names.get<ordered_by_id>();
        auto& actual_it = act_prop_names.get<ordered_by_id>();
        std::set_intersection(expect_it.begin(), expect_it.end(), actual_it.begin(), actual_it.end(), std::back_inserter(prop_intersection));

        // Lets find the indexes for use
        std::vector<std::pair<int,int>> indprop_indexes;
        auto& exp_by_ins = exp_prop_names.get<ordered_by_insertion>();
        auto& act_by_ins = act_prop_names.get<ordered_by_insertion>();
        for(const auto& item:prop_intersection) {
            const auto it1 = expect_it.find(item);
            const auto it2 = actual_it.find(item);
            const auto pos1 = std::distance(exp_by_ins.begin(), bmi::project<ordered_by_insertion>(exp_prop_names, it1));
            const auto pos2 = std::distance(act_by_ins.begin(), bmi::project<ordered_by_insertion>(act_prop_names, it2));
            indprop_indexes.emplace_back(static_cast<int>(pos1), static_cast<int>(pos2));
        }
        for(const auto item:indprop_indexes) {
            const int expected_layer = item.first;
            const int actual_layer = item.second;
            const int dum_failed = check_expected_property(expected,expected_layer, actual,actual_layer, float_differ, float_print);
            if (dum_failed > 0) {
                
            }
            diff_count += dum_failed;
        }
        diff_count += verify_string_array_with_col_order("prop_display_name", expected.prop_display_name, actual.prop_display_name, indprop_indexes);
        printf("num differences: %d\n", diff_count);
        return diff_count;
    };

    int compare_mass_balance(const parm& expected, const parm& actual)
    {
        int diff_count = 0;
        const std::function<int(float, float, float*)> float_differ = [&](float v1, float v2, float* diff)
        {
            return check_diff<float>(v1, v2, rel_diff_limit, abs_diff_limit, diff);
        };
        const std::function<void(const std::string&, float, float, float, int)> float_print = [
        ](const std::string& name, float expected_val, float actual_val, float diff, int is_diff)
        {
            printf("%s: expected: %g actual: %g. %sdiff: %g .\n", name.c_str(), expected_val, actual_val,
                is_diff > 1 ? is_diff == 2 ? "rel_" : "abs_" : "", diff);
        };
        const std::function<int(int, int, int*)> int_differ = [](int v1, int v2, int* diff)
        {
            *diff = v1 - v2;
            return *diff != 0 ? 1 : 0;
        };
        const std::function<void(const std::string&, int, int, int, int)> int_print = [
        ](const std::string& name, int expected_val, int actual_val, int diff, int is_diff)
        {
            printf("%s: expected: %d actual: %d. diff: %d .\n", name.c_str(), expected_val, actual_val, diff);
        };

        diff_count += compare_scalar(std::string("nodorder"), static_cast<int>(expected.nodorder),
            static_cast<int>(actual.nodorder), int_differ, int_print);
        diff_count += compare_scalar(std::string("znull"), expected.znull, actual.znull, float_differ, float_print);
        Tools::gpm_version_number exp_ver(expected.version);
        const Tools::gpm_version_number act_ver(actual.version);
        if (!exp_ver.version_compatible(act_ver)) {
            diff_count += compare_exact_scalar(std::string("version"), expected.version, actual.version);
        }
        // This is where we should not depend on order

        const std::function<int(double, double, double*)> double_differ = [&](double v1, double v2, double* diff)
        {
            return check_diff<double>(v1, v2, rel_diff_limit, abs_diff_limit, diff);
        };
        const std::function<void(const std::string&, double, double, double, int)> double_print = [
        ](const std::string& name, double expected_val, double actual_val, double diff, int is_diff)
        {
            printf("%s: expected: %g actual: %g. %sdiff: %g .\n", name.c_str(), expected_val, actual_val,
                is_diff > 1 ? is_diff == 2 ? "rel_" : "abs_" : "", diff);
        };

        diff_count += verify_array_1d("xcorners", expected.xcorners, actual.xcorners, float_differ, float_print);

        diff_count += verify_array_1d("ycorners", expected.ycorners, actual.ycorners, float_differ, float_print);

        diff_count += verify_array_1d("isnodal", expected.isnodal, actual.isnodal, int_differ, int_print);

        const std::function<int(std::string, std::string, std::string*)> string_differ = [
        ](const std::string& v1, const std::string& v2, const std::string* diff)
        {
            return v1 != v2 ? 1 : 0;
        };
        const std::function<void(const std::string&, std::string, std::string, std::string, int)> string_print = [
        ](const std::string& name, const std::string& expected_val, const std::string& actual_val,
            const std::string& diff, int is_diff)
        {
            print_diff_with_strings(name, expected_val, actual_val);
        };
        diff_count += verify_array_1d("base_layer_map", expected.base_layer_map, actual.base_layer_map, string_differ,
            string_print);
        diff_count += verify_string_array_2d("sediment_prop_map", actual.sediment_prop_map, expected.sediment_prop_map);
        diff_count += check_string_array("propnam", expected.propnam, actual.propnam);

        // If we want to do age matches as well, we need to do some age and time diffs
        // These will affect time, age and grid names

        diff_count += verify_array_1d("ageset", expected.ageset, actual.ageset, double_differ, double_print);
        diff_count += verify_array_1d("timeset", expected.timeset, actual.timeset, double_differ, double_print);


        diff_count += check_string_array("gridnam", expected.gridnam, actual.gridnam);
		auto act_prop_id_row = gpm_format_helper::copy_row_to_vector(actual.prop_display_name, 0);
		auto exp_prop_id_row = gpm_format_helper::copy_row_to_vector(expected.prop_display_name, 0);
		index_list_type exp_prop_names{ exp_prop_id_row.begin(), exp_prop_id_row.end() };
        index_list_type act_prop_names{ act_prop_id_row.begin(), act_prop_id_row.end() };
        std::vector<std::string> prop_intersection;
        auto& expect_it = exp_prop_names.get<ordered_by_id>();
        auto& actual_it = act_prop_names.get<ordered_by_id>();
        std::set_intersection(expect_it.begin(), expect_it.end(), actual_it.begin(), actual_it.end(), std::back_inserter(prop_intersection));

        // Lets find the indexes for use
        std::vector<std::pair<int, int>> indprop_indexes;
        auto& exp_by_ins = exp_prop_names.get<ordered_by_insertion>();
        auto& act_by_ins = act_prop_names.get<ordered_by_insertion>();
        for (const auto& item : prop_intersection) {
            const auto it1 = expect_it.find(item);
            const auto it2 = actual_it.find(item);
            const auto pos1 = std::distance(exp_by_ins.begin(), bmi::project<ordered_by_insertion>(exp_prop_names, it1));
            const auto pos2 = std::distance(act_by_ins.begin(), bmi::project<ordered_by_insertion>(act_prop_names, it2));
            indprop_indexes.emplace_back(static_cast<int>(pos1), static_cast<int>(pos2));
        }

        const int dum_failed = check_expected_property_for_mass_balance(expected, actual, float_differ, float_print);
        if (dum_failed > 0) {

        }
        diff_count += dum_failed;
        // Need to do it sematically

        diff_count += verify_string_array_2d("prop_display_name", actual.prop_display_name, expected.prop_display_name);
        printf("num differences: %d\n", diff_count);
        return diff_count;
    };


    static int check_string_array(const std::string& name, const std::vector<std::string>& expected, const std::vector<std::string>& actual)
    {
        int ret_num=0;
        index_list_type exp_prop_names{expected.begin(), expected.end()};
        index_list_type act_prop_names{actual.begin(), actual.end()};
        std::vector<std::string> prop_intersection;
        auto& expect_it = exp_prop_names.get<ordered_by_id>();
        auto& actual_it = act_prop_names.get<ordered_by_id>();
        std::set_intersection(expect_it.begin(), expect_it.end(), actual_it.begin(), actual_it.end(),
                              std::back_inserter(prop_intersection));
        if (exp_prop_names.size() != act_prop_names.size() || prop_intersection.size() != exp_prop_names.size()) {
            // Discrepancy
            std::vector<std::string> exp_intersection;
            std::vector<std::string> act_intersection;
            std::set_difference(expect_it.begin(), expect_it.end(), prop_intersection.begin(), prop_intersection.end(),
                                std::back_inserter(exp_intersection));
            std::set_difference(actual_it.begin(), actual_it.end(), prop_intersection.begin(), prop_intersection.end(),
                                std::back_inserter(act_intersection));
            ret_num += exp_intersection.size()+ act_intersection.size();
            printf("%s : difference in content\n", name.c_str());
            if (!exp_intersection.empty()) {
                printf("%s : expected set difference ", name.c_str());
                for (auto it : exp_intersection) {
                    auto t1 = expect_it.equal_range(it);
                    printf("%s ", it.c_str());
                }
                printf("\n");
            }
            if (!act_intersection.empty()) {
                printf("%s : actual set difference ", name.c_str());
               auto& act_by_ins = act_prop_names.get<ordered_by_insertion>();
               for (auto it : act_intersection) {
                   auto t1 = actual_it.equal_range(it);
                   for (auto iter1 = t1.first; iter1 != t1.second; ++iter1) {
                       const auto pos2 = std::distance(act_by_ins.begin(),
                                                       bmi::project<ordered_by_insertion>(act_prop_names, iter1));
                       printf("%s (%lld) ", it.c_str(), pos2);
                   }
                }
                printf("\n");
            }

        }
        return ret_num;
    }
    static int check_data_ranges(const parm& actual)
    {
        // Check that we have all the needed sizes
        printf("Begin data range diffs: \n");
        auto prefixes = parm_descr_holder::get_grid_map_1_index_name_prefixes();
        auto prefs = parm_descr_holder::get_grid_map_names();
        int ret_val = 0;
        range_mapper_type functors;
        const float surface_max = 10000.0F; // Max surface in the system
        const float velocity_max = 100.0F; // one third of sound velcoity
        const float travel_time_max = 3600.0F * 24.0f * 7.0f; // One week travel
        const float no_val = actual.znull;
        std::set<int_extent_1d::index_type> checked_arrays;
        boost::icl::continuous_interval<float> top_range = boost::icl::continuous_interval<float>::closed(
            -surface_max, surface_max);
        boost::icl::continuous_interval<float> sed_range = boost::icl::continuous_interval<float>::closed(0, 1);
        boost::icl::continuous_interval<float> vel_range = boost::icl::continuous_interval<float>::closed(
            -velocity_max, velocity_max);
        boost::icl::continuous_interval<float> travel_range = boost::icl::continuous_interval<float>::closed(
            0, travel_time_max);
        const std::function<bool(float)> top_range_func = [&top_range](float val)
        {
            return boost::icl::contains(top_range, val);
        };
        const std::function<bool(float)> sediment_range_func = [&sed_range](float val)
        {
            const auto is_contained = boost::icl::contains(sed_range, val);
            return is_contained;
        };
        const std::function<bool(float)> velocity_range_func = [&vel_range](float val)
        {
            return boost::icl::contains(vel_range, val);
        };
        const std::function<bool(float)> travel_range_func = [&travel_range](float val)
        {
            return boost::icl::contains(travel_range, val);
        };
        const std::function<bool(float)> is_novalue = [=](float val) { return (val == no_val); };
        auto top_validator = [=](float val) { return top_range_func(val) || is_novalue(val) ? 0 : 1; };
        auto sed_validator = [=](float val) { return sediment_range_func(val) ? 0 : 1; };
        auto vel_validator = [=](float val) { return velocity_range_func(val) || is_novalue(val) ? 0 : 1; };
        auto tra_validator = [=](float val) { return travel_range_func(val) || is_novalue(val) ? 0 : 1; };
        functors[prefs[TOP]] = std::make_pair(true, std::make_pair(top_validator, top_range)); // exact check
        functors[prefs[DEPODEPTH]] = std::make_pair(true, std::make_pair(top_validator, top_range));
        functors[prefs[EROSION]] = std::make_pair(true, std::make_pair(top_validator, top_range));
        functors[prefs[WATER_TOP_FLUID]] = std::make_pair(true, std::make_pair(top_validator, top_range));
        functors[prefs[WATER_TOP_FLUID_VEL_X]] = std::make_pair(true, std::make_pair(vel_validator, vel_range));
        functors[prefs[WATER_TOP_FLUID_VEL_Y]] = std::make_pair(true, std::make_pair(vel_validator, vel_range));
        functors[prefs[WATER_TOP_TURB]] = std::make_pair(true, std::make_pair(top_validator, top_range));
        functors[prefs[WATER_TOP_TURB_VEL_X]] = std::make_pair(true, std::make_pair(vel_validator, vel_range));
        functors[prefs[WATER_TOP_TURB_VEL_Y]] = std::make_pair(true, std::make_pair(vel_validator, vel_range));
        functors[prefs[WAVE_LONGSHORE_VEL_X]] = std::make_pair(true, std::make_pair(vel_validator, vel_range));
        functors[prefs[WAVE_LONGSHORE_VEL_Y]] = std::make_pair(true, std::make_pair(vel_validator, vel_range));
        functors[prefs[WAVE_TRAVEL_TIME]] = std::make_pair(true, std::make_pair(tra_validator, travel_range));
        functors[prefixes[SED_j]] = std::make_pair(false, std::make_pair(sed_validator, sed_range));
        auto props = actual.propnam;
        for (int attr = 0; attr < props.size(); ++attr) {
            auto prop_name = props[attr];
            const std::function<bool(std::pair<range_mapper_type::key_type, range_mapper_type::mapped_type>)> afunc
                = [&prop_name](const std::pair<range_mapper_type::key_type, range_mapper_type::mapped_type>& it)
            {
                if (it.second.first) {
                    return prop_name == it.first;
                }
                return boost::starts_with(prop_name, it.first);
            };
            const auto it = std::find_if(functors.begin(), functors.end(), afunc);
            if (it != functors.end()) {
                // Find the array
                printf("Checking %s\n", prop_name.c_str());
                const auto func = it->second.second.first;
                interval_type interval = it->second.second.second;
                auto rows = actual.indprop.row_extent(); // Time 
                auto cols = actual.indprop.col_extent(); // Surfaces
                for (auto time = rows.lower(); time < rows.upper(); ++time) {
                    for (auto surface = cols.lower(); surface < cols.upper(); ++surface) {
                        const auto& refs(actual.indprop(attr, time, surface));
                        if (refs.is_initialized()) {
                            // Has to be either a constant or an array
                            int num = 0;
                            if (refs.is_constant()) {
                                num = func(refs.constant_value());
                            }
                            else {
                                if (checked_arrays.find(refs.index_value()) == checked_arrays.end()) {
                                    auto& array_ref(refs.const_array_holder());
                                    num = std::accumulate(array_ref.begin(), array_ref.end(), 0,
                                                          [=](int init, float val) { return init + func(val); });
                                    checked_arrays.insert(refs.index_value());
                                }
                            }
                            if (num > 0) {
                                std::string grid = "constant";
                                if (refs.is_index()) {
                                    grid = actual.gridnam[refs.index_value()];
                                }
                                printf("%s(%td, %td):%s is out of range (%f, %f) with %d\n", prop_name.c_str(), time,
                                       surface, grid.c_str(), interval.lower(), interval.upper(), num);
                            }
                            ret_val += num;
                        }
                    }
                }
            }
        }
        printf("End data range diffs: \n");
        return ret_val;
    }

	static int check_sediment_ranges(const parm& actual)
	{
		// Check that we have all the needed sizes
		printf("Begin sediment range diffs: \n");
		auto prefixes = parm_descr_holder::get_grid_map_1_index_name_prefixes();
		int ret_val = 0;
		const float no_val = actual.znull;
		std::set<int> checked_arrays;
		boost::icl::continuous_interval<float> sed_range = boost::icl::continuous_interval<float>::closed(0.999f, 1.001f);
		const std::function<bool(float)> sediment_range_func = [&sed_range](float val)
		{
			const auto is_contained = boost::icl::contains(sed_range, val);
			return is_contained;
		};

		const std::function<bool(float)> is_novalue = [=](float val) { return (val == no_val); };
        const auto sed_validator = [=](float val) { return sediment_range_func(val) ? 0 : 1; };
        const auto prefix = prefixes[SED_j];
		std::vector<std::string> sediments;
		std::copy_if(actual.propnam.begin(), actual.propnam.end(), std::back_inserter(sediments), [=](const std::string& val) {return boost::starts_with(val, prefix); });
        // Find the prop
		std::vector<int> prop_indexes;
        std::transform(sediments.begin(), sediments.end(), std::back_inserter(prop_indexes),[&actual](const std::string& val)
        {
			auto diff = static_cast<int>(std::distance(actual.propnam.begin(),
			                                           std::find(actual.propnam.begin(), actual.propnam.end(), val)));
			return diff;
		});
	    const auto exts = actual.zgrid(0).extents();
		float_2darray_type sum_sediments(exts, 0.0F);
        const auto rows = actual.indprop.row_extent(); // Time 
        const auto cols = actual.indprop.col_extent(); // Surfaces
		for (auto time = rows.lower(); time < rows.upper(); ++time) {
			for (auto surface = cols.lower(); surface < cols.upper(); ++surface) {
				std::vector<function_2d> stack;
				for (auto index : prop_indexes) {
					stack.push_back(actual.indprop(index, time, surface));
				}
				if (std::all_of(stack.begin(), stack.end(), [](const function_2d& val)
				{
					return val.is_initialized();
				})) {
					sum_sediments.fill(0.0F);
					for (const auto& func : stack) {
						if (func.is_index()) {
							gpm_add(sum_sediments, func.const_array_holder(), &sum_sediments);
						}
						else {
							gpm_add(sum_sediments, func.constant_value(), &sum_sediments);
						}
					}
					auto num = std::accumulate(sum_sediments.begin(), sum_sediments.end(), 0,
						[=](int init, float val) { return init + sed_validator(val); });
					if (num > 0) {
						auto minmax =std::minmax_element(sum_sediments.begin(), sum_sediments.end());
						printf("Sediments(%td, %td) is out of range (%f, %f) with %d. min= %f, max= %f\n", time,
							surface, sed_range.lower(), sed_range.upper(), num, *minmax.first, *minmax.second);
						ret_val += num;
					}
				}
			}
		}
		printf("End sediment range diffs: \n");
		return ret_val;
	}
    static int check_data_ordering(const parm& actual)
    {
		int ret=1;
        if (std::is_sorted(actual.ageset.begin(), actual.ageset.end())) {
			std::set<double> uni(actual.ageset.begin(), actual.ageset.end());
			ret = uni.size() == actual.ageset.size() ? 0 : 1;
			if (ret != 0) {
				printf("Ageset is unordered or have duplicates\n");
			}
        }
		if (std::is_sorted(actual.timeset.begin(), actual.timeset.end())) {
			std::set<double> uni(actual.timeset.begin(), actual.timeset.end());
            auto tmp = uni.size() == actual.timeset.size() ? 0 : 1;
			ret += tmp;
			if (tmp != 0) {
				printf("Timeset is unordered or have duplicates\n");
			}
		}
		return ret;
    }

    static int compare_sizes(const std::vector<TypeDescr>& type_info,
                             const acf_base_reader::array_dim_size_type& expected_dims,
                             const acf_base_reader::array_dim_size_type& actual_dims)
    {
        int diff = 0;
        for (const auto& item_desc : type_info) {
            auto expected_item_data = expected_dims[item_desc.id];
            auto actual_item_data = actual_dims[item_desc.id];
            if (expected_item_data != actual_item_data) {
                for (int i = 0; i < std::min(expected_item_data.size(), actual_item_data.size()); ++i) {
                    if (expected_item_data[i] != actual_item_data[i]) {
                        print_diff(actual_item_data[i], expected_item_data[i], item_desc.name, diff);

                    }
                }
            }
        }
        return diff;
    }

    static int compare_count(const std::vector<TypeDescr>& type_info, const acf_base_reader::parameter_num_type& expected_nums,
                             const acf_base_reader::parameter_num_type& actual_nums)
    {
        int diff = 0;
        if (expected_nums != actual_nums) {
            for (const auto& item_desc : type_info) {
                if (expected_nums[item_desc.id] != actual_nums[item_desc.id]) {
                    print_diff(actual_nums[item_desc.id], expected_nums[item_desc.id], item_desc.name, diff);
                }
            }
        }
        return diff;
    }

private:
    typedef std::function<int(float)> func_type;
    typedef boost::icl::continuous_interval<float> interval_type;
    typedef std::pair<bool, std::pair<func_type, interval_type>> range_check_type;
    typedef std::map<std::string, range_check_type> range_mapper_type;


    template <typename T>
    static int verify_array_1d(const std::string& name, const std::vector<T>& expected, const std::vector<T>& actual,
                               std::function<int(T, T, T*)> differ,
                               std::function<void(const std::string&, T, T, T, int)> printer)
    {
        int numfound = 0;
        int i = 0;
        if (expected.size() == actual.size()) {
            for (const T& actual_val : actual) {
                T expected_val = expected[i++];
                T diff; // A bit on the wrong side JT
                int is_diff = differ(expected_val, actual_val, &diff);
                if (is_diff != 0) {
                    printer(name, expected_val, actual_val, diff, is_diff);
                    ++numfound;
                }

            }
        }
        else {
            const size_t actual_size = actual.size();
            const size_t expected_size = expected.size();
            print_diff(actual_size, expected_size, name, numfound);
        }

        return numfound;
    };

    template <typename T>
    static int verify_array_2d(const std::string& name, const gpm_array_2d_base<T>& expected_results,
                               const gpm_array_2d_base<T>& test_results)
    {
        int numfound = 0;
        if (test_results.size() == expected_results.size()) {
            for (int i = test_results.row_extent().lower(); i < test_results.row_extent().upper(); i++) {
                for (int j = test_results.col_extent().lower(); j < test_results.col_extent().upper(); j++) {
                    T expected = expected_results(i, j);
                    T actual = test_results(i, j);
                    if (expected != actual) {
                        print_diff(actual, expected, name, numfound);
                    }
                }
            }
        }
        else {
            const size_t actual_size = test_results.size();
            const size_t expected_size = expected_results.size();
            print_diff(actual_size, expected_size, name, numfound);
        }

        return numfound;
    };
};

template <typename T>
int test_utils::check_expected_property(const parm& expected_parm, int expeced_index,
                                        const parm& actual_parm, int actual_index,
                                        std::function<int(T, T, T*)> differ,
                                        std::function<void(const std::string&, T, T, T, int)> printer)
{
    int numfound = 0;
    auto exp_prop_name = expected_parm.propnam[expeced_index];
    const auto act_prop_name = actual_parm.propnam[actual_index];
    const auto& expected_prop=expected_parm.indprop(expeced_index);
    const auto& actual_prop=actual_parm.indprop(actual_index);
    auto extents = expected_prop.extents() & actual_prop.extents();
    if (extents != expected_prop.extents()) {
        // Print out property name

    }
    const auto& row_ext = extents.row_extent();
    const auto& col_ext = extents.col_extent();
    for(auto ti = row_ext.lower(), num_ti = row_ext.upper(); ti < num_ti;++ti) {
        for(auto tj = col_ext.lower(), num_tj = col_ext.upper(); tj < num_tj;++tj) {
            const auto& exp_arr1=expected_prop(ti,tj);
            const auto& act_arr1=actual_prop(ti,tj);
            if (exp_arr1.is_initialized()&& act_arr1.is_initialized()){
                if (exp_arr1.is_constant() && act_arr1.is_constant()) {
                    T diff;
                    int is_diff = differ(exp_arr1.constant_value(), act_arr1.constant_value(), &diff);
                    if (is_diff != 0) {
                        printer(exp_prop_name, exp_arr1.constant_value(), act_arr1.constant_value(), diff, is_diff);
                        ++numfound;
                    }
                }
                else {
                    // One is arrays, check them
					gpm_vbl_array_2d<bool> has_diff(exp_arr1.extents());
                    auto exp_name = exp_arr1.index_value() > -1? expected_parm.gridnam[exp_arr1.index_value()]: exp_prop_name;
                    auto act_name = act_arr1.index_value() > -1 ? actual_parm.gridnam[act_arr1.index_value()]: act_prop_name;
                    for (auto i = exp_arr1.extents().row_extent().lower(), numi = exp_arr1.extents().row_extent().upper(); i < numi; i++ ) {
                        for (auto j = exp_arr1.extents().col_extent().lower(), numj = exp_arr1.extents().col_extent().upper(); j < numj;  j++) {
                            T real_expected = exp_arr1(i, j);
                            T real_actual = act_arr1(i, j);
                            T diff; // A bit on the wrong side JT
                            const int is_diff = differ(real_expected, real_actual, &diff);
                            if (is_diff != 0) {
								has_diff(i, j) = true;
                                numfound++;
                            }
                        }
                    }
					auto sum = std::accumulate(has_diff.begin(), has_diff.end(), 0, [](int init, bool has_val) {return init + (has_val ? 1 : 0); });
					auto total = exp_arr1.extents().size();
                    if (sum/static_cast<double>(total) > 0.05) {
						printf("grid(Exp:%s,Act:%s, T:%td, A:%td) differ in more than 5 percent of the samples \n",
							exp_name.c_str(), act_name.c_str(),
							ti, tj);

                    }
					else {
						for (auto i = exp_arr1.extents().row_extent().lower(), numi = exp_arr1.extents().row_extent().upper(); i < numi; i++) {
							for (auto j = exp_arr1.extents().col_extent().lower(), numj = exp_arr1.extents().col_extent().upper(); j < numj; j++) {
							    if (has_diff(i, j)) {
							        T real_expected = exp_arr1(i, j);
							        T real_actual = act_arr1(i, j);
							        T diff; // A bit on the wrong side JT
							        const int is_diff = differ(real_expected, real_actual, &diff);
							        printf("grid(Exp:%s,Act:%s, T:%td, A:%td, (%td,%td)) ",
							               exp_name.c_str(), act_name.c_str(),
							               ti, tj, i, j);
							        printer("", real_expected, real_actual, diff, is_diff);
							    }
							}
						}
					}
               }
            }
            else {
                if (exp_arr1.is_initialized() || act_arr1.is_initialized()) {
                    ++numfound;
                    printf("grid(Exp:%s,Act:%s, T:%td, A:%td, [%s, %s]) \n",
                           exp_prop_name.c_str(), act_prop_name.c_str(),
                           ti, tj, exp_arr1.is_initialized() ? "true" : "false",
                           act_arr1.is_initialized() ? "true" : "false");
                }
            }
        }
    }
    return numfound;
}
template <typename T>
int test_utils::check_expected_property_for_mass_balance(const parm& expected_parm,
    const parm& actual_parm,
    std::function<int(T, T, T*)> differ,
    std::function<void(const std::string&, T, T, T, int)> printer)
{
    index_list_type exp_prop_names{ expected_parm.propnam.begin(), expected_parm.propnam.end() };
    index_list_type act_prop_names{ actual_parm.propnam.begin(), actual_parm.propnam.end() };
    std::vector<std::string> prop_intersection;
    auto& expect_it = exp_prop_names.get<ordered_by_id>();
    auto& actual_it = act_prop_names.get<ordered_by_id>();
    std::set_intersection(expect_it.begin(), expect_it.end(), actual_it.begin(), actual_it.end(), std::back_inserter(prop_intersection));
    std::vector<std::pair<int, int>> indprop_indexes;
    auto& exp_by_ins = exp_prop_names.get<ordered_by_insertion>();
    auto& act_by_ins = act_prop_names.get<ordered_by_insertion>();
    for (const auto& item : prop_intersection) {
        const auto it1 = expect_it.find(item);
        const auto it2 = actual_it.find(item);
        const auto pos1 = std::distance(exp_by_ins.begin(), bmi::project<ordered_by_insertion>(exp_prop_names, it1));
        const auto pos2 = std::distance(act_by_ins.begin(), bmi::project<ordered_by_insertion>(act_prop_names, it2));
        indprop_indexes.emplace_back(static_cast<int>(pos1), static_cast<int>(pos2));
    }
    int numfound = 0;

    auto number_top_eroded_failed = 0;
    auto number_top_deposited_failed = 0;
    for (const auto item : indprop_indexes) {
        const int expected_index = item.first;
        const int actual_index = item.second;


        auto exp_prop_name = expected_parm.propnam[expected_index];
        const auto act_prop_name = actual_parm.propnam[actual_index];
        const auto& expected_prop = expected_parm.indprop(expected_index);
        const auto& actual_prop = actual_parm.indprop(actual_index);
        auto extents = expected_prop.extents() & actual_prop.extents();
        if (extents != expected_prop.extents()) {
            // Print out property name
        }

        std::string s_erosion = "EROSION";
        //std::cout << "exp_prop_name=" << exp_prop_name << "\n";
        // std::cout << "s_erosion=" << s_erosion << "\n";

        if (exp_prop_name == s_erosion) {
            //New loop to look for the TOP index.
            //std::cout << "Aqui arribem\n";
            for (const auto item2 : indprop_indexes) {
                const int expected_index2 = item2.first;
                const int actual_index2 = item2.second;
                auto exp_prop_name_2 = expected_parm.propnam[expected_index2];
                const auto act_prop_name_2 = actual_parm.propnam[actual_index2];
                const auto& expected_prop_2 = expected_parm.indprop(expected_index2);
                const auto& actual_prop_2 = actual_parm.indprop(actual_index2);
                std::string s_top = "TOP";
                //std::cout << "exp_prop_name_2=" << exp_prop_name_2 << "\n";
                //std::cout << "s_top=" << s_top << "\n";
                if (exp_prop_name_2 == s_top)
                {

                    //std::cout << "exp_prop_name_2=" << exp_prop_name_2 << "\n";
                    //std::cout << "s_top=" << s_top << "\n";
                    const auto& row_ext = extents.row_extent();
                    const auto& col_ext = extents.col_extent();
                    for (auto ti = row_ext.lower(), num_ti = row_ext.upper(); ti < num_ti; ++ti) {
                        for (auto tj = col_ext.lower(), num_tj = col_ext.upper(); tj < num_tj; ++tj) {
                            const auto& erosion = actual_prop(ti, tj);
                            const auto& top_new = actual_prop_2(ti, tj);
                            const auto& top_old = expected_prop_2(ti, tj);

                            if (top_old.is_initialized() && top_new.is_initialized() && erosion.is_initialized()) {
                                if (top_old.is_constant() && top_new.is_constant() && erosion.is_constant()) {
                                    if (erosion.constant_value() > 0.0F) {
                                        if (top_old.constant_value() > top_new.constant_value()) number_top_eroded_failed += number_top_eroded_failed;
                                    }
                                    else {
                                        if (top_old.constant_value() < top_new.constant_value()) number_top_deposited_failed += number_top_deposited_failed;
                                    }
                                }
                                else {
                                    // One is arrays, check them
                                    auto top_old_name = top_old.index_value() > -1 ? expected_parm.gridnam[top_old.index_value()] : exp_prop_name_2;
                                    auto top_new_name = top_new.index_value() > -1 ? actual_parm.gridnam[top_new.index_value()] : act_prop_name_2;
                                    for (auto i = top_old.extents().row_extent().lower(), numi = top_old.extents().row_extent().upper(); i < numi; i++) {
                                        for (auto j = top_old.extents().col_extent().lower(), numj = top_old.extents().col_extent().upper(); j < numj; j++) {
                                            //printf("grid(Exp:%s,Act:%s, T:%d, A:%d)\n ",
                                            //top_old_name.c_str(), top_new_name.c_str(),
                                            // ti, tj);
                                            //printer("", real_expected, real_actual, diff, is_diff);
                                            if (erosion(i, j) > 0.0F) {
                                                if (top_old(i, j) > top_new(i, j)) number_top_eroded_failed += number_top_eroded_failed;
                                            }
                                            else {
                                                if (top_old(i, j) < top_new(i, j)) number_top_deposited_failed += number_top_deposited_failed;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }


            if (number_top_eroded_failed != 0) {
                std::cout << "expected_index=" << expected_index << "\t" << "Number of nodes out of the condition=" << number_top_eroded_failed + number_top_deposited_failed << "\n";
                std::cout << "Number of nodes failing in the eroded area=" << number_top_eroded_failed << "\n";
                std::cout << "Number of nodes failing in the deposition area=" << number_top_deposited_failed << "\n";

            }
        }
    }
    return number_top_eroded_failed + number_top_deposited_failed;
}
}

int semantic_diff_parm_hdf5_format(const std::string & output_file, const std::string & org_file_ver, bool check_same_ages, float rel_diff, float abs_diff)
{
    gpm_hdf5_reader reader;
    struct parm actual_results;
    const parameter_descr_type actual_parm_descr = test_utils::get_descriptor();
    grid_param_wrapper hdf5_results_grid_param_wrap(actual_results, actual_parm_descr);
    reader.set_parse_type_info(actual_parm_descr);
    grid_param_mapper actual_results_map_grid_it(hdf5_results_grid_param_wrap, reader);
    reader.parse_file(output_file);
    assign_zgrid_to_indprop(&actual_results); // So we get the right setup

    //compare hdf5 file vs hdf5 org
    struct parm expected_org_results;
    gpm_hdf5_reader org_reader;
    test_utils tester;
    tester.rel_diff_limit=rel_diff;
    tester.abs_diff_limit=abs_diff;
    const parameter_descr_type expected_org_results_parm_descr = test_utils::get_descriptor();
    grid_param_wrapper expected_org_results_grid_param_wrap(expected_org_results, expected_org_results_parm_descr);
    org_reader.set_parse_type_info(expected_org_results_parm_descr);
    grid_param_mapper expected_org_results_map_grid_it(expected_org_results_grid_param_wrap, org_reader);
    org_reader.parse_file(org_file_ver);
    assign_zgrid_to_indprop(&expected_org_results); // So we get the rigth setup

    int differences_count = tester.compare(expected_org_results, actual_results);
    differences_count += test_utils::compare_count(expected_org_results_parm_descr, org_reader.num_read(),
                                                   reader.num_read());
    differences_count += test_utils::compare_sizes(expected_org_results_parm_descr, org_reader.array_sizes_read(),
                                                   reader.array_sizes_read());
    differences_count += test_utils::check_data_ranges(actual_results);
	differences_count += test_utils::check_sediment_ranges(actual_results);
	differences_count += test_utils::check_data_ordering(actual_results);
    if (differences_count != 0) {
        printf("HDF5 file %s and \nHDF5 org file %s are not matched.\nFailed\n", output_file.c_str(), org_file_ver.c_str());
    }
    return  differences_count;
}

int diff_parm_mass_balance_in_hdf5_format(const std::string & output_file, const std::string & org_file_ver, bool check_same_ages, float rel_diff, float abs_diff)
{
    gpm_hdf5_reader reader;
    struct parm actual_results;
    const parameter_descr_type actual_parm_descr = test_utils::get_descriptor();
    grid_param_wrapper hdf5_results_grid_param_wrap(actual_results, actual_parm_descr);
    reader.set_parse_type_info(actual_parm_descr);
    grid_param_mapper actual_results_map_grid_it(hdf5_results_grid_param_wrap, reader);
    reader.parse_file(output_file);
    assign_zgrid_to_indprop(&actual_results); // So we get the right setup

                                              //compare hdf5 file vs hdf5 org
    struct parm expected_org_results;
    gpm_hdf5_reader org_reader;
    test_utils tester;
    tester.rel_diff_limit = rel_diff;
    tester.abs_diff_limit = abs_diff;
    const parameter_descr_type expected_org_results_parm_descr = test_utils::get_descriptor();
    grid_param_wrapper expected_org_results_grid_param_wrap(expected_org_results, expected_org_results_parm_descr);
    org_reader.set_parse_type_info(expected_org_results_parm_descr);
    grid_param_mapper expected_org_results_map_grid_it(expected_org_results_grid_param_wrap, org_reader);
    org_reader.parse_file(org_file_ver);
    assign_zgrid_to_indprop(&expected_org_results); // So we get the rigth setup

    int differences_count = tester.compare_mass_balance(expected_org_results, actual_results);
    differences_count += test_utils::compare_count(expected_org_results_parm_descr, org_reader.num_read(),
        reader.num_read());
    differences_count += test_utils::compare_sizes(expected_org_results_parm_descr, org_reader.array_sizes_read(),
        reader.array_sizes_read());
    differences_count += test_utils::check_data_ranges(actual_results);
    differences_count += test_utils::check_sediment_ranges(actual_results);
    if (differences_count != 0) {
        printf("HDF5 file %s and \nHDF5 org file %s are not matched.\nFailed\n", output_file.c_str(), org_file_ver.c_str());
    }
    return  differences_count;
}
