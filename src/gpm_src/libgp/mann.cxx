// -- Schlumberger Private --

#include "mann.h"
#include "gpm_numerics.h"
#include <cmath>
#include <limits> 
#include <cassert>


//#define MAXITER 30 //30
//#define MINDIF 0.1F //0.1F
//#define MINDIFP 0.1F //0.1F
//#define ATTEN 1.0F // 1.0F Optional factor for attenuating effect of side friction
// (1.0 => no attenuation)

namespace Slb { namespace Exploration { namespace Gpm {

// cube root approximation using bit hack for 64-bit float 
// adapted from Kahan's cbrt
// No check on NaN's or infinite or zero
inline double cbrt_5d(double d)
{
    const unsigned int B1 = 715094163;
    double t = 0.0;
    auto* pt = (unsigned int*)&t;
    auto* px = (unsigned int*)&d;
    pt[1] = px[1] / 3 + B1;
    return t;
}


namespace {
// make these local in file scope
inline bool vel_less_fv(float vel, float fv)
{
    return vel < fv;
}

inline bool fv_less_eq_vel_less_0(float vel, float fv)
{
    return fv <= vel && vel < 0;
}

inline bool zero_less_eq_vel(float vel, float)
{
    return 0 <= vel;
}

inline bool fv_less_equal_vel(float vel, float fv)
{
    return fv <= vel;
}

inline bool zero_less_eq_vel_less_fv(float vel, float fv)
{
    return 0 <= vel && vel < fv;
}

inline bool vel_less_0(float vel, float)
{
    return vel < 0;
}
}

int manning_1d::calculate(float s, float r, float d43, float vl, float vr, float* v, float df) const
{
    /* Calculates steady flow velocity v, given:
    k1, k2, k3: coefficients
    s = slope, ( = minus derivative of topography)
    r = depth
    d = cell side
    vl = left velocity
    vr = right velocity
    *v = previous velocity, and result velocity
        (if input near result velocity, convergence is faster;
         if unknown, provide 0, method still works)
  
         NOTE: df is not yet used (will be used when incorporating momentum).
    */

    // Limit slope
    if (s > 1.0F) s = 1.0F;
    if (s < -1.0F) s = -1.0F;
	//if (r < min_depth) r = min_depth;

    //float r13 = (float)pow(r,0.3333f);
    const auto r13 = static_cast<float>(std::cbrt(r));

    /* OLD METHOD:
    int i = 0;
    float dif, difp;
  
    do{
      float fun, der;
      iret = fxmann(*v, k1, k2, k3, s, r, r13, d43, vl, vr, &fun, &der);
      if(iret!=0) return iret;
      
      // Todo get Dan to look at this one JT
      //    if(gpm_isfinite(fun)&& gpm_isfinite(der)){
          if(der>epsilon || der<-epsilon) dif = -fun/der;
          else if (fun> epsilon) dif =  0.1F;
          else if (fun<-epsilon) dif = -0.1F;
          else dif = 0;
      //}
      //else {
      //    dif = 0;
      //}
  
      (*v) += dif;
      difp = dif/(*v);
      i++;
  
      if (!gpm_isfinite(*v))
        throw (std::string("Instability in 1D steady flow calculation (Manning equation, function mann(...))\n"));
  
      //printf("fun = %f\n",fun);
      //printf("der = %f\n",der);
      //printf("v   = %f\n",v  );
  
    } while (i<MAXITER &&
      (dif>=MINDIF || dif<=-MINDIF) &&
      (difp>=MINDIFP || difp<=-MINDIFP));
    */

    // Find v from k1, k2, k3, s, r, r13, d43, vl, vr
    const float d = k1 * r * s;
    const float e = k2 / r13;
    const float f = k3 * r / d43;

    const float fk = 2 * f + e; // Always positive
    const float fv = ((vl + vr) * f) / fk; // Positive, 0, or negative

    /* Old calculation of *v , not involving momentum
    float vs = d / fk;
    if (vs>0)
        vs = sqrt(vs);
    else
        vs = -sqrt(-vs);
  
    *v = fv + vs;
    */

    // New calculation of *v, involving momentum
    if (fv < 0) {
        // Assume v < fv
        if (calculate_velocity(-df, fk, fv, d, vel_less_fv, v)) {
            return 0;
        }
        // Assume fv <= v < 0
        if (calculate_velocity(-df, -fk, fv, d, fv_less_eq_vel_less_0, v)) {
            return 0;
        }
        // Assume 0 <= v
        if (calculate_velocity(df, -fk, fv, d, zero_less_eq_vel, v)) {
            return 0;
        }
    } // if (fv<0)
    else {
        // Assume fv <= v
        if (calculate_velocity(df, -fk, fv, d, fv_less_equal_vel, v)) {
            return 0;
        }
        // Assume 0 <= v < fv
        if (calculate_velocity(df, fk, fv, d, zero_less_eq_vel_less_fv, v)) {
            return 0;
        }
        // Assume v < 0
        if (calculate_velocity(-df, fk, fv, d, vel_less_0, v)) {
            return 0;
        }
    } // if (fv<0) else

    return -1; // Error, should not happen
}

void manning_1d::set_cell_sizes(float delta_x, float delta_y)
{
    _delta_x = delta_x;
    _delta_y = delta_y;
    make_manning_coeffs();
}

void manning_1d::make_manning_coeffs()
{
    _d43_x = static_cast<float>(pow(_delta_x, 1.3333F));
    _d43_y = static_cast<float>(pow(_delta_y, 1.3333F));
}

manning_base::manning_base()
{
    mannings_coeff = std::sqrt(k2 / k1);
}

void manning_base::set_mannings_coeff(double coeff)
{
    assert(coeff > 0);
    mannings_coeff = coeff;
    k2 = static_cast<float>(coeff * coeff * k1);
    k3 = k2; // Same on the side
}

manning_1d::manning_1d():_delta_x(0.0F), _delta_y(0.0F)
{
}

manning_1d::manning_1d(float delta_x, float delta_y) : _delta_x(delta_x), _delta_y(delta_y)
{
    epsilon = 1e-8F;
    make_manning_coeffs();
}

int manning_1d::calculate_use_dx(float s, float r, float vl, float vr, float* v, float df) const
{
    if (r <= 0) {
        *v = 0;
        return 0;
    }
    return calculate(s, r, _d43_x, vl, vr, v, df);
}

int manning_1d::calculate_use_dy(float s, float r, float vl, float vr, float* v, float df) const
{
    if (r <= 0) {
        *v = 0;
        return 0;
    }
    return calculate(s, r, _d43_y, vl, vr, v, df);
}

float manning_1d::delta_x() const
{
    return _delta_x;
}

float manning_1d::delta_y() const
{
    return _delta_y;
}

bool manning_1d::calculate_velocity(float df, float fk, float fv, float d, bool (*check_velocity)(float, float),
                                    float*  v) const
{
    const float dfs = df;
    const float fks = fk;
	const float sumf = fks + dfs;
	const float min_val = std::numeric_limits<float>::min();
    float disc = - dfs * fks * fv * fv - sumf * d;
    if (disc >= 0 ) {
        disc = sqrt(disc);
		if (std::abs(sumf) > min_val) {
			float vtry = (fks * fv - disc) / sumf;
			if (check_velocity(vtry, fv)) {
				*v = vtry;
				return true;
			}
			vtry = (fks * fv + disc) / sumf;
			if (check_velocity(vtry, fv)) {
				*v = vtry;
				return true;
			}
		}
		else {
		    // Check numerator
            // If any of them is ~0, then return 0
            if (std::abs(fks * fv - disc) <=min_val || std::abs(fks * fv + disc) <= min_val) {
				*v = 0.0F;
				return true;
            }
		}
    }
    return false;
}

}}}
