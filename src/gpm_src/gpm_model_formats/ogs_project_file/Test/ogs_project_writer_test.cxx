// -- Schlumberger Private --

#include "ogs_project_writer.h"
#include <vector>
#include <string>

using namespace Slb::Exploration::Gpm;


int Test_ogs_project_writer_simple(int, char *[]) {

	ogs_project_writer tmp;
	std::vector<std::pair<std::string, std::string>> items = { std::make_pair("name", "GW23"), std::make_pair("type","GROUNDWATER_FLOW") };

	auto a = tmp.make_simple_subtree("process", items);
	auto aa = *a.begin();
	auto bb = aa.first;
	auto tes = aa.second.empty();
	std::vector<std::pair<std::string, std::string>> sub_items = { std::make_pair("process_variable","pressure"), std::make_pair("process_variable", "temp") };
	auto b = tmp.make_simple_subtree(sub_items);
	auto hopp = a.find("process");
    for(auto ii:a) {
		auto vv = ii.first;
		auto vvv = ii.second.data();
		int tull = 0;
    }
	//a.push_back(std::make_pair("", b));
	a.put_child("process.process_variables", b);
	auto isd = *a.begin();
	auto next = isd.second;
	//a.insert(a.end(), *b.begin());
	//tmp.add_to_subtree(a, "process_variables", sub_items);
	//a.add("process.process_variables.process_variable", sub_items.front().second);
	boost::property_tree::ptree res;
	auto pp = tmp.make_liquid_flow_process();
	res.add_child("OpenGeoSysProject.process", pp);
	boost::property_tree::write_xml("test1.xml", res);
	ogs_project_writer test2;
    // Set these two for each time, and get the right files
	test2.model_file_name = "seaonlyc1.hdf5.20.vtu";
	test2.output_prefix = "seaonlyc1_20";
	test2.generate_project_file("test.prj");
    return 0;
}



