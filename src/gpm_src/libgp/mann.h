// -- Schlumberger Private --

#ifndef MANN_H
#define MANN_H
#include <functional>

namespace Slb { namespace Exploration { namespace Gpm {
// Here is where we should have all the stuff needed
// Here are all the running parameters needed
// Just convenience for now
class manning_base {
public:
    // Also, rate of change in all variables must be decreasing
    // mannings relation is 
    // mannings n=sqrt(k2/k1)
    // In the default case that is 0.0316
    //
    float k1 = 10000.F; // Density*gravity
    float k2 = 10.F; // Bottom friction
    float k3 = 10.F; // Side friction
    float epsilon = 1e-10F;
	float min_depth = 0.001F; // The minimal depth that we calculate velocity on;

    double mannings_coeff;
    float mannings_unsteady_coeff = 0.003F; // The mannings coefficient for unsteady, to be consolidated with the normal version

    manning_base();

    void set_mannings_coeff(double coeff);
};

class manning_1d : public manning_base {
public:
    typedef std::function<int(float, float, float, float, float*, float)> calc_func_type;
	typedef std::function<int(float, float, float, float, float*, float, bool)> calc_func_two_type;
	manning_1d();
    manning_1d(float delta_x, float delta_y);
    void set_cell_sizes(float delta_x, float delta_y);
    int calculate_use_dx(float s, float r, float vl, float vr, float* v, float df) const;
    int calculate_use_dy(float s, float r, float vl, float vr, float* v, float df) const;
    float delta_x()const;
    float delta_y()const;
private:
    void make_manning_coeffs();
    int calculate(float s, float r, float d43, float vl, float vr, float* v, float df) const;

    bool calculate_velocity(float df, float fk, float fv, float d, bool (*check_velocity)(float vtry, float fv),
                            float*  v) const;

    float _delta_x;
    float _delta_y;
    float _d43_x{};
    float _d43_y{};
};

}}}
#endif
