// -- Schlumberger Private --

#include "mapstc.h"
#include "grid_param_mapper.h"
#include "gpm_vbl_array_2d.h"
#include "param_utils.h"
#include <boost/lexical_cast.hpp>

using namespace Slb::Exploration::Gpm;

namespace details{
template <typename T>
static int compare_scalar(const std::string& name, const T& expected, const T& actual) {
    if (expected != actual) {
        //printf("%s: expected %s actual %s\n", name.c_str(), expected, actual);
        return 1;
    }

    return 0;
}

template <>
int compare_scalar<std::string>(const std::string& name, const std::string& expected, const std::string& actual) {
    if (expected != actual) {
        //printf("%s: expected %s actual %s\n", name.c_str(), expected.c_str(), actual.c_str());
        return 1;
    }

    return 0;
}

template <typename T>
static void print_diff(T& actual, T& expected, std::string name, int& numfound) {
    printf("%s : expected value %f actual value %f.\n", name.c_str(), expected, actual);
    ++numfound;
}

template <>
void print_diff<int>(int& actual, int& expected, std::string name, int& numfound) {
    printf("%s : expected value %d actual value %d.\n", name.c_str(), expected, actual);
    ++numfound;
}

template <>
void print_diff<std::size_t>(std::size_t& actual, std::size_t& expected, std::string name, int& numfound) {
    printf("%s : expected value %zd actual value %zd.\n", name.c_str(), expected, actual);
    ++numfound;
}

template <>
void print_diff<std::string>(std::string& actual, std::string& expected, std::string name, int& numfound) {
    printf("%s : expected value %s actual value %s.\n", name.c_str(), expected.c_str(), actual.c_str());
    ++numfound;
}

template <typename T>
static int verify_array_3d(std::string name, gpm_vbl_vector_array_3d<T>& expected_results, gpm_vbl_vector_array_3d<T>& test_results) {
    int numfound = 0;
    if (test_results.size() == expected_results.size()) {
        for (auto k = test_results.layer_extent().lower(); k < test_results.layer_extent().upper(); k++) {
            for (auto i = test_results.row_extent().lower(); i < test_results.row_extent().upper(); i++) {
                for (auto j = test_results.col_extent().lower(); j < test_results.col_extent().upper(); j++) {
                    T expected = expected_results(k, i, j);
                    T actual = test_results(k, i, j);
                    if (expected != actual) {
                        numfound++;
                        //print_diff(actual, expected, name, numfound);
                    }
                }
            }
        }
    }
    else {
        size_t actual_size = test_results.size();
        size_t expected_size = expected_results.size();
        print_diff(actual_size, expected_size, name, numfound);
    }

    return numfound;
}

static int verify_statistics(std::string name, gpm_vbl_array_3d<float>& expected_results, gpm_vbl_array_3d<float>& test_results) {
    int numfound = 0;
    if (test_results.size() == expected_results.size()) {
        for (auto k = test_results.layer_extent().lower(); k < test_results.layer_extent().upper(); k++) {
            for (auto i = test_results.row_extent().lower(); i < test_results.row_extent().upper(); i++) {
                for (auto j = test_results.col_extent().lower(); j < test_results.col_extent().upper(); j++) {
                    float expected = expected_results(k, i, j);
                    float actual = test_results(k, i, j);
                    if (expected != actual) {
                        numfound++;
                        //print_diff(actual, expected, name, numfound);
                    }
                }
            }
        }
    }
    else {
        size_t actual_size = test_results.size();
        size_t expected_size = expected_results.size();
        print_diff(actual_size, expected_size, name, numfound);
    }

    return numfound;
}

template <>
int verify_array_3d<function_2d>(std::string name, gpm_vbl_vector_array_3d<function_2d>& expected_results, gpm_vbl_vector_array_3d<function_2d>& test_results) {
    int numfound = 0;
    if (test_results.size() == expected_results.size()) {
        auto layer_start = test_results.layer_extent().lower();
        auto layer_end = test_results.layer_extent().upper();
        for (auto k = layer_start; k < layer_end; k++) {
            auto row_start = test_results.row_extent().lower();
            auto row_end = test_results.row_extent().upper();
            for (auto i = row_start; i < row_end; i++) {
                auto col_start = test_results.col_extent().lower();
                auto col_end = test_results.col_extent().upper();
                for (auto j = col_start; j < col_end; j++) {
                    function_2d expected = expected_results(k, i, j);
                    function_2d actual = test_results(k, i, j);
                    if (expected != actual) {
                        numfound++;
                        //print_diff(actual, expected, name, numfound);
                    }
                }
            }
        }
    }
    else {
        size_t actual_size = test_results.size();
        size_t expected_size = expected_results.size();
        print_diff(actual_size, expected_size, name, numfound);
    }

    return numfound;
}
}

