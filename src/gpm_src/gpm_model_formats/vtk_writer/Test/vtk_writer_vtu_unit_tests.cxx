#include <array>
#include <map>
#include "vtk_writer_vtu.h"
#include "gpm_model_converter.h"

bool check_instance(const std::vector<std::vector<int>>& vec, const std::vector<std::vector<int>>& oracle )
{
	if (vec.size() != oracle.size()) {
		return false;
	}
	for (int i = 0; i < vec.size(); ++i) {
		if (vec[i].size() != oracle[i].size()) {
			return false;
		}
		for (int j = 0; j < vec[i].size(); ++j) {
			if (vec[i][j] != oracle[i][j]) {
				return false;
			}
		}
	}
	return true;
}

int Test_vtu_configs(int argc, char* argv[])
{
    // Do category 7, with a wedge and a pyramid
    // List list is the wedge, second is the pyramid
	std::array<unsigned long, 4> bottom = { 0,1,2,3 };
	std::array<unsigned long, 4> top = { 4,5,6,7 };
	std::map<unsigned long, unsigned long> map_to_prev;
	Slb::Exploration::Gpm::vtk_writer_vtu::vec_point test;
	map_to_prev.clear();
	map_to_prev[4] = 0; // Origin the same
	const auto transfer_func = [&map_to_prev](unsigned long ind) {return (map_to_prev.find(ind) == map_to_prev.end()) ? ind : map_to_prev.at(ind); };
	std::array<unsigned long, 4> top_mapped{};
	std::transform(top.begin(), top.end(), top_mapped.begin(), transfer_func);

    const auto res1 = Slb::Exploration::Gpm::gpm_model_converter::check_cell_pointers(test, bottom, top_mapped);
    const std::vector<std::vector<int>> oracle1 = { {5,6,7, 1,2,3},{1,5,7,3,0} };
    if (!check_instance(res1, oracle1)) {
		return 1;
    }
	map_to_prev.clear();
	map_to_prev[5] = 1;
	std::transform(top.begin(), top.end(), top_mapped.begin(), transfer_func);
	const auto res2 = Slb::Exploration::Gpm::gpm_model_converter::check_cell_pointers(test, bottom, top_mapped);
    const std::vector<std::vector<int>> oracle2 = { {4,6,7,0,2,3},{0,2,6,4,1} };
	if (!check_instance(res2, oracle2)) {
		return 1;
	}
	map_to_prev.clear();
	map_to_prev[6] = 2;
	std::transform(top.begin(), top.end(), top_mapped.begin(), transfer_func);
	const auto res3 = Slb::Exploration::Gpm::gpm_model_converter::check_cell_pointers(test, bottom, top_mapped);
    const std::vector<std::vector<int>> oracle3 = { {4,5,7,0,1,3},{1,3,7,5,2} };
	if (!check_instance(res3, oracle3)) {
		return 1;
	}
	map_to_prev.clear();
	map_to_prev[7] = 3;
	std::transform(top.begin(), top.end(), top_mapped.begin(), transfer_func);
	auto res4 = Slb::Exploration::Gpm::gpm_model_converter::check_cell_pointers(test, bottom, top_mapped);
    const std::vector<std::vector<int>> oracle4 = { {4,5,6,0,1,2},{0,4,6,2,3} };
	if (!check_instance(res4, oracle4)) {
		return 1;
	}
	return 0;
}

