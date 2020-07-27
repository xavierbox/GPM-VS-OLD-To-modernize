// -- Schlumberger Private --

#include "gpm_1d_interpolator.h"
#include "gpm_vbl_array_2d.h"
#include <numeric>
#include <algorithm>

using namespace Slb::Exploration::Gpm;

int gpm_1d_ordering_test_failed(int argc, char* argv[]) {
	float_2darray_type f1(5, 5, -1e8);
	f1(0, 0) = 1;
	f1(0, 1) = 3;
	f1(0, 2) = 2;
	f1(1, 0) = 10;
	f1(1, 1) = 10;
	f1(1, 2) = 50;
	gpm_1d_linear_array2d interp(f1, -1e8);
	auto res= interp.are_x_strictly_increasing() ? 1 : 0;
	return res;
}
int gpm_1d_ordering_test(int argc, char* argv[]) {
	float_2darray_type f1(5, 5, -1e8);
	f1(0, 0) = 1;
	f1(0, 1) = 3;
	f1(0, 2) = 2;
	f1(1, 0) = 10;
	f1(1, 1) = 11;
	f1(1, 2) = 50;
	gpm_1d_linear_array2d interp(f1, -1e8);
	return interp.are_x_strictly_increasing() ? 0 : 1;
}
int gpm_1d_interpolator_test(int argc, char* argv[]) {
    // 
    float_2darray_type f1(5, 5, -1e8);
    f1(0, 0) = 1;
    f1(0, 1) = 3;
    f1(0, 2) = 2;
    f1(1, 0) = 10;
    f1(1, 1) = 20;
    f1(1, 2) = 50;
    gpm_1d_linear_array2d interp(f1, -1e8);
    float y0 = interp.get(10.0F);
    float y2 = interp.get(50.0F);
    float y3 = interp.get(60.0F);

    return 0;
}

int gpm_1d_interpolator_diff_test(int argc, char* argv[]) {

    float y_vals[] = {1.832146f, 8.175948f, 22.07409f, 44.76883f, 128.4015f, 2.852452f, 3.076321f, 4.552962f, 5.573268f, 5.573268f};
    float x_vals[] = {-100.0F , -77.7778f, -55.5556f, -33.3333f, -11.1111f, 11.1111f, 33.3333f, 55.5556f, 77.7778f, 100.0F};
    float_2darray_type f1(2, 10, -1e8);
    for (int i = 0; i < 10; ++i) {
        f1(0, i) = y_vals[i];
        f1(1, i) = x_vals[i];
    }
    gpm_1d_linear_array2d interp_1(f1, -1e8);
    std::vector<float> results;
    for (int i = 0; i < 10; ++i) {
        auto v1 = interp_1.get(x_vals[i]);
        results.push_back(v1);
    }
    const auto res = std::equal(results.begin(), results.end(), y_vals, [](float a, float b) { return std::abs(a - b) < 1e-4; });

    return res? 0:1;
}

int gpm_1d_integration_test(int argc, char* argv[]) {
    float_2darray_type f1(2, 3, -1e8);
    // Y values
    f1(0, 0) = 1;
    f1(0, 1) = 3;
    f1(0, 2) = 2;
    // X values
    f1(1, 0) = 10;
    f1(1, 1) = 30;
    f1(1, 2) = 50;
    gpm_1d_trapezoid_integration_array2d integrator1(std::make_shared<const gpm_1d_linear_array2d>(f1, -1e8f));
    auto sum1_1 = integrator1.integrate(10, 50);
    auto sum1_2 = integrator1.integrate(-10, 60);
    auto sum1_3 = integrator1.integrate(20, 21);
    auto sum1_4 = integrator1.integrate(50, 51);
    auto sum1_5 = integrator1.integrate(49, 51);
    auto sum1_6 = integrator1.integrate(19, 51);
    auto sum1_7 = integrator1.integrate(9, 11);
    auto sum1_8 = integrator1.integrate(9, 10);
    auto sum1_9 = integrator1.integrate(9, 9.99f);
    auto sum1_10 = integrator1.integrate(51, 52);

    return 0;
}

int gpm_1d_integration_test_clipped(int argc, char* argv[]) {
    std::vector<float> x;
    std::vector<float> y;
    x.push_back(0.5);
    x.push_back(1.0);
    y.resize(2, 1.0);
    gpm_1d_trapezoid_integration_vector_ref test(&(x[0]), &(y[0]), x.size());
    std::vector<double> results;
    std::vector<double> answers;
    results.push_back(test.integrate(0.2F, 0.6F));
    answers.push_back(0.1F);// is 0.1
    results.push_back(test.integrate(0.8F, 1.6f));
    answers.push_back(0.2F); // is 0.2
    results.push_back(test.integrate(0.6F, 0.9f));
    answers.push_back(0.3F);// is 0.3
    results.push_back(test.integrate(0.2F, 0.5F));
    answers.push_back(0.0F); // is 0
    results.push_back(test.integrate(1.0F, 1.2f));
    answers.push_back(0.0F); // is 0
    results.push_back(test.integrate(0.4F, 1.1f));
    answers.push_back(0.5F); // is 0.5
    // Now on to several spans
    x.push_back(2);
    y.push_back(2); // May reallocate, need new pointer
    gpm_1d_trapezoid_integration_vector_ref test1(&(x[0]), &(y[0]), x.size());
    results.push_back(test1.integrate(0.4F, 2.1f));
    answers.push_back(2.0F); // is 2.0
    results.push_back(test1.integrate(1.0F, 2.0F));
    answers.push_back(1.5f); // is 1.5
    results.push_back(test1.integrate(0.9f, 1.1f));
    answers.push_back(0.205f); // is 0.205
    auto res = std::equal(results.begin(), results.end(), answers.begin(), [](double a, double b) { return std::abs(a - b) < 1e-4; });
    return res ? 0 : 1;
}
