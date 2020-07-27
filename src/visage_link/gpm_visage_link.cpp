#include <iostream>
#include <fstream>
#include <string>

#include <iterator>
#include <algorithm>
#include <functional>
#include <tuple>
#include <vector>
#include <map>
#include "Vector3.h"
#include "Range.h"
#include "StructuredGrid.h"

#include "gpm_visage_link.h"
#include "gpm_visage_results.h"
#include "gpm_plugin_description.h"
#include "gpm_plugin_helpers.h"

using namespace std;
using attr_lookup_type = std::map<std::string, std::vector<Slb::Exploration::Gpm::Api::array_2d_indexer<float>>>;
using gpm_attribute = vector<Slb::Exploration::Gpm::Api::array_2d_indexer<float>>;

//initialization
gpm_visage_link::gpm_visage_link( )
    {
        //this is a HARD-CODED default configuration. It will be removed. 
    initialize_defaut_gpm_visage_options( );
    }


    //HARD-CODED CONSTANTS AND CONFIGURATIONS USED ONLY DURING DEBUGGING/DEVELOPMENT
void gpm_visage_link::initialize_defaut_gpm_visage_options( )
    {
    cout << "initialize_defaut_gpm_visage_options" << std::endl;
    _gpm_visage_prop_name_map["POR"] = "POROSITY"; //this means "POR" in gpm and "POROSITY" in visage. 
    _time_step = 0;

    //the defauls were tailored for gpm. edgeloads =0, constraints =0, apply displacements.
    _visage_options->pinchout_tolerance( ) = 0.001f;
    _visage_options->path( ) = "D:\\GPMTESTS\\VISAGE_IO";

    _visage_options->sea_water_density( ) = 1000.00;
    _visage_options.model_name( ) = "PALEOV3";

    vector< pair<string, float> > elastic_defaults
        {
            {"YOUNGMOD",11.0E5}, //KPA
            {"POISSONR",0.3},
            {"DENSITY",2700.0},    //KG/M3
            {"BIOTC",1.0},
            {"POROSITY",0.3}
        };

    vector< pair<string, float> > plastic_defaults
        {
            {"COHESION",50.0e3}, //KPA
            {"FRICTION",30.0},
            {"DILATION",15.0},
            {"FLUIDITY",1.0},
            {"HARDENING",-0.005},
            {"TENSILE_STRENGTH",50.0e3},
            {"MAX_COHESION_CHANGE",0.01} //KPA
        };
    for (auto &default_value : elastic_defaults)
        _data_arrays->set_array( default_value.first, default_value.second );

    //for (auto &default_value : plastic_defaults)
    //    _data_arrays->set_array(default_value.first, default_value.second);

    /////////
    sediment_descriptions = sediment_description::get_default_sediments( 10 );

    _visage_options->set_boundary_condition( new StrainBoundaryCondition( 0, -0.001f ) );
    _visage_options->set_boundary_condition( new StrainBoundaryCondition( 1, 0.0f ) );
    _visage_options->set_boundary_condition( new DisplacementSurfaceBoundaryCondition( 2 ) );

    cout << "Visage IO path: " << _visage_options->path( ) << std::endl;
    cout << "Model name: " << _visage_options.model_name( ) << std::endl;
    cout << "Boundary conditions: " << endl;


    vector<float> poro{ 0.0f,	0.02f,	0.05f,	0.1f,	0.15f,	0.2f,	0.25f,	0.3f,	0.35f,	0.4f,	0.45f,	0.5f,	0.6f,	0.65f };
    vector<float> ym_mult{ 30.0f,	 30.0f,	20.0f,	     10.f,	  4.0f,	  2.0f,	   2.0f,     1.0f,	1.0f,   0.75f,	0.5f,	0.2f,	0.1f,	0.05f };

    //vector<float> ym_mult{ 1,	1,	1,	1,	1,	1,	1, 1,	1,	1,	1,	1,	1,	1 };

    stiffness_mult_table.set_values( poro, ym_mult );
    stiffness_mult_table.name( ) = "Stiffness_Multiplier";
    stiffness_mult_table.controller_var_name( ) = "porosity";
    stiffness_mult_table.dependant_name( ) = "e_Mult";

    _visage_options->use_tables( ) = true;
    _visage_options->add_table( 1, stiffness_mult_table );
    }

void gpm_visage_link::initialize_visage_geometry( )
    {
    gpm_plugin_api_model_definition *model_def = &gpm_model_definition;
    const float *x = model_def->x_coordinates;
    const float *y = model_def->y_coordinates;

    fVector3 li( x[1] - x[0], y[1] - y[0], 0.0 );
    fvector3 lj( x[3] - x[0], y[3] - y[0], 0.0 );

    cout << "Initializing vs geometry extents" << endl;

    int initial_layers = 0;
    _visage_options->geometry( ) = StructuredGrid(
        (int)model_def->num_columns,
        (int)model_def->num_rows,
        initial_layers,
        { sqrt( li.selfDot( ) ), sqrt( lj.selfDot( ) )
        },
        CoordinateMapping3D
        {
            li.normalize( ),
            lj.normalize( ),
            {0.0, 0.0, 1.0}, /**/
            {0.0,0.0,0.0}
            //{x[0], y[0], 0.0}
        } );
    std::cout << _visage_options->geometry( ) << std::endl;
    }

void gpm_visage_link::initialize_model_extents( const gpm_plugin_api_model_definition *model_def )
    {
        //struct gpm_plugin_api_model_definition {
        //    size_t num_rows;
        //    size_t num_columns;
        //    float x_coordinates[4];
        //    float y_coordinates[4];
        //};

        //just keep a copy
    gpm_model_definition.num_columns = model_def->num_columns;
    gpm_model_definition.num_rows = model_def->num_rows;
    std::copy( model_def->x_coordinates, model_def->x_coordinates + 4, gpm_model_definition.x_coordinates );
    std::copy( model_def->y_coordinates, model_def->y_coordinates + 4, gpm_model_definition.y_coordinates );
    //gpm_geometry = _visage_options->geometry();
    }

