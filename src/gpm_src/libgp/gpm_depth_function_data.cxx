#include "gpm_depth_function_data.h"
#include "mapstc_descr.h"
#include <boost/algorithm/string.hpp>
#include <boost/numeric/conversion/cast.hpp>

namespace Slb { namespace Exploration { namespace Gpm{
using namespace Tools;

depth_function_data::depth_function_data(struct parm& f, const gpm_logger& logger): _f(f), 
	_diffusion_curve(float_2darray_type(1,1,0),0,0), 
	_light_curve(float_2darray_type(1,1,0),0,0),
	_temperature_curve(float_2darray_type(1,1,0),0,0),ret_val(0), intdif(-1), intlig(-1), inttem(-1)
{
	// Find intdif, index to diffusion coefficients in depth file */
	//  intdif,                     // Index to diffusion coefficients in depth file
	//  intlig,			            // Index to light coefficients in depth file
	//  inttem,			            // Index to temperature coefficients in depth file
	int ndep = boost::numeric_cast<int>(_f.gridnam.size());
	// Now lets use the enums to get the strings
	std::string dif_name=parmd_descr_holder::grid_depth_name[DIF];
	std::string lig_name=parmd_descr_holder::grid_depth_name[LIG];
	std::string tem_name=parmd_descr_holder::grid_depth_name[TEM];

	bool use_xvals = f.zgrid.rows() == 2;

	// Need some better checking here
	for(int k=0;k<ndep;k++){
		if(boost::iequals(_f.gridnam[k],dif_name)){
			intdif = k;
		}
	}
	if (intdif>=0) {
		_diffusion_curve = use_xvals? gpm_1d_linear_array2d(_f.zgrid(intdif), f.znull): gpm_1d_linear_array2d(_f.zgrid(intdif), _f.xmin,_f.xmax);
	}
	for(int k=0;k<ndep;k++){
		if(boost::iequals(_f.gridnam[k],lig_name)){
			intlig = k;
		}
	}
	if (intlig>=0) {
		_light_curve = use_xvals? gpm_1d_linear_array2d(_f.zgrid(intlig), f.znull): gpm_1d_linear_array2d(_f.zgrid(intlig), _f.xmin,_f.xmax);
	}

	for(int k=0;k<ndep;k++){
		if(boost::iequals(_f.gridnam[k],tem_name)){
			inttem = k;
		}
	}
	if (inttem>=0) {
		_temperature_curve = use_xvals? gpm_1d_linear_array2d(_f.zgrid(inttem), f.znull): gpm_1d_linear_array2d(_f.zgrid(inttem), _f.xmin,_f.xmax);
	}
	logger.print(Tools::LOG_VERBOSE,"intdif = %d\n",intdif);
	logger.print(Tools::LOG_VERBOSE,"intlig = %d\n",intlig);
	logger.print(Tools::LOG_VERBOSE,"inttem = %d\n",inttem);
}


depth_function_data::~depth_function_data()
{
}

int depth_function_data::initialized_value()const
{
	return ret_val;
}


bool depth_function_data::has_diffusion_curve() const {
	return intdif >=0;
}
bool depth_function_data::has_light_curve() const {
	return intlig>=0;
}

bool depth_function_data::has_temperature_curve() const {
	return inttem>=0;
}
gpm_1d_linear_array2d depth_function_data::diffusion_curve()const {
	return _diffusion_curve;
}
gpm_1d_linear_array2d depth_function_data::light_curve() const{
	return _light_curve;
}

gpm_1d_linear_array2d depth_function_data::temperature_curve() const{
	return _temperature_curve;
}

}}}
