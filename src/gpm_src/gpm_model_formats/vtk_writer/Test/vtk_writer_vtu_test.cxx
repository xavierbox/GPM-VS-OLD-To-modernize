// -- Schlumberger Private --

#include "gpm_model_definition.h"
#include "gpm_parm_methods.h"
#include <iostream>
#include <vector>
#include <string>

#include <boost/core/null_deleter.hpp>

#include "vtk_reader_vtu.h"
#include "gpm_parm_methods.h"
#include "vtk_writer_vtu.h"

using namespace std;
using namespace Slb::Exploration::Gpm;


int Test_vtu_writer_simple_with_props(int, char* []) {

	const Tools::gpm_default_logger logger(Tools::LOG_VERBOSE, true);

	logger.print(Tools::LOG_NORMAL, " Creating VTU file for GPM model with poro and perm properties\n");
	const auto nx = 5;
	const auto ny = 10;
	const auto nz = 2;
  
    const std::vector<float> xf= { 20,40,30,10 };
	const std::vector<float> yf={ 10,20,40,30 };
	const auto ext = int_extent_2d(nx + 1, ny + 1);

	// Need to asetup the processes somewhat early
	gpm_model_definition my_model(ext);

	my_model.set_world_coordinates(xf, yf);

	const std::shared_ptr<gpm_model_definition> gpm_model(new gpm_model_definition(my_model));
	gpm_model_definition def;

	const auto nn = (nx + 1) * (ny + 1) * (nz + 1);

	// setting up the layers
	const auto face_1 = 10;
	const auto face_2 = 20;
	const auto face_3 = 30;

	std::vector<function_2d> surfaces(3);
	surfaces[0].set_constant(face_1);
	surfaces[1].set_constant(face_2);
	surfaces[2].set_constant(face_3);

	std::vector< vtk_writer_vtu::function_2d_read_type> surfs(surfaces.begin(), surfaces.end());

	std::vector<vtk_writer_vtu::property_stack_holder> props;

	// Add PORO, 3 parts
	auto tmp = std::vector<vtk_writer_vtu::function_2d_read_type>(3, [](gpm_array_index_defs::index_type i, gpm_array_index_defs::index_type) {return 0.25; } );
    vtk_writer_vtu::property_stack_holder val = {std::string("PORO"), tmp };
	props.push_back(val);

	// Add PERM
	tmp = std::vector<vtk_writer_vtu::function_2d_read_type>(3,  [](gpm_array_index_defs::index_type i, gpm_array_index_defs::index_type) {return 100; } );
	props.push_back({ "PERMEABILITY_VERT",tmp });

    vtk_writer_vtu vtk_writer;
    vtk_writer.set_file_name("Test_vtu_writer_simple_with_props_time_");
    vtk_writer.set_write_mode_to_ascii();
    auto error = vtk_writer.write_model_for_timestep(surfs, props, my_model,logger);
    if (error) {
        return error;
    }

    return 0;
}

