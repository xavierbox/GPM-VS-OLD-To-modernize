// /* Schlumberger Private */

#include "mapstc_descr.h"
#include "acf_base_helper.h"
#include "grid_param_mapper.h"
#include "gpm_control_file_io.h"
#include "gpm_parm_methods.h"
#include "grid_param_reader.h"
#include "gpm_hdf5_writer.h"
#include "gpm_parm_helper_methods.h"
#include <boost/algorithm/string/predicate.hpp>
#include <memory>

using namespace Slb::Exploration::Gpm;


int main(int argc, char *argv[])
{
	// We need to find the old name, the new name 
	std::vector<std::string> program_options(argv, argv + argc);
	if (program_options.size() < 2) {
		printf("The format is: map filename\n");
	}
    const Tools::gpm_default_logger logger(Tools::LogLevel::LOG_NORMAL, true);

	struct parm p;
	acf_base_reader::parameter_num_type inumr;
	acf_base_reader::array_dim_size_type in_idimr;
    auto map_file_name = program_options[1];
	auto iret = gpm_read_map_file(map_file_name, "MAP", p, inumr, in_idimr, logger);
	if (iret != 0) {
		return iret;
	}
	acf_base_writer::array_dim_size_type idimr = acf_base_helper::convert(in_idimr);

	if (inumr[TOP_SLOPE_X] != 0 && inumr[TOP_SLOPE_Y] != 0) {
		// Find TOP_SLOPE_X and TOP_SLOPE_Y
		auto top_x = parm_descr_holder::get_grid_map_name(TOP_SLOPE_X);
		auto top_y = parm_descr_holder::get_grid_map_name(TOP_SLOPE_Y);
		auto x_index = std::distance(p.propnam.begin(), std::find(p.propnam.begin(), p.propnam.end(), top_x));
		auto y_index = std::distance(p.propnam.begin(), std::find(p.propnam.begin(), p.propnam.end(), top_y));
		std::vector<int> x_index_vec;
		std::vector<int> y_index_vec;
		find_grid_diagonal_indexes(p, x_index, &x_index_vec);
		find_grid_diagonal_indexes(p, y_index, &y_index_vec);
		for (int t = 0; t < p.timeset.size(); ++t) {
			if (x_index_vec[t] != 0) {
				auto holder = p.indprop(x_index, t, x_index_vec[t]);
				p.indprop(x_index, t, 0) = holder;
				for (auto age = 1; age <= x_index_vec[t]; ++age) {
					p.indprop(x_index, t, age) = function_2d();
				}
			}
			if (y_index_vec[t] != 0) {
				auto holder = p.indprop(y_index, t, y_index_vec[t]);
				p.indprop(y_index, t, 0) = holder;
				for (auto age = 1; age <= y_index_vec[t]; ++age) {
					p.indprop(y_index, t, age) = function_2d();
				}
			}
		}


		for (int itim = 0; itim < p.numtims(); itim++) {
			calculate_statistics(&p, itim, logger);
		}

		adjust_dimensions(p, inumr, idimr, logger);
		auto out_file = std::unique_ptr<acf_base_writer>(new gpm_hdf5_writer());
		out_file->open_file(map_file_name, true);
		out_file->set_write_info(parm_descr_holder::get_parm_descr(), inumr, idimr);

		grid_param_reader read_hdf5_grid_it(p, *out_file);
		out_file->write();
	}
}
