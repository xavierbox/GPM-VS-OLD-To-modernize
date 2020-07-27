

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
    if (program_options.size() < 3) {
        printf("The format is: hdf5_filename vtk_file_name timestep\n");
        return -1;
    }

    parm p;
    acf_base_reader::parameter_num_type inumr;
    acf_base_reader::array_dim_size_type idimr;
    Tools::gpm_default_logger logger(Tools::LOG_VERBOSE, true);
    const auto iret = gpm_read_map_file(program_options[1], "MAP", p, inumr, idimr, logger);
    if (iret != 0) {
        return iret;
    }
    // Lets find the data to write out
    const int_extent_2d model_extent(p.numrows(), p.numcols());
    gpm_model_definition my_model(model_extent);
    my_model.set_world_coordinates(p.xcorners, p.ycorners);

    const auto top_id = parm_descr_holder::get_grid_map_name(TOP);
    const auto top_iter = std::find(p.propnam.begin(), p.propnam.end(), top_id);
    if (top_iter == p.propnam.end()) {
        logger.print(Tools::LOG_VERBOSE, "No tops found in model");
        return -1;
    }
    const auto top_index = std::distance(p.propnam.begin(), top_iter);
    const auto time_index = boost::lexical_cast<int>(program_options[3]);
    if (p.timeset.size() <= time_index) {
        logger.print(Tools::LOG_VERBOSE, "Timestep not contained in model");
        return -1;
    }
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
    const auto top_surf = find_last_function_in_stack(p, top_index, time_index);
    const gpm_vtk_output::accessor_2d_float_type surf = [=](int i, int j) { return top_surf(i, j); }; 
    std::vector<gpm_vtk_output::accessor_2d_float_type> surf_vec(1, surf);
    setup.geometries = gpm_vtk_output::name_accessor_float_type("geometry", surf_vec);

    // Now we have the float arrays
    std::set<std::string> done;
    done.insert(top_id);
    const auto src_id = parm_descr_holder::get_grid_map_name(SRC);
    done.insert(src_id);
    auto props = p.propnam;
    auto vecs = parm_descr_holder::get_grid_map_vector_2d();
    auto all_names = parm_descr_holder::get_grid_map_names();
    std::vector<std::string> vector_names;
    for (const auto& item : vecs) {
        vector_names.push_back(all_names[item.second.first]);
        vector_names.push_back(all_names[item.second.second]);
    }
    std::sort(vector_names.begin(), vector_names.end());
    std::sort(props.begin(), props.end());
    std::vector<std::string> float_props;
    std::set_difference(props.begin(), props.end(), vector_names.begin(), vector_names.end(),
                        std::back_inserter(float_props));
    std::vector<gpm_vtk_output::name_accessor_float_type> prop_vecs{};
    for (auto loc_prop : float_props) {
        if (done.count(loc_prop) == 0) {
            const auto iter = std::find(p.propnam.begin(), p.propnam.end(), loc_prop);
            const auto index = std::distance(p.propnam.begin(), iter);
            auto float_prop = find_last_function_in_stack(p, index, time_index);
            if (float_prop.is_initialized()) {
                gpm_vtk_output::accessor_2d_float_type float_surf = [=](int i, int j) { return float_prop(i, j); };
                prop_vecs.emplace_back(loc_prop, std::vector<gpm_vtk_output::accessor_2d_float_type>(1, float_prop));
            }
            done.insert(loc_prop);
        }
    }
    if (!prop_vecs.empty()) {
        setup.float_properties = prop_vecs;
    }
    //Lets do the int part, which is srcs for now
    const auto src_iter = std::find(p.propnam.begin(), p.propnam.end(), src_id);
    if (src_iter != p.propnam.end()) {
        const auto index = std::distance(p.propnam.begin(), src_iter);
        auto int_prop = find_last_function_in_stack(p, index, time_index);
        if (int_prop.is_initialized()) {
            const gpm_vtk_output::accessor_2d_int_type src = [=](int i, int j) { return static_cast<int>(int_prop(i, j)); };
            setup.int_properties.emplace_back(src_id, std::vector<gpm_vtk_output::accessor_2d_int_type>(1, src));
        }

    }
    std::vector<gpm_vtk_output::name_vector_accessor_type> vector_properties;
    std::vector<std::string> vector_props;
    std::set_intersection(props.begin(), props.end(), vector_names.begin(), vector_names.end(),
                          std::back_inserter(vector_props));
    if (!vector_props.empty()) {
        auto vec_names = parm_descr_holder::get_grid_map_vector_2d_names();
        for (const auto item : vecs) {
            // Need both vectors search for one
            const auto x_iter = std::find(p.propnam.begin(), p.propnam.end(), all_names[item.second.first]);
            const auto y_iter = std::find(p.propnam.begin(), p.propnam.end(), all_names[item.second.second]);
            if (x_iter != p.propnam.end() && y_iter != p.propnam.end()) {
                const auto x_index = std::distance(p.propnam.begin(), x_iter);
                const auto y_index = std::distance(p.propnam.begin(), y_iter);
                const auto x_prop = find_last_function_in_stack(p, x_index, time_index);
                const auto y_prop = find_last_function_in_stack(p, y_index, time_index);
                if (x_prop.is_initialized() && y_prop.is_initialized()) {
                    gpm_vtk_output::accessor_2d_float_type x_func = [=](int i, int j) { return x_prop(i, j); };
                    gpm_vtk_output::accessor_2d_float_type y_func = [=](int i, int j) { return y_prop(i, j); };
                    const auto vec_func = std::make_pair(x_func, y_func);
                    vector_properties.emplace_back(vec_names[item.first],
                                                   std::vector<gpm_vtk_output::vector_accessor_2d_type>(
                                                       1, vec_func));
                }
            }
        }
        setup.vector_properties = vector_properties;
    }
    setup.write_file(program_options[2]);
    return 0;
}
