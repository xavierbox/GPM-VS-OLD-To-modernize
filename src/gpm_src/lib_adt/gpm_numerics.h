#ifndef GPM_NUMERICS_H
#define GPM_NUMERICS_H
namespace Slb { namespace Exploration { namespace Gpm {
bool gpm_isnan(float val);
bool gpm_isfinite(float val);
bool gpm_isinf(float val);
bool gpm_isnan(double val);
bool gpm_isfinite(double val);
bool gpm_isinf(double val);
float gpm_modf(float val, int* intpart);
float gpm_modf(float val, long* intpart);
float gpm_modf(float val, long long* intpart);
int gpm_round_to_nearest(float val);
int gpm_round_to_nearest(double val);

template< typename T> T gpm_clip_value(T val, T lower, T upper) {
    if (val < lower) {
        return lower;
    }
    if (upper < val ) {
        return upper;
    }
    return val;
}
// Something before we get to c++17
template< typename T> T clamp(T v, T lo, T hi) {
    return clamp(v, lo, hi, std::less<T>() );
}
template< typename T, typename Compare> T clamp(T v, T lo, T hi, Compare comp) {
    return comp(v, lo) ? lo : comp(hi, v) ? hi : v;
}
}}}
#endif