int Test_wedge_configs(int argc, char* argv[])
{
	// Do category 7, with a wedge and a pyramid
	// List list is the wedge, second is the pyramid
    const std::array<unsigned long, 4> bottom = { 0,1,2,3 };
    const std::array<unsigned long, 4> top = { 4,5,6,7 };
	std::map<unsigned long, unsigned long> map_to_prev;
	Slb::Exploration::Gpm::vtk_writer_vtu::vec_point test;
	map_to_prev.clear();
	map_to_prev[4] = 0; // Origin the same
	map_to_prev[5] = 1;
	const auto transfer_func = [&map_to_prev](unsigned long ind) {return (map_to_prev.find(ind) == map_to_prev.end()) ? ind : map_to_prev.at(ind); };
	std::array<unsigned long, 4> top_mapped{};
	std::transform(top.begin(), top.end(), top_mapped.begin(), transfer_func);

	const auto res1 = Slb::Exploration::Gpm::gpm_model_converter::check_cell_pointers(test, bottom, top_mapped);
	const std::vector<std::vector<int>> oracle1 = { {1,2,6,0,3,7} };
	if (!check_instance(res1, oracle1)) {
		return 1;
	}
	map_to_prev.clear();
	map_to_prev[5] = 1;
	map_to_prev[6] = 2;
	std::transform(top.begin(), top.end(), top_mapped.begin(), transfer_func);
	const auto res2 = Slb::Exploration::Gpm::gpm_model_converter::check_cell_pointers(test, bottom, top_mapped);
	const std::vector<std::vector<int>> oracle2 = { {2,3,7,1,0,4}};
	if (!check_instance(res2, oracle2)) {
		return 1;
	}
	map_to_prev.clear();
	map_to_prev[6] = 2;
	map_to_prev[7] = 3;
	std::transform(top.begin(), top.end(), top_mapped.begin(), transfer_func);
	const auto res3 = Slb::Exploration::Gpm::gpm_model_converter::check_cell_pointers(test, bottom, top_mapped);
	const std::vector<std::vector<int>> oracle3 = { {3,0,4,2,1,5} };
	if (!check_instance(res3, oracle3)) {
		return 1;
	}
	map_to_prev.clear();
	map_to_prev[7] = 3;
	map_to_prev[4] = 0;
	std::transform(top.begin(), top.end(), top_mapped.begin(), transfer_func);
	auto res4 = Slb::Exploration::Gpm::gpm_model_converter::check_cell_pointers(test, bottom, top_mapped);
	const std::vector<std::vector<int>> oracle4 = { {0,1,5,3,2,6} };
	if (!check_instance(res4, oracle4)) {
		return 1;
	}
	return 0;
}
int Test_tetra_configs(int argc, char* argv[])
{
	// Do category 7, with a wedge and a pyramid
	// List list is the wedge, second is the pyramid
	std::array<unsigned long, 4> bottom = { 0,1,2,3 };
	std::array<unsigned long, 4> top = { 4,5,6,7 };
	std::map<unsigned long, unsigned long> map_to_prev;
    const Slb::Exploration::Gpm::vtk_writer_vtu::vec_point test;
	map_to_prev.clear();
	map_to_prev[4] = 0; // Origin the same
	map_to_prev[6] = 2;
	const auto transfer_func = [&map_to_prev](unsigned long ind) {return (map_to_prev.find(ind) == map_to_prev.end()) ? ind : map_to_prev.at(ind); };
	std::array<unsigned long, 4> top_mapped{};
	std::transform(top.begin(), top.end(), top_mapped.begin(), transfer_func);
	const auto res1 = Slb::Exploration::Gpm::gpm_model_converter::check_cell_pointers(test, bottom, top_mapped);
	const std::vector<std::vector<int>> oracle1 = { {0,1,2,5},{0,2,3,7} };
	if (!check_instance(res1, oracle1)) {
		return 1;
	}
	map_to_prev.clear();
	map_to_prev[5] = 1;
	map_to_prev[7] = 3;
	std::transform(top.begin(), top.end(), top_mapped.begin(), transfer_func);
	const auto res2 = Slb::Exploration::Gpm::gpm_model_converter::check_cell_pointers(test, bottom, top_mapped);
	const std::vector<std::vector<int>> oracle2 = { {0,1,3,4},{1,2,3,6} };
	if (!check_instance(res2, oracle2)) {
		return 1;
	}
	return 0;
}
