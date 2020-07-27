// -- Schlumberger Private --

#include "mapstc.h"
#include "grid_param_mapper.h"
#include "gpm_hdf5_reader.h"
#include "gpm_hdf5_writer.h"
#include "grid_param_reader.h"
#include "test_utils.cxx"
#include "gpm_model_reader.h"
#include "acf_json_reader.h"
#include "acf_base_helper.h"

using namespace Slb::Exploration::Gpm;

int read_hdf5_test(int argc, char* argv[]) {
    int res = -1;
    auto input_path = std::string(argv[2]);

    struct parm expected_results;
    acf_json_reader grid_parser;
    parameter_descr_type expected_result_parm_descr = test_utils::get_descriptor();
    std::string expected_result_file(input_path + "/"+"sfsed.ctl.json");
    grid_param_wrapper expected_results_grid_param_wrap(expected_results, expected_result_parm_descr);
    grid_parser.set_parse_type_info(expected_result_parm_descr);
    grid_param_mapper expected_results_map_grid_it(expected_results_grid_param_wrap, grid_parser);
    grid_parser.parse_file(expected_result_file);
    auto loc_idimr = grid_parser.array_sizes_read();
    auto loc_inumr = grid_parser.num_read();

    gpm_hdf5_writer grid_hdf5_writer;
    std::string output_path = std::string(argv[3]);
    std::string output_file(output_path + "/"+"sfsed.ctl.hdf5");
    auto loc_out_idimr=acf_base_helper::convert(loc_idimr);
	grid_hdf5_writer.set_write_info(expected_result_parm_descr, loc_inumr, loc_out_idimr);
    if (grid_hdf5_writer.open_file(output_file, true)) {
        grid_param_reader read_hdf5_grid_it(expected_results, grid_hdf5_writer);
        grid_hdf5_writer.write();
    }

    gpm_hdf5_reader reader;
    struct parm actual_results;
    parameter_descr_type actual_parm_descr = test_utils::get_descriptor();
    grid_param_wrapper hdf5_results_grid_param_wrap(actual_results, actual_parm_descr);
    reader.set_parse_type_info(actual_parm_descr);
    grid_param_mapper actual_results_map_grid_it(hdf5_results_grid_param_wrap, reader);
    reader.parse_file(output_file);
    int differences_count = test_utils::compare(expected_results, actual_results);
    if (differences_count == 0) {
        printf("Correct");
        return 0;
    }
    printf("Failed");
    return -1;
}

int read_hdf5_api_test(int argc, char* argv[]) {
    auto input_path = std::string(argv[2]);
    auto full_p = input_path+"/"+std::string(argv[1]);
    gpm_model_reader reader;
	reader.open(full_p);
    auto znull = reader.get_novalue();
    auto tnum = reader.get_num_timesteps();
    std::vector<double> times(tnum);
    lin_span<double> th(times.data(), tnum);
    reader.get_timesteps(&th);
    std::vector<double> ages(reader.get_num_ages());
    lin_span<double> age_h(ages.data(), ages.size());
    reader.get_ages(&age_h);
    std::vector<float> x(4), y(4);
    lin_span<float> x_h(x.data(), x.size()), y_h(y.data(), y.size());
    reader.get_world_corners(&x_h,&y_h);
    int nr, nc;
    reader.get_model_size(&nr, &nc);
    int propnum = reader.get_num_property_ids();
    std::vector<std::string> props(propnum);
    reader.get_property_ids(&props);
    auto sum_surfs = reader.get_num_geometry(1);
    gpm_vbl_array_2d<float> dummy(nr,nc);
    std::vector<gpm_vbl_array_2d<float>> surfs(sum_surfs,dummy);
    std::vector<lin_span<float>> surf_holder;
    std::transform(surfs.begin(), surfs.end(), std::back_inserter(surf_holder), [](gpm_vbl_array_2d<float>& val){return lin_span<float>(val.begin(),val.size());});
    reader.get_geometry(1, &surf_holder);
    const auto numDepodepthSurfaces = reader.get_num_property(props[0], 1);
    std::vector<gpm_vbl_array_2d<float>> depo(numDepodepthSurfaces, dummy);
    std::vector<lin_span<float>> depo_holder;
    std::transform(depo.begin(), depo.end(), std::back_inserter(depo_holder), [](gpm_vbl_array_2d<float>& val){return lin_span<float>(val.begin(),val.size());});
	std::vector<int> positions(numDepodepthSurfaces);
	reader.get_property(props[0], 1, &depo_holder, &positions);

	float min = znull;
	float max = znull;
	reader.get_property_statistics(props[0], 1, min, max);

	float minTop = znull;
	float maxTop = znull;
	reader.get_geometry_statistics(1, minTop, maxTop);

	auto baseSedimentsCount = reader.get_num_sediment_ids_in_baselayer();
	std::vector<std::string> base_layer_sediments(baseSedimentsCount);
	reader.get_sediment_ids_in_baselayer(&base_layer_sediments);

	auto sedimentsCount = reader.get_num_sediment_info_in_model();
	std::vector<std::string> sed_ids(sedimentsCount);
	std::vector<std::string> sed_names(sedimentsCount);
	std::vector<std::string> sed_props(sedimentsCount);
	reader.get_sediment_info_in_model(&sed_ids, &sed_names, &sed_props);

    auto propNamesCount = reader.get_num_property_display_names();
	std::vector<std::string> property_names(propNamesCount);
    std::vector<std::string> property_ids(propNamesCount);
	reader.get_property_display_names(&property_ids, &property_names);

    auto nodalCount = reader.get_num_isnodal();
    std::vector<int> nodal(nodalCount);
    lin_span<int> nodal_holder(nodal.data(), nodalCount);
    reader.get_isnodal(&nodal_holder);

    reader.close();
    return 0;
}