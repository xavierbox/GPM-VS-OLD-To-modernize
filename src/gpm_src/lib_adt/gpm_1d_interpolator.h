// -- Schlumberger Private --

#ifndef GPM_1D_INTERPOLATOR_H
#define GPM_1D_INTERPOLATOR_H

#include "gpm_basic_defs.h"
#include "gpm_vbl_array_2d.h"
#include <vector>
#include <functional>

namespace Slb { namespace Exploration { namespace Gpm {

class gpm_1d_interpolator {
public:
    virtual float get(float x) const =0;
    virtual float get(double x) const =0;
    virtual ~gpm_1d_interpolator();
    // method is void func(x,y)
    virtual void visit_discrete_values(const std::function<void(float, float)>& call_back) const = 0;
	// This restriction needs to hold for providers
	bool are_x_strictly_increasing() const;
};

class gpm_1d_constant: public gpm_1d_interpolator {
public:
    typedef gpm_1d_interpolator base;
    explicit gpm_1d_constant(float val);
    virtual float get(float x) const override;
    virtual float get(double x) const override;
    virtual void visit_discrete_values(const std::function<void(float, float)>& call_back) const override;
private:
    float _val;
};

// Take ownership of the vector
class gpm_1d_linear_vec: public gpm_1d_interpolator {
public:
    typedef gpm_1d_interpolator base;
    gpm_1d_linear_vec(const std::vector<float>& x, const std::vector<float>& y);
    virtual float get(float x) const override;
    virtual float get(double x) const override;
    const std::vector<float>& y_data() const;
    const std::vector<float>& x_data() const;
    virtual void visit_discrete_values(const std::function<void(float, float)>& call_back) const override;
private:
    std::vector<float> _x;
    std::vector<float> _y;
};

// Here we clip the data, and the holder outside needs to hold the data in scope
class gpm_1d_clipped_linear_vec: public gpm_1d_interpolator {
public:
    typedef gpm_1d_interpolator base;
	typedef gpm_array_index_defs::size_type size_type;
	gpm_1d_clipped_linear_vec(float const* x_p, float const* y_p, size_type num_items, float outval = 0.0F);
    virtual float get(float x) const override;
    virtual float get(double x) const override;
    virtual void visit_discrete_values(const std::function<void(float, float)>& call_back) const override;
private:
    float const* _x;
    float const* _y;
	size_type _num;
    float _outval;
};

class gpm_1d_linear_array2d: public gpm_1d_interpolator {
public:
    typedef gpm_1d_interpolator base;
    gpm_1d_linear_array2d(const float_2darray_base_type& vec, float znull);
    virtual float get(float x) const override;
    virtual float get(double x) const override;
    const float_2darray_base_type& data() const;
    virtual void visit_discrete_values(const std::function<void(float, float)>& call_back) const override;
    static gpm_1d_linear_array2d make_constant(float x, float y);
private:
    float_2darray_type _holder;
};

class gpm_1d_linear_array2d_ref: gpm_1d_interpolator {
public:
    typedef gpm_1d_interpolator base;
    virtual float get(float x) const override;
    virtual float get(double x) const override;
private:
    const float_2darray_base_type& _holder;
};

class gpm_1d_integration {
public:
    virtual double integrate(double lower, double upper) const = 0;
    virtual ~gpm_1d_integration();
    virtual void visit_discrete_values(const std::function<void(float, float)>& call_back) const = 0;
	// This restriction needs to hold for providers
	bool are_x_strictly_increasing() const;
};

class gpm_1d_constant_integration: public gpm_1d_integration {
public:
    explicit gpm_1d_constant_integration(float const_val);
    ~gpm_1d_constant_integration();
	double integrate(double lower, double upper) const override;
    virtual void visit_discrete_values(const std::function<void(float, float)>& call_back) const override;
private:
    float _const_val;
};


class gpm_1d_trapezoid_integration_array2d: public gpm_1d_integration {
public:
    explicit gpm_1d_trapezoid_integration_array2d(const std::shared_ptr<const gpm_1d_linear_array2d>& holder);
    ~gpm_1d_trapezoid_integration_array2d();
	double integrate(double lower, double upper) const override;
    virtual void visit_discrete_values(const std::function<void(float, float)>& call_back) const override;
private:
    std::shared_ptr<const gpm_1d_linear_array2d> _holder;
};

// Here we clip the data, and the holder outside needs to hold the data in scope
class gpm_1d_trapezoid_integration_vector_ref: public gpm_1d_integration {
public:
	typedef gpm_array_index_defs::size_type size_type;
	explicit gpm_1d_trapezoid_integration_vector_ref(float const* x_p, float const* y_p, size_type num_items, float outval = 0.0F);
    ~gpm_1d_trapezoid_integration_vector_ref();
	double integrate(double lower, double upper) const override;
    virtual void visit_discrete_values(const std::function<void(float, float)>& call_back) const override;
private:
    float const* _x;
    float const* _y;
    size_type _num;
    float _outval;
};


class gpm_1d_trapezoid_integration_vector: public gpm_1d_integration {
public:
    explicit gpm_1d_trapezoid_integration_vector(const std::shared_ptr<gpm_1d_linear_vec>& holder);
    ~gpm_1d_trapezoid_integration_vector();
	double integrate(double lower, double upper) const override;
    virtual void visit_discrete_values(const std::function<void(float, float)>& call_back) const override;
private:
    std::shared_ptr<gpm_1d_linear_vec> _holder;
};

float extrapolate_float(float x, float xkm1, float ykm1, float xk, float yk);
// Here we can put in the next linear one that has an x array as well
}}}
#endif
