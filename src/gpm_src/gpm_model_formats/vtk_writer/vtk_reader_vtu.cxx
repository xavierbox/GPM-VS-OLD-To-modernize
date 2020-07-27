// -- Schlumberger Private --

#include "mapstc.h"

#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPointData.h>
#include <vtkCellArray.h>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkPoints.h>
#include <vtkQuad.h>
#include <vtkCellDataToPointData.h>

#include <string>
#include <numeric>
#include <algorithm>

#include "vtk_reader_vtu.h"
#include "gpm_parm_methods.h"
#include "vtk_writer_vtu.h"

using namespace std;

namespace Slb { namespace Exploration { namespace Gpm {


enum number_of_points_for_cell { n_tetra = 4, n_pyramid  = 5, n_wedge = 6, n_hexahedron = 8};

using namespace Tools;


vtk_reader_vtu::vtk_reader_vtu() : m_parm_(nullptr) {
}


int vtk_reader_vtu::reader(const std::string& prop_name,/* const gpm_model_definition& model,*/ std::vector<double>& prop_array, const Slb::Exploration::Gpm::Tools::gpm_logger& logger)  {

	//read all the data from the file
	auto reader = vtkSmartPointer<vtkXMLUnstructuredGridReader>::New();
	reader->SetFileName(m_file_name_.c_str());
	if (!reader->CanReadFile(m_file_name_.c_str())) {
		logger.print(LOG_NORMAL, "Error: Unable to open output file '%s'\n", m_file_name_.c_str());
		return 1;
	}
	reader->Update();

	auto* ugrid = reader->GetOutput();

	for (vtkIdType i = 0; i < ugrid->GetPointData()->GetNumberOfArrays(); i++)
	{
		auto arr = ugrid->GetPointData()->GetArray(i);
		auto n_comp = ugrid->GetPointData()->GetArray(i)->GetNumberOfComponents();
		prop_array.reserve(ugrid->GetPointData()->GetNumberOfTuples());
		if (n_comp == 1 && prop_name.compare(ugrid->GetPointData()->GetArrayName(i)) == 0)  {
			for (auto ii = 0; ii < arr->GetNumberOfTuples(); ii++)
			{
				prop_array.emplace_back(arr->GetTuple1(ii));
			}
		}
	}
	return 0;
}

bool vtk_reader_vtu::get_ogs_result_property(const gpm_model_definition& model,
                                             const gpm_logger& logger, const std::vector<property_stack_holder>& stack, const std::string& root_fil_name, const std::string& ogs_binary) {
 
    logger.print(LOG_VERBOSE, "\n--------------------------------------------------------\n");
    logger.print(LOG_VERBOSE, "Running OpenGeoSys with projectfile: %s\n", m_file_name_.c_str());

    std::stringstream command_line;
    command_line << ogs_binary.c_str() << " " << m_file_name_.c_str();
    logger.print(LOG_VERBOSE, "Command line: %s\n", command_line.str().c_str());

    // run OGS
#ifdef WIN32
    system(command_line.str().c_str());
#endif

    // Read back property
	std::vector<double> prop_array;
    // compute ogs results filename
    auto ogs_file_result_name = m_file_name_;
    auto startpos = ogs_file_result_name.find(".prj");
    ogs_file_result_name.erase(startpos, 4);
    ogs_file_result_name += "_pcs_0_ts_1_t_1.000000.vtu";
    logger.print(LOG_VERBOSE, "OpenGeoSys resultsfile: %s\n", ogs_file_result_name.c_str());

    auto origin_file_result_name = root_fil_name;
    origin_file_result_name += "_origional.vtu";
    logger.print(LOG_VERBOSE, "Origin resultsfile: %s\n", origin_file_result_name.c_str());


    // need to access mapping in origional vtu file 
    auto org_reader = vtkSmartPointer<vtkXMLUnstructuredGridReader>::New();
    org_reader->SetFileName(origin_file_result_name.c_str());
    if (!org_reader->CanReadFile(origin_file_result_name.c_str())) {
        logger.print(LOG_NORMAL, "Error: Unable to open output file '%s'\n", origin_file_result_name.c_str());
        return true;
    }
    org_reader->Update();
    auto* org_ugrid = org_reader->GetOutput();
    std::vector<int> org_mapping;
    for (vtkIdType i = 0; i < org_ugrid->GetPointData()->GetNumberOfArrays(); i++)
    {
        auto arr = org_ugrid->GetPointData()->GetArray(i);
        std::string comp_name = org_ugrid->GetPointData()->GetArrayName(i);
        auto n_comp = org_ugrid->GetPointData()->GetArray(i)->GetNumberOfComponents();

        if (comp_name == "OrgToCompressMapping") {
            logger.print(LOG_VERBOSE, "\nPropertyname: %s\n", comp_name.c_str());
            logger.print(LOG_VERBOSE, "Number Of Elements: %u\n", arr->GetNumberOfTuples());
            logger.print(LOG_VERBOSE, "Number Of Comp: %u\n", n_comp);
            org_mapping.reserve(arr->GetNumberOfTuples());
            if (n_comp == 1) {
                for (auto ii = 0; ii < arr->GetNumberOfTuples(); ii++)
                {
                    org_mapping.emplace_back(arr->GetTuple1(ii));
                }
            }
        }

    }

    auto ogs_reader = vtkSmartPointer<vtkXMLUnstructuredGridReader>::New();
    ogs_reader->SetFileName(ogs_file_result_name.c_str());
    if (!ogs_reader->CanReadFile(ogs_file_result_name.c_str())) {
        logger.print(LOG_NORMAL, "Error: Unable to open output file '%s'\n", ogs_file_result_name.c_str());
        return true;
    }
    ogs_reader->Update();

    auto* ogs_ugrid = ogs_reader->GetOutput();

    auto n_origional = org_mapping.size();
    auto n_compressed = *max_element(org_mapping.begin(), org_mapping.end()) +1;
    logger.print(LOG_VERBOSE, "Max Compressed element: %u Max UnCompressed element: %u\n", n_compressed, n_origional);


    for (vtkIdType i = 0; i < ogs_ugrid->GetPointData()->GetNumberOfArrays(); i++)
    {
        auto n_comp = ogs_ugrid->GetPointData()->GetArray(i)->GetNumberOfComponents();
        std::string comp_name = ogs_ugrid->GetPointData()->GetArrayName(i);

        if (comp_name == stack.back().name && n_comp == 1) {
            auto arr = ogs_ugrid->GetPointData()->GetArray(i);
            logger.print(LOG_VERBOSE, "\nPropertyname: %s\n", comp_name.c_str());
            logger.print(LOG_VERBOSE, "Number Of Elements: %u\n", arr->GetNumberOfTuples());
            logger.print(LOG_VERBOSE, "Number Of Comp: %u\n", n_comp);

            prop_array.resize(n_origional);
            std::fill(prop_array.begin(), prop_array.end(), -1.0);
            std::vector<double> my_prop(n_compressed, -1.0);

            if (n_comp == 1) {
                auto indx = 0;
                for (auto& p : my_prop) {
                    p = arr->GetTuple1(indx);
                    indx++;
                }
                auto max_value = *max_element(my_prop.begin(), my_prop.end());
                auto min_value = *min_element(my_prop.begin(), my_prop.end());
                auto avg_value = accumulate(my_prop.begin(), my_prop.end(), 0.0) / my_prop.size();
                logger.print(LOG_VERBOSE, "(Compressed grid): Max property value : %f Min property value: %f Avg property value: %f\n", max_value, min_value, avg_value);

                auto org_indx = 0;
                for (auto& p : prop_array)
                {
                    if (org_mapping[org_indx] > -1) {

                       p = my_prop[org_mapping[org_indx]];
                    }
                    org_indx++;
                }

                max_value = *max_element(prop_array.begin(), prop_array.end());
                min_value = *min_element(prop_array.begin(), prop_array.end());
                auto n_undef = std::count(prop_array.begin(), prop_array.end(), -1.0);
                logger.print(LOG_VERBOSE, "(Origin grid): Max property value: %f Min property value: %f Number of undef %u\n", max_value, min_value, n_undef);
                std::replace(prop_array.begin(), prop_array.end(), -1.0, avg_value);
                
                auto row_ext = model.model_extent().row_extent();
				auto col_ext = model.model_extent().col_extent();
                auto ncols = col_ext.size();
                auto nrows = row_ext.size();
                auto& gpm_array_2d_bases = stack.back().stack;
                if (prop_array.size() != (static_cast<unsigned int>(gpm_array_2d_bases.size()) * nrows * ncols)) {
                    logger.print(
                        LOG_NORMAL,
                        "Number pressure nodes is not matching the GPM grid(number of pressure nodes %u --> GPM nodes %u) \n",
                        prop_array.size(), static_cast<unsigned int>(gpm_array_2d_bases.size()) * nrows * ncols);
                }

				for (auto layer = 0; layer < gpm_array_2d_bases.size(); layer++)
				{
					for (auto i : row_ext) {
						for (auto j : col_ext) {
							const auto indx = layer * nrows * ncols + i * ncols + j;
							gpm_array_2d_bases[layer]->at(i, j) = static_cast<float> (prop_array.at(indx));
						}
					}
				}

            }
        }
    }
    
    logger.print(LOG_VERBOSE, "\n--------------------------------------------------------\n");
    return false;
}

void vtk_reader_vtu::set_read_info(const std::string& fil_name, grid_parameter_type* param) {
    m_file_name_ = fil_name;
    m_parm_ = param;
}



int vtk_reader_vtu::get_data_for_file(const std::string& fil_name,
     std::vector<boost::geometry::model::point <double, 3, boost::geometry::cs::cartesian> >& gpoints,
     std::vector<std::vector<int> >& cellindexes,
     std::vector<std::string>& propname,
     std::vector<std::vector<double> >& props) {

     //read all the data from the file
     auto reader = vtkSmartPointer<vtkXMLUnstructuredGridReader>::New();
     reader->SetFileName(fil_name.c_str());
     reader->Update();

     auto* ugrid = reader->GetOutput();

     // extract geometry points
     for (auto i = 0; i < ugrid->GetNumberOfPoints(); i++)
     {
         const auto pd = ugrid->GetPoints()->GetPoint(i);
         point pp = point(pd[0], pd[1], pd[2]);
         gpoints.emplace_back(pp);
     }

     auto *cellPointIds = vtkIdList::New();
     for (vtkIdType cell_id = 0; cell_id < ugrid->GetNumberOfCells(); ++cell_id)
     {
         ugrid->GetCellPoints(cell_id, cellPointIds);
         const auto cell_size = cellPointIds->GetNumberOfIds();
         auto *cell_ptr = cellPointIds->GetPointer(0);

         std::vector<int> ci(cell_size, 0);
         for (auto pts = 0; pts < cell_size; ++pts)  {
             ci.at(pts) = static_cast<int> (cell_ptr[pts]);
         }
         cellindexes.emplace_back(ci);
     }
     cellPointIds->Delete();

     const auto number_of_point_arrays = ugrid->GetPointData()->GetNumberOfArrays();
	 props.reserve(number_of_point_arrays);
     for (vtkIdType i = 0; i < number_of_point_arrays; i++)
     {
         propname.emplace_back(ugrid->GetPointData()->GetArrayName(i));

         auto arr = ugrid->GetPointData()->GetArray(i);
		 std::vector<double>  prop;
		 prop.reserve(ugrid->GetPointData() ->GetNumberOfTuples());
		 for (auto ii = 0; ii < arr->GetNumberOfTuples(); ii++)
         {
             prop.emplace_back(arr->GetTuple1(ii));
         }
         props.emplace_back(prop);
     }

     return 0;
 }

