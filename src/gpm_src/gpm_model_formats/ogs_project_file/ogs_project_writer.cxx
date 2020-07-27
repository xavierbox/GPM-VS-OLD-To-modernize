#include "ogs_project_writer.h"
#include <boost/algorithm/string.hpp>

namespace Slb { namespace Exploration { namespace Gpm {

boost::property_tree::ptree ogs_project_writer::make_mesh_tree()
{
    boost::property_tree::ptree tree;
    std::vector<std::pair<std::string, std::string>> items;
    items.emplace_back("mesh", model_file_name);
    if (do_top_file) {
        std::string tmp = model_file_name;
        boost::replace_last(tmp, ".vtu", "_tops.vtu");
        items.emplace_back("mesh", tmp);
    }
    return make_simple_subtree(items);
}

boost::property_tree::ptree ogs_project_writer::make_liquid_flow_process()
{
	boost::property_tree::ptree tree;
	std::vector<std::pair<std::string, std::string>> items;
	items.emplace_back("name", "LiquidFlow");
	items.emplace_back("type", "LIQUID_FLOW");
	items.emplace_back("integration_order", "2");
	auto part1 = make_simple_subtree(items);
    // Darcy part
	const std::vector<std::pair<std::string, std::string>> garcy_items = { {"axis_id","2"}, {"g","9.81"} };
    const auto darcy_part = make_simple_subtree(garcy_items);
	part1.add_child("darcy_gravity", darcy_part);
	part1.add_child("process_variables", make_simple_subtree({ {"process_variable","pressure" }}));
	tree.add("secondary_variable.<xmlattr>.type", "static");
	tree.add("secondary_variable.<xmlattr>.internal_name", "darcy_velocity");
	tree.add("secondary_variable.<xmlattr>.output_name", "v");
	part1.add_child("secondary_variables", tree);
	tree.clear();
	boost::property_tree::ptree part2;
	part2.add_child("fluid.density", make_simple_subtree({ {"type","Constant"}, {"value","1.e3"} }));
	part2.add_child("fluid.viscosity", make_simple_subtree({ {"type","Constant"}, {"value","1.e3"} }));
	boost::property_tree::ptree part3;
	part3.add_child("permeability", 
		make_simple_subtree({ {"permeability_tensor_entries","PERMEABILITY_HORI"}, {"type","Constant"} }));
	part3.add_child("porosity",
		make_simple_subtree({ {"porosity_parameter","POR"}, {"type","Constant"} }));
	part3.add_child("storage",
		make_simple_subtree({ {"value","0.0"}, {"type","Constant"} }));
	boost::property_tree::ptree part4;
	part4.add_child("porous_medium", part3);
	part4.put("porous_medium.<xmlattr>.id", "0");
	part2.add_child("porous_medium", part4);
	part1.add_child("material_property", part2);
	return part1;
}

boost::property_tree::ptree ogs_project_writer::make_process_time_loop()
{
	boost::property_tree::ptree time_loop;
	boost::property_tree::ptree tree;
	tree.add("nonlinear_solver", "basic_picard");
    // Convergenece criteria
	std::vector<std::pair<std::string, std::string>> items;
	items.emplace_back("norm_type", "NORM2");
	items.emplace_back("type", "DeltaX");
	items.emplace_back("abstol", "1e-15");
    tree.add_child("convergence_criterion", make_simple_subtree(items));
    tree.add_child("time_discretization", make_simple_subtree({{"type", "BackwardEuler"}}));
    tree.add_child("time_stepping",
                   make_simple_subtree({
                       {"type", "FixedTimeStepping"}, {"t_initial", "0.0"}, {"t_end", "1"}
                   }));
    tree.add_child("time_stepping.timesteps.pair",
                   make_simple_subtree({
                       {"repeat", "1"}, {"delta_t", "1"}
                   }));
	boost::property_tree::ptree new_tree;
	new_tree.add_child("process", tree);
	new_tree.put("process.<xmlattr>.ref", "LiquidFlow");
	time_loop.add_child("processes", new_tree);
	time_loop.add("output.type", "VTK");
	time_loop.add("output.prefix", output_prefix); // Should have a number here
	time_loop.add_child("output.variables", make_simple_subtree({
		{"variable", "pressure"}, {"variable", "v"} }));
	return time_loop;
}

boost::property_tree::ptree ogs_project_writer::make_parameters()
{
    boost::property_tree::ptree tree;
    tree.add_child("parameter", make_simple_subtree({
                       {"name", "p0"}, {"type", "Constant"}, {"value", "0"}
                   }));
    tree.add_child("parameter", make_simple_subtree({
                       {"name", "PERMEABILITY_HORI"}, {"type", "MeshElement"}, {"field_name", "PERMEABILITY_HORI"}
                   }));
    tree.add_child("parameter", make_simple_subtree({
                       {"name", "POR"}, {"type", "MeshElement"}, {"field_name", "POR"}
                   }));

    tree.add_child("parameter", make_simple_subtree({
                       {"name", "water_pressure"}, {"type", "MeshNode"},
                       {"mesh", top_mesh_name}, {"field_name", "WATER_PRESSURE"}
                   }));
    return tree;
}

boost::property_tree::ptree ogs_project_writer::make_process_variables()
{
	boost::property_tree::ptree tree;

	std::vector<std::pair<std::string, std::string>> items;
	items.emplace_back("name", "pressure");
	items.emplace_back("mesh", model_name);
	items.emplace_back("components", "1");
	items.emplace_back("order", "1");
	items.emplace_back("initial_condition", "p0");
	tree.add_child("process_variable", make_simple_subtree(items));
	items.clear();
	items.emplace_back("type", "Dirichlet");
	items.emplace_back("mesh", top_mesh_name);
	items.emplace_back("parameter", "water_pressure");
	tree.add_child("process_variable.boundary_conditions.boundary_condition", make_simple_subtree(items));

	return tree;
}

boost::property_tree::ptree ogs_project_writer::make_non_linear_solvers()
{
	//<nonlinear_solvers>
	//	<nonlinear_solver>
	//	<name>basic_picard< / name>
	//	<type>Picard< / type>
	//	< max_iter>10 < / max_iter >
	//	<linear_solver>general_linear_solver< / linear_solver>
	//	< / nonlinear_solver>
	//	< / nonlinear_solvers>
	boost::property_tree::ptree tree;
	std::vector<std::pair<std::string, std::string>> items;
	items.emplace_back("name", "basic_picard");
	items.emplace_back("type", "Picard");
	items.emplace_back("max_iter", "10");
	items.emplace_back("linear_solver", "general_linear_solver");

	tree.add_child("nonlinear_solver", make_simple_subtree(items));
	return tree;
}

boost::property_tree::ptree ogs_project_writer::make_linear_solvers()
{
    std::vector<std::pair<std::string, std::string>> items;
	items.emplace_back("name", "general_linear_solver");
	items.emplace_back("lis", "-i cg - p jacobi - tol 1e-16 - maxiter 10000");
	boost::property_tree::ptree tree;
    tree.add_child("linear_solver",make_simple_subtree(items));
	items.clear();

	items.emplace_back("solver_type", "CG");
	items.emplace_back("precon_type", "DIAGONAL");
	items.emplace_back("max_iteration_step", "10000");
	items.emplace_back("error_tolerance", "1e-16");
	tree.add_child("linear_solver.eigen", make_simple_subtree(items));

	items.clear();
	items.emplace_back("prefix", "lf");
	items.emplace_back("parameters", "-lf_ksp_type cg -lf_pc_type bjacobi -lf_ksp_rtol 1e-16 -lf_ksp_max_it 10000");
	tree.add_child("linear_solver.petsc", make_simple_subtree(items));
	return tree;
}

void ogs_project_writer::generate_project_file(const std::string& file)
{
    // Do mesh part
    // Setup som estuff
	model_name = model_file_name;
	boost::replace_last(model_name, ".vtu", "");
	if (do_top_file) {
		top_mesh_file_name = model_file_name;
		boost::replace_last(top_mesh_file_name, ".vtu", "_tops.vtu");
		top_mesh_name = top_mesh_file_name;
		boost::replace_last(top_mesh_name, ".vtu", "");
	}
	auto mesh_tree = make_mesh_tree();
	auto processes_tree = make_liquid_flow_process();
	auto time_loop_tree = make_process_time_loop();
	auto parameter_tree = make_parameters();
	auto process_vars_tree = make_process_variables();
	auto non_linear_tree = make_non_linear_solvers();
	auto linear_tree = make_linear_solvers();
	boost::property_tree::ptree tree;
	tree.add_child("OpenGeoSysProject.meshes", mesh_tree);
	tree.add_child("OpenGeoSysProject.processes.process", processes_tree);
	tree.add_child("OpenGeoSysProject.time_loop", time_loop_tree);
	tree.add_child("OpenGeoSysProject.parameters", parameter_tree);
	tree.add_child("OpenGeoSysProject.process_variables", process_vars_tree);
	tree.add_child("OpenGeoSysProject.nonlinear_solvers", non_linear_tree);
	tree.add_child("OpenGeoSysProject.linear_solvers", linear_tree);
	boost::property_tree::xml_writer_settings<boost::property_tree::ptree::key_type> setting;
	setting.indent_count = 2;
	boost::property_tree::write_xml(file, tree, std::locale(), setting);
}

boost::property_tree::ptree ogs_project_writer::make_simple_subtree(const std::string& attr,
    const std::vector<std::pair<std::string, std::string>>& items)
{
	boost::property_tree::ptree tree;
    for(auto& item:items) {
		std::string attr_loc = attr + "." + item.first;
		tree.add(attr_loc, item.second);
    }
	return tree;
}

boost::property_tree::ptree ogs_project_writer::make_simple_subtree(const std::vector<std::pair<std::string, std::string>>& items)
{
	boost::property_tree::ptree tree;
	for (auto& item : items) {
		std::string attr_loc = item.first;
		tree.add(attr_loc, item.second);
	}
	return tree;
}

void ogs_project_writer::add_to_subtree(boost::property_tree::ptree& pt, const std::string& attr,
    const std::vector<std::pair<std::string, std::string>>& items)
{
	auto process = *pt.begin();
	std::string id = process.first;
	std::string head = id + "." + attr;
	for (auto& item : items) {
		std::string attr_loc = head + "." + item.first;
		pt.put(attr_loc, item.second);
	}
}

boost::property_tree::ptree ogs_project_writer::make_timestepping(const std::string& type)
{
    boost::property_tree::ptree tree;
    tree.put("time_stepping.type", type);
    return tree;
}

boost::property_tree::ptree ogs_project_writer::time_discretization(const std::string& type)
{
	boost::property_tree::ptree tree;
	tree.put("time_discretization.type", type);
	return tree;
}

inline boost::property_tree::ptree ogs_project_writer::convergence_criteria(const std::string& type)
{
	//<convergence_criterion>
	//	<type>DeltaX< / type>
	//	<norm_type>NORM2< / norm_type>
	//	<abstol>1e-15< / abstol>
	//	< / convergence_criterion>
	//
	boost::property_tree::ptree tree;
	tree.put("convergence_criterion.type", "DeltaX");
	tree.put("convergence_criterion.norm_type", "NORM2");
	tree.put("convergence_criterion.norm_type", "1e-15");
	return tree;

}

}}}