int Test_vtu_writer_reader_consistency(int, char*[]) {

    const Tools::gpm_default_logger logger(Tools::LOG_VERBOSE, true);

    logger.print(Tools::LOG_NORMAL, " Testing write and read consistency for VTU files\n");
    const auto nx = 5;
    const auto ny = 10;
    const auto nz = 2;

    const std::vector<float> xf = { 20,40,30,10 };
    const std::vector<float> yf = { 10,20,40,30 };
    const auto ext = int_extent_2d(nx + 1, ny + 1);

    // model 
    gpm_model_definition my_model(ext);

    my_model.set_world_coordinates(xf, yf);

    const std::shared_ptr<gpm_model_definition> gpm_model(new gpm_model_definition(my_model));
    gpm_model_definition def;

    const auto nn = (nx + 1) * (ny + 1) * (nz + 1);
    const auto n = nx*ny*nz;

    // setting up the layers
    const auto face_1 = 10;
    const auto face_2 = 20;
    const auto face_3 = 30;

    std::vector<function_2d> surfaces(3);
    surfaces[0].set_constant(face_1);
    surfaces[1].set_constant(face_2);
    surfaces[2].set_constant(face_3);

    std::vector< vtk_writer_vtu::function_2d_read_type> surfs(surfaces.begin(), surfaces.end());

    std::vector<vtk_writer_vtu::property_stack_holder> props;

    // Add SEDs (point props)
    auto tmp = std::vector<vtk_writer_vtu::function_2d_read_type>(3, [](gpm_array_index_defs::index_type i, gpm_array_index_defs::index_type) {return 0.25; });
    vtk_writer_vtu::property_stack_holder val = { std::string("SED1"), tmp };
    props.push_back(val);

    tmp = std::vector<vtk_writer_vtu::function_2d_read_type>(3, [](gpm_array_index_defs::index_type i, gpm_array_index_defs::index_type) {return 0.1; });
    props.push_back({ "SED2",tmp });

    tmp = std::vector<vtk_writer_vtu::function_2d_read_type>(3, [](gpm_array_index_defs::index_type i, gpm_array_index_defs::index_type) {return 0.0; });
    props.push_back({ "SED3",tmp });

    // write vtu file
    vtk_writer_vtu vtk_writer;
    string vtu_file_root_name = "Test_vtu_writer_for_write_and_read";
    vtk_writer.set_file_name(vtu_file_root_name);
    vtk_writer.set_write_mode_to_ascii();
    auto error = vtk_writer.write_model_for_timestep(surfs, props, my_model, logger);
    if (error) {
        return error;
    }

    // read back file for comparison check
    Slb::Exploration::Gpm::vtk_reader_vtu::vec_point dummy_gpoints;
    std::vector<std::vector<int> > dummy_cellindexes;
    std::vector<std::string> propname;
    std::vector<std::vector<double> > readprops;

    string vtu_read_file = vtu_file_root_name + ".vtu";

    // reading geometry and point properties
    if (!vtk_reader_vtu::get_data_for_file(vtu_read_file, dummy_gpoints, dummy_cellindexes, propname, readprops)) {
        logger.print(Tools::LOG_VERBOSE, "Including number of point data properties: %u \n" ,propname.size());
        auto idx = 0;
        for (auto &p : readprops) {
            logger.print(Tools::LOG_NORMAL, " --> %s \n", propname[idx++].c_str());
            if (p.size() != nn) {
                logger.print(Tools::LOG_NORMAL, " -----> Incorrect number of elements (write %u read %u) \n", n, p.size());
                return 1;
            }
        }
    }
    else {
        logger.print(Tools::LOG_NORMAL, "Reading File %s failed\n", vtu_read_file.c_str());
        return 1;
    }
    logger.print(Tools::LOG_NORMAL, "\n");

    return 0;
}


int Test_vtu_writer_simple_no_props(int /*argc*/, char *[] /* argv []*/) {

    const Tools::gpm_default_logger logger(Tools::LOG_VERBOSE, true);

    logger.print(Slb::Exploration::Gpm::Tools::LOG_NORMAL," Creating VTU file for GPM model without bulk properties\n");
    const auto nx = 20;
    const auto ny = 10;

    const std::vector<float> xf = { 20,40,30,10 };
    const std::vector<float> yf = { 10,20,40,30 };
    const auto ext = int_extent_2d(nx + 1, ny + 1);

    // Need to asetup the processes somewhat early
    gpm_model_definition my_model(ext);

    my_model.set_world_coordinates(xf, yf);
    
    const std::shared_ptr<gpm_model_definition> gpm_model(new gpm_model_definition(my_model));
    gpm_model_definition def;

    // setting up the layers
    const auto face_1 = 10;
    const auto face_2 = 20;
    const auto face_3 = 30;

    std::vector<function_2d> surfaces(3);
    surfaces[0].set_constant(face_1);
    surfaces[1].set_constant(face_2);
    surfaces[2].set_constant(face_3);
	std::vector< vtk_writer_vtu::function_2d_read_type> surfs(surfaces.begin(), surfaces.end());

    const std::vector<vtk_writer_vtu::property_stack_holder > dummy_props;

    vtk_writer_vtu vtk_writer;
    vtk_writer.set_file_name("Test_vtu_writer_simple_no_props_time_");
    vtk_writer.set_write_mode_to_ascii();
    auto error = vtk_writer.write_model_for_timestep(surfs, dummy_props, my_model,logger);
	if (error) {
        return error;
    }

    return 0;
}

