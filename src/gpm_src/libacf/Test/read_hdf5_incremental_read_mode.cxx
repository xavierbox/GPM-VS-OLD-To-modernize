// -- Schlumberger Private --

#include "mapstc.h"
#include "test_utils.cxx"
#include "gpm_model_reader.h"
#include "acf_base_helper.h"

using namespace Slb::Exploration::Gpm;

bool compare_timesteps(gpm_model_reader& actual, gpm_model_reader& expected) {
    const auto actual_tnum = actual.get_num_timesteps();
	std::vector<double> actual_times(actual_tnum);
	lin_span<double> th(actual_times.data(), actual_tnum);
	actual.get_timesteps(&th);

    const auto expected_tnum = expected.get_num_timesteps();
	std::vector<double> expected_times(expected_tnum);
	lin_span<double> th2(expected_times.data(), expected_tnum);
	expected.get_timesteps(&th2);

	int diff_count = 0;
	diff_count += compare_scalar("Timesteps count", expected_tnum, actual_tnum);
	diff_count += test_utils::verify_array_1d("Timestep values", expected_times, actual_times, std::string(""));
	return diff_count == 0;
}

bool compare_ages(gpm_model_reader& actual, const gpm_model_reader& expected) {
	auto actual_num_age = actual.get_num_ages();
	std::vector<double> actual_ages(actual_num_age);
	lin_span<double> age_h(actual_ages.data(), actual_ages.size());
	actual.get_ages(&age_h);

	auto expected_num_age = expected.get_num_ages();
	std::vector<double> expected_ages(expected_num_age);
	lin_span<double> age_h2(expected_ages.data(), expected_ages.size());
	expected.get_ages(&age_h2);

	int diff_count = 0;
	diff_count += compare_scalar("Timesteps count", expected_num_age, actual_num_age);
	diff_count += test_utils::verify_array_1d("Timestep values", expected_ages, actual_ages, std::string(""));
	return diff_count == 0;
}

bool compare_world_corners(gpm_model_reader& actual, const gpm_model_reader& expected) {
	std::vector<float> actual_x_values(4), actual_y_values(4);
	lin_span<float> x_h(actual_x_values.data(), actual_x_values.size()), y_h(actual_y_values.data(), actual_y_values.size());
	actual.get_world_corners(&x_h, &y_h);

	std::vector<float> expected_x_values(4), expected_y_values(4);
	lin_span<float> x_h2(expected_x_values.data(), expected_x_values.size()), y_h2(expected_y_values.data(), expected_y_values.size());
	expected.get_world_corners(&x_h2, &y_h2);

	int diff_count = 0;
	diff_count += test_utils::verify_array_1d("X values", expected_x_values, actual_x_values, std::string(""));
	diff_count += test_utils::verify_array_1d("Y values", expected_y_values, actual_y_values, std::string(""));
	return diff_count == 0;
}

bool compare_model_size(gpm_model_reader& actual, const gpm_model_reader& expected) {
	int actual_nr, actual_nc;
	actual.get_model_size(&actual_nr, &actual_nc);

	int expected_nr, expected_nc;
	expected.get_model_size(&expected_nr, &expected_nc);

	int diff_count = 0;
	diff_count += compare_scalar("Rows count", expected_nr, actual_nr);
	diff_count += compare_scalar("Columns count", expected_nc, actual_nc);
	return diff_count == 0;
}

bool compare_property_ids(gpm_model_reader& actual, const gpm_model_reader& expected) {

	auto actual_propNamesCount = actual.get_num_property_display_names();
	std::vector<std::string> actual_property_names(actual_propNamesCount);
	std::vector<std::string> actual_property_ids(actual_propNamesCount);
	actual.get_property_display_names(&actual_property_ids, &actual_property_names);

	int propnum2 = expected.get_num_property_ids();
	std::vector<std::string> props2(propnum2);
	expected.get_property_ids(&props2);

	auto expected_propNamesCount = expected.get_num_property_display_names();
	std::vector<std::string> expected_property_names(expected_propNamesCount);
	std::vector<std::string> expected_property_ids(expected_propNamesCount);
	expected.get_property_display_names(&expected_property_ids, &expected_property_names);

	int diff_count = 0;
	diff_count += compare_scalar("Property names count", expected_propNamesCount, actual_propNamesCount);
	diff_count += test_utils::verify_array_1d("Property display names", expected_property_names, actual_property_names, std::string(""));
	diff_count += test_utils::verify_array_1d("Property Ids", expected_property_ids, actual_property_ids, std::string(""));
	return diff_count == 0;
}