 int vtk_reader_vtu::do_compare_files(const std::string& fil_name_1,
     const std::string& fil_name_2,
     const Slb::Exploration::Gpm::Tools::gpm_logger& logger) {

     // file 1
     vec_point gpoints_1;
     std::vector<std::vector<int> > cellindexes_1;
     std::vector<std::string> propname_1;
     std::vector<std::vector<double> > props_1;
     if (!get_data_for_file(fil_name_1, gpoints_1, cellindexes_1, propname_1, props_1)) {
         logger.print(LOG_VERBOSE, "File %s : NPoints %u NCellInd %u NProps %u \n", fil_name_1.c_str(), gpoints_1.size(), cellindexes_1.size(), propname_1.size());
         logger.print(LOG_VERBOSE, "Including properties: \n");
         for (auto &pn : propname_1) { logger.print(LOG_NORMAL, " --> %s \n", pn.c_str()); }
     }
     else {
         logger.print(LOG_NORMAL, "Reading File %s failed\n", fil_name_1.c_str());
     }
     logger.print(LOG_NORMAL, "\n");

     // file 2
     vec_point gpoints_2;
     std::vector<std::vector<int> > cellindexes_2;
     std::vector<std::string> propname_2;
     std::vector<std::vector<double> > props_2;
     if (!get_data_for_file(fil_name_2, gpoints_2, cellindexes_2, propname_2, props_2)) {
         logger.print(LOG_VERBOSE, "File %s : NPoints %u NCellInd %u NProps %u \n", fil_name_2.c_str(), gpoints_2.size(), cellindexes_2.size(), propname_2.size());
         logger.print(LOG_NORMAL, "Including properties: \n");
         for (auto &pn : propname_2) { logger.print(LOG_NORMAL, " --> %s \n", pn.c_str()); }
     }
     else {
         logger.print(LOG_NORMAL, "Reading File %s failed\n", fil_name_2.c_str());
     }
     logger.print(LOG_NORMAL, "\n");

     // compare point geometry
     if (gpoints_2.size() == gpoints_1.size()) {
         auto n_diff_pnts = 0;
         auto ind = 0;
         for (auto& pnt : gpoints_1) {
             n_diff_pnts += (boost::geometry::distance(pnt, gpoints_2[ind]) != 0.0) ? 1 : 0;
             ind++;
         }
         logger.print(LOG_NORMAL, "Number Of deviating points in grid = %u \n", n_diff_pnts);
     }
     else {
         return 1;
     }

     if (cellindexes_1.size() == cellindexes_2.size()) {
         auto n_diff_cell = 0;
         auto ind = 0;
         for (auto& cpt : cellindexes_1) {
             n_diff_cell += (equal(cpt.begin(), cpt.end(), cellindexes_2[ind].begin())) ? 0 : 1;
             ind++;
         }
         logger.print(LOG_NORMAL, "Number Of deviating cell indexes = %u \n", n_diff_cell);

     }
     else {
         return 2;
     }


     return 0;
 }
}}}