int Test_vtu_writer_simple_wedge_pyramid(int /*argc*/, char* [] /* argv []*/) {

	const Tools::gpm_default_logger logger(Tools::LOG_VERBOSE, true);

	logger.print(Slb::Exploration::Gpm::Tools::LOG_NORMAL, " Creating VTU file for GPM model without bulk properties\n");
	const auto nx = 1;
	const auto ny = 1;

	const std::vector<float> xf = { 0,10,10,0 };
	const std::vector<float> yf = { 0,0,9,9 };
	const auto ext = int_extent_2d(nx + 1, ny + 1);

	// Need to asetup the processes somewhat early
	gpm_model_definition my_model(ext);

	my_model.set_world_coordinates(xf, yf);

	const std::shared_ptr<gpm_model_definition> gpm_model(new gpm_model_definition(my_model));
	gpm_model_definition def;
	gpm_vbl_vector_array_3d<float> holder(4, 2, 2, 0.0f);
	holder(1).fill(1);
	holder(2).fill(2);
	holder(2, 0, 1) = 1;
	holder(3).fill(2);
	holder(3, 0, 1) = 1.0f;
	holder(3, 1, 0) = 3.0f;
	holder(3, 1, 1) = 3.0f;
	auto ptr = std::shared_ptr<float_3darray_vec_base_type>(&holder, boost::null_deleter());
	// setting up the layers

	std::vector<function_2d> surfaces(4);
	surfaces[0].set_array(ptr);
	surfaces[0].set_index(0);
	surfaces[1].set_array(ptr);
	surfaces[1].set_index(1);
	surfaces[2].set_array(ptr);
	surfaces[2].set_index(2);
    surfaces[3].set_array(ptr);
	surfaces[3].set_index(3);
	std::vector< vtk_writer_vtu::function_2d_read_type> surfs(surfaces.begin(), surfaces.end());

	const std::vector<vtk_writer_vtu::property_stack_holder> dummy_props;

	vtk_writer_vtu vtk_writer;
	vtk_writer.set_file_name("Test_vtu_writer_simple_wedge_pyramid_");
	vtk_writer.set_write_mode_to_ascii();
	auto error = vtk_writer.write_model_for_timestep(surfs, dummy_props, my_model,logger);
	if (error) {
		return error;
	}

	return 0;
}

int Test_vtu_writer_wedge_pyramid(int /*argc*/, char* [] /* argv []*/) {

	const Tools::gpm_default_logger logger(Tools::LOG_VERBOSE, true);

	logger.print(Slb::Exploration::Gpm::Tools::LOG_NORMAL, " Creating VTU file for GPM model without bulk properties\n");
	const auto nx = 1;
	const auto ny = 1;

	const std::vector<float> xf = { 800,900,900,800 };
	const std::vector<float> yf = { 0,0,99.947F,99.947F };
	const auto ext = int_extent_2d(nx + 1, ny + 1);

	// Need to asetup the processes somewhat early
	gpm_model_definition my_model(ext);

	my_model.set_world_coordinates(xf, yf);

	const std::shared_ptr<gpm_model_definition> gpm_model(new gpm_model_definition(my_model));
	gpm_model_definition def;
	gpm_vbl_vector_array_3d<float> holder(2, 2, 2, 0.0f);
	holder(0,0,0)=249.503F;
	holder(0, 0, 1) = 239.925F;
	holder(0, 1, 1) = 243.657F;
	holder(0, 1, 0) = 251.829F;
	holder(1, 0, 0) = 249.971F;
	holder(1, 0, 1) = 242.716F;
	holder(1, 1, 1) = 244.826F;
	holder(1, 1, 0) = 251.829F;
	auto ptr = std::shared_ptr<float_3darray_vec_base_type>(&holder, boost::null_deleter());
	// setting up the layers

	std::vector<function_2d> surfaces(2);
	surfaces[0].set_array(ptr);
	surfaces[0].set_index(0);
	surfaces[1].set_array(ptr);
	surfaces[1].set_index(1);
	std::vector< vtk_writer_vtu::function_2d_read_type> surfs(surfaces.begin(), surfaces.end());

	const std::vector<vtk_writer_vtu::property_stack_holder> dummy_props;

	vtk_writer_vtu vtk_writer;
	vtk_writer.set_file_name("Test_vtu_writer_wedge_pyramid_");
	vtk_writer.set_write_mode_to_ascii();
	auto error = vtk_writer.write_model_for_timestep(surfs, dummy_props, my_model,logger);
	if (error) {
		return error;
	}

	return 0;
}

