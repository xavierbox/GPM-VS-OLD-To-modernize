#ifndef OGS_PROJECT_WRITER_H
#define OGS_PROJECT_WRITER_H
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <string>

namespace Slb { namespace Exploration { namespace Gpm {
class ogs_project_writer {
public:
	std::string model_file_name="seaonlyc1.hdf5.20.vtu";
	bool do_top_file=true;
	bool do_right_side_file;
	bool do_left_side_file;
	bool do_upper_side_file;
	bool do_lower_side_file;

	std::string output_prefix = "isotropic_gravity_driven3D";

	std::string top_mesh_file_name;
	std::string top_mesh_name;
	std::string model_name;

	boost::property_tree::ptree make_mesh_tree();
	boost::property_tree::ptree make_liquid_flow_process();
	boost::property_tree::ptree make_process_time_loop();
	boost::property_tree::ptree make_parameters();
	boost::property_tree::ptree make_process_variables();
	boost::property_tree::ptree make_non_linear_solvers();
	boost::property_tree::ptree make_linear_solvers();
	void generate_project_file(const std::string& file);
    // Need better typing of these
	static boost::property_tree::ptree make_simple_subtree(const std::string& attr, const std::vector<std::pair<std::string, std::string>>& items);
    boost::property_tree::ptree make_simple_subtree(const std::vector<std::pair<std::string, std::string>>& items);
    static void add_to_subtree(boost::property_tree::ptree& pt, const std::string& attr, const std::vector<std::pair<std::string, std::string>>& items);
    static boost::property_tree::ptree make_timestepping(const std::string& type);
	static boost::property_tree::ptree time_discretization(const std::string& type);
	static boost::property_tree::ptree convergence_criteria(const std::string& type);
};



}}}
#endif