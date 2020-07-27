// -- Schlumberger Private --

#include <boost/math/special_functions/fpclassify.hpp>
#include <cmath>
namespace Slb { namespace Exploration { namespace Gpm {

template <typename T>
bool gpm_isnan_impl(T val) {
    return (boost::math::isnan)(val);
}

template <typename T>
bool gpm_isfinite_impl(T val) {
    return (boost::math::isfinite)(val);
}

template <typename T>
bool gpm_isinf_impl(T val) {
    return (boost::math::isinf)(val);
}

template <typename T>
int gpm_round_to_nearest_impl(T val) {
    return static_cast<int>(val >= 0 ? val + 0.5 : val - 0.5);
}

// Have it here just for now
bool gpm_isnan(float val) {
    return gpm_isnan_impl(val);
}

bool gpm_isfinite(float val) {
    return gpm_isfinite_impl(val);
}

bool gpm_isinf(float val) {
    return gpm_isinf_impl(val);
}

bool gpm_isnan(double val) {
    return gpm_isnan_impl(val);
}

bool gpm_isfinite(double val) {
    return gpm_isfinite_impl(val);
}

bool gpm_isinf(double val) {
    return gpm_isinf_impl(val);
}

int gpm_round_to_nearest(float val) {
    return gpm_round_to_nearest_impl(val);
}

int gpm_round_to_nearest(double val) {
    return gpm_round_to_nearest_impl(val);
}
// Just to use instead of the casting
// Similar to boost.math.modf
float gpm_modf(float val, int* intpart)
{
	float intp;
    const auto res = std::modf(val, &intp);
	*intpart = static_cast<int>(intp);
	return res;
}

float gpm_modf(float val, long* intpart)
{
	float intp;
	const auto res = std::modf(val, &intp);
	*intpart = static_cast<long>(intp);
	return res;
}
float gpm_modf(float val, long long* intpart)
{
	float intp;
	const auto res = std::modf(val, &intp);
	*intpart = static_cast<long long>(intp);
	return res;
}

}}}