int Test_vtu_writer_all_wedges(int /*argc*/, char* [] /* argv []*/) {

	const Tools::gpm_default_logger logger(Tools::LOG_VERBOSE, true);

	logger.print(Slb::Exploration::Gpm::Tools::LOG_NORMAL, " Creating VTU file for GPM model without bulk properties\n");
	const auto nx = 1;
	const auto ny = 1;

	const std::vector<float> xf = { 0,10,10,0 };
	const std::vector<float> yf = { 0,0,9,9 };
	const auto ext = int_extent_2d(nx + 1, ny + 1);

	// Need to asetup the processes somewhat early
	gpm_model_definition my_model(ext);

	my_model.set_world_coordinates(xf, yf);

	const std::shared_ptr<gpm_model_definition> gpm_model(new gpm_model_definition(my_model));
	gpm_model_definition def;
	gpm_vbl_vector_array_3d<float> holder(2, 2, 2, 0.0f);
	auto ptr = std::shared_ptr<float_3darray_vec_base_type>(&holder, boost::null_deleter());
	// setting up the layers

	std::vector<function_2d> surfaces(2);
	for (int i = 0; i < surfaces.size(); ++i) {
		surfaces[i].set_array(ptr);
		surfaces[i].set_index(i);
	}
	std::vector< vtk_writer_vtu::function_2d_read_type> surfs(surfaces.begin(), surfaces.end());

	const std::vector<vtk_writer_vtu::property_stack_holder> dummy_props;

	holder(0).fill(0);
	holder(1).fill(1);
	holder(1, 0, 0) = holder(1, 1, 0) = 0;

	vtk_writer_vtu vtk_writer;
	vtk_writer.set_file_name("Test_vtu_writer_wedges_03");
	vtk_writer.set_write_mode_to_ascii();
	auto error = vtk_writer.write_model_for_timestep(surfs, dummy_props, my_model, logger);
	if (error) {
		return error;
	}
	holder(1).fill(1);
	holder(1, 0, 0) = holder(1, 0, 1) = 0;
	vtk_writer.set_file_name("Test_vtu_writer_wedges_01");
	error = vtk_writer.write_model_for_timestep(surfs, dummy_props, my_model, logger);
	if (error) {
		return error;
	}

	return 0;
}


int Test_vtu_writer_compare_files(int /*argc*/, char *[]) {

    const Tools::gpm_default_logger logger(Tools::LOG_VERBOSE, true);
    logger.print(Slb::Exploration::Gpm::Tools::LOG_NORMAL, " Comparing two VTU files\n");

    std::string name[3];
    std::cout << "Enter your name : \n";

    for (auto& i:name)
	{
        std::getline(std::cin, i);
    }
	for (auto& i : name)
	{
        std::cout << "\nYou entered : " << i <<std::endl;
    }

	vtk_reader_vtu vtk_reader;
	vtk_reader.set_file_name("dummy");
	vtk_reader_vtu::do_compare_files(name[1], name[2],logger);

    return 0;

}
