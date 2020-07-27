// -- Schlumberger Private --

#include "grid_param_mapper.h"
#include "param_wrapper_helper.h"
#include "acf_base_reader.h"
#include "param_utils.h"
#include "gpm_version_number.h"
#include "gpm_hdf5_reader.h"
#include "acf_json_reader.h"
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/core/null_deleter.hpp>
#include "acf_format_selector.h"
#include "parm_prop_descr.h"

namespace Slb { namespace Exploration { namespace Gpm {

// Grid param wrapper
grid_param_wrapper::grid_param_wrapper(grid_parameter_type& param, bool initialize) : m_param(param) {
    m_helper.init(parm_type_descr_holder::get_parm_descr());
    if (initialize) {
        initialize_struct();
    }
}

grid_param_wrapper::grid_param_wrapper(grid_parameter_type& param, const parameter_descr_type& type_info, bool initialize) : m_param(param) {
    m_helper.init(type_info);
    if (initialize) {
        initialize_struct(type_info);
    }
}

void grid_param_wrapper::free_struct() {
    free_arrays();
    m_helper.clear();
}

void grid_param_wrapper::initialize_struct(const parameter_descr_type& type_info) {
    m_helper.clear();
    m_helper.init(type_info);
    initialize_params();
}

void grid_param_wrapper::initialize_struct() {
    initialize_struct(parm_type_descr_holder::get_parm_descr());
}


void grid_param_wrapper::free_arrays() const
{
    //param_wrapper_helper::array_size_mapper_type& array_sizes(m_helper.array_sizes);

    m_param.timeset.clear();
    m_param.ageset.clear();

    m_param.isnodal.clear();

    m_param.gridnam.clear();
    m_param.propnam.clear();

    m_param.zgrid.clear();

    m_param.indprop.clear();
}

void grid_param_wrapper::initialize_params() const
{
    initialize_pod_params();
    initialize_params_in_arrays();
}

void grid_param_wrapper::initialize_params_in_arrays() const
{
    m_param.zgrid.fill(FLOAT_IO_NO_VALUE);
    param_wrapper_helper::initialize_arrays(m_param.timeset, 0.0F);
    param_wrapper_helper::initialize_arrays(m_param.ageset, 0.0F);
    param_wrapper_helper::initialize_arrays(m_param.isnodal, 0);
    param_wrapper_helper::initialize_arrays(m_param.xcorners, 0.0F);
    param_wrapper_helper::initialize_arrays(m_param.ycorners, 0.0F);
}

void grid_param_wrapper::initialize_pod_params() const
{
    m_param.nodorder = 0;
    m_param.znull = FLOAT_IO_NO_VALUE;
    m_param.version = "1.0.0.0";
}

// Grid param mapper
grid_param_mapper::grid_param_mapper(grid_param_wrapper& param, acf_base_reader& parser) : m_param(param), m_parser(parser) {
    parser.connect_string(std::bind(&grid_param_mapper::string_mapper, this, std::placeholders::_1, std::placeholders::_2));
    parser.connect_int(std::bind(&grid_param_mapper::int_mapper, this, std::placeholders::_1, std::placeholders::_2));
    parser.connect_float(std::bind(&grid_param_mapper::float_mapper, this, std::placeholders::_1, std::placeholders::_2));
    parser.connect_double(std::bind(&grid_param_mapper::double_mapper, this, std::placeholders::_1, std::placeholders::_2));
    parser.connect_string_array(std::bind(&grid_param_mapper::string_array_item_mapper, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    parser.connect_int_array(std::bind(&grid_param_mapper::int_array_item_mapper, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    parser.connect_float_array(std::bind(&grid_param_mapper::float_array_item_mapper, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    parser.connect_double_array(std::bind(&grid_param_mapper::double_array_item_mapper, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    parser.connect_string_array2(std::bind(&grid_param_mapper::string_array_mapper, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    parser.connect_int_array2(std::bind(&grid_param_mapper::int_array_mapper, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    parser.connect_float_array2(std::bind(&grid_param_mapper::float_array_mapper, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    parser.connect_double_array2(std::bind(&grid_param_mapper::double_array_mapper, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
}

// Here we use the read values of arrays and resize them to the actual values
// need to make the mapper as well
// Here we assume that id is equal to grid index
void grid_param_mapper::adjust_arrays_to_read_size() const
{
    auto read_numbers(m_parser.num_read());
    auto read_array_sizes(m_parser.array_sizes_read());
    //std::map<int, std::vector<int > > array_sizes(m_parser.array_sizes());
    std::vector<TypeDescr> type_descr(m_parser.type_descriptors());
    // We assume that we have index equality, this we do the mapping
    std::map<int, int> index_mapper(TypeDescr::make_index_mapper(type_descr));

    // Nows lets reason about the sizes
    for (parameter_descr_type::const_iterator it = type_descr.begin();
         it != type_descr.end(); ++it) {
        if (it->num_dimensions > 0) { // An array
            int key = it->id;
            if (read_numbers[index_mapper[key]] > 0) {
                // Got a number to react on
                const fixed_array_type read_size = read_array_sizes[index_mapper[key]];
                const TypeDescr loc_type = *it;
                // Here we shoudl check that we have numbers that compare

                //std::vector<int> loc_vec=it->second;

                if (loc_type.pod_type == STRING && loc_type.num_dimensions > 0) {
                    // We have an array of strings as it is at the moment
                    adjust_string_arrays(loc_type, read_size);
                }
                else if (loc_type.pod_type == INTEGER) {
                    adjust_integer_arrays(loc_type, read_size);
                }
                else if (loc_type.pod_type == FLOAT) {
                    adjust_float_arrays(loc_type, read_size);
                }
            }
        }
    }
}

void grid_param_mapper::connect_indprop_to_zgrid() const
{
	grid_parameter_type& param = m_param.m_param;
    const auto arr = std::shared_ptr<float_3darray_vec_base_type>(&(param.zgrid), boost::null_deleter());
	for(auto l = param.indprop.layer_extent().lower(); l !=  param.indprop.layer_extent().upper();++l)
	{
		for (auto i = param.indprop.row_extent().lower(); i != param.indprop.row_extent().upper();++i)
		{
			for (auto j = param.indprop.col_extent().lower(); j < param.indprop.col_extent().upper();++j)
			{
				if (param.indprop(l,i,j).is_initialized() && param.indprop(l, i, j).is_index())
				{
					param.indprop(l, i, j).set_array(arr);
				}
			}
		}
	}
}

int grid_param_mapper::check_sediment_entries(const Tools::gpm_logger& logger) const
{
    grid_parameter_type& param = m_param.m_param;
    const auto& base_map = param.base_layer_map;
    const auto& sed_prop_map = param.sediment_prop_map;
    if (sed_prop_map.row_extent().size()< 3 )
    {
        logger.print(Tools::LOG_NORMAL, "Sediment property map must have 3 rows, begin=%d, end=%d\n", sed_prop_map.row_extent().lower(), sed_prop_map.row_extent().upper());
        return 2;
    }
    // Check for empty strings in id and name
    std::set<std::string> ids;
    for(auto j = sed_prop_map.col_extent().lower(); j != sed_prop_map.col_extent().upper();++j) {
        auto id = sed_prop_map(0, j);
        auto propname = sed_prop_map(2, j);
        boost::trim(id);
        boost::trim(propname);
        if (id.empty() || propname.empty()) {
            logger.print(Tools::LOG_NORMAL, "Sediment id or property name is empty\n");
            return 2;
        }
        ids.insert(id);
    }
    if (std::any_of(base_map.begin(), base_map.end(), [](std::string str) {boost::trim(str); return str.empty();})) {
        logger.print(Tools::LOG_NORMAL, "Sediment id in base layer is empty string\n");
        return 2;
    }
    // Now we check that the base ids are in the sediment ids
    int num_missed = 0;
    for(const auto& it:base_map) {
        if (ids.count(it)== 0) {
            ++num_missed;
        }
    }
    if (num_missed > 0) {
        logger.print(Tools::LOG_NORMAL, "Sediment id in base layer is missing\n");
        return 2;
    }
    return num_missed;
}
void grid_param_mapper::adjust_integer_arrays(const TypeDescr& loc_type, const fixed_array_type& read_dims) const
{
    const int loc_size = loc_type.num_dimensions;
    const auto key = loc_type.id;
    if (loc_size == 1) {
        const auto it = read_dims.rbegin();
        const auto new_size = *it;
        if (key == ISNODAL) {
            m_param.m_param.isnodal.resize(new_size); 
        }
    }
}

void grid_param_mapper::adjust_float_arrays(const TypeDescr& loc_type, const fixed_array_type& read_dims) const
{
    const int loc_size = loc_type.num_dimensions;
    const auto key = loc_type.id;
    if (loc_size == 1) {
        const auto it = read_dims.rbegin();
        const int new_size = *it;
        if (key == TIMESET) { m_param.m_param.timeset.resize(new_size); }
        else if (key == AGESET) { m_param.m_param.ageset.resize(new_size); }
    }
    if (loc_size == 2) {
    }
    if (loc_size == 3) {
        if (key == ZGRID) {
            const auto it = read_dims.rbegin();
            m_param.m_param.zgrid.resize(*(it + 2), *(it + 1), *it);
        }
        if (key == INDPROP_STATS) {
            const auto it = read_dims.rbegin();
            m_param.m_param.indprop_stats.resize(*(it + 2), *(it + 1), *it);
        }
    }
}

void grid_param_mapper::adjust_string_arrays(const TypeDescr& loc_type, const fixed_array_type& read_dims) const
{
    const int loc_size = loc_type.num_dimensions;
    const auto key = loc_type.id;
    if (loc_size == 1) {
        const int new_size = read_dims.back();
        if (key == GRIDNAM) {
            m_param.m_param.gridnam.resize(new_size);
        }
        else if (key == PROPNAM) {
            m_param.m_param.propnam.resize(new_size);
        }
		else if (key == HISTORY) { m_param.m_param.history.resize(new_size); }
     }
    else if (loc_size == 2) {
        const int new_size = read_dims.back();
       if (key == PROP_DISPLAY_NAME) {
            m_param.m_param.prop_display_name.resize(2,new_size);
        }
        
    }
    else if (loc_size == 3) {
        if (key == INDPROP) {
            //Lazy at the moemnt
            const auto it = read_dims.rbegin();
            m_param.m_param.indprop.resize(*(it + 2), *(it + 1), *it);
            //for(int i=0; i < m_param.m_param.indprop.size();++i){
            //	m_param.m_param.indprop[i].resize(*(it+1), *it);
            //}
        }
    }
}

template <class T>
void add_val_to_array(std::vector<T>& arr, grid_param_mapper::index_type index, T val) {
    param_helper::add_val_to_array(arr, index, val);
}

template <class T>
void add_val_to_array(gpm_vbl_array_2d<T>& arr, grid_param_mapper::index_type i, grid_param_mapper::index_type j, T val) {
    param_helper::add_val_to_array(arr, i, j, val);
}

void grid_param_mapper::string_mapper(const key_type& key, const std::string& val) const
{
    grid_parameter_type& loc_param(m_param.m_param);
    m_param.m_helper.add_variable_count(key);
    if (key == TITLE) {
        loc_param.title = val;
    }
    else if (key == PARM_VERSION) {
        loc_param.version = val;
    }
}

void grid_param_mapper::int_mapper(const key_type& key, int val) const
{
    grid_parameter_type& loc_param(m_param.m_param);
    m_param.m_helper.add_variable_count(key);

    if (key == NODORDER) {
        loc_param.nodorder = val;
    }
}

void grid_param_mapper::float_mapper(const key_type& key, float val) const
{
    grid_parameter_type& loc_param(m_param.m_param);
    m_param.m_helper.add_variable_count(key);

    if (key == ZNULL) {
        loc_param.znull = val;
    }
    else if (key == BASE_ERODABILITY) {
        loc_param.base_erodability = val;
    }

}

void grid_param_mapper::double_mapper(const key_type& key, double val) const
{
    m_param.m_helper.add_variable_count(key);
}

void grid_param_mapper::string_array_item_mapper(const key_type& key, const std::vector<index_type>& indexes, const std::string& val) const
{
    grid_parameter_type& loc_param(m_param.m_param);
    //int len = m_param.m_helper.array_sizes[key].back();
    m_param.m_helper.add_variable_count(key);
    m_param.m_helper.add_array_indexes(key, indexes);

    if (indexes.size() == 1) {
        if (key == GRIDNAM) {
            add_val_to_array(loc_param.gridnam, indexes[0], val);
        }
        else if (key == PROPNAM) {
            add_val_to_array(loc_param.propnam, indexes[0], val);
        }
        else if (key == BASE_LAYER_MAP) {
            add_val_to_array(loc_param.base_layer_map, indexes[0], val);
        }
		else if (key == HISTORY) {
			add_val_to_array(loc_param.history, indexes[0], val);
		}
    }
    else if (indexes.size() == 2) {
        if (key == SED_PROP_MAP) {
            add_val_to_array(loc_param.sediment_prop_map, indexes[0], indexes[1], val);
        }
        else if (key == PROP_DISPLAY_NAME) {
            add_val_to_array(loc_param.prop_display_name, indexes[0], indexes[1], val);
        }
    }
    else if (indexes.size() == 3) {
        if (key == INDPROP) {
            // Here is where we find the stuff to put in
            const auto i = indexes[0];
            const auto j = indexes[1];
            const auto k = indexes[2];
            if (!loc_param.indprop.extents().contains(int_extent_3d::make_index(i, j, k))) {
                const int_extent_3d exts = loc_param.indprop.extents() | int_extent_3d::make_index(i, j, k);
                loc_param.indprop.resize(exts);
            }

            function_2d obj;
            if (val[0] == '#') { // We point to a grid
                std::string tmp = val;
                const auto ind = boost::lexical_cast<index_type>(tmp.erase(0, 1));
                obj.set_index(ind);

            }
            else if (val.empty()) {
                // Add an empty one
            }
            else {// Float value
                const auto loc_val = boost::lexical_cast<float>(val);
                obj.set_constant(loc_val);
            }
            loc_param.indprop(indexes[0], indexes[1], indexes[2]) = obj;
        }
    }
}

void grid_param_mapper::int_array_item_mapper(const key_type& key, const std::vector<index_type>& indexes, int val) const
{
    m_param.m_helper.add_variable_count(key);
    m_param.m_helper.add_array_indexes(key, indexes);
    grid_parameter_type& loc_param(m_param.m_param);
    if (indexes.size() == 1) {
        if (key == ISNODAL) {
            add_val_to_array(loc_param.isnodal, indexes[0], val);
        }
    }
}

void grid_param_mapper::float_array_item_mapper(const key_type& key, const std::vector<index_type>& indexes, float val) const
{
    /* ARRAYS */
    grid_parameter_type& loc_param(m_param.m_param);
    m_param.m_helper.add_variable_count(key);
    m_param.m_helper.add_array_indexes(key, indexes);

    // Lets do the safeguard
    if (indexes.size() == 1) {
        if (key == XCORNERS) {
            add_val_to_array(loc_param.xcorners, indexes[0], val);
        }
        else if (key == YCORNERS) {
            add_val_to_array(loc_param.ycorners, indexes[0], val);
        }
    }
    if (indexes.size() == 2) {
    }
    if (indexes.size() == 3) {
        const auto i = indexes[0];
        const auto j = indexes[1];
        const auto k = indexes[2];
        const auto index_ext = int_extent_3d::make_index(i, j, k);
        if (key == ZGRID) {
            const bool is_ok = loc_param.zgrid.extents().contains(index_ext);
			if (!is_ok) {
				// need to realloc
				const int_extent_3d tmp = loc_param.zgrid.extents() | index_ext;
				loc_param.zgrid.resize(tmp);
			}
            loc_param.zgrid(i, j, k) = val;
        }
        else if (key == INDPROP_STATS) {
            const bool is_ok = loc_param.indprop_stats.extents().contains(index_ext);
			if (!is_ok) {
				// need to realloc
				const int_extent_3d tmp = loc_param.indprop_stats.extents() | index_ext;
				loc_param.indprop_stats.resize(tmp);
			}
            loc_param.indprop_stats(i, j, k) = val;         
        }
    }
}

void grid_param_mapper::double_array_item_mapper(const key_type& key, const std::vector<index_type>& indexes, double val) const
{
    /* ARRAYS */
    grid_parameter_type& loc_param(m_param.m_param);
    m_param.m_helper.add_variable_count(key);
    m_param.m_helper.add_array_indexes(key, indexes);

    // Lets do the safeguard
    if (indexes.size() == 1) {
        if (key == TIMESET) { add_val_to_array(loc_param.timeset, indexes[0], val); }
        else if (key == AGESET) { add_val_to_array(loc_param.ageset, indexes[0], val); }
    }
    if (indexes.size() == 2) {
    }
    if (indexes.size() == 3) {
    }
}

void grid_param_mapper::string_array_mapper(const key_type& key, const std::vector<index_type>& offset, const std::vector<size_type>& size, const std::vector<std::string>& values) const
{
    grid_parameter_type& loc_param(m_param.m_param);
    m_param.m_helper.add_variable_count(key, static_cast<index_type>(values.size()));

    if (offset.size() == 1) {
        const auto column_offset = offset[0];
        const auto columns = size[0];

        if (key == GRIDNAM) {
            for (auto i = 0; i < columns; i++) {
                add_val_to_array(loc_param.gridnam, column_offset + i, values[i]);
            }
        }
        else if (key == PROPNAM) {
            for (auto i = 0; i < columns; i++) {
                add_val_to_array(loc_param.propnam, column_offset + i, values[i]);
            }
        }
        else if (key == BASE_LAYER_MAP) {
            for (auto i = 0; i < columns; i++) {
                add_val_to_array(loc_param.base_layer_map, column_offset + i, values[i]);
            }
        }
		else if (key == HISTORY) {
			for (auto i = 0; i < columns; i++) {
				add_val_to_array(loc_param.history, column_offset + i, values[i]);
			}
		}

    }
    else if (offset.size() == 2) {
        const auto col_offset = offset[0];
        const auto row_offset = offset[1];
        const auto columns = size[0];
        const auto rows = size[1];

        if (key == SED_PROP_MAP) {
            for (auto row = 0; row < rows; row++) {
                for (auto col = 0; col < columns; col++) {
                    const auto val = values[col + row * columns];
                    add_val_to_array(loc_param.sediment_prop_map, row + row_offset, col + col_offset, val);
                }
            }
        }
        else if (key == PROP_DISPLAY_NAME) {
            for (auto row = 0; row < rows; row++) {
                for (auto col = 0; col < columns; col++) {
                    const auto val = values[col + row * columns];
                    add_val_to_array(loc_param.prop_display_name, row + row_offset, col + col_offset, val);
                }
            }
        }
    }
    else if (offset.size() == 3) {
        const auto column_offset = offset[0];
        const auto row_offset = offset[1];
        const auto layer_offset = offset[2];

        const auto columns = size[0];
        const auto rows = size[1];
        const auto layers = size[2];

        if (key == INDPROP) {
            for (auto row = 0; row < rows; row++) {
                for (auto column = 0; column < columns; column++) {
                    for (auto layer = 0; layer < layers; layer++) {
                        const auto index_i = row + row_offset;
                        const auto index_j = column + column_offset;
                        const auto index_k = layer + layer_offset;
                        auto val = values[column + row * columns + layer * columns * rows];

                        if (!loc_param.indprop.extents().contains(int_extent_3d::make_index(index_k, index_i, index_j))) {
                            const int_extent_3d exts = loc_param.indprop.extents() | int_extent_3d::make_index(index_k, index_i, index_j);
                            loc_param.indprop.resize(exts);
                        }

                        function_2d obj;
                        if (val[0] == '#') {
                            std::string tmp = val;
                            const auto ind = boost::lexical_cast<int>(tmp.erase(0, 1));
                            obj.set_index(ind);
                        }
                        else if (val.empty()) {
                        }
                        else {
                            const auto loc_val = boost::lexical_cast<float>(val);
                            obj.set_constant(loc_val);
                        }

                        loc_param.indprop(index_k, index_i, index_j) = obj;
                    }
                }
            }
        }
    }
}

void grid_param_mapper::int_array_mapper(const key_type& key, const std::vector<index_type>& offset, const std::vector<size_type>& size, const std::vector<int>& values) const
{
    m_param.m_helper.add_variable_count(key, static_cast<int>(values.size()));
    grid_parameter_type& loc_param(m_param.m_param);
    if (offset.size() == 1) {
        const auto column_offset = offset[0];
        const auto columns = size[0];
        if (key == ISNODAL) {
            for (int i = 0; i < columns; i++) {
                add_val_to_array(loc_param.isnodal, column_offset + i, values[i]);
            }
        }
    }
}

void grid_param_mapper::float_array_mapper(const key_type& key, const std::vector<index_type>& offset, const std::vector<size_type>& size, const std::vector<float>& values) const
{
    /* ARRAYS */
    grid_parameter_type& loc_param(m_param.m_param);
    m_param.m_helper.add_variable_count(key, static_cast<int>(values.size()));

    // Lets do the safeguard
    if (offset.size() == 1) {
        const auto column_offset = offset[0];
        const auto columns = size[0];
        if (key == XCORNERS) {
            for (int i = 0; i < columns; i++) {
                add_val_to_array(loc_param.xcorners, column_offset + i, values[i]);
            }
        }
        else if (key == YCORNERS) {
            for (int i = 0; i < columns; i++) {
                add_val_to_array(loc_param.ycorners, column_offset + i, values[i]);
            }
        }
    }
    if (offset.size() == 3) {
        const auto column_offset = offset[0];
        const auto row_offset = offset[1];
        const auto layer_offset = offset[2];

        const auto columns = size[0];
        const auto rows = size[1];
        const auto layers = size[2];

        const int_extent_3d new_ext(layer_offset, layer_offset + layers, row_offset, row_offset + rows, column_offset,
                                    column_offset + columns);

        if (key == ZGRID) {
            if (!loc_param.zgrid.extents().contains(new_ext)) {
                const int_extent_3d tmp = loc_param.zgrid.extents() | new_ext;
                loc_param.zgrid.resize(tmp);
            }
            for (auto layer = 0; layer < layers; layer++) {
                const auto index_k = layer + layer_offset;
                const auto layer_array_offset = layer * columns * rows;
                for (auto row = 0; row < rows; row++) {
                    const auto index_i = row + row_offset;
                    const auto row_array_offset = columns * row;
                    for (auto column = 0; column < columns; column++) {
                        const auto index_j = column + column_offset;
                        const auto value = values[column + row_array_offset + layer_array_offset];
                        // Todo could have done some streaming also? JT
                        loc_param.zgrid(index_k, index_i, index_j) = value;
                    }
                }
            }
        }
        else if (key == INDPROP_STATS) {
            if (!loc_param.indprop_stats.extents().contains(new_ext)) {
                const int_extent_3d tmp = loc_param.indprop_stats.extents() | new_ext;
                loc_param.indprop_stats.resize(tmp);
            }
            for (auto layer = 0; layer < layers; layer++) {
                const auto index_k = layer + layer_offset;
                const auto layer_array_offset = layer * columns * rows;
                for (auto row = 0; row < rows; row++) {
                    const auto index_i = row + row_offset;
                    const auto row_array_offset = columns * row;
                    for (auto column = 0; column < columns; column++) {
                        const auto index_j = column + column_offset;
                        const auto value = values[column + row_array_offset + layer_array_offset];
                        // Todo could have done some streaming also? JT
                        loc_param.indprop_stats(index_k, index_i, index_j) = value;
                    }
                }
            }
        }
    }
}

void grid_param_mapper::double_array_mapper(const key_type& key, const std::vector<index_type>& offset,
                                            const std::vector<size_type>& size, const std::vector<double>& values) const
{
    /* ARRAYS */
    grid_parameter_type& loc_param(m_param.m_param);
    m_param.m_helper.add_variable_count(key, static_cast<int>(values.size()));

    if (offset.size() == 1) {
        const auto column_offset = offset[0];
        const auto columns = size[0];
        if (key == TIMESET) {
            for (int i = 0; i < columns; i++) {
                add_val_to_array(loc_param.timeset, column_offset + i, values[i]);
            }
        }
        else if (key == AGESET) {
            for (int i = 0; i < columns; i++) {
                add_val_to_array(loc_param.ageset, column_offset + i, values[i]);
            }
        }
    }
}


// Here is the reader
int gpm_read_map_file(const std::string& map_file_name, const std::string& file_type, parm& p, acf_base_reader::parameter_num_type& inumr, acf_base_reader::array_dim_size_type& idimr, const Tools::gpm_logger& logger)
{
    grid_param_wrapper grid_param_wrap(p); // Initialize the stuff

    std::shared_ptr<acf_base_reader> grid_parser(acf_format::find_reader_according_to_suffix(map_file_name));

    grid_parser->set_parse_type_info(parm_type_descr_holder::get_parm_descr());
    grid_param_mapper map_grid_it(grid_param_wrap, *grid_parser);
    grid_parser->parse_file(map_file_name);

    int iret = grid_parser->error_number();

    // Perhaps react to missing indexes JT
    std::vector<int> wrong_indexes = grid_parser->has_missing_indexes();

    if (iret == -1) {
        logger.print(Tools::LOG_NORMAL, "\nWarning reading %s file\n", file_type.c_str());
        logger.print(Tools::LOG_NORMAL, "'%s'\n", grid_parser->error().c_str());
    }
    if (iret <= -2) {
        logger.print(Tools::LOG_NORMAL, "\nError reading %s file: %s\n", file_type.c_str(), map_file_name.c_str());
        logger.print(Tools::LOG_NORMAL, "'%s'\n", grid_parser->error().c_str());
        return 2;
    }
    Tools::gpm_version_number found(p.version);
    const Tools::gpm_version_number current(parm_descr_holder::get_current_version());
    if (!found.version_compatible(current)) {
        logger.print(Tools::LOG_NORMAL, "%s file file version number wrong\n", file_type.c_str());
        return 2;
    }

    map_grid_it.adjust_arrays_to_read_size();
    map_grid_it.connect_indprop_to_zgrid();
    // Lets check the sediments
    iret = map_grid_it.check_sediment_entries(logger);
    if (iret != 0) {
        return iret;
    }
    inumr = grid_parser->num_read();
    idimr = grid_parser->array_sizes_read();
    return 0;
}
}}}
