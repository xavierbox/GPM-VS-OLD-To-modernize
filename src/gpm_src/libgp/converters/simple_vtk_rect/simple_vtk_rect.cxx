

#include "mapstc.h"
#include "gpm_type_descriptor.h"
#include "grid_param_mapper.h"
#include "gpm_write_vtk_file.h"
#include "gpm_model_definition.h"
#include "gpm_parm_methods.h"
#include <boost/lexical_cast.hpp>
#include <string>
#include <vector>

using namespace Slb::Exploration::Gpm;

function_2d
find_last_function_in_stack(const parm& p, int prop_index, int time_index)
{
    std::vector<function_2d> tops;
    find_grid_functions_for_timestep(p, prop_index, time_index, 0, p.numtops() - 1, &tops);
    const auto it = std::find_if(tops.rbegin(), tops.rend(), [](const function_2d& func){return func.is_initialized();});
    if (it != tops.rend()) {
        return *it;
    }
    return function_2d();
}

int main(int argc, char* argv[])
{
    // We need to find the hdf5 file, 
    // and we need the output filename
    // and the timestep to write out
    std::vector<std::string> program_options(argv, argv + argc);
    if (program_options.size() < 2) {
        printf("The format is: vtk_file_name\n");
        return -1;
    }

    parm p;
    acf_base_reader::parameter_num_type inumr;
    acf_base_reader::array_dim_size_type idimr;
    Tools::gpm_default_logger logger(Tools::LOG_VERBOSE, true);
    // Lets find the data to write out
    const int_extent_2d model_extent(21, 21);
    gpm_model_definition my_model(model_extent);
	my_model.set_world_coordinates({ 0,1,1,0 }, {0,0,1,1});

    gpm_vtk_output setup;
    setup.convert_to_world = [&](int i, int j, float* x, float* y)
    {
        gpm_model_definition::index_pt ij{static_cast<float> (i),static_cast<float> (j)};
        auto xy = my_model.convert_from_index_to_world(ij);
        *x = xy.x;
        *y = xy.y;
    };
    setup.extent = model_extent;
    std::vector<function_2d> tops;
    const gpm_vtk_output::accessor_2d_float_type surf = [=](int i, int j) { return 0.0F; }; 
    std::vector<gpm_vtk_output::accessor_2d_float_type> surf_vec(1, surf);
    setup.geometries = gpm_vtk_output::name_accessor_float_type("geometry", surf_vec);

    setup.write_file_to_new_unstructured(program_options[1]);
    return 0;
}
