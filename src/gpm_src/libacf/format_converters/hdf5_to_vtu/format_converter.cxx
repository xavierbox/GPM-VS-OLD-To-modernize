// -- Schlumberger Private --

#include "mapstc_descr.h"
#include "grid_param_mapper.h"
#include "vtk_writer_vtu.h"


using namespace Slb::Exploration::Gpm;


int main( int argc, char *argv[])
{
	const Tools::gpm_default_logger logger(Tools::LOG_NORMAL, true);
	// We need to find the old name, the new name 
	std::vector<std::string> program_options(argv, argv+argc);
	if (program_options.size() < 3){
		logger.print(Tools::LOG_IMPORTANT,"The format is: hdf5_file  vtu_file_name\n");
		return 1;
	}
	try {
		struct parm p;			// map file
		acf_base_reader::parameter_num_type inumr;
		acf_base_writer::array_dim_size_type idimr;
		acf_base_reader::array_dim_size_type in_idimr;

		const auto iret = gpm_read_map_file(program_options[1], "MAP", p, inumr, in_idimr, logger);
		if (iret != 0) {
			return iret;
		}
		const int_extent_2d model_extent(p.numrows(), p.numcols());
		gpm_model_definition my_model(model_extent);
		{
			my_model.set_world_coordinates(p.xcorners, p.ycorners);
		}
		auto param_list = p.propnam;
		param_list.erase(std::remove(param_list.begin(), param_list.end(), "TOP"), param_list.end());
        // Need two surfaces
		for (int i = 1; i < p.timeset.size(); ++i) {
			vtk_writer_vtu vtu_writer;
			const auto file_root_name = program_options[2] + std::string(".") + std::to_string(i);
			vtu_writer.set_write_info(file_root_name, &p, false,true);
			vtu_writer.set_properties_to_write(param_list);
			const auto ret = vtu_writer.write(i, my_model, logger);
			if (ret) {
				logger.print(Tools::LOG_NORMAL, "Error: Failed to generate complete set VTU files (Error = %d)\n", ret);
			}
		}
		return 0;
	}
    catch (...) {
		logger.print(Tools::LOG_NORMAL, "Error: Exception thrown. Failed to generate complete set VTU files \n");
		return 1;
    }
}