using namespace details;
class test_utils {
public:
    static parameter_descr_type get_descriptor() {
       return parm_type_descr_holder::get_parm_descr();
    };

    static int compare(parm& expected, parm& actual) {
        int diff_count = 0;
        diff_count += compare_scalar(std::string("nodorder"), expected.nodorder, actual.nodorder);
        diff_count += compare_scalar(std::string("znull"), expected.znull, actual.znull);
        diff_count += compare_scalar(std::string("version"), expected.version, actual.version);
        diff_count += verify_array_2d("sediment_prop_map", expected.sediment_prop_map, actual.sediment_prop_map);
        diff_count += verify_array_1d("ageset", expected.ageset, actual.ageset, std::string(""));
        diff_count += verify_array_1d("timeset", expected.timeset, actual.timeset, std::string(""));
        diff_count += verify_array_1d("xcorners", expected.xcorners, actual.xcorners, std::string(""));
        diff_count += verify_array_1d("ycorners", expected.ycorners, actual.ycorners, std::string(""));
        diff_count += verify_array_1d("gridnam", expected.gridnam, actual.gridnam, std::string("no name"));
        diff_count += verify_array_1d("propnam", expected.propnam, actual.propnam, std::string("no name"));
        diff_count += verify_array_1d("base_layer_map", expected.base_layer_map, actual.base_layer_map, std::string(""));
        diff_count += verify_array_3d<function_2d>("indprop", expected.indprop, actual.indprop);
        diff_count += verify_array_3d<float>("zgrid", expected.zgrid, actual.zgrid);
        diff_count += verify_array_3d<float>("inprop_stat", expected.indprop_stats, actual.indprop_stats);
        return diff_count;
    };

    template <typename T>
    static int verify_array_1d(const std::string& name, const std::vector<T>& expected, const std::vector<T>& actual, const std::string& empty_value) {
        int numfound = 0;
        int i = 0;
        if (expected.size() == actual.size()) {
            for (auto iterator = actual.begin(); iterator != actual.end(); ++iterator) {
                T expected_val = expected[i++];
                T actual_val = *iterator;
                std::string v1_str = boost::lexical_cast<std::string>(actual_val);

                if (expected_val != actual_val && v1_str != empty_value) {
                    print_diff(actual_val, expected_val, name, numfound);
                }
            }
        }
        else {
            size_t actual_size = actual.size();
            size_t expected_size = expected.size();
            print_diff(actual_size, expected_size, name, numfound);
        }

        return numfound;
    };

    template <typename T>
    static int verify_array_2d(const std::string& name, gpm_vbl_array_2d<T>& expected_results, gpm_vbl_array_2d<T>& test_results) {
        int numfound = 0;
        if (test_results.size() == expected_results.size()) {
            auto rows_start = test_results.row_extent().lower();
            auto rows_end = test_results.row_extent().upper();
            for (auto i = rows_start; i < rows_end; i++) {
				auto cols_start = test_results.col_extent().lower();
				auto cols_end = test_results.col_extent().upper();
                for (auto j = cols_start; j < cols_end; j++) {
                    T expected = expected_results(i, j);
                    T actual = test_results(i, j);
                    if (expected != actual) {
                        print_diff(actual, expected, name, numfound);
                    }
                }
            }
        }
        else {
            size_t actual_size = test_results.size();
            size_t expected_size = expected_results.size();
            print_diff(actual_size, expected_size, name, numfound);
        }

        return numfound;
	};

private:
    static int check_diff(float v1, float v2, float epsilon) {
        double a_diff = std::abs(v2 - v1);
        return a_diff > epsilon;
    }
};
