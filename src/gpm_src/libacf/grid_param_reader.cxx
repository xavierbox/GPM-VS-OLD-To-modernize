// -- Schlumberger Private --

#include "grid_param_reader.h"
#include "mapstc_descr.h"


namespace Slb { namespace Exploration { namespace Gpm {


std::string grid_param_reader::string_reader(const key_type& key) const
{
    std::string ret_val;
    if (key == TITLE) {
        ret_val = m_param.title;
    }
    else if (key == PARM_VERSION) {
        ret_val = m_param.version;
    }
    return ret_val;
}

int grid_param_reader::int_reader(const key_type& key) const
{
    int val = 0;
    if (key == NODORDER) { val = m_param.nodorder; }
    else if (key == NUMROWS) { val = m_param.numrows(); }
    else if (key == NUMCOLS) { val = m_param.numcols(); }
    else if (key == GREF_START_INDEX) { val = static_cast<int>(m_param.gref_start_index); }
    else if (key == GREF_END_INDEX) { val = static_cast<int>(m_param.gref_end_index); }
    return val;
}

float grid_param_reader::float_reader(const key_type& key) const
{
    float val = 0.0F;
    if (key == ZNULL) { val = m_param.znull; }
    else if (key == BASE_ERODABILITY) { val = m_param.base_erodability; }
    return val;
}

double grid_param_reader::double_reader(const key_type& key) const
{
    const double val = 0.0;
    return val;
}

std::string grid_param_reader::string_array_reader(const key_type& key, const std::vector<index_type>& indexes) const
{
    std::string ret_val;
    if (indexes.size() == 1) {
        if (key == GRIDNAM) {
            ret_val = m_param.gridnam[indexes[0]];
        }
        else if (key == PROPNAM) {
            ret_val = m_param.propnam[indexes[0]];
        }
        else if (key == BASE_LAYER_MAP) {
            ret_val = m_param.base_layer_map[indexes[0]];
        }
		else if (key == HISTORY) {
			ret_val = m_param.history[indexes[0]];
		}
    }
    else if (indexes.size() == 2) {
        if (key == SED_PROP_MAP) {
            ret_val = m_param.sediment_prop_map(indexes[0], indexes[1]);
        }
        else if (key == PROP_DISPLAY_NAME) {
            ret_val = m_param.prop_display_name(indexes[0], indexes[1]);
        }
    }
    else if (indexes.size() == 3) {
        if (key == INDPROP) {
            function_2d& obj(m_param.indprop(indexes[0], indexes[1], indexes[2]));
            const auto buffer_count = 128;
            char tmp[buffer_count];
            if (obj.is_initialized()) {
                if (obj.is_constant()) {
                    snprintf(tmp, buffer_count, "%+.5f", obj.constant_value());
                    ret_val = tmp;
                }
                else {
                    ret_val = "#" + std::to_string(obj.index_value());
                }
            }
        }
    }

    return ret_val;

}

int grid_param_reader::int_array_reader(const key_type& key, const std::vector<index_type>& indexes) const
{
    int val = 0;
    if (indexes.size() == 1) {
        if (key == ISNODAL) {
            val = m_param.isnodal[indexes[0]];
        }
    }
    return val;
}

float grid_param_reader::float_array_reader(const key_type& key, const std::vector<index_type>& indexes) const
{
    /* ARRAYS */
    float val = 0.0F;
    if (indexes.size() == 1) {
        if (key == XCORNERS) {
            val = m_param.xcorners[indexes[0]];
        }
        else if (key == YCORNERS) {
            val = m_param.ycorners[indexes[0]];
        }
    }
    if (indexes.size() == 2) {
    }
    if (indexes.size() == 3) {
        if (key == ZGRID) { val = m_param.zgrid(indexes[0], indexes[1], indexes[2]); }
        else if (key == INDPROP_STATS){val = m_param.indprop_stats(indexes[0], indexes[1], indexes[2]);}
    }
    return val;
}

double grid_param_reader::double_array_reader(const key_type& key, const std::vector<index_type>& indexes) const
{
    /* ARRAYS */
    double val = 0.0F;
    if (indexes.size() == 1) {
        if (key == TIMESET) { val = m_param.timeset[indexes[0]]; }
        else if (key == AGESET) { val = m_param.ageset[indexes[0]]; }
    }
    if (indexes.size() == 2) {
    }
    if (indexes.size() == 3) {
    }
    return val;
}

}}}