set<string> gpm_visage_link::list_visage_required_array_names( ) //const
    {
    set<string> names{
    "STRAINXX"
    ,"STRAINYY"
    ,"STRAINZZ"
    ,"PRESSURE"
    , "TOTSTRXX"
    , "TOTSTRYY"
    , "TOTSTRZZ"
    , "TOTSTRXY"
    , "TOTSTRYZ"
    , "TOTSTRZX"
    , "YOUNGMOD"
    , "ROCKDISZ"
    , "NRCKDISZ"

    ,"PLSTRNXX"
    ,"PLSTRNYY"
    ,"PLSTRNZZ"
    ,"PLSTRNXY"
    ,"PLSTRNYZ"
    ,"PLSTRNZX"
        };

    return names;
    }

std::vector<gpm_visage_link::property_type> gpm_visage_link::list_wanted_attribute_names( bool include_top ) //const
    {
    std::vector<property_type> atts = {
        { "PRESSURE", false }
        ,{ "TOTSTRXX", false }
        ,{ "TOTSTRYY", false }
        ,{ "TOTSTRZZ", false }
        ,{ "TOTSTRXY", false }
        ,{ "TOTSTRYZ", false }
        ,{ "TOTSTRZX", false }
        ,{ "YOUNGMOD", false }

        ,{ "CMP_POROSITY", false}
        ,{ "CMP_YMOD", false}

        ,{ "STRAINXX", false}
        ,{ "STRAINYY", false}
        ,{ "STRAINZZ", false}

        //,{ "EFFSTRXX", false }
        //,{ "EFFSTRYY", false }
        //,{ "EFFSTRZZ", false }
        //,{ "EFFSTRXY", false }
        //,{ "EFFSTRYZ", false }
        //,{ "EFFSTRZX", false }

        ,{ "ROCKDISX", false }
        ,{ "ROCKDISY", false }
        ,{ "ROCKDISZ", false }
        ,{ "NRCKDISX", false }
        ,{ "NRCKDISY", false }
        ,{ "NRCKDISZ", false }

        ,{ "YIELDMOD", false }
        ,{ "YLDVAL_F", false }

    ,{"PLSTRNXX",false}
    ,{"PLSTRNYY",false}
    ,{"PLSTRNZZ",false}
    ,{"PLSTRNXY",false}
    ,{"PLSTRNYZ",false}
    ,{"PLSTRNZX",false}
        };

    if (include_top)
        atts.push_back( { "TOP", false } );

    return atts;
    }

std::vector<std::pair<std::string, bool>> gpm_visage_link::list_needed_attribute_names( const vector<string> &all_atts ) const
    {
        //we need the top
    vector<pair<string, bool>> required{ {"TOP", false}, {"SEALEVEL",false} };

    if (find( all_atts.begin( ), all_atts.end( ), "POR" ) != all_atts.end( ))
        {
        required.push_back( { "POR",false } );
        }

        //we need everything that starts with SED: SED1, SED2....these are in the .sed.json file as well.
    for_each( begin( all_atts ), end( all_atts ), [&required, key = "SED"]( const string &name )
        {
        if (name.find( key ) != std::string::npos)
            required.push_back( { name, false } );
        } );

    return required;
    }

    //updating

bool gpm_visage_link::update_needed_attribute_values( const attr_lookup_type& attributes, ArrayData &data )//, bool flip_surfaces)
    {
    vector<string> all_atts;
    for (auto &p : attributes) all_atts.push_back( p.first );

    std::vector<std::pair<std::string, bool>> props = list_needed_attribute_names( all_atts );
    auto vs_name_if_known = [&]( string gpm_name ) -> std::string
        {
        return _gpm_visage_prop_name_map.find( gpm_name ) == _gpm_visage_prop_name_map.end( ) ? gpm_name : _gpm_visage_prop_name_map[gpm_name];
        };

        //these are properties that are distributed in each node. sealevel, for instance, is not. It is a single value
    for (auto& p : props)
        {
        if ((p.first != "TOP") && (p.first != "SEALEVEL"))
            {
            std::vector<float>&values = data.get_or_create_array( vs_name_if_known( p.first ) );
            read_gpm_property_values( attributes, p, values );
            }
        }

    return true;
    }

void gpm_visage_link::read_gpm_property_values( const attr_lookup_type& attributes, const std::pair<std::string, bool> &prop, std::vector<float>&nodal_values )
    {
    const auto& geom = attributes.at( "TOP" );
    const auto& por = attributes.at( prop.first );

    int  nrows = (int)geom.at( 0 ).num_rows( ), ncols = (int)geom.at( 0 ).num_cols( );
    int  count = (int)geom.size( )*nrows*ncols; //numsurfaces*ncols*nrows
    //std::vector<float> values(count);

    nodal_values.resize( count );
    int counter = 0, offset = 0;
    for (int k = 0; k < por.size( ); k++)
        {
        int  ii = !false ? k : (int)geom.size( ) - 1 - k;
        int offset = !false ? k * nrows*ncols : ii * ncols*nrows;
        counter = 0;
        auto& p = por.at( ii );
        for (int j = 0; j < nrows; ++j)
            {
            for (int i = 0; i < ncols; ++i)
                {
                nodal_values[offset + counter++] = p( j, i );
                }
            }
        }
    }