bool compare_sediments(gpm_model_reader& actual, const gpm_model_reader& expected) {
	auto actual_baseSedimentsCount = actual.get_num_sediment_ids_in_baselayer();
	std::vector<std::string> actual_base_layer_sediments(actual_baseSedimentsCount);
	actual.get_sediment_ids_in_baselayer(&actual_base_layer_sediments);

	auto actual_sedimentsCount = actual.get_num_sediment_info_in_model();
	std::vector<std::string> actual_sed_ids(actual_sedimentsCount);
	std::vector<std::string> actual_sed_names(actual_sedimentsCount);
	std::vector<std::string> actual_sed_props(actual_sedimentsCount);
	actual.get_sediment_info_in_model(&actual_sed_ids, &actual_sed_names, &actual_sed_props);

	auto expected_baseSedimentsCount = expected.get_num_sediment_ids_in_baselayer();
	std::vector<std::string> expected_base_layer_sediments(expected_baseSedimentsCount);
	expected.get_sediment_ids_in_baselayer(&expected_base_layer_sediments);

	auto expected_sedimentsCount = expected.get_num_sediment_info_in_model();
	std::vector<std::string> expected_sed_ids(expected_sedimentsCount);
	std::vector<std::string> expected_sed_names(expected_sedimentsCount);
	std::vector<std::string> expected_sed_props(expected_sedimentsCount);
	expected.get_sediment_info_in_model(&expected_sed_ids, &expected_sed_names, &expected_sed_props);

	int diff_count = 0;
	diff_count += compare_scalar("Base sediments count", expected_baseSedimentsCount, actual_baseSedimentsCount);
	diff_count += compare_scalar("Sediments count", expected_sedimentsCount, actual_sedimentsCount);
	diff_count += test_utils::verify_array_1d("base_layer_sediments", expected_base_layer_sediments, actual_base_layer_sediments,  std::string(""));
	diff_count += test_utils::verify_array_1d("sed_ids", expected_sed_ids, actual_sed_ids,  std::string(""));
	diff_count += test_utils::verify_array_1d("sed_names", expected_sed_names, actual_sed_names,  std::string(""));
	diff_count += test_utils::verify_array_1d("sed_props", expected_sed_props, actual_sed_props,  std::string(""));
	return diff_count == 0;
}

bool compare_isnodal(gpm_model_reader& actual, const gpm_model_reader& expected) {
	auto actual_nodalCount = actual.get_num_isnodal();
	std::vector<int> actual_nodal(actual_nodalCount);
	lin_span<int> nodal_holder(actual_nodal.data(), actual_nodalCount);
	actual.get_isnodal(&nodal_holder);

	auto expected_nodalCount = expected.get_num_isnodal();
	std::vector<int> expected_nodal(actual_nodalCount);
	lin_span<int> nodal_holder2(expected_nodal.data(), expected_nodalCount);
	expected.get_isnodal(&nodal_holder2);

	int diff_count = 0;
	diff_count += compare_scalar("nodalCount", expected_nodalCount, actual_nodalCount);
	diff_count += test_utils::verify_array_1d("nodal data", expected_nodal, actual_nodal, std::string(""));
	return diff_count == 0;
}

bool compare_statistics(gpm_model_reader& actual, const gpm_model_reader& expected) {
	auto znull = actual.get_novalue();
	int propnum = actual.get_num_property_ids();
	std::vector<std::string> props(propnum);
	actual.get_property_ids(&props);

	float actual_min = znull;
	float actual_max = znull;
	actual.get_property_statistics(props[0], 1, actual_min, actual_max);

	float actual_minTop = znull;
	float actual_maxTop = znull;
	actual.get_geometry_statistics(1, actual_minTop, actual_maxTop);
    
	float expected_min = znull;
	float expected_max = znull;
	expected.get_property_statistics(props[0], 1, expected_min, expected_max);

	float expected_minTop = znull;
	float expected_maxTop = znull;
	expected.get_geometry_statistics(1, expected_minTop, expected_maxTop);

	int diff_count = 0;
	diff_count += compare_scalar("min", expected_min, actual_min);
	diff_count += compare_scalar("max", expected_max, actual_max);
	diff_count += compare_scalar("minTop", expected_minTop, actual_minTop);
	diff_count += compare_scalar("maxTop", expected_maxTop, actual_maxTop);
	return diff_count == 0;
}

