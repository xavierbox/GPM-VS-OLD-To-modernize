#ifndef acf_base_helper_h
#define acf_base_helper_h
#include "acf_base_reader.h"
#include "acf_base_writer.h"
#include <numeric>
#include <boost/numeric/conversion/cast.hpp>

namespace Slb { namespace Exploration { namespace Gpm {

class acf_base_helper {
public:
    static acf_base_writer::array_dim_size_type convert(const acf_base_reader::array_dim_size_type& in_idimr)
    {
        acf_base_writer::array_dim_size_type out_idimr;
        std::transform(in_idimr.begin(), in_idimr.end(), std::back_inserter(out_idimr),
                       [](const acf_base_reader::array_dim_size_type::value_type& val)
                       {
                           acf_base_writer::array_dim_size_type::value_type tmp;
                           std::transform(val.begin(), val.end(), tmp.begin(),
                                          [](const acf_base_reader::array_dim_size_type::value_type::value_type& val1)
                                          {
                                              return acf_base_writer::array_dim_size_type::value_type::value_type(val1);
                                          });
                           return tmp;
                       });
        return out_idimr;
    }

    static std::size_t get_size(const acf_base_writer::io_array_dim_type& io)
    {
        return std::accumulate(io.begin(), io.end(), std::size_t(1),
                               [](std::size_t a, const acf_base_writer::io_array_dim_type::value_type& arr)
        {
            return a * arr.size();
        });
    }

    static void fill(acf_base_writer::io_array_dim_type* io_p, std::size_t val)
    {
        std::fill(io_p->begin(), io_p->end(), acf_base_writer::io_array_dim_type::value_type(val));
    }

    static std::size_t set_last_value(acf_base_writer::io_array_dim_type* io_p, std::size_t val)
    {
		fill(io_p, 1);
		io_p->back() = acf_base_writer::io_array_dim_type::value_type(val);
		return val;
    }
	static std::size_t set_array_values(acf_base_writer::io_array_dim_type* io_p, const std::vector<size_t>& vals)
	{
		const auto ext_cast_func = [](size_t val) {return acf_base_writer::array_dim_size_type::value_type::value_type(boost::numeric_cast<int>(val)); };
		fill(io_p, 1);
		std::transform(vals.rbegin(), vals.rend(), io_p->rbegin(), ext_cast_func);
	    const auto res = get_size(*io_p);
		return res;
	}

};
}}}
#endif