void gpm_visage_link::update_top_history( StructuredGrid &geometry )
    {
    cout << "Updating top history" << endl;
    old_gpm_top_ptr = std::move( new_gpm_top_ptr );
    int top_surface_index = geometry.nsurfaces( ) - 1;
    new_gpm_top_ptr.reset( new StructuredSurface( geometry->get_structured_surface( top_surface_index ) ) );
    }

void gpm_visage_link::update_basement_history( const gpm_attribute &top )//StructuredGeometry &geometry, bool flip_surface_order)
    {
    cout << "[update_basement_history] Updating basement history. " << endl;

    old_gpm_basement_ptr = std::move( new_gpm_basement_ptr );

    new_gpm_basement_ptr.reset( new StructuredSurface( _visage_options->geometry( )->get_structured_surface( 0 ) ) );

    vector<float> &heights = new_gpm_basement_ptr->get_heights( );
    int counter = 0;
    auto &holder = top.at( 0 );
    for (int row = 0; row < top.at( 0 ).num_rows( ); row++)
        {
        for (int col = 0; col < top.at( 0 ).num_cols( ); col++)
            {
            heights[counter++] = holder( row, col );
            }
        }

        //if I have and old and a new, it means that I am already getting a potentially modified
        //basement.  There is already some history. Lets figure out the displacements to be used as BC.
    vector<float> displacement;
    DisplacementSurfaceBoundaryCondition *bc = static_cast<DisplacementSurfaceBoundaryCondition*>(_visage_options->get_boundary_condition( 2 ));
    if (old_gpm_basement_ptr)
        {
            //lets see the basement displacement
        transform( begin( new_gpm_basement_ptr->get_heights( ) ),
            end( new_gpm_basement_ptr->get_heights( ) ),
            old_gpm_basement_ptr->get_heights( ).begin( ),
            back_inserter( displacement ), []( const float &h1, const float& h2 )
            {
            return h1 - h2;
            } );
        bc->set_node_displacement( displacement );
        }
    else
        {
        bc->clear_displacement( );
        displacement.resize( new_gpm_basement_ptr->total_nodes( ), 0.0f );
        }
    }

bool gpm_visage_link::read_geometry_from_gpm_top( StructuredGrid &geometry, const attr_lookup_type& attributes )//, bool flip = false)
    {
    auto &top = attributes.at( "TOP" );
    geometry.set_num_surfaces( (int)top.size( ) );

    //for(auto k : IntRange( top.size())
    //{   
    //auto[ it1, it2 ] = gpm_att_iterator::begin_end_surface( k );
    //std::copy(it1, it2, geometry->begin_surface( k ));
    //}



    //get the z's of each surface starting from the bottom up and copying them top-bottom (as vs needs)
    int  nrows = (int)top.at( 0 ).num_rows( ), ncols = (int)top.at( 0 ).num_cols( ), count = (int)top.size( )*nrows*ncols;

    //temporary placeholder
    std::vector<float> zvalues( geometry->nodes_per_layer( ) );

    int counter = 0, offset = 0;
    for (int k = 0; k < top.size( ); k++)
        {
        counter = 0;
        auto& p = top.at( k );

        for (auto nrow = 0; nrow < nrows; ++nrow)
            {
            for (auto ncol = 0; ncol < ncols; ++ncol)
                {
                zvalues[counter++] = p( nrow, ncol );
                }
            }
        int n = !false ? k : ((int)top.size( ) - 1) - k;
        geometry->set_z_values( n, zvalues.data( ) );
        }

    return true;
    }

    //all this just gets values from gpm
    //reads all the surfaces as they come ans stores them in gometrty();
    //update_geometry_from_gpm(attributes, flip_surface_order);

    /*old version, the plugin takes care of coordinate transforms
    //vs coordinate reference
    CoordinateMapping3D vs(fvector3(1.0, 0.0, 0), fvector3(0.0, 0.0, -1.0), fvector3(0.0, 1.0, 0.0)); //"VS Coordinate reference
    //gpm-vs coordinate reference is gpm's one, which in turn appears to be Petrel one.
    StructuredGeometry &_geometry = _visage_options->geometry();
    vector<float> xyz = _geometry->get_local_coordinates();
    _geometry->reference().convert_to(xyz, vs);
    //update whatever options we need for the next step.
    _visage_options->update_step(0 > _time_step ? 0 : _time_step);
    string mii_file_name = VisageDeckWritter::write_deck(&_visage_options, &xyz, &_data_arrays);
    _time_step += 1;
    */

bool  gpm_visage_link::read_visage_results( int last_step, string &error )
    {
    std::vector<gpm_visage_link::property_type> props_wanted = list_wanted_attribute_names( false );

    //set<string> wanted = list_visage_required_array_names();

    string file_to_parse = _vs_results_reader.get_results_file( _visage_options->model_name( ), _visage_options->path( ), last_step );
    vector<string> names = _vs_results_reader.get_key_names( file_to_parse );

    int ret_code = 0, total_read = 0;
    if (!file_to_parse.empty( ))
        {

        cout << "\n\nGeometry surfaces: " << geometry( )->nsurfaces( ) << " rows: " << geometry( )->nrows( ) << " cols: " << geometry( )->ncols( ) << " elements:" << geometry( )->total_elements( ) << " nodes " << geometry( )->total_nodes( ) << endl;
        map<string, float> unit_conversion = { {"YOUNGMOD",1.00} }; //FIXME
        for (auto prop : props_wanted)
            {
            string name = prop.name;

            //if (_data_arrays.contains( name ))
             //   cout << "\n\nEntered in rwding " << name << " with n values " << _data_arrays->get_array( name ).size( ) << endl;
           //else
            //    cout << "Entered in rwding " << name << " with n values " << 0 << endl;


            if (name == "YOUNGMOD")
                total_read += (1 - _vs_results_reader.read_result( file_to_parse, name, _data_arrays, &unit_conversion, "CMP_YMOD" ));
            else
                total_read += (1 - _vs_results_reader.read_result( file_to_parse, name, _data_arrays, &unit_conversion ));

            //cout << "\n\nLeaving in rwding " << name << " with n values " << _data_arrays->get_array( name ).size( ) << endl;

            }
        }
    else
        {
        error += "\nError parsing results from geomechanics simulation. X file not found";
        return false;
        }

        //some results are derived from others. Example: porosity. We will compute it here as well.
    vector<float>& cmp_porosity = _data_arrays.get_array( "CMP_POROSITY" );
    vector<float>& initial_porosity = _data_arrays.get_array( "POROSITY" );

    //the total volumetric strain: assume it is cummulative.
    vector<float>& exx = _data_arrays.get_array( "STRAINXX" );
    vector<float>& eyy = _data_arrays.get_array( "STRAINYY" );
    vector<float>& ezz = _data_arrays.get_array( "STRAINZZ" );

    //for (auto n : IntRange(0, ezz.size()))
    //{
    //    float delta = (-exx[n] - eyy[n] - ezz[n]);
    //    cmp_porosity[n] = std::min<float>(0.8f, std::max<float>(0.01f, initial_porosity[n] + delta));
    //}

    return true;//total_read > 0 ? true : false;
    }

    //obsolete/ not in use.
bool  gpm_visage_link::read_wanted_visage_results( int last_step, string &error )
    {
    bool read_top = false;
    vector<string> names;
    std::vector<property_type> wanted = list_wanted_attribute_names( read_top );
    std::transform( begin( wanted ), end( wanted ), back_inserter( names ), []( const property_type &p ) { return p.name; } );

    string model_name = _visage_options->model_name( ),
        path = _visage_options->path( ),
        file_to_parse = _vs_results_reader.get_results_file( model_name, path, last_step );

    int ret_code = 0, total_read = 0;
    if (!file_to_parse.empty( ))
        {
        map<string, float> unit_conversion = { {"YOUNGMOD",1000.00f} };
        for (auto name : names)
            {
            total_read += (1 - _vs_results_reader.read_result( file_to_parse, name, _data_arrays, &unit_conversion ));
            }
        }
    else
        {
        error += "\nError parsing results from geomechanics simulation. X file not found";
        return false;
        }

    return total_read > 0 ? true : false;
    }

bool  gpm_visage_link::copy_wanted_visage_to_gpm( map<string, gpm_attribute>  &attributes, string &error )
    {
    bool update_top = false, success = true;
    std::vector<property_type> wanted = list_wanted_attribute_names( update_top );

    for (auto &p : wanted)
        {
        gpm_attribute &att = attributes.at( p.name );
        if (_data_arrays.contains( p.name ))
            success &= copy_visage_to_attribute( att, p.name, error );
        }

    return success;
    }

    //StructuredGeometry &vs_geometry = _visage_options->geometry();
    //int ncols = vs_geometry->ncols(), nrows = vs_geometry->nrows(), nsurfaces = vs_geometry->nsurfaces(),
    //total_nodes = vs_geometry->total_nodes(), total_elements = vs_geometry.num_cells();
    //
    //bool update_top = false;
    //std::vector<property_type> wanted = list_wanted_attribute_names(update_top);
    //std::vector<float> nodal_values;
    //for (auto &p : wanted)
    //{
    //    int offset = 0, counter = 0;
    //    if (!_data_arrays.contains(p.name))
    //    {
    //        error += "\nTrying to update a result: " + p.name + " that wasnt read from the X files";
    //        return 1;
    //    }
    //
    //    vector<float> &values = _data_arrays->get_array(p.name);
    //    if (values.size() == total_elements)
    //    {
    //        //We need to pass them to gpm as nodal properties.
    //        StructuredGeometry::elemental_to_nodal(ncols, nrows, nsurfaces, nodal_values, values);
    //    }
    //    else if (values.size() == total_nodes)
    //    {
    //        copy(begin(values), end(values), begin(nodal_values));
    //    }
    //    else
    //    {
    //        error += "\nThe number of values read does not match elements or nodes when reading X files";
    //        return 1;
    //    }
    //
    //    //update gpm stuff.
    //    //do we need to flip the vertical?
    //    auto& pres = attributes.at(p.name); //this is gpm stuff
    //    for (int surface = 0; surface < nsurfaces; surface++)
    //    {
    //        //where to start copying from my array,
    //        if (flip_surfaces())
    //        {
    //            offset = (int)nodal_values.size() - (ncols*nrows)*(surface + 1);
    //            counter = 0;
    //        }
    //
    //        for (int j = 0; j < nrows; j++)
    //        {
    //            for (int i = 0; i < ncols; i++)
    //            {
    //                pres[surface](j, i) = nodal_values[offset + counter];
    //                counter += 1;
    //            }
    //        }
    //    }
    //
    //}
    //
    //return true;
    //}
    //

