#ifndef GPM_DEPTH_FUNCTION_DATA_H
#define GPM_DEPTH_FUNCTION_DATA_H

#include "mapstc.h"
#include "libutl/gpm_logger.h"
#include "libgp/gpm_1d_interpolator.h"

namespace Slb { namespace Exploration { namespace Gpm{

	// This is where we make sense of the parsed flow data that we need
	class depth_function_data{
	public:

		depth_function_data(struct parm& f, const Tools::gpm_logger& logger);
		~depth_function_data();
		int initialized_value()const ; // Should be zero for all to be ok; Check after constructuion
		bool has_diffusion_curve() const;
		bool has_light_curve() const;
		bool has_temperature_curve() const;
		gpm_1d_linear_array2d diffusion_curve() const;
		gpm_1d_linear_array2d light_curve() const;
		gpm_1d_linear_array2d temperature_curve() const;
	private:
		depth_function_data(const depth_function_data& rhs);
		depth_function_data& operator=(const depth_function_data& rhs);
		struct parm& _f;
		//gpm_1d_interpolator* _wave_amp_func;
		//gpm_1d_interpolator* _wave_phase_func;
		gpm_1d_linear_array2d _diffusion_curve;
		gpm_1d_linear_array2d _light_curve;
		gpm_1d_linear_array2d _temperature_curve;
		int ret_val;
		int intdif;
		int intlig;
		int inttem;
	};

}}}

#endif