bool compare_geometry(gpm_model_reader& actual, const gpm_model_reader& expected) {
	int nr, nc;
	actual.get_model_size(&nr, &nc);

	auto actual_sum_surfs = actual.get_num_geometry(1);
	gpm_vbl_array_2d<float> dummy(nr, nc);
	std::vector<gpm_vbl_array_2d<float>> actual_surfs(actual_sum_surfs, dummy);
	std::vector<lin_span<float>> surf_holder;
	std::transform(actual_surfs.begin(), actual_surfs.end(), std::back_inserter(surf_holder), [](gpm_vbl_array_2d<float>& val) {return lin_span<float>(val.begin(), val.size()); });
	actual.get_geometry(1, &surf_holder);

	auto expected_sum_surfs = expected.get_num_geometry(1);
	gpm_vbl_array_2d<float> dummy2(nr, nc);
	std::vector<gpm_vbl_array_2d<float>> expected_surfs(actual_sum_surfs, dummy);
	std::vector<lin_span<float>> surf_holder2;
	std::transform(expected_surfs.begin(), expected_surfs.end(), std::back_inserter(surf_holder2), [](gpm_vbl_array_2d<float>& val) {return lin_span<float>(val.begin(), val.size()); });
	expected.get_geometry(1, &surf_holder2);

	int diff_count = 0;
	diff_count += compare_scalar("sum_surfs", expected_sum_surfs, actual_sum_surfs);

	for (auto i = 0; i < actual_surfs.size(); i++) {
		diff_count += test_utils::verify_array_2d("Surface values", expected_surfs[i], actual_surfs[i]);
	}

	return diff_count == 0;
}

bool compare_property_values(gpm_model_reader& actual, const gpm_model_reader& expected)
{
	int nr, nc;
	actual.get_model_size(&nr, &nc);
	int propnum = actual.get_num_property_ids();
	std::vector<std::string> props(propnum);
	actual.get_property_ids(&props);
	gpm_vbl_array_2d<float> dummy(nr, nc);
	
    const auto actual_numDepodepthSurfaces = actual.get_num_property(props[0], 1);
	std::vector<gpm_vbl_array_2d<float>> actual_depo(actual_numDepodepthSurfaces, dummy);
	std::vector<lin_span<float>> depo_holder;
	std::transform(actual_depo.begin(), actual_depo.end(), std::back_inserter(depo_holder), [](gpm_vbl_array_2d<float>& val) {return lin_span<float>(val.begin(), val.size()); });
	std::vector<int> positions(actual_numDepodepthSurfaces);
	actual.get_property(props[0], 1, &depo_holder, &positions);

	const auto expected_numDepodepthSurfaces = expected.get_num_property(props[0], 1);
	std::vector<gpm_vbl_array_2d<float>> expected_depo(expected_numDepodepthSurfaces, dummy);
	std::vector<lin_span<float>> depo_holder2;
	std::transform(expected_depo.begin(), expected_depo.end(), std::back_inserter(depo_holder2), [](gpm_vbl_array_2d<float>& val) {return lin_span<float>(val.begin(), val.size()); });
	std::vector<int> positions2(expected_numDepodepthSurfaces);
	expected.get_property(props[0], 1, &depo_holder2, &positions2);

	int diff_count = 0;
	diff_count += compare_scalar("numDepodepthSurfaces", expected_numDepodepthSurfaces, actual_numDepodepthSurfaces);

	for (auto i = 0; i < actual_depo.size(); i++) {
		diff_count += test_utils::verify_array_2d("Surface values", expected_depo[i], actual_depo[i]);
	}

	return diff_count == 0;
}

int read_hdf5_incremental_mode_api_test(int argc, char* argv[]) {
	auto input_path = std::string(argv[2]);
	auto full_p = input_path + "/" + std::string(argv[1]);
	gpm_model_reader expected;
	expected.open(full_p);

	gpm_model_reader actual;
	actual.open(full_p, 2);

	if (!compare_scalar("ZNULL", expected.get_novalue(), actual.get_novalue()) == 0)
		return -1;

	if (!compare_timesteps(actual, expected)) {
		return -1;
	}

	if (!compare_ages(actual, expected)) {
		return -1;
	}

	if (!compare_world_corners(actual, expected)) {
		return -1;
	}

	if (!compare_model_size(actual, expected)) {
		return -1;
	}

	if (!compare_property_ids(actual, expected)) {
		return -1;
	}

	if (!compare_sediments(actual, expected)) {
		return -1;
	}

	if (!compare_isnodal(actual, expected)) {
		return -1;
	}

	if (!compare_statistics(actual, expected)) {
		return -1;
	}

	if (!compare_geometry(actual, expected)) {
		return -1;
	}

	if (!compare_property_values(actual, expected)) {
		return -1;
	}

	actual.close();
	expected.close();
	return 0;
}