bool gpm_visage_link::update_gpm_and_visage_geometris_from_visage_results( map<string, gpm_attribute>  &attributes, string &error )
    {
        //basically, we modify now the property "TOP" using the cummulated displacements in visage.
    if (attributes.find( "TOP" ) == attributes.end( ))
        {
        error += "[update_gpm_geometry_from_visage] Attribute TOP was not found ";
        cout << error << endl;
        return false;
        }
    auto prop = attributes.at( "TOP" );

    std::vector<float> nodal_values;
    StructuredGrid &geometry = _visage_options->geometry( );
    auto[ncols, nrows, nsurfaces, total_nodes, total_elements] = geometry->get_geometry_description( );

    nodal_values.resize( total_nodes );
    if (_data_arrays.contains( "NRCKDISZ" ))
        {
        std::vector<float> &values = (_data_arrays.get_array( "NRCKDISZ" ));
        copy( begin( values ), end( values ), begin( nodal_values ) );
        }
    else if (_data_arrays.contains( "ROCKDISZ" ))
        {
        std::vector<float> &values = (_data_arrays.get_array( "ROCKDISZ" ));
        nodal_values = StructuredBase::elemental_to_nodal( ncols, nrows, nsurfaces, values );//, values);
        }
    else
        {
        error += "\n[update_gpm_geometry_from_visage] The number of values read does not match elements or nodes when reading X files";
        cout << error << endl;
        return false;
        }

        //this is the gpm part
        //debug: at this point, bas ein vs must be identical to base in gpm

    int counter = 0, nodes_per_layer = geometry->nodes_per_layer( );
    for (int surface = 1; surface < nsurfaces; surface++) //from 1 because 0 is base and vs does not change the base.
        {
        counter = 0;
        vector<float> &vs_depths = geometry->get_local_depths( surface );

        auto& holder = prop[surface];
        for (int j = 0; j < nrows; j++)
            {
            for (int i = 0; i < ncols; i++)
                {
                holder( j, i ) = vs_depths[counter] + nodal_values[surface*nodes_per_layer + counter];
                counter += 1;
                }
            }
        }

        //now the visage part !!!!!! the displacements we just got, are NOT CUMMULATIVE.
    geometry->displace_all_nodes( nodal_values );

    return true;
    }

int  gpm_visage_link::run_visage( string mii_file )
    {
    std::cout << "Calling eclrun visage " << mii_file << "  --np=4" << std::endl;
    std::string command = "eclrun visage  " + mii_file + "   --np=4";
    int ret_code = system( command.c_str( ) );
    std::cout << "return code  " << ret_code << std::endl;

    return ret_code;
    }

vector<float> gpm_visage_link::get_gpm_property_from_seds( const attr_lookup_type& attributes, string property_name )
    {
    if (attributes.find( property_name ) != attributes.end( ))
        return get_values( attributes.at( property_name ) );

    vector<string> sed_keys = {};
    for_each( begin( attributes ), end( attributes ), [&sed_keys, key = "SED"]( const pair<string, gpm_attribute> &att )
        {
        if (att.first.find( key ) != std::string::npos) sed_keys.push_back( { att.first, false } );
        } );

    vector<float> ret_array;
    for (string key : sed_keys)
        {   //these are the weights
        const auto& p = attributes.at( key );
        vector<float> weights = get_values( p );//_visage_options->geometry()->nodal_to_elemental(get_values(p));//, weights); //these are all nodal

        //this is the value
        const auto &sediment = sediment_descriptions[key];
        float val = sediment_descriptions[key][property_name];
        ret_array.resize( weights.size( ), 0.0f ); //do not use val here, because there may be data that we need to override

        for (int n : IntRange( 0, (int)weights.size( ) ))
            ret_array[n] += val * weights[n];
        }

    return ret_array;
    }

    //will copy a youngs modulus a density and a porosity to the data_arrays
