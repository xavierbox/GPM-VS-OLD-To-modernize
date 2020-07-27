// -- Schlumberger Private --

#ifndef _gpm_function_2d_h_
#define _gpm_function_2d_h_


//typedef boost::multi_array<int, 2> int_2darray_type;
#include "gpm_basic_defs.h"
#include "gpm_int_extent_2d.h"
#include "gpm_vbl_vector_array_3d.h"

namespace Slb { namespace Exploration { namespace Gpm {
// This one is just going to emulate a 2d function consisting of either an array or a constant
// Used in indprop to get emulate a 2d function concept
class function_2d {
public:
	using index_type = std::ptrdiff_t;
    function_2d(): m_is_constant(false), m_const(0), m_arr_index(-1), m_is_initialized(false)
    {
    }

    explicit function_2d(float const_val, const int_extent_2d& ext = int_extent_2d(-10000, 10000, -10000, 10000)): m_is_constant(true), m_const(const_val), m_arr_index(-1),
                                                                                                                   m_is_initialized(true), m_ext(ext)
    {
    }

    explicit function_2d(int index): m_is_constant(false), m_const(0), m_arr_index(index),
                                     m_is_initialized(true)
    {
    }

    // Copy and assign ok for now
    void set_constant(float const_val, const int_extent_2d& ext = int_extent_2d(-10000, 10000, -10000, 10000))
    {
        m_is_constant = true;
        m_const = const_val;
        m_ext = ext;
        m_is_initialized = true;
    }

    void set_array(const std::shared_ptr<float_3darray_vec_base_type>& arr) { m_arr = arr; }

    void set_index(index_type index)
    {
        m_arr_index = index;
        m_is_constant = false;
        m_is_initialized = true;
    }

    bool is_initialized() const { return m_is_initialized; }
    bool is_constant() const { return m_is_initialized && m_is_constant; }
    bool is_index() const { return m_is_initialized && !m_is_constant; }
    bool can_index() const { return is_index() && m_arr != nullptr; }
    float constant_value() const { return m_const; }

    const float_2darray_base_type&
    const_array_holder() const { return (*m_arr)(m_arr_index); }

	float_2darray_base_type&
	array_holder() const { return (*m_arr)(m_arr_index); }

	index_type index_value() const
    {
        return m_arr_index;
    }

    bool is_equal(const function_2d& rhs) const
    {
        bool is_const(rhs.is_constant() && is_constant());
        bool is_array(!rhs.is_constant() && !is_constant());
        bool is_eq = false;
        if (is_const) {
            // Need to do the sprintf thing to be semantically equal
            float diff = std::fabs(rhs.constant_value() - constant_value());
            if (diff < 1e-3) {
                is_eq = true;
            }
        }
        if (is_array) {
            is_eq = rhs.index_value() == index_value();
        }
        return is_eq;
    }

    float operator()(index_type i, index_type j) const
    {
        if (m_is_constant) {
            return m_const;
        }
        return (*m_arr)(m_arr_index, i, j);
    }

    float at(index_type i, index_type j) const
    {
        if (m_is_constant) {
            return m_const;
        }
        return (*m_arr)(m_arr_index, i, j);
    }

    int_extent_2d extents() const
    {
        if (is_constant()) {
            return m_ext;
        }
        if (is_index()) {
            return (*m_arr)[m_arr_index].extents();
        }
        return int_extent_2d(0, 0, 0, 0);
    }

    bool operator==(const function_2d& rhs) const
    {
        if (this == &rhs || (!rhs.is_initialized() && !is_initialized())) {
            return true;
        }
        if (!is_initialized() || !rhs.is_initialized()) {
            return false;
        }
        // Both initialized
        if (is_constant() && rhs.is_constant()) {
            return m_const == rhs.m_const;
        }
        // Now both must be arrays
        if ((m_arr == rhs.m_arr) && (m_arr_index == rhs.m_arr_index)) {
            return true;
        }
        return false;
    }

    bool operator!=(const function_2d& rhs) const
    {
        return !operator==(rhs);
    }

    bool operator<(const function_2d& rhs) const
    {
        if (operator==(rhs)) {
            return false;
        }
        if (!is_initialized() && rhs.is_initialized()) {
            return true;
        }
        if (is_initialized() && !rhs.is_initialized()) {
            return false;
        }
        if (is_constant() && rhs.is_constant()) {
            return constant_value() < rhs.constant_value();
        }
        // Both arrays
        // First same array address
        if (m_arr == rhs.m_arr) {
            return m_arr_index < rhs.m_arr_index;
        }
        return m_arr < rhs.m_arr;
    }

private:
    bool m_is_constant;
    float m_const;
    //    float ***m_arr;
	index_type m_arr_index;
    bool m_is_initialized;
    int_extent_2d m_ext;
    // This should be the one to use
    std::shared_ptr<float_3darray_vec_base_type> m_arr;
};

typedef gpm_array_2d_base<function_2d> function_2darray_type;

}}}// Namespace GPM


#endif
