// -- Schlumberger Private --

#include "mapstc.h"
#include "grid_param_mapper.h"
#include "gpm_hdf5_reader.h"
#include "gpm_hdf5_writer.h"
#include "grid_param_reader.h"
#include "test_utils.cxx"
#include "acf_json_reader.h"
#include "acf_base_helper.h"

using namespace Slb::Exploration::Gpm;

int write_hdf5_multiple_cycles_test(int argc, char* argv[]) {
    const auto input_path = std::string(argv[2]);

    acf_json_reader data_parser;
    const parameter_descr_type first_cycle_parm_descr = test_utils::get_descriptor();
    const std::string first_cycle_file(input_path + "/"+"Case1.001.json");

    struct parm first_cycle_file_data;
    grid_param_wrapper first_cycle_grid_param_wrap(first_cycle_file_data, first_cycle_parm_descr);
    data_parser.set_parse_type_info(first_cycle_parm_descr);
    grid_param_mapper first_cycle_map_grid_it(first_cycle_grid_param_wrap, data_parser);
    data_parser.parse_file(first_cycle_file);
    const auto first_cycle_idimr = data_parser.array_sizes_read();
    const auto first_cycle_inumr = data_parser.num_read();

    gpm_hdf5_writer grid_hdf5_writer;
    const std::string output_file_name = std::string("Case1.hdf5");
    const std::string output_path(argv[3]);
    const std::string output_file(output_path + "/"+output_file_name);

    const auto first_cycle_out_idimr = acf_base_helper::convert(first_cycle_idimr);
	grid_hdf5_writer.set_write_info(first_cycle_parm_descr, first_cycle_inumr, first_cycle_out_idimr);
    if (grid_hdf5_writer.open_file(output_file, true)) {
        grid_param_reader read_hdf5_grid_it(first_cycle_file_data, grid_hdf5_writer);
        grid_hdf5_writer.write();
        grid_hdf5_writer.disconnect_slots();
    }

    const std::string second_simulation_cycle_file(input_path + "/"+"Case1.002.json");
    const parameter_descr_type second_cycle_parm_descr = test_utils::get_descriptor();

    struct parm second_cycle_file_data;
    grid_param_wrapper second_cycle_grid_param_wrap(second_cycle_file_data, second_cycle_parm_descr);
    data_parser.set_parse_type_info(second_cycle_parm_descr);
    grid_param_mapper second_cycle_map_grid_it(second_cycle_grid_param_wrap, data_parser);
    data_parser.parse_file(second_simulation_cycle_file);
    const auto second_cycle_idimr = data_parser.array_sizes_read();
    const auto second_cycle_inumr = data_parser.num_read(); {
        const auto second_cycle_out_idimr= acf_base_helper::convert(second_cycle_idimr);
        grid_param_reader read_hdf5_grid_it(second_cycle_file_data, grid_hdf5_writer);
        grid_hdf5_writer.append(second_cycle_parm_descr, second_cycle_inumr, second_cycle_out_idimr);
        grid_hdf5_writer.disconnect_slots();
    }

    const std::string third_cycle_file(input_path + "/"+"Case1.003.json");
    const parameter_descr_type third_cycle_parm_descr = test_utils::get_descriptor();
    struct parm third_cycle_file_data;

    grid_param_wrapper third_cycle_grid_param_wrap(third_cycle_file_data, third_cycle_parm_descr);
    data_parser.set_parse_type_info(third_cycle_parm_descr);
    grid_param_mapper third_cycle_map_grid_it(third_cycle_grid_param_wrap, data_parser);
    data_parser.parse_file(third_cycle_file);
    const auto third_cycle_idimr = data_parser.array_sizes_read();
    const auto third_cycle_inumr = data_parser.num_read(); {
        const auto third_cycle_out_idimr= acf_base_helper::convert(third_cycle_idimr);
        grid_param_reader read_hdf5_grid_it(third_cycle_file_data, grid_hdf5_writer);
        grid_hdf5_writer.append(third_cycle_parm_descr, third_cycle_inumr, third_cycle_out_idimr);
        grid_hdf5_writer.disconnect_slots();
    }

    const std::string expected_result_file(input_path + "/"+"Case1.json");
    struct parm expected_results;
    const parameter_descr_type expected_parm_descr = test_utils::get_descriptor();
    grid_param_wrapper expected_result_grid_param_wrap(expected_results, expected_parm_descr);
    data_parser.set_parse_type_info(expected_parm_descr);
    grid_param_mapper map_grid_it4(expected_result_grid_param_wrap, data_parser);
    data_parser.parse_file(expected_result_file);
    auto expected_idimr = data_parser.array_sizes_read();
    auto expected_inumr = data_parser.num_read();

    const parameter_descr_type read_parm_descr = test_utils::get_descriptor();
    struct parm hdf5_results;
    grid_param_wrapper grid_param_wrap(hdf5_results, read_parm_descr);
    gpm_hdf5_reader reader;
    reader.set_parse_type_info(read_parm_descr);
    grid_param_mapper map_grid_it(grid_param_wrap, reader);
    reader.parse_file(output_file);

    const int differences_count = test_utils::compare(expected_results, hdf5_results);
    if (differences_count == 0) {
        printf("Correct");
        return 0;
    }
    printf("Failed");
    return -1;
}