void gpm_visage_link::initialize_mechanical_properties( int scenario, const attr_lookup_type& attributes, int old_num_surfaces, int new_num_surfaces )
    {
        //this will be moved to a PropertyUpdater class
    if (scenario == 1)
        {
            //read the porosity from gpm
        vector<float> nodal_porosities = get_gpm_property_from_seds( attributes, "POROSITY" );
        vector<float> elemental_porosities = geometry( )->nodal_to_elemental( nodal_porosities );

        //get geometry descrition
        auto[ncols, nrows, nsurfaces, total_nodes, total_elements] = _visage_options->geometry( )->get_geometry_description( );

        int old_element_count = (ncols - 1)*(nrows - 1)*(old_num_surfaces > 0 ? (old_num_surfaces - 1) : 0);
        int new_element_count = (ncols - 1)*(nrows - 1)* (new_num_surfaces > 0 ? (new_num_surfaces - 1) : 0);
        int offset = old_element_count;

        //copy that porosity only in the new cells. This will be the starting porosity for visage
        string porosity_keys[] = { "CMP_POROSITY","POROSITY" };
        for (auto prop_name : porosity_keys)
            {
            auto &data_array = _data_arrays.get_or_create_array( prop_name );
            if (data_array.size( ) != total_elements)
                {
                data_array.resize( total_elements ); //do not use resize(xx,val) here)!!
                std::fill( data_array.begin( ) + offset, data_array.end( ), 0.0 );
                }

            for (int n : IntRange( offset, total_elements ))
                data_array[n] = elemental_porosities[n];
            }

            //we need the initial condition for YM, DENS, PR, these come as effective properties of the SEDii
            //first list the SEDs we have and
        vector<string> sed_keys = {};
        for_each( begin( attributes ), end( attributes ), [&sed_keys, key = "SED"]( const pair<string, gpm_attribute> &att )
            {
            if (att.first.find( key ) != std::string::npos) sed_keys.push_back( { att.first, false } );
            } );

        for (string key : sed_keys)
            {   //these are the weights
            const auto& p = attributes.at( key );
            vector<float> weights = _visage_options->geometry( )->nodal_to_elemental( get_values( p, 0, new_num_surfaces ) );//, weights); //these are all nodal

            //all of these are elemental properties.
            for (string prop : { "YOUNGMOD", "DENSITY", "POISSONR"})
                {
                    //this is the value
                const auto &sediment = sediment_descriptions[key];
                float rho = sediment_descriptions[key][prop];

                auto &data_array = _data_arrays.get_or_create_array( prop );
                if (data_array.size( ) != total_elements)
                    {
                    data_array.resize( total_elements ); //do not use val here, because there may be data that we need to override
                    std::fill( data_array.begin( ) + offset, data_array.end( ), 0.0 );
                    }

                for (int n : IntRange( offset, total_elements ))
                    data_array[n] += rho * weights[n];
                }
            }
        }
    }

    //
    //
    //
    //    if(scenario == 1000 )
    //    {
    //        //read the porosity from gpm
    //        vector<float> nodal_porosities = get_values( attributes.at("POR"));
    //        vector<float> elemental_porosities = geometry()->nodal_to_elemental( nodal_porosities );
    //
    //        //we need the initial condition for YM, DENS,
    //        //there is one single table for all the model. the model REGION = 1 everywhere
    //    }
    //
    //
    //
    //
    //    if(scenario == 3000)
    //    {
    //    //first list the SEDs we have and
    //    vector<string> sed_keys = {};
    //    for_each(begin(attributes), end(attributes), [&sed_keys, key = "SED"](const pair<string, gpm_attribute> &att)
    //    {
    //        if (att.first.find(key) != std::string::npos) sed_keys.push_back({ att.first, false });
    //    });
    //
    //    //set_array("YOUNGMOD", 5.00e7f);
    //    //set_array("POISSONR", 0.3f);
    //    //set_array("DENSITY", 25.0*4.00144f);// 400.144f);
    //    //set_array("BIOTC", 1.0f);
    //    //set_array("POROSITY", 0.0f);
    //
    //    int ncols, nrows, nsurfaces, total_nodes, total_elements;
    //    std::tie(ncols, nrows, nsurfaces, total_nodes, total_elements) = _visage_options->geometry()->get_geometry_description();
    //    int old_element_count = (ncols - 1)*(nrows - 1)*(old_num_surfaces > 0 ? (old_num_surfaces - 1) : 0);
    //    int new_element_count = (ncols - 1)*(nrows - 1)* (new_num_surfaces > 0 ? (new_num_surfaces - 1) : 0);
    //    int offset = old_element_count;
    //
    //
    //    for (string key : sed_keys)
    //    {   //these are the weights
    //        const auto& p = attributes.at(key);
    //        vector<float> weights =
    //            _visage_options->geometry()->nodal_to_elemental(ncols, nrows, nsurfaces, get_values(p, 0, new_num_surfaces));//, weights); //these are all nodal
    //
    //            //all of these are elemental properties.
    //        for (string prop : { "DENSITY", "POROSITY", "YOUNGMOD", "POISSONR"})
    //        {
    //            //this is the value
    //            const auto &sediment = sediment_descriptions[key];
    //            float rho = sediment_descriptions[key][prop];
    //
    //            auto &data_array = _data_arrays.get_or_create_array(prop);
    //            if (data_array.size() != total_elements)
    //            {
    //                data_array.resize(total_elements); //do not use val here, because there may be data that we need to override
    //                std::fill(data_array.begin() + offset, data_array.end(), 0.0);
    //            }
    //
    //            for (int n : range(offset, total_elements))
    //                data_array[n] += rho * weights[n];
    //        }
    //    }
    //
    //    }
    //
    //
    //}

int gpm_visage_link::solve_schema1( const attr_lookup_type& attributes, std::string& error )
    {
        //this is the last known geometry for visage, which can be totally uninitialized
    StructuredGrid &geometry = _visage_options->geometry( );

    //if we have zero surfaces, we havent even initialize the vs geometry.It must be the 1st step
    if (geometry->nsurfaces( ) == 0)
        {
        initialize_visage_geometry( ); //sets coordinate reference, ncols, nrows, extents, etc. no surfaces
        read_geometry_from_gpm_top( geometry, attributes );
        initialize_mechanical_properties( 1, attributes, 0, geometry->nsurfaces( ) );
        }

    //now we also need to update the basement. GPM could have changed it
    const gpm_attribute &top = attributes.at( "TOP" );
    update_basement_history( top );

    //old: visage knows about   new: gpm comes with extra ones. all listed bottom-up
    int old_num_surfaces = geometry->nsurfaces( ), new_num_surfaces = top.size( );

    geometry->set_num_surfaces( new_num_surfaces );
    update_needed_attribute_values( attributes, _data_arrays );
    initialize_mechanical_properties( 1, attributes, old_num_surfaces, new_num_surfaces );

    _visage_options->update_step( 0 > _time_step ? 0 : _time_step );
    if (old_num_surfaces != new_num_surfaces)
        {
          //solve first the base, without adding anything.
        geometry->set_num_surfaces( old_num_surfaces );

        string mii_file_name = VisageDeckWritter::write_deck( &_visage_options, &_data_arrays );
        if (run_visage( mii_file_name ) != 0)
            {
            error += ("Visage run failed.  MII file: " + mii_file_name);
            was_error( ) = true;
            }
        }

    geometry->set_num_surfaces( new_num_surfaces );
    auto gpm_only_this_step = get_gpm_height( top, new_num_surfaces - 1 );
    auto &znew = geometry->get_local_depths( new_num_surfaces - 1 );
    auto &zbelow = geometry->get_local_depths( new_num_surfaces - 2 );


    for (auto &n : IntRange( 0, (int)znew.size( ) ))
        znew[n] = /*zbelow[n] +*/gpm_only_this_step[n];

    string mii_file_name = VisageDeckWritter::write_deck( &_visage_options, &_data_arrays );
    if (run_visage( mii_file_name ) != 0)
        {
        error += ("Visage run failed.  MII file: " + mii_file_name);
        was_error( ) = true;
        }

    return 0;
    }

    //update visage geometry, update visage required properties from gpm then runs visage
