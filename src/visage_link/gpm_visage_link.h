#ifndef _VISAGE_LINK_H_
#define _VISAGE_LINK_H_ 1
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <fstream>
#include <filesystem>
namespace fs = std::experimental::filesystem;

#include <vector>
#include <iterator>
#include <set>
#include "gpm_plugin_helpers.h"
#include "gpm_plugin_description.h"
#include "gpm_visage_sed_description.h"

#include "gpm_attribute_iterator.h"

#include "Vector3.h"
#include "CoordinateMapping.h"
#include "StructuredGrid.h"
#include "BaseTypesSimulationOptions.h"
#include "VisageDeckSimulationOptions.h"
#include "VisageDeckWritter.h"
#include "VisageDeckSimulationOptions.h"
#include "VisageDeckWritter.h"
#include "gpm_visage_results.h"
#include "FileSystemUtils.h"
#include "EclipseReader.h"
#include "utils.h"



using namespace std;

class gpm_visage_link
{

   
    using attr_lookup_type = std::map<std::string, std::vector<Slb::Exploration::Gpm::Api::array_2d_indexer<float>>>;
    using gpm_attribute = vector<Slb::Exploration::Gpm::Api::array_2d_indexer<float>>;

    vector<float> get_values(const gpm_attribute &att, int k1 = 0, int k2 = -1)
    {
        vector<float> values;
        if (att.size() <= 0) return values;

        const int nrows = (int)att[0].num_rows(), ncols = (int)att[0].num_cols();
        values.resize(att.size() * ncols * nrows, 0);

        int counter = 0;
        if (k2 < 0) k2 = att.size();
        if (k1 < 0) k1 = 0;

        for (int ns : IntRange(k1, k2))
        {
            auto& holder = att[ns];

            for (int row = 0; row < nrows; row++)
            {
                for (int col = 0; col < ncols; col++)
                {
                    values[counter++] = holder(row, col);
                }
            }
        }

        return values;
    }

    std::tuple<int, int, int, int> node_values_count(const gpm_attribute &p, int  k = 0) const
    {
        const auto& geo = p.at(k);
        int size = p.size(), rows = geo.num_rows(), cols = geo.num_cols();

        return make_tuple(size*rows*cols, size, rows, cols);
    }

public:

    int solve_schema1( const attr_lookup_type& attributes, std::string& error );

    struct property_type { std::string name; bool top_layer_only; };

    gpm_visage_link();



    gpm_visage_link* operator->() { return this; }

    //initialization

    //void initialize_gpm_visage_keywords();

    //temporary: encloses hard-coded configs for debugging and launching
    void initialize_defaut_gpm_visage_options();

    void initialize_model_extents(const gpm_plugin_api_model_definition *model_def);

    void initialize_visage_geometry();

    void initialize_mechanical_properties(int scenario, const attr_lookup_type& attributes, int old_num_surfaces, int new_num_surfaces);

    //updating

    bool update_needed_attribute_values(const attr_lookup_type& attributes, ArrayData &data);//, bool it_commes_as_bottom_up = true);

    void update_basement_history(const gpm_attribute &top);

    void update_top_history(StructuredGrid &geometry /*const attr_lookup_type& attributes, bool flip_surface_order = true*/);

    int  update_results(map<string, vector<Slb::Exploration::Gpm::Api::array_2d_indexer<float>>>  &attributes, std::string& error, int step = -1);

    //retrieving information from gpm
    vector<float> gpm_visage_link::get_gpm_property_from_seds(const attr_lookup_type& attributes, string property_name);

    bool read_geometry_from_gpm_top(StructuredGrid &geometry, const attr_lookup_type& attributes);//, bool bottom_up = true);

    void read_gpm_property_values(const attr_lookup_type& attributes, const pair<string, bool> &prop, vector<float>&elemental_values);//

    bool update_sea_level(const attr_lookup_type& attributes)
    {
        //read the sea-level surface. This is a constant
        float  sea_level = attributes.at("SEALEVEL").at(0)(0, 0);
        _visage_options->sea_level() = sea_level;
        std::cout << "Setting sealevel to " << sea_level << std::endl;
        return true;
    }

    //passing info to gpm

    vector<pair<string, bool>> list_needed_attribute_names(const vector<string> &all_atts) const;//<string, bool> pairs of names properties needed from gpm

    vector<property_type> list_wanted_attribute_names(bool include_top = true);// const;

    set<string> list_visage_required_array_names();

    //fields

    StructuredGrid& geometry() { return _visage_options->geometry(); }

    //running external vs process

    int run_timestep(const attr_lookup_type& attributes, std::string& error);

    bool  read_wanted_visage_results(int step, string &error);

