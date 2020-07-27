#include "../gpm_carbonate_process.h"
#include "../gpm_sediment_model.h"
#include "../libgp/gpm_1d_interpolator.h"
#include "../libutl/gpm_logger.h"
#include "libgp.h"
#include "gpm_test_model.h"
#include "gpm_vbl_vector_array_3d.h"
#include <vector>
#include <valarray>
using namespace Slb::Exploration::Gpm;


int gpm_carbonate_test( int argc, char *argv[])
{

	Test::gpm_test_model test_model(3,16, 10.0f, 9.9f);
	test_model.set_surface(Test::col_dip_surf(4));

	gpm_carbonate_process carb_process;
	float baserod = 1;
	float tfact = 0.01f;
	std::vector<int> carb_index_map(1,0);
	std::vector<float> max_growth(1,0);
	std::vector<float> rel_growth(1,0);
	std::vector<float> wave_half(1,0);
	int num_sediments_defined = 1;
	sediment_mix_type init(1,1.0);
	sediment_layer_type my_seds(test_model.model()->model_extent(), init);
	float_2darray_type my_erod(test_model.model()->model_extent(), 1.0f);
	std::string carbfile = "dummy";
	int icyc_flag=0;
	float slevnow = 0;
	float dtime=1;
	float_2darray_type light(2, 4, 0);
	water_vel_vector water_vel; //  dummy
	float_2darray_type ergdis; // dummy
	light(0,1)= 100;
	light(1,1)= .1f;
	light(0,2)= 50;
	light(1,2)= 5;
	light(0,3)= 1;
	light(1,3)= 20;
	// Add a light function
	gpm_1d_linear_array2d ligvec_func(light, 1e-9f);
	gpm_1d_linear_array2d temvec_func(ergdis, 1e-9f);
	carb_process.initialize_carbonate_process(test_model.model(), test_model.logger(), num_sediments_defined, tfact, baserod, carb_index_map,
		max_growth, rel_growth, wave_half, std::shared_ptr<gpm_1d_bounded_interpolator>(&ligvec_func, null_deleter()), std::shared_ptr<gpm_1d_bounded_interpolator>(&temvec_func, null_deleter()));

	int iret=carb_process.process(slevnow, test_model.surface_ptr(), test_model.surface(), ergdis,  dtime, &my_seds, &my_erod);

	return 0;
}