int gpm_visage_link::run_timestep( const attr_lookup_type& attributes, std::string& error )
    {
    static int run_timestep_counter = -1;
    std::cout << "[run_timestep] run_timestep counter " << ++run_timestep_counter << endl;
    if (run_timestep_counter == 0) was_error( ) = false;

    //at present, we dont have a way of stopping the GPM engine when we have an error in VS. 
    //so the best at present is to skip future VS steps and finish as quick as possible. 
    if (was_error( ))
        {
        cout << endl << "---------skipping simulation of step " << run_timestep_counter << "------------" << endl << endl;
        return 0;
        }

    int solution_schema = 1;
    int ret_value = 0;
    if (solution_schema == 1)
        {
        solve_schema1( attributes, error );
        _time_step += 1;
        return error.empty( ) ? 0 : 1;
        }


        //this is the last known geometry for visage, which can be totally uninitialized
    StructuredGrid &geometry = _visage_options->geometry( );

    //if we have zero surfaces, we havent even initialize the vs geometry.
    //It must be the 1st step
    if (geometry->nsurfaces( ) == 0)
        {
        initialize_visage_geometry( ); //sets coordinate reference, ncols, nrows, extents, etc. no surfaces
        read_geometry_from_gpm_top( geometry, attributes );
        initialize_mechanical_properties( 1, attributes, 0, geometry->nsurfaces( ) );
        }

        //now we also need to update the basement. GPM could have changed it
    const gpm_attribute &top = attributes.at( "TOP" );
    update_basement_history( top );

    //old: visage knows about   new: gpm comes with extra ones. all listed bottom-up
    int old_num_surfaces = geometry->nsurfaces( ), new_num_surfaces = top.size( );

#ifdef _DEBUG
    string path = _visage_options->path( );
    ofstream file( path + "\\Arrived_fom_GPM_seen_before_by_visage_step_" + to_string( run_timestep_counter ) + "_start.dat" );
    std::vector<fVector3> coordinates;// = geometry->get_local_coordinates_vector(ii);
    for (int ii = 0; ii < old_num_surfaces; ii++)
        {
        coordinates = geometry->get_local_coordinates_vector( ii );
        for (auto &v : coordinates) file << v << endl;

        }
    file.close( );

    file.open( path + "\\Arrived_fom_GPM_NOT_seen_before_by_visage_step_" + to_string( run_timestep_counter ) + "_start.dat" );
    for (int k = old_num_surfaces; k < new_num_surfaces; k++)
        {
        auto temp_gpm_only_this_step = get_gpm_height( top, k );
        for (int n : IntRange( 0, (int)temp_gpm_only_this_step.size( ) ))
            {
            float z = temp_gpm_only_this_step[n];
            file << coordinates[n][0] << " " << coordinates[n][1] << "  " << z << endl;
            }

            //gpm_below = get_gpm_height(top, 0);
            //gpm_above = get_gpm_height(top, 1);
            //vector<float> nodal_thickness = get_gpm_difference(top, 1, 0);  //K - K-1
        }
    file.close( );
#endif 

//lets assume for now that only one layer is deposited every time that i get here, old_num_surfaces + 1 = new_num_surfaces
//if(old_num_surfaces!= new_num_surfaces)
//{
//    //how much new material is being deposited ? 
//    int top_id = new_num_surfaces - 1, below_id = top_id - 1; 
//    auto &new_gpm_top = get_gpm_height(top, top_id );
//    auto &prev_gpm_top_after_cmp = geometry->get_local_depths( below_id );

//    vector<float> thickness;
//    transform(new_gpm_top.begin(), new_gpm_top.end(), prev_gpm_top_after_cmp.begin(), back_inserter(thickness) , std::minus<float>() );


//    //new surface to be added to VS
//    geometry->set_num_surfaces(1 + geometry->nsurfaces()); //old surfaces not modified, new not initialized.

//    //where? 
//    auto &zbelow = geometry->get_local_depths(below_id);
//    auto &znew   = geometry->get_local_depths(top_id);
//    transform(zbelow.begin(), zbelow.end(), thickness.begin(), znew.begin(), std::plus<float>());
//}



    for (int k = old_num_surfaces; k < new_num_surfaces; k++)
        {
        geometry->set_num_surfaces( 1 + geometry->nsurfaces( ) ); //old surfaces not modified, new not initialized.
        auto &zbelow = geometry->get_local_depths( k - 1 );
        auto &znew = geometry->get_local_depths( k );
        vector<float> nodal_thickness = get_gpm_difference( top, k, k - 1 );  //K - K-1
        for (auto &n : IntRange( 0, (int)znew.size( ) ))
            znew[n] = zbelow[n] + nodal_thickness[n];
        }



        //for (int k = old_num_surfaces; k < new_num_surfaces; k++)
        //{
        //    geometry->set_num_surfaces(1 + geometry->nsurfaces()); //old surfaces not modified, new not initialized.

        //    //auto &znew   = geometry->get_local_depths(k);
        //    //auto &tmp    = get_gpm_height(top, k);
        //    //auto &zbelow = geometry->get_local_depths(k - 1);
        //    //for(auto n : IntRange(0, znew.size())) 
        //    //znew[n] = /*zbelow[n] +*/ tmp[n];

        //    //znew = get_gpm_height(top, k);
        //    vector<float> nodal_thickness = get_gpm_difference(top, k, k - 1);  //K - K-1
        //    //auto &znew = geometry->get_local_depths(k);
        //    //this is the last layer known by visage "<<endl;
        //    auto &zbelow = geometry->get_local_depths(k - 1);
        //    //" this is the new one just added at the top "<<endl;
        //    auto &znew = geometry->get_local_depths(k);
        //    //this is the new height of the newly added surface to visage " << endl;
        //    for (auto &n : IntRange(0, znew.size()))
        //        znew[n] = zbelow[n] + nodal_thickness[n];

        //    //H(x,y,z)= gpm( 11 ) (only gpm start ) - ***dummy** gpm ( 10 ) final ( step 10 visage )
        //    //Visage(11) = Visage(10) + H(x,y,z) 
        //    auto gpm_only_this_step = get_gpm_height(top, k);
        //    auto gpm_and_visage_last_step = geometry->get_local_depths(k - 1);

        //    //float> delta(gpm_only_this_step.size(),0);
        //    for (auto n : IntRange(0, gpm_only_this_step.size()))
        //    {
        //        float delta = gpm_only_this_step[n] - gpm_and_visage_last_step[n];
        //        znew[n] = gpm_and_visage_last_step[n] + fabs( delta ) ;
        //        //new top for  visage
        //    }

        //    //H t_-1 = dthickness( 0, 10)   //gpm & visage 
        //    //H_t_   = dthickness(0, 10)    //
        //    //dh( x,y,z) = Ht-1 - Ht
        //    //z(11) = z(11) + dh 


        //}


#ifdef _DEBUG 
    file.open( path + "\\Just_Added_to_VS_step_" + to_string( run_timestep_counter ) + "_start.dat" );
    for (int ii = old_num_surfaces; ii < new_num_surfaces; ii++)
        {
        std::vector<fVector3> coordinates = geometry->get_local_coordinates_vector( ii );
        for (auto &v : coordinates) file << v << endl;
        }
    file.close( );
#endif 

//reads from gpm some values. Whatever we need, it is read here except TOP and SEALEVEL
    update_needed_attribute_values( attributes, _data_arrays );

    //this doenst do anything if old_num_surfaces = new_num_surfaces
    if (old_num_surfaces != new_num_surfaces)
        initialize_mechanical_properties( 1, attributes, old_num_surfaces, new_num_surfaces );

    //update_step simply updates some options in visage
    _visage_options->update_step( 0 > _time_step ? 0 : _time_step );
    string mii_file_name = VisageDeckWritter::write_deck( &_visage_options, &_data_arrays );

    if (run_visage( mii_file_name ) != 0)
        {
        error += ("Visage run failed.  MII file: " + mii_file_name);
        was_error( ) = true;
        }

    _time_step += 1;

    return error.empty( ) ? 0 : 1;
    }

    //the results are arriving from Visage. The basement shouldnt have changed. But the thicknes of the layers above may have.
