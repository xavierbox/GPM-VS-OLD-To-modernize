// -- Schlumberger Private --

#include "gpm_1d_interpolator.h"
#include "gpm_numerics.h"
#include <algorithm>

namespace Slb { namespace Exploration { namespace Gpm { namespace { // Local namespace

// Now we can put in extrapolation policies as well
// Type Tx is the value type of the x we are seaking
// Type Tlim is the type of the limits
// Type Ty is the type of the array
template <typename Tx, typename Tlim, typename Ty>
Ty vecint(Tlim xmin, Tlim xmax, gpm_array_index_defs::size_type nx, const Ty* yvec, Tx xval) {
	// Returns a value yval at position xval
	// interpolated linearly into a vector of values
	int ind;
	Tx xrel, frac;
	// Everything but NaN should work
	if (!gpm_isnan(xval))
	{
		if (xval <= xmin || nx <= 1) return yvec[0];
		if (xval >= xmax) return yvec[nx - 1];
		xrel = (xval - xmin) / (xmax - xmin) * (nx - 1);
		ind = static_cast<int>(xrel);
		frac = xrel - static_cast<Tx>(ind);

		// Cast to the type 
		return static_cast<Ty>(yvec[ind] + (yvec[ind + 1] - yvec[ind]) * frac);
	}
	else
	{
		throw std::invalid_argument("NaN recieved as xval");
	}
}

template <typename T>
float vecint(float xmin, float xmax, int /*nx*/, const std::vector<float>& yvec, T xval) {
	return vecint(xmin, xmax, yvec.size(), &(yvec[0]), xval);
}

template <typename Tx, typename Txp, typename Typ>
Typ interpolate(Tx x, Txp xkm1, Typ ykm1, Txp xk, Typ yk)
{
    auto xdiff = xk -xkm1;
    auto ydiff = yk - ykm1;
    return static_cast<Typ>(ykm1 + (x - xkm1) / xdiff * ydiff);
}

template <typename Txp, typename Typ, typename Tx>
Typ dvecint_ext(gpm_array_index_defs::size_type nx, Txp const* xvec, Typ const* yvec, Tx xval) {
	int i = 0;
	while (i < nx && xvec[i] < xval) i++;
    if (i <= 0) {
        if (xvec[1] > xvec[0]) {
            return interpolate(xval, xvec[0], yvec[0], xvec[1], yvec[1]);
        }
        return yvec[0];
    }
	if (i >= nx) {
        if (xvec[nx-1] > xvec[nx-2]) {
            return interpolate(xval, xvec[nx-2], yvec[nx-2], xvec[nx-1], yvec[nx-1]);
        }
	    return yvec[nx - 1];
    }

	if (xvec[i] > xvec[i - 1])
	{
		return interpolate(xval, xvec[i - 1], yvec[i - 1], xvec[i], yvec[i]);
	}
	return yvec[i];
}

template <typename Txp, typename Typ, typename Tx>
Typ dvecinti(gpm_array_index_defs::size_type nx, Txp const* xvec, Typ const* yvec, Tx xval) {
	int i = 0;
	while (i < nx && xvec[i] < xval) i++;
	if (i <= 0) return yvec[0];
	if (i >= nx) return yvec[nx - 1];

	if (xvec[i] > xvec[i - 1])
	{
		return interpolate(xval, xvec[i - 1], yvec[i - 1], xvec[i], yvec[i]);
	}
	return yvec[i];
}

template <typename Txp, typename Typ, typename Tx, typename To>
To dvecinti_cut(gpm_array_index_defs::size_type nx, Txp const* xvec, Typ const* yvec, Tx xval, To outval) {
	// Finds interpolated value within irregularly sampled
	//   x-ascending (possibly with successive equal x) array
	// nx = number of samples
	// xvec = x vector
	// yvec = y vector
	// xval = xvalue where to interpolate
	// Returns initval if outside range

	if (xval < xvec[0])
	{
		return outval;
	}
	if (xval > xvec[nx - 1])
	{
		return outval;
	}
	return static_cast<To>(dvecinti(nx, xvec, yvec, xval));
}


template <typename Tx, typename Tlim, typename Ty>
Ty vec_integrate(Tlim xmin, Tlim xmax, gpm_array_index_defs::size_type nx, const Ty* yvec, Tx xmin_int, Tx xmax_int) {
	// Everything but NaN should work
	if (xmax_int <= xmin || nx <= 1) return Ty(yvec[0] * (xmax_int - xmin_int));
	if (xmin_int >= xmax) return Ty(yvec[nx - 1] * (xmax_int - xmin_int));
	Ty dx = (xmax - xmin) / (nx - 1);
	Tx xrel_begin = (xmin_int - xmin) / (xmax - xmin) * (nx - 1);
	Tx xrel_end = (xmax_int - xmin) / (xmax - xmin) * (nx - 1);
	auto begin_ind = std::max(static_cast<gpm_array_index_defs::size_type>(xrel_begin) + 1, gpm_array_index_defs::size_type(0));
	auto end_ind = std::min(static_cast<gpm_array_index_defs::size_type>(xrel_end), nx - 1);
	begin_ind = std::min(begin_ind, end_ind); // Now they should be clipped properly
	double sum = 0;
	for (int i = begin_ind; i <= end_ind - 1; ++i)
	{
		Ty upp_val = yvec[i + 1];
		Ty lo_val = yvec[i];
		sum += dx * (upp_val + lo_val) * 0.5F;
	}
	// Low end

	sum += ((xmin + begin_ind * dx) - xmin_int) * (vecint(xmin, xmax, nx, yvec, xmin_int) + yvec[begin_ind]) * 0.5F;
	// high ned
	sum += (xmax_int - (xmin + end_ind * dx)) * (vecint(xmin, xmax, nx, yvec, xmax_int) + yvec[end_ind]) * 0.5F;
	// Cast to the type 
	return Ty(sum);
}

double trapesoidal_integrate_repl_border(const gpm_1d_linear_array2d& holder, double lower, double upper) {
	// Now we need to find the stuff
	assert(lower < upper);
	assert(holder.data().cols() > 0);
	// Simple stuff
	// Nice rectangle
	if (holder.data().cols() == 1)
	{
		return (upper - lower) * holder.get(lower);
	}
	double sum = 0;

	const float* it_begin = &(holder.data()(1, 0));
	const float* it_end = it_begin + holder.data().cols();
	const float* it_low = std::lower_bound(it_begin, it_end, lower); // larger or equal to lower
	const float* it_upp = std::upper_bound(it_begin, it_end, upper); // larger than upper
	if (it_low == it_end)
	{ // Nice rectangle off to the high end
		return (upper - lower) * holder.get(lower);
	}
	if (it_upp == it_begin)
	{ //Nice rectangle off the low end
		return (upper - lower) * holder.get(upper);
	}
	// Here is the hard work
	const float* it_last = it_upp - 1;
	if (it_low == it_upp)
	{ // In an interval 
		return (upper - lower) * (holder.get(lower) + holder.get(upper)) * 0.5F;
	}
	for (const float* it = it_low; it < it_last; ++it)
	{
		float upp_val = *(it + 1);
		float lo_val = *it;
		sum += (upp_val - lo_val) * (holder.get(upp_val) + holder.get(lo_val)) * 0.5F;
	}
	// Low end
	sum += ((*it_low) - lower) * (holder.get(*it_low) + holder.get(lower)) * 0.5F;
	// high ned
	sum += (upper - (*(it_last))) * (holder.get(*(it_last)) + holder.get(upper)) * 0.5F;

	return sum;
}

template <typename Ta, typename Tl> // Ta is the array pointer, Ta is the 
Ta trapesoidal_integrate(Ta const* x_p, Ta const* y_p, gpm_array_index_defs::size_type num, Tl lower, Tl upper, Ta initval = Ta(0)) {
	double sum = 0;
	Ta const* it_begin = x_p;
	Ta const* it_end = it_begin + num;
	Ta const* it_low = std::lower_bound(it_begin, it_end, lower); // larger or equal to lower
	Ta const* it_upp = std::upper_bound(it_begin, it_end, upper); // larger than upper
	if (it_low == it_end)
	{ // Nice rectangle off to the high end
		return initval;
	}
	if (it_upp == it_begin)
	{ //Nice rectangle off the low end
		return initval;
	}
	// Here is the hard work
	Ta const* it_last = it_upp - 1;
	gpm_1d_clipped_linear_vec holder(it_begin, y_p, num, initval);
	if (it_low == it_upp)
	{ // In an interval 
		return Ta((upper - lower) * (holder.get(lower) + holder.get(upper)) * 0.5F);
	}
	for (const float* it = it_low; it < it_last; ++it)
	{
		float upp_val = *(it + 1);
		float lo_val = *it;
		sum += (upp_val - lo_val) * (holder.get(upp_val) + holder.get(lo_val)) * 0.5F;
	}
	// Low end
	if (lower > (*it_begin))
	{
		sum += ((*it_low) - lower) * (holder.get(*it_low) + holder.get(lower)) * 0.5F;
	}
	// high end
	if (upper <= (*(it_end - 1)))
	{
		sum += (upper - (*(it_last))) * (holder.get(*(it_last)) + holder.get(upper)) * 0.5F;
	}

	return Ta(sum);
}

float trapesoidal_integrate(const std::vector<float>& x, const std::vector<float>& y, float lower, float upper) {
	// Now we need to find the stuff
	assert(lower < upper);
	assert(x.size() > 1);
	assert(x.size() == y.size());
	// Simple stuff
	// Nice rectangle
	return trapesoidal_integrate(&(x[0]), &(y[0]), x.size(), lower, upper);
}
} // End local namespace

gpm_1d_interpolator::~gpm_1d_interpolator() = default;

bool gpm_1d_interpolator::are_x_strictly_increasing() const {
	std::vector<float> x_vals;
	std::function<void(float, float)> accum = [&x_vals](float x, float y) {x_vals.push_back(x);};
	visit_discrete_values(accum);
	auto res= std::is_sorted(x_vals.begin(), x_vals.end(), std::less_equal<float>());
	return res;
}
gpm_1d_linear_vec::gpm_1d_linear_vec(const std::vector<float>& x, const std::vector<float>& y) : _x(x), _y(y) {
}

float gpm_1d_linear_vec::get(float x) const {
    return dvecinti(_y.size(), &(_x[0]), &(_y[0]), x);
}

float gpm_1d_linear_vec::get(double x) const {
    return dvecinti(_y.size(), &(_x[0]), &(_y[0]), static_cast<float>(x));
}

const std::vector<float>& gpm_1d_linear_vec::y_data() const {
    return _y;
}
const std::vector<float>& gpm_1d_linear_vec::x_data() const {
    return _x;
}

void gpm_1d_linear_vec::visit_discrete_values(const std::function<void(float, float)>& call_back) const
{
    if (!_y.empty()) {
        for (int i = 0; i < _y.size(); ++i) {
            call_back(_x[i], _y[i]);
        }
    }
}

gpm_1d_clipped_linear_vec::gpm_1d_clipped_linear_vec(float const* x_p, float const* y_p, size_type num_items, float outval): _x(x_p), _y(y_p), _num(num_items), _outval(outval) {
}

float gpm_1d_clipped_linear_vec::get(float x) const {
    return dvecinti_cut(_num, _x, _y, x, _outval);
}

float gpm_1d_clipped_linear_vec::get(double x) const {
    return dvecinti_cut(_num, _x, _y, x, _outval);
}

void gpm_1d_clipped_linear_vec::visit_discrete_values(const std::function<void(float, float)>& call_back) const {
    for (int i = 0; i < _num; ++i) {
        call_back(_x[i], _y[i]);
    }
}

gpm_1d_linear_array2d::gpm_1d_linear_array2d(const float_2darray_base_type& vec, float znull) {
    // Find all y and x values not == znull
    int count = 0;
    for (int i = 0; i < vec.cols(); ++i) {
        if (vec(0, i) != znull && vec(1, i) != znull) {
            ++count;
        }
    }
    _holder = float_2darray_type(2, count, 0);
    count = 0;
    for (int i = 0; i < vec.cols(); ++i) {
        if (vec(0, i) != znull && vec(1, i) != znull) {
            _holder(0, count) = vec(0, i);
            _holder(1, count) = vec(1, i);
            ++count;
        }
    }

}

float gpm_1d_linear_array2d::get(float x) const
{
    // Assume that we have linear memory layout row wise

    return dvecinti(_holder.cols(), &(_holder(1, 0)), &(_holder(0, 0)), x);
}

float gpm_1d_linear_array2d::get(double x) const
{
    // Assume that we have linear memory layout row wise
    return dvecinti(_holder.cols(), &(_holder(1, 0)), &(_holder(0, 0)), static_cast<float>(x));
}

const float_2darray_base_type& gpm_1d_linear_array2d::data() const {
    return _holder;
}

void gpm_1d_linear_array2d::visit_discrete_values(const std::function<void(float, float)>& call_back) const {
    for (int i = 0; i < _holder.cols(); ++i) {
        call_back(_holder(1, i), _holder(0, i));
    }
}

gpm_1d_linear_array2d gpm_1d_linear_array2d::make_constant(float x, float y) {
    float_2darray_type tmp(2, 1);
    tmp(0, 0) = y;
    tmp(1, 0) = x;
    return gpm_1d_linear_array2d(tmp, -1e8);
}

float gpm_1d_linear_array2d_ref::get(float x) const
{
    // Assume that we have linear memory layout row wise

    return dvecinti(_holder.cols(), &(_holder(1, 0)), &(_holder(0, 0)), x);
}

float gpm_1d_linear_array2d_ref::get(double x) const
{
    // Assume that we have linear memory layout row wise

    return dvecinti(_holder.cols(), &(_holder(1, 0)), &(_holder(0, 0)), static_cast<float>(x));
}

gpm_1d_integration::~gpm_1d_integration() = default;

gpm_1d_trapezoid_integration_array2d::gpm_1d_trapezoid_integration_array2d(const std::shared_ptr<const gpm_1d_linear_array2d>& holder): _holder(holder) {
}

gpm_1d_trapezoid_integration_array2d::~gpm_1d_trapezoid_integration_array2d() = default;

double gpm_1d_trapezoid_integration_array2d::integrate(double lower, double upper) const {
    return trapesoidal_integrate_repl_border(*_holder, lower, upper);
}

void gpm_1d_trapezoid_integration_array2d::visit_discrete_values(const std::function<void(float, float)>& call_back) const {
    _holder->visit_discrete_values(call_back);
}

gpm_1d_trapezoid_integration_vector_ref::gpm_1d_trapezoid_integration_vector_ref(float const* x_p, float const* y_p, size_type num_items, float outval): _x(x_p), _y(y_p), _num(num_items), _outval(outval) {
    assert(num_items>1);
}

gpm_1d_trapezoid_integration_vector_ref::~gpm_1d_trapezoid_integration_vector_ref() = default;

double gpm_1d_trapezoid_integration_vector_ref::integrate(double lower, double upper) const {
    return trapesoidal_integrate(_x, _y, _num, lower, upper);
}

void gpm_1d_trapezoid_integration_vector_ref::visit_discrete_values(const std::function<void(float, float)>& call_back) const {
    for (int i = 0; i < _num; ++i) {
        call_back(_x[i], _y[i]);
    }
}

gpm_1d_trapezoid_integration_vector::gpm_1d_trapezoid_integration_vector(const std::shared_ptr<gpm_1d_linear_vec>& holder): _holder(holder) {
}

gpm_1d_trapezoid_integration_vector::~gpm_1d_trapezoid_integration_vector() = default;

double gpm_1d_trapezoid_integration_vector::integrate(double lower, double upper) const {
    // Now we need to find the stuff
    assert(lower < upper);
    assert(_holder->y_data().size() > 0);
    // Simple stuff
    // Nice rectangle
    if (_holder->y_data().size() == 1) {
        return (upper - lower) * _holder->get(lower);
    }
    return trapesoidal_integrate(_holder->x_data(), _holder->y_data(), static_cast<float>(lower), static_cast<float>(upper));
}

void gpm_1d_trapezoid_integration_vector::visit_discrete_values(const std::function<void(float, float)>& call_back) const {
    _holder->visit_discrete_values(call_back);
}

float extrapolate_float(float x, float xkm1, float ykm1, float xk, float yk)
{
    return interpolate(x, xkm1, ykm1, xk, yk);
}


gpm_1d_constant::gpm_1d_constant(float val): _val(val) {

}

float gpm_1d_constant::get(float x) const {
    return _val;
}

float gpm_1d_constant::get(double x) const {
    return _val;
}

void gpm_1d_constant::visit_discrete_values(const std::function<void(float, float)>& call_back) const {
    call_back(0, _val);
}


bool gpm_1d_integration::are_x_strictly_increasing() const {
	std::vector<float> x_vals;
	std::function<void(float, float)> accum = [&x_vals](float x, float y) {x_vals.push_back(x);};
	visit_discrete_values(accum);
	return std::is_sorted(x_vals.begin(), x_vals.end(), std::less_equal<float>());
}

gpm_1d_constant_integration::gpm_1d_constant_integration(float const_val): _const_val(const_val) {
}

gpm_1d_constant_integration::~gpm_1d_constant_integration() = default;

double gpm_1d_constant_integration::integrate(double lower, double upper) const {
    return (upper - lower) * _const_val;
}

void gpm_1d_constant_integration::visit_discrete_values(const std::function<void(float, float)>& call_back) const {
    call_back(0, _const_val);
}

}}}
