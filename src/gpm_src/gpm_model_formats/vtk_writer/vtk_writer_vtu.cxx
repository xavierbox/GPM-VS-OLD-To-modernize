// -- Schlumberger Private --

#include "vtk_writer_vtu.h"
#include "gpm_parm_methods.h"
#include "gpm_model_converter.h"

#include "mapstc.h"

#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>
#include <vtkCellArray.h>
#include <vtkAppendFilter.h>
#include <vtkXMLUnstructuredGridWriter.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkXMLDataSetWriter.h>
#include <vtkPyramid.h>
#include <vtkTetra.h>
#include <vtkHexahedron.h>
#include <vtkQuad.h>
#include <vtkTriangle.h>
#include <vtkLine.h>
#include <vtkVertex.h>
#include <vtkWedge.h>
#include <vtkCellDataToPointData.h>
#include <vtkDataArray.h>	 	
#include <vtkCellData.h>
#include <vtkDoubleArray.h>
#include <vtkUnsignedLongLongArray.h>
#include <vtkLongLongArray.h>

#include <boost/range/combine.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <string>
#include <numeric>
#include <algorithm>
#include <iterator>
#include <map>
#include <vector>


using namespace std;
namespace bg = boost::geometry;

namespace Slb { namespace Exploration { namespace Gpm {


unsigned int is_tetra(unsigned int i) { return (i == VTK_TETRA); }
unsigned int is_pyramid(unsigned int i) { return (i == VTK_PYRAMID); }
unsigned int is_wedge(unsigned int i) { return (i == VTK_WEDGE); }
unsigned int is_hexahedron(unsigned int i) { return (i == VTK_HEXAHEDRON); }
unsigned int n_valid_pnts(unsigned int i) { return (i != -1); }
unsigned int is_triag(unsigned int i) { return (i == -1); }

using namespace Tools;


template<typename T> vtkSmartPointer<T> set_cell_id(vtkSmartPointer<T> geo_cell, vector<int>& point_ind)
{
	for (size_t i = 0; i < point_ind.size(); i++)
		geo_cell->GetPointIds()->SetId(i, point_ind[i]);

	return geo_cell;
}

void convert_node_to_cell_props(const std::vector<double>& node_props, vtkSmartPointer<vtkUnstructuredGrid>& dataset, std::vector<double>& cell_props) {

    vtkIdList *cellPointIds = vtkIdList::New();
    auto ncell = 0;
    cell_props.reserve(dataset->GetNumberOfCells());
    for (vtkIdType cellId = 0; cellId < dataset->GetNumberOfCells(); ++cellId)
    {
        dataset->GetCellPoints(cellId, cellPointIds);
        const auto cell_size = cellPointIds->GetNumberOfIds();
        auto *cell_ptr = cellPointIds->GetPointer(0);

        // generate cumulative value over each cell ptr
        auto val = 0.0;
        for (auto pts = 0; pts < cell_size; ++pts)
        {
            const auto cell_ptr_ind = cell_ptr[pts];
            val += node_props.at(cell_ptr_ind);
        }
        // set average value
        val = val / cell_size;
        cell_props.emplace_back(val);
        ncell++;
    }
	cellPointIds->Delete();
}

bool save_cell_props(vtkSmartPointer<vtkCellArray>& cell_array, std::vector<int>& cell_types, std::vector<int>& cp)
{
	auto vali_type = true;
    if (cp.size() == gpm_model_converter::n_hexahedron) {
		cell_types.push_back(VTK_HEXAHEDRON);
		cell_array->InsertNextCell(set_cell_id(vtkSmartPointer<vtkHexahedron>::New(), cp));
    }
    else if (cp.size() == gpm_model_converter::n_pyramid) {
		cell_types.push_back(VTK_PYRAMID);
		cell_array->InsertNextCell(set_cell_id(vtkSmartPointer<vtkPyramid>::New(), cp));
    }
    else if (cp.size() == gpm_model_converter::n_wedge) {
		cell_types.push_back(VTK_WEDGE);
		cell_array->InsertNextCell(set_cell_id(vtkSmartPointer<vtkWedge>::New(), cp));
    }
    else if (cp.size() == gpm_model_converter::n_tetra) {
		cell_types.push_back(VTK_TETRA);
		cell_array->InsertNextCell( set_cell_id(vtkSmartPointer<vtkTetra>::New(), cp));
    }
    else {
		vali_type = false;
	}
	return vali_type;
}

vtkSmartPointer<vtkUnstructuredGrid> clean_ugrid_points_and_cells(vtkSmartPointer<vtkUnstructuredGrid>& old_dataset,
    const std::string file_name, bool binary_write, bool origional_write,
    std::vector<long long>* map_origin_to_compressed_p,
    const gpm_logger& logger)
{
    const auto number_of_old_points = old_dataset->GetNumberOfPoints();
    std::vector<bool> point_referenced_by_cell(number_of_old_points, false);

    // filter out unused cells (org --> compressed)
    auto *cell_point_ids = vtkIdList::New();
    for (vtkIdType cell_id = 0; cell_id < old_dataset->GetNumberOfCells(); ++cell_id)
    {
        old_dataset->GetCellPoints(cell_id, cell_point_ids);

        const auto cell_size = cell_point_ids->GetNumberOfIds();
        auto *cell_ptr = cell_point_ids->GetPointer(0);

        std::vector<int> ci(cell_size, 0);
        for (auto pts = 0; pts < cell_size; ++pts)
        {
            const auto cell_index = static_cast<int> (cell_ptr[pts]);
            point_referenced_by_cell.at(cell_index) = true;
        }
    }
    cell_point_ids->Delete();

    const auto number_of_actual_points = count(point_referenced_by_cell.begin(), point_referenced_by_cell.end(), true);

    // check if there are any cleanup to do
    if (number_of_actual_points == number_of_old_points) {
        return old_dataset;
    }

    std::vector<long long> map_from_compressed_to_origin(number_of_actual_points, -1);
    auto& map_from_original_to_compressed(*map_origin_to_compressed_p);
    map_from_original_to_compressed.resize(number_of_old_points, -1);
    std::fill(map_from_original_to_compressed.begin(), map_from_original_to_compressed.end(), -1);

    // map geometry points
    auto new_points = vtkSmartPointer<vtkPoints>::New();
    auto new_index_mapping = 0;
    for (auto i = 0; i < old_dataset->GetNumberOfPoints(); i++)
    {
        const auto pd = old_dataset->GetPoints()->GetPoint(i);
        // add only points that are referenced
        if (point_referenced_by_cell.at(i)) {
            new_points->InsertNextPoint(pd[0], pd[1], pd[2]);
            map_from_compressed_to_origin.at(new_index_mapping) = i;
            map_from_original_to_compressed.at(i) = new_index_mapping;
            new_index_mapping++;
        }
    }

    // generate new cell points
    auto cell_array = vtkSmartPointer<vtkCellArray>::New();
    vector<int> cell_types;
    auto *new_cell_point_ids = vtkIdList::New();
    for (vtkIdType cell_id = 0; cell_id < old_dataset->GetNumberOfCells(); ++cell_id)
    {
        old_dataset->GetCellPoints(cell_id, new_cell_point_ids);

        const auto cell_size = new_cell_point_ids->GetNumberOfIds();
        auto *cell_ptr = new_cell_point_ids->GetPointer(0);

        std::vector<int> ci;
        ci.reserve(cell_size);
        for (auto pts = 0; pts < cell_size; ++pts)
        {
            const auto cell_index = static_cast<int> (cell_ptr[pts]);
            ci.emplace_back(map_from_original_to_compressed[cell_index]);
        }
        if (!save_cell_props(cell_array, cell_types, ci)) {
            logger.print(LOG_NORMAL, "Cell id (%u) not valid\n", cell_id);
        }
    }
    new_cell_point_ids->Delete();

    auto new_dataset = vtkSmartPointer<vtkUnstructuredGrid>::New();
    new_dataset->SetPoints(new_points);
    new_dataset->SetCells(&(cell_types)[0], cell_array);

    // generate compressed to org mapping
    auto* c_2_o_mapping = vtkUnsignedLongLongArray::New();
    c_2_o_mapping->SetNumberOfComponents(1);
    c_2_o_mapping->SetName("CompressToOrgMapping");
    for (auto val : map_from_compressed_to_origin) {
        c_2_o_mapping->InsertNextValue(val);
    }
    new_dataset->GetPointData()->AddArray(c_2_o_mapping);
    c_2_o_mapping->Delete();

    // copy celldata arrays
    const auto number_of_cell_based_arrays = old_dataset->GetCellData()->GetNumberOfArrays();
    for (vtkIdType i = 0; i < number_of_cell_based_arrays; i++)
    {
        const auto p = old_dataset->GetCellData()->GetArray(i);
        new_dataset->GetCellData()->AddArray(p);
    }

    // copy pointdata arrays
    const auto number_of_point_based_arrays = old_dataset->GetPointData()->GetNumberOfArrays();
    for (vtkIdType i = 0; i < number_of_point_based_arrays; i++)
    {
        const auto p = old_dataset->GetPointData()->GetArray(i);
        auto* prop_to_copy = vtkDoubleArray::New();
        prop_to_copy->SetNumberOfComponents(1);
        prop_to_copy->SetName(old_dataset->GetPointData()->GetArrayName(i));
        for (auto c = 0; c < p->GetNumberOfValues(); c++) {
            if (point_referenced_by_cell.at(c)) {
                prop_to_copy->InsertNextValue(p->GetTuple1(c));
            }
        }
        new_dataset->GetPointData()->AddArray(prop_to_copy);
        prop_to_copy->Delete();
    }

    // add mapping to org dataset 
    auto* o_2_c_mapping = vtkLongLongArray::New();
    o_2_c_mapping->SetNumberOfComponents(1);
    o_2_c_mapping->SetName("OrgToCompressMapping");
    for (auto val : map_from_original_to_compressed) {
        o_2_c_mapping->InsertNextValue(val);
    }
    old_dataset->GetPointData()->AddArray(o_2_c_mapping);
    o_2_c_mapping->Delete();

    // write old dataset to the "_origional" file
    if (origional_write) {
        auto append = vtkSmartPointer<vtkAppendFilter>::New();
        append->AddInputData(old_dataset);
        append->Update();

        auto unstructured_grid_writer = vtkSmartPointer<vtkXMLUnstructuredGridWriter>::New();
        const auto org_filename = file_name + "_origional.vtu";
        std::vector<std::string> filenames;
        filenames.emplace_back(org_filename.c_str());
        unstructured_grid_writer->SetFileName(filenames.back().c_str());
        if (binary_write) {
            unstructured_grid_writer->SetDataModeToBinary();
        }
        else {
            unstructured_grid_writer->SetDataModeToAscii();
        }
        unstructured_grid_writer->SetInputData(append->GetOutput());
        unstructured_grid_writer->Write();
        //unstructured_grid_writer->Delete();  MXG: causes crash if set 
    }

    // return new compressed dataset
	return new_dataset;
}

vtk_writer_vtu::vtk_writer_vtu(): m_parm_(nullptr), m_binary_write_(true), m_origional_write_(true){
}


int vtk_writer_vtu::write(int cycle_time, const gpm_model_definition& model, const gpm_logger& logger)  {

    auto error = write_vtu_model_file(cycle_time,model,logger);
    if (!error) {
        error = write_vtu_tops_file(cycle_time, model, logger);
    }
    return error;
}

void vtk_writer_vtu::set_write_info(const std::string& fil_name, grid_parameter_type* param,bool binary_write, bool origional_write) {
    m_file_name_ = fil_name;
    m_parm_ = param;
    m_binary_write_ = binary_write;
    m_origional_write_ = origional_write;
}


void vtk_writer_vtu::add_valid_cell_to_mapping(std::vector<bool>& used_point_index, const std::vector<int>& cell_ptr)
{
	for (auto cell_index : cell_ptr) {
		used_point_index.at(cell_index) = true;
	}
}

void vtk_writer_vtu::get_tetra_cells(std::vector<std::pair<int, double> >& v_dist,
                                     std::vector<int>& t1, std::vector<int>& t2, 
                                     std::vector<int>& t3, std::vector<int>& t4)
{
    // find collapsed corner
    const auto collapsed_vertex_id = std::find_if(v_dist.begin(), v_dist.end(),
        [](const std::pair<int, double>& p)
    {
        return p.second == 0.0;
    });

    // one edge (in z -dir is collapsed - convert this cell into four tetrahedra
    auto node_index_collapsed = -1;
    if (collapsed_vertex_id != v_dist.end()) {
        node_index_collapsed = distance(v_dist.begin(), collapsed_vertex_id);

        if (node_index_collapsed == 0) {
            t1.emplace_back(0); t1.emplace_back(2);  t1.emplace_back(3); t1.emplace_back(6);
            t2.emplace_back(5); t2.emplace_back(2);  t2.emplace_back(6); t2.emplace_back(7);
            t3.emplace_back(0); t3.emplace_back(1);  t3.emplace_back(2); t3.emplace_back(5);
            t4.emplace_back(0); t4.emplace_back(2);  t4.emplace_back(6); t4.emplace_back(5);
        }
        else if (node_index_collapsed == 1) {
            t1.emplace_back(1); t1.emplace_back(3);  t1.emplace_back(0); t1.emplace_back(7);
            t2.emplace_back(6); t2.emplace_back(3);  t2.emplace_back(7); t2.emplace_back(4);
            t3.emplace_back(1); t3.emplace_back(2);  t3.emplace_back(3); t3.emplace_back(6);
            t4.emplace_back(1); t4.emplace_back(3);  t4.emplace_back(7); t4.emplace_back(6);
        }
        else if (node_index_collapsed == 2) {
            t1.emplace_back(2); t1.emplace_back(0);  t1.emplace_back(1); t1.emplace_back(4);
            t2.emplace_back(7); t2.emplace_back(0);  t2.emplace_back(4); t2.emplace_back(5);
            t3.emplace_back(2); t3.emplace_back(3);  t3.emplace_back(0); t3.emplace_back(7);
            t4.emplace_back(2); t4.emplace_back(0);  t4.emplace_back(4); t4.emplace_back(7);
        }
        else if (node_index_collapsed == 3) {
            t1.emplace_back(3); t1.emplace_back(1);  t1.emplace_back(2); t1.emplace_back(5);
            t2.emplace_back(4); t2.emplace_back(1);  t2.emplace_back(5); t2.emplace_back(6);
            t3.emplace_back(3); t3.emplace_back(0);  t3.emplace_back(1); t3.emplace_back(4);
            t4.emplace_back(3); t4.emplace_back(1);  t4.emplace_back(5); t4.emplace_back(4);
        }
    }
    else {
        //
    }   
}


std::vector<std::string> vtk_writer_vtu::get_properties_to_write() const
{
	return prop_names;
}

void vtk_writer_vtu::set_properties_to_write(const std::vector<std::string>& props)
{
	prop_names = props;
}

void vtk_writer_vtu::add_properties_to_write(const std::vector<std::string>& props)
{
	prop_names.insert(prop_names.end(), props.begin(), props.end());
}


int vtk_writer_vtu::write_vtu_tops_file(int cycle_time, const gpm_model_definition& model, const gpm_logger& logger) const {
	grid_parameter_type& p(*m_parm_);

	
	const auto numrows = p.numrows();
	const auto numcols = p.numcols();

	// Find the top surfaces
	std::vector<int> diag;  // The diagonal that will follow the surfaces
	const auto ind = get_property_index(p, "TOP");
	if (ind < 0) {
		logger.print(Tools::LOG_NORMAL, "TOP not present\n");
		return 1;
	}

	find_grid_diagonal_indexes(p, ind, &diag);
	const auto time = cycle_time;

	std::vector<function_2d> surfaces;
	for (auto i = 0; i <= diag[time]; ++i) {
		auto tmp = p.indprop(ind, time, i);
		if (tmp.is_initialized()) {
			surfaces.emplace_back(tmp);
		}
		else {
			surfaces.emplace_back(surfaces.back());
		}
	}

	// set TOP surface to the upper surface
	const auto top_surface = surfaces.back();

	/*  SEA LEVEL */
	diag.clear();
	const auto indsea = get_property_index(p, "SEALEVEL");
	if (indsea < 0) {
		logger.print(Tools::LOG_NORMAL, "SEALEVEL not present\n");
		return 1;
	}
	find_grid_diagonal_indexes(p, indsea, &diag);

	surfaces.clear();
	for (auto i = 0; i <= diag[time]; ++i) {
		auto tmp = p.indprop(indsea, time, i);
		if (tmp.is_initialized()) {
			surfaces.emplace_back(tmp);
		}
		else {
			surfaces.emplace_back(surfaces.back());
		}
	}
	auto sealevel = surfaces.back();

	/*  EROSION */
	{
		diag.clear();
		const auto indero = get_property_index(p, "EROSION");
		if (indero < 0) {
			logger.print(Tools::LOG_NORMAL, "EROSION not present\n");
			return 1;
		}
		find_grid_diagonal_indexes(p, indero, &diag);

		surfaces.clear();
		for (auto i = 0; i <= diag[time]; ++i) {
			auto tmp = p.indprop(indero, time, i);
			if (tmp.is_initialized()) {
				surfaces.emplace_back(tmp);
			}
			else {
				surfaces.emplace_back(surfaces.back());
			}
		}
	}
	auto erosion = surfaces.back();

	double_2darray_type depo_depth(model.model_extent());
	double_2darray_type water_weight(model.model_extent());
	double_2darray_type water_head(model.model_extent());
	for (auto j = 0; j < numrows; ++j) {
		for (auto k = 0; k < numcols; ++k) {
			const double node_seal_level = sealevel.is_initialized() ? sealevel(j, k) : 0.0;
			const double node_top_surface = top_surface.is_initialized() ? top_surface(j, k) : 0.0;
			const double node_depo_depth = -node_seal_level + node_top_surface;
			depo_depth(j, k) = node_depo_depth;
			water_weight(j, k) = (node_depo_depth > 0.0) ? 0.0 : -1.0 * node_depo_depth * 9.8 * 1000;
			water_head(j, k) = std::max(0.0, -node_depo_depth);
		}
	}

	// Now to the attributes
	std::vector<property_stack_holder> prop_holder = {
		{"SEALEVEL", sealevel, 0.0},
		{"EROSION", erosion, 0.0},
		{"DEPODEPTH", depo_depth},
		{"WATER_PRESSURE", water_weight},
		{"HEAD", water_head}
	};
	int error = write_tops_for_timestep(top_surface, prop_holder, model);
    if ( error != 0) {
		return error;
    }
	return 0;
}

// Each property should only have one array
int vtk_writer_vtu::write_tops_for_timestep(const function_2d_read_type& top_surface,
                                            const std::vector<property_stack_holder>& property_stack,
                                            const gpm_model_definition& model) const
{
    const int_extent_1d row_ext = model.model_extent().row_extent();
    const int_extent_1d col_ext = model.model_extent().col_extent();

    vtkSmartPointer<vtkPoints> mypoints =
        vtkSmartPointer<vtkPoints>::New();
    const auto array_size = model.model_extent().size();
	const auto numcols = col_ext.size();
    std::vector<point> gpoints;
    gpoints.reserve(array_size);
    gpm_model_definition::index_pt tmp_ij = { 0,0 };

	std::function<long long(long long)> real_index_mapper = [this](long long index) {return this->_origin_to_compressed_map[index]; };
	std::function<long long(long long)> default_index_mapper = [](long long index) {return index; };
	std::function<long long(long long)> index_mapper = _origin_to_compressed_map.empty() ? default_index_mapper : real_index_mapper;
    // maped node id's
    std::vector<unsigned long> original_subsurface_node_ids;
    original_subsurface_node_ids.reserve(array_size);
    unsigned long node_id = static_cast<unsigned long> ((current_stack_size > 1? current_stack_size-1: 1) * array_size);

    // points
    for (auto j : row_ext) {
       tmp_ij.i = static_cast<double>(j);
       for (auto k : col_ext) {
           // points
           tmp_ij.j = static_cast<double>(k);
           const auto tmp_xy = model.convert_from_index_to_world(tmp_ij);
		   if (index_mapper(_top_model_index_map.get(j, k)) > -1)
			   mypoints->InsertNextPoint(tmp_xy.x, tmp_xy.y, top_surface(j, k));
           original_subsurface_node_ids.emplace_back(node_id);
           node_id++;
       }
     }

    // cell ptrs
    auto cell_array = vtkSmartPointer<vtkCellArray>::New();
    vector<int> cell_types;
	for (auto i = row_ext.lower(); i < row_ext.upper() - 1; ++i) {
		for (auto j = col_ext.lower(); j < col_ext.upper() - 1; ++j) {

			// generate cells for surface
			vector <int> cell_pnt = { 0,0,0,0 };
			const auto p0 = j + i * numcols;
			const auto p1 = p0 + 1;
			const auto p2 = p1 + numcols;
			const auto p3 = p0 + numcols;

			cell_pnt[0] = static_cast<int> ((index_mapper(_top_model_index_map.get(i, j)) > -1) ? p0 : -1);
			cell_pnt[1] = static_cast<int> ((index_mapper(_top_model_index_map.get(i, j + 1)) > -1) ? p1 : -1);
			cell_pnt[2] = static_cast<int> ((index_mapper(_top_model_index_map.get(i + 1, j + 1)) > -1) ? p2 : -1);
			cell_pnt[3] = static_cast<int> ((index_mapper(_top_model_index_map.get(i + 1, j)) > -1) ? p3 : -1);

			vector <int> valid_pnts;
			for (auto& cp : cell_pnt) {
				if (cp != -1)
					valid_pnts.emplace_back(cp);
			}
			if (valid_pnts.size() == 4) {
				cell_types.push_back(VTK_QUAD);
				cell_array->InsertNextCell(set_cell_id(vtkSmartPointer<vtkQuad>::New(), valid_pnts));
			}
			else if (valid_pnts.size() == 3) {
				cell_types.push_back(VTK_TRIANGLE);
				cell_array->InsertNextCell(set_cell_id(vtkSmartPointer<vtkTriangle>::New(), valid_pnts));
			}
			else if (valid_pnts.size() == 2) {
				cell_types.push_back(VTK_LINE);
				cell_array->InsertNextCell(set_cell_id(vtkSmartPointer<vtkLine>::New(), valid_pnts));
			}
			else if (valid_pnts.size() == 1) {
				cell_types.push_back(VTK_VERTEX);
				cell_array->InsertNextCell(set_cell_id(vtkSmartPointer<vtkVertex>::New(), valid_pnts));
			}
		}
	}
	auto u_grid = vtkSmartPointer<vtkUnstructuredGrid>::New();
    u_grid->SetPoints(mypoints);
    u_grid->SetCells(&(cell_types)[0], cell_array);

    // write node properties
    auto* fvec = vtkUnsignedLongLongArray::New();
    fvec->SetNumberOfComponents(1);
    fvec->SetName("bulk_node_ids");
	for (auto j : row_ext) {
		for (auto k : col_ext) {
            const auto top_ind = _top_model_index_map(j, k);
            const auto mapped_ind = index_mapper(top_ind);
			if (mapped_ind > -1) fvec->InsertNextValue(mapped_ind);
		}
    }
    u_grid->GetPointData()->AddArray(fvec);
    fvec->Delete();

    // Now to the attributes
	for (auto& it : property_stack) {
		auto* sl_prop = vtkDoubleArray::New();
		sl_prop->SetNumberOfComponents(1);
		sl_prop->SetName(it.name.c_str());
		const auto& func(it.stack.back());
		for (auto j :row_ext) {
			for (auto k :col_ext) {
				const double val = func(j, k);
				if (index_mapper(_top_model_index_map.get(j, k)) > -1)
					sl_prop->InsertNextValue(val);
			}
		}
		u_grid->GetPointData()->AddArray(sl_prop);
		sl_prop->Delete();
	}

    auto append = vtkSmartPointer<vtkAppendFilter>::New();
    append->AddInputData(u_grid);
    append->Update();

    auto unstructured_grid_writer = vtkSmartPointer<vtkXMLUnstructuredGridWriter>::New();

	const auto vtk_string = m_file_name_ + "_tops.vtu";
    std::vector<std::string> filenames;
    filenames.emplace_back(vtk_string.c_str());
    unstructured_grid_writer->SetFileName(filenames.back().c_str());
    if (m_binary_write_) {
        unstructured_grid_writer->SetDataModeToBinary();
    }
    else {
        unstructured_grid_writer->SetDataModeToAscii();
    }
    unstructured_grid_writer->SetInputData(append->GetOutput());
    unstructured_grid_writer->Write();
	//unstructured_grid_writer->Delete();  MXG: causes crash if set 
    return 0;
}

int vtk_writer_vtu::write_vtu_model_file(int cycle_time,const gpm_model_definition& model, const gpm_logger& logger)  {
    grid_parameter_type& p(*m_parm_);

    const auto numrows = p.numrows();
    const auto numcols = p.numcols();
    // Find the top surfaces
	std::vector<int> diag;  // The diagonal that will follow the surfaces
	const auto ind = get_property_index(p, "TOP");
	find_grid_diagonal_indexes(p, ind, &diag);
    const auto time = cycle_time;
    {
        std::vector<function_2d_read_type> surfaces;
        for (auto i = 0; i <= diag[time]; ++i) {
            auto tmp = p.indprop(ind, time, i);
            if (tmp.is_initialized()) {
                surfaces.emplace_back(tmp);
            }
            else {
                surfaces.emplace_back(surfaces.back());
            }
        }

        std::vector<std::string> propnames;
		std::copy_if(p.propnam.begin(), p.propnam.end(), std::back_inserter(propnames), [this](const std::string& val) {return is_property_to_be_included(val); });

        std::vector<property_stack_holder> props;
        for (const auto& propname : propnames) {
            const auto prop_ind = get_property_index(p, propname);
            std::vector<function_2d_read_type> layers;
            layers.clear();
            for (size_t i = 0; i < surfaces.size(); ++i) {
                auto tmp = p.indprop(prop_ind, time, i);
                if (tmp.is_initialized()) {
                    layers.emplace_back(tmp);
                }
                else {
					if (layers.empty()) {
						if (tmp.is_initialized()) {
							layers.emplace_back(tmp);
						}
						else {
							layers.emplace_back(function_2d_read_type());
						}
					}
					else {
						layers.emplace_back(layers.back());
					}
                }
            }
			if (std::any_of(layers.begin(), layers.end(), [](const function_2d_read_type& item) {return item; }) ){
				props.push_back({ propname, layers });
			}

        }

        auto error = write_model_for_timestep(surfaces, props,model,logger);
        if (error) {
            return 1;
        }
    }
    return 0;
}

int vtk_writer_vtu::write_model_for_timestep(const std::vector<function_2d_read_type>& surface_stack,
                                               const std::vector<property_stack_holder>& property_stacks,
                                               const gpm_model_definition& model,
	const gpm_logger& logger)
{

        std::string vtk_string = m_file_name_  + ".vtu";

        int_extent_1d row_ext = model.model_extent().row_extent();
        int_extent_1d col_ext = model.model_extent().col_extent();
		_top_model_index_map.resize(model.model_extent());
		current_stack_size = static_cast<int> (surface_stack.size());

		std::vector<long long>* origin_to_compressed_map_p(&_origin_to_compressed_map);
		gpm_array_2d_base<long long>* top_index_map_p(&_top_model_index_map);
        vtkSmartPointer<vtkPoints> mypoints =
            vtkSmartPointer<vtkPoints>::New();

        std::vector<point> gpoints;
        const auto numlayers = surface_stack.size();
		const auto numrows = row_ext.size();
		const auto numcols = col_ext.size();
		gpoints.reserve(numlayers*numrows*numcols);
        gpm_model_definition::index_pt tmp_ij = {0,0};

        for (const auto tmp : surface_stack){
		   for (auto j :row_ext) {
			   tmp_ij.i = static_cast<double>(j);
               for (auto k : col_ext) {
                   tmp_ij.j = static_cast<double>(k);
                   const auto tmp_xy = model.convert_from_index_to_world(tmp_ij);
                   mypoints->InsertNextPoint(tmp_xy.x, tmp_xy.y, tmp(j, k));
                   point pp = point(tmp_xy.x, tmp_xy.y, tmp(j, k));
                   gpoints.emplace_back(pp);
               }
            }   
        }
		const auto node_layer = numrows * numcols;
    // Convert properties to arrays
    // The assumption is that we need as many proprty samples as surface samples
		std::vector<std::vector<double>> props_holder;
		for (auto& a_prop : property_stacks) {
			std::vector<double> prop;
			for(auto& tmp:a_prop.stack){
				for (auto j :row_ext) {
					for (auto k :col_ext) {
						prop.emplace_back(tmp?tmp(j, k):0.0);
					}
				}
			}
			props_holder.emplace_back(prop);
		}


        auto cell_array = vtkSmartPointer<vtkCellArray>::New();
        vector<int> cell_types;
        auto u_grid = vtkSmartPointer<vtkUnstructuredGrid>::New();

        auto cell_id = 0;

        for (auto i = row_ext.lower(); i < row_ext.upper() - 1; i++) {
            for (auto j = col_ext.lower(); j < col_ext.upper() - 1; j++) {
                auto n_pyr = 0;
                auto n_hex = 0;
                auto n_wed = 0;
                auto n_tet = 0;
                auto test_flag = false;
				unsigned long p0_bot = static_cast<unsigned long> (j + i * numcols);
				unsigned long p1_bot = p0_bot + 1;
                std::vector<std::vector<unsigned long>> p_index = {
                    {p0_bot}, {p1_bot},
                    {static_cast<unsigned long>(p1_bot + numcols)},
                    {static_cast<unsigned long>(p0_bot + numcols)}
                };

				for (size_t layer = 1; layer < surface_stack.size(); ++layer) {
					unsigned long p0 = static_cast<unsigned long>(j + layer * node_layer + i * numcols);
					unsigned long p1 = p0 + 1;
					unsigned long p2 = static_cast<unsigned long>(p1 + numcols);
					unsigned long p3 = static_cast<unsigned long>(p0 + numcols);
					// Check the pointers
					std::array<unsigned long, 4> top = {
						p0 , p1 , p2 , p3 
					};

					for (auto ind = 0UL; ind < top.size(); ++ind) {
						auto dist = boost::geometry::distance(gpoints[p_index[ind].back()], gpoints[top[ind]]);
						if (dist > 0.001) {
							p_index[ind].push_back(top[ind]);
						}
					}
				}
				top_index_map_p->at(i, j) = p_index[0].back();
				top_index_map_p->at(i, j+1) = p_index[1].back();
				top_index_map_p->at(i+1, j + 1) = p_index[2].back();
				top_index_map_p->at(i + 1, j ) = p_index[3].back();

				std::array<unsigned long, 4> sizes{};
				std::transform(p_index.begin(), p_index.end(), sizes.begin(), [](const std::vector<unsigned long> & it) {return static_cast<unsigned long>(it.size()); });
				auto max_num = *std::max_element(sizes.begin(), sizes.end());
				std::array<unsigned long, 4> bottom = { p_index[0].front(), p_index[1].front(), p_index[2].front(), p_index[3].front() };
				for (auto count = 1UL; count < max_num; ++count)
				{
					std::array<unsigned long, 4> top{};
					for (auto tmp1 = 0; tmp1 < 4; ++tmp1) {
						top[tmp1] = count < p_index[tmp1].size() ? p_index[tmp1][count] : p_index[tmp1].back();
					}
                    auto cell_ptr = gpm_model_converter::check_cell_pointers(gpoints, bottom, top);
                    for (auto cp : cell_ptr) {
                        if (!save_cell_props(cell_array, cell_types, cp)) {
							logger.print(LOG_NORMAL, "Cell (%u, %u, %u) not valid\n", j, i, count);
						}
                        else {
                            if (cp.size() == gpm_model_converter::n_hexahedron) {
                                n_hex++;
                            }
                            else if (cp.size() == gpm_model_converter::n_pyramid) {
                                n_pyr++;
                            }
                            else if (cp.size() == gpm_model_converter::n_wedge) {
                                n_wed++;
                            }
                            else if (cp.size() == gpm_model_converter::n_tetra) {
                                n_tet++;
                            }
							cell_id++;
                        }
                    }
					bottom = top;
                }
            }
        }


        u_grid->SetPoints(mypoints);

        if (!cell_types.empty()) {
            u_grid->SetCells(&(cell_types)[0], cell_array);
			logger.print(LOG_NORMAL/*VERBOSE*/, "\nTotal number of cells:         %u\n", cell_types.size());
			logger.print(LOG_NORMAL/*VERBOSE*/, "Number of cells pyramid cells: %u\n", count_if(cell_types.begin(), cell_types.end(), is_pyramid));
			logger.print(LOG_NORMAL/*VERBOSE*/, "Number of cells hexagon cells: %u\n", count_if(cell_types.begin(), cell_types.end(), is_hexahedron));
			logger.print(LOG_NORMAL/*VERBOSE*/, "Number of cells wedge cells:   %u\n", count_if(cell_types.begin(), cell_types.end(), is_wedge));
			logger.print(LOG_NORMAL/*VERBOSE*/, "Number of cells tetra cells:   %u\n", count_if(cell_types.begin(), cell_types.end(), is_tetra));
		}
        else {
			logger.print(LOG_NORMAL, "VTU Export: No cells present in grid\n");
			return 1;

        }
		std::vector<std::string> propnames;
        std::transform(property_stacks.begin(), property_stacks.end(), std::back_inserter(propnames),
                       [](const property_stack_holder& it) { return it.name; });
        // write properties
		for (const auto& tup : boost::combine(propnames, props_holder)) {
			const std::string& name(tup.get<0>());
			const std::vector<double>& node_vals(tup.get<1>());
			const double conversion = boost::starts_with(name, "PERM") ? 1e-15 : 1.0;
			auto* fvec = vtkDoubleArray::New();
			fvec->SetNumberOfComponents(1);
			fvec->SetName(name.c_str());

		    // cell props
            if (convert_to_cell_property(name)) {
                std::vector<double> cell_vals;
                convert_node_to_cell_props(node_vals, u_grid, cell_vals);
                for (auto val : cell_vals) {
                    fvec->InsertNextValue(val*conversion);
                }
                u_grid->GetCellData()->AddArray(fvec);
				if (boost::iequals(name, "PERMEABILITY_HORI"))
				{
					auto* hcvec = vtkDoubleArray::New();
					hcvec->SetNumberOfComponents(1);
					hcvec->SetName("HYDRAULIC_CONDUCTIVITY");
					for (auto val : cell_vals) {
						hcvec->InsertNextValue(val*9.81e-9);
					}
					u_grid->GetCellData()->AddArray(hcvec);
					hcvec->Delete();
				}
            }
            // node props
            else {
                for (auto val : node_vals) {
                	fvec->InsertNextValue(val*conversion);
                }
                u_grid->GetPointData()->AddArray(fvec);
            }
			fvec->Delete();
		}

        // report tensor product if hperm and vperm are present
		const auto index_hperm = std::find(propnames.begin(), propnames.end(), "PERMEABILITY_HORI");
		const auto index_vperm = std::find(propnames.begin(), propnames.end(), "PERMEABILITY_VERT");
        if (index_hperm != propnames.end() && index_vperm != propnames.end()) {
            const auto pos_hperm = std::distance(propnames.begin(), index_hperm);
			const auto pos_vperm = std::distance(propnames.begin(), index_vperm);
            
			std::vector<double> cell_prmh;
			convert_node_to_cell_props(props_holder[pos_hperm], u_grid, cell_prmh);

			std::vector<double> cell_prmv;
			convert_node_to_cell_props(props_holder[pos_vperm], u_grid, cell_prmv);

			auto* tensor = vtkDoubleArray::New();
			tensor->SetNumberOfComponents(9);
			tensor->SetName("PERMABILITY_TENSOR");

			for (const auto& prm : boost::combine(cell_prmh, cell_prmv)) {
				const auto conversion = 1e-15;
				tensor->InsertNextTuple9(prm.get<0>()*conversion, 0.0, 0.0,
					                     0.0, prm.get<0>()*conversion, 0.0,
					                     0.0, 0.0, prm.get<1>()*conversion);
			}
			u_grid->GetCellData()->AddArray(tensor);
			tensor->Delete();
		}

		auto append = vtkSmartPointer<vtkAppendFilter>::New();

        // cleanup grid
		const auto dataset = clean_ugrid_points_and_cells(u_grid,m_file_name_,m_binary_write_, m_origional_write_,
			                                              origin_to_compressed_map_p, logger);
		append->AddInputData(dataset);

        append->Update();

        auto unstructured_grid_writer = vtkSmartPointer<vtkXMLUnstructuredGridWriter>::New();
        std::vector<std::string> filenames;
        filenames.emplace_back(vtk_string.c_str());
        unstructured_grid_writer->SetFileName(filenames.back().c_str());
        if (m_binary_write_)  { 
            unstructured_grid_writer->SetDataModeToBinary();
        }
        else {
            unstructured_grid_writer->SetDataModeToAscii();
        }
		unstructured_grid_writer->SetInputData(append->GetOutput());
		unstructured_grid_writer->Write();
		//unstructured_grid_writer->Delete();  MXG: causes crash if set 
        return 0;
}

bool vtk_writer_vtu::convert_to_cell_property(const std::string& property)
{
    const vector<string> valid_props = {"POR", "PERMEA","WAVE"};

	const auto p = find_if(valid_props.begin(), valid_props.end(), [&property](const std::string& str){return boost::starts_with(property, str); });
    return p != valid_props.end();
}

 bool vtk_writer_vtu::is_property_to_be_included(const std::string& property)const {

	const auto p = find_if(prop_names.begin(), prop_names.end(), [&property](const std::string& str){return boost::starts_with(property, str); });

    return p != prop_names.end();

}

}}}