int  gpm_visage_link::update_results( map<string, vector<Slb::Exploration::Gpm::Api::array_2d_indexer<float>>>  &attributes, std::string& error, int step )
    {
    static int update_results_counter = -1;
    std::cout << "[update_results_counter] " << ++update_results_counter << endl;
    if (was_error( ))
        {
        cout << endl << "---------bypassing the processing of resuylts------------" << endl << endl;
        return 0;
        }

        //if -1 passed as argument, try to get the last known step (default)
    int last_step = (step == -1 ? _visage_options->step( ) : step);

    //reads vs results and puts them into _data_arrays example: TOTSTRZZ, ROCKDISZZ, etc...whatever is "wanted"
    //and that gpm will display in Petrel
    //if (!read_wanted_visage_results(last_step, error)) { return 1; } //obsolete

    //replaced read_wanted_visage_results
    if (!read_visage_results( last_step, error )) { return 1; }

    auto& x = _data_arrays;
    ;
    ;
    ;


    //copies vs results to gpm, uprocessed. These are plain visage results
    ///////copy_wanted_visage_to_gpm(attributes, error);

    //properties that we modify in gpm (if they exist) as a result of visage results, for instance, porosity
    //if (!update_gpm_properties_from_visage_results(attributes, error)) { return 1; }



#ifdef _DEBUG 
    std::vector<fVector3> coordinates;
    string path = _visage_options->path( );
    ofstream file( path + "\\Arrived_VISAGE_step_" + to_string( update_results_counter ) + "_end.dat" );
    for (int ii = 0; ii < _visage_options->geometry( )->nsurfaces( ); ii++)
        {
        coordinates = _visage_options->geometry( )->get_local_coordinates_vector( ii );
        for (auto &v : coordinates) file << v << endl;
        }
    file.close( );

    path = _visage_options->path( );
    file.open( path + "\\Arrived_FROMGPM_AFTER_VISAGE_step_" + to_string( update_results_counter ) + "_end.dat" );
    for (int ii = 0; ii < _visage_options->geometry( )->nsurfaces( ); ii++)
        {
        auto h = get_gpm_height( attributes["TOP"], ii );
        for (auto n = 0; n < coordinates.size( ); n++) file << coordinates[n][0] << "  " << coordinates[n][1] << "  " << h[n] << endl;
        }
    file.close( );

    //moves gpm nodes to match visage geometry
    if (!update_gpm_and_visage_geometris_from_visage_results( attributes, error )) { return 1; }


    path = _visage_options->path( );
    file.open( path + "\\MOVED_GPM_NODES_step_" + to_string( update_results_counter ) + "_end.dat" );
    for (int ii = 0; ii < _visage_options->geometry( )->nsurfaces( ); ii++)
        {
        auto h = get_gpm_height( attributes["TOP"], ii );
        for (auto n = 0; n < coordinates.size( ); n++) file << coordinates[n][0] << "  " << coordinates[n][1] << "  " << h[n] << endl;
        }
    file.close( );
#endif 


////PATCH 
//StructuredGrid &geometry = _visage_options->geometry();
//int top_surface_index = geometry->nsurfaces() - 1;
//vector<float> local_depths = geometry->get_local_depths(top_surface_index);

//last_known_layer.resize(geometry->ncols() * geometry->nrows());
//std::copy(local_depths.begin(), local_depths.end(), last_known_layer.begin());


//predict fractures, etc...
// if( !process_visage_results() ) return 1;

// free all hijacked memory
// delete_not_required_results();
//
    return 0;
    }