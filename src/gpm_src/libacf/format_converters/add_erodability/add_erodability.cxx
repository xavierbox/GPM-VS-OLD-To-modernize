// /* Schlumberger Private */

#include "mapstc_descr.h"
#include "acf_base_helper.h"
#include "grid_param_mapper.h"
#include "gpm_control_file_io.h"
#include <boost/algorithm/string/predicate.hpp>
#include "gpm_parm_methods.h"
#include "grid_param_reader.h"
#include "gpm_hdf5_writer.h"
#include <memory>

using namespace Slb::Exploration::Gpm;


int main(int argc, char *argv[])
{
	// We need to find the old name, the new name 
	std::vector<std::string> program_options(argv, argv + argc);
	if (program_options.size() < 3) {
		printf("The format is: ctl file, map filename\n");
	}
    const Tools::gpm_default_logger logger(Tools::LogLevel::LOG_NORMAL, true);

	struct parm p;
	acf_base_reader::parameter_num_type inumr;
	acf_base_reader::array_dim_size_type in_idimr;
    auto map_file_name = program_options[2];
	auto iret = gpm_read_map_file(map_file_name, "MAP", p, inumr, in_idimr, logger);
	if (iret != 0) {
		return iret;
	}
	acf_base_writer::array_dim_size_type idimr = acf_base_helper::convert(in_idimr);

	if (inumr[BASE_ERODABILITY] == 0 || inumr[TIMESET] == 0 || inumr[AGESET] == 0) {
		// Need to get it from the ctl file
		gpm_control_file_io ctl_io;
		struct parc c;
		iret = ctl_io.read_parameters(program_options[1], logger, false);
		if (iret != 0) {
			return iret;
		}
		ctl_io.get_parms(false, &c);
		if (inumr[BASE_ERODABILITY] == 0) {
			p.base_erodability = c.baserod; // Simple copy for now
			inumr[PARM_MEMBERS::BASE_ERODABILITY] = 1;
			acf_base_helper::fill(&idimr[PARM_MEMBERS::BASE_ERODABILITY], 1);
		}
		if (inumr[TIMESET] == 0) {
			p.timeset.push_back(c.timbeg);
			inumr[TIMESET] = 1;
			acf_base_helper::fill(&idimr[PARM_MEMBERS::TIMESET], 1);
		}
		if (inumr[AGESET] == 0) {
			p.ageset.push_back(c.timbeg);
			inumr[AGESET] = 1;
			acf_base_helper::fill(&idimr[PARM_MEMBERS::AGESET], 1);
		}
	}
	auto erod_id = parm_descr_holder::get_grid_map_1_index_name_prefix(ERODABILITY_j);
	if (!std::any_of(p.propnam.begin(), p.propnam.end(), [&erod_id](const std::string& item) {return boost::istarts_with(item, erod_id); })) {
		// Lets add the base erodability
		std::vector<property_description> needed_properties;
		int n = std::accumulate(p.propnam.begin(), p.propnam.end(), 0, [](int init, const std::string& val) {return init + (boost::istarts_with(val, "SED") ? 1 : 0); });
		std::vector<std::string> erods;
		for (int i = 0; i < n; ++i) {
		    const auto sed_id = i+1;
			auto name = parm_descr_holder::grid_map_index_name(ERODABILITY_j, sed_id);
			erods.push_back(name);
			needed_properties.emplace_back(name, "Sediment erodibility " + std::to_string(sed_id), false);
		}
		std::map<std::string, bool> properties_created; // Maybe we need to know if they were created in this run?
		const std::map<PARM_GRID_MAP_NAMES, std::string>& prop_mapper(parm_descr_holder::get_grid_map_names());
		auto iret_vector = gpm_add_needed_properties(&p, needed_properties, &properties_created, logger);

        if (p.indprop_stats.empty()) {
			p.indprop_stats.resize(p.propnam.size(), p.numtims(), 2, true, p.znull);
        }
		else {
			p.indprop_stats.resize(p.propnam.size(), p.indprop_stats.rows(), p.indprop_stats.cols(), true, p.znull); // Min and max statistics
		}
	    // Now add the basement part
		for (auto& item : erods) {
			auto it = std::find(p.propnam.begin(), p.propnam.end(), item);
			auto dist = std::distance(p.propnam.begin(), it);
			p.indprop(dist,0, 0).set_constant(p.base_erodability);
		}
		for (int itim = 0; itim < p.numtims(); itim++) {
			calculate_statistics(&p, itim, logger);
		}
        // If we have more ages, should we install sediments
		const auto top_id = parm_descr_holder::get_grid_map_name(TOP);
		const auto top_it = std::find(p.propnam.begin(), p.propnam.end(), top_id);
		std::vector<int> diag_indexes;
		find_grid_diagonal_indexes(p, std::distance(p.propnam.begin(), top_it), &diag_indexes);
        if (diag_indexes.size() > 1 || (diag_indexes.size()==1 && diag_indexes.back()>0)) {
			logger.print(Tools::LOG_NORMAL, "Error: Do not handle multiple time right now '%s'\n", map_file_name.c_str());
			return 2;
        }
		adjust_dimensions(p, inumr, idimr, logger);
		auto out_file = std::unique_ptr<acf_base_writer>(new gpm_hdf5_writer());
		if (!out_file->set_write_info(map_file_name, parm_descr_holder::get_parm_descr(), inumr, idimr)) {
			logger.print(Tools::LOG_NORMAL, "Error: Unable to open output file '%s'\n", map_file_name.c_str());
			return 1;
		}
		grid_param_reader read_hdf5_grid_it(p, *out_file);
		out_file->write();
	}
}