    bool  read_visage_results(int step, string &error);

    bool  copy_wanted_visage_to_gpm(map<string, vector<Slb::Exploration::Gpm::Api::array_2d_indexer<float>>>  &attributes, string &error);

    bool  update_gpm_and_visage_geometris_from_visage_results(map<string, vector<Slb::Exploration::Gpm::Api::array_2d_indexer<float>>>  &attributes, string &error);

    int  run_visage(string mii_file);

    //gpm_plugin_api_process_attribute_parms *process_params;
    //const attr_lookup_type* _gpm_attr_lookup_type;
    //StructuredGeometry gpm_geometry;

    bool _error;
    bool was_error() const { return _error; }
    bool& was_error() { return _error; }

private:

    bool copy_visage_to_attribute(gpm_attribute &prop, const string &name, string &error)
    {
        //debug

        auto[ncols, nrows, nsurfaces, total_nodes, total_elements] = _visage_options->geometry()->get_geometry_description();
        std::vector<float> nodal_values;
        {
            if (!_data_arrays.contains(name))
            {
                cout << "[copy_visage_to_attribute] visage array " << name << " not found when copying ts values to gpm" << endl;
                return false;
            }

            vector<float>& values = _data_arrays.get_array(name);
            nodal_values.resize(total_nodes);

            if (values.size() == total_elements)
            {
                //We need to pass them to gpm as nodal properties.
                nodal_values = StructuredGrid::elemental_to_nodal(ncols, nrows, nsurfaces, values);//, values);
            }
            else if (values.size() == total_nodes) //inneficient !!
            {
                copy(begin(values), end(values), begin(nodal_values));
            }
            else if (values.size() == 1) //constant !!
            {
                nodal_values.resize(total_nodes, values[0]);
            }

            else
            {
                error += "\n[update_gpm_geometry_from_visage] The number of values read does not match elements or nodes when reading X files";
                return false;
            }
        }

        int counter = 0, offset = 0;
        for (int surface = 0; surface < nsurfaces; surface++)
        {
            auto& my_array_holder = prop.at(surface);
            for (int j = 0; j < nrows; j++)
            {
                for (int i = 0; i < ncols; i++)
                {
                    //prop[surface](j, i) = nodal_values[offset + counter];
                    my_array_holder(j, i) = nodal_values[offset + counter];
                    counter += 1;
                }
            }
        }

        return true;
    }
    //helpers
    string get_visage_name_or_same(std::string gpm_name)
    {
        return _gpm_visage_prop_name_map.find(gpm_name) == _gpm_visage_prop_name_map.end() ? gpm_name : _gpm_visage_prop_name_map[gpm_name];
    }

    vector<float> get_gpm_difference(const gpm_attribute &top, int k1, int k2)
    {
        int counter = 0;
        vector<float> diff(top[k1].num_rows() * top[k1].num_cols(), 0.0f);
        //diff.resize(top[k1].num_rows() * top[k1].num_cols(), 0.0f);

        for (int row = 0; row < top[k1].num_rows(); row++)
        {
            for (int col = 0; col < top[k1].num_cols(); col++)
            {
                diff[counter++] = top[k1](row, col) - top[k2](row, col);
            }
        }

        return diff;
    }

    vector<float> get_gpm_height(const vector<Slb::Exploration::Gpm::Api::array_2d_indexer<float>> &top, int k1)
    {
        int counter = 0;
        vector<float> height(top[k1].num_rows() * top[k1].num_cols(), 0.0f);

        for (int row = 0; row < top[k1].num_rows(); row++)
        {
            for (int col = 0; col < top[k1].num_cols(); col++)
            {
                height[counter++] = top[k1](row, col);
            }
        }

        return height;
    }



    gpm_plugin_api_model_definition  gpm_model_definition;
    map<string, sediment_description> sediment_descriptions;

    int _time_step;

    std::map<string, string> _gpm_visage_prop_name_map;

    std::shared_ptr< StructuredSurface> old_gpm_basement_ptr;
    std::shared_ptr< StructuredSurface> new_gpm_basement_ptr;
    std::shared_ptr< StructuredSurface> old_gpm_top_ptr;
    std::shared_ptr< StructuredSurface> new_gpm_top_ptr;

    VisageDeckSimulationOptions _visage_options;
    VisageDeckWritter _deck_writter;
    VisageResultsReader  _vs_results_reader;
    ArrayData _data_arrays;

    //experimental. Temporal
    Table stiffness_mult_table;
    vector<float> gpm_below, gpm_above, last_known_layer;


    void export_all_surfaces_gpm(string suffix);

    void export_all_surfaces_visage(string suffix)
    {

    }

};

#endif
