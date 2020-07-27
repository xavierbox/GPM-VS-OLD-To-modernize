// -- Schlumberger Private --

#include "gpm_parm_methods.h"
#include "acf_base_reader.h"
#include "acf_base_writer.h"
#include "acf_base_helper.h"
#include "gpm_logger.h"
#include "mapstc_descr.h"

#include <boost/algorithm/string.hpp>

namespace Slb { namespace Exploration { namespace Gpm {


void adjust_dimensions(const parm& p, acf_base_writer::parameter_num_type& inumr, acf_base_writer::array_dim_size_type& idimr,
                       const Tools::gpm_logger& logger)
{
    // For stats, but won't kick in before we have added a 1 to index 0 JT
    std::vector<size_t> sizes = { p.indprop.layers() ,p.propnam.size() ,p.isnodal.size() };

    const std::pair<std::vector<size_t>::const_iterator, std::vector<size_t>::const_iterator> full_range = std::
        make_pair(sizes.begin(), sizes.end());
    const std::pair<std::vector<size_t>::const_iterator, std::vector<size_t>::const_iterator> bounds = std::equal_range(
        sizes.begin(), sizes.end(), p.indprop.layers());
    if (bounds != full_range) {
        logger.print(Tools::LOG_NORMAL, "Indprop, name, min, max and isnodal sizes are not consistant\n");
    }
    // Ensure property names and limits get written
	inumr[PROPNAM]=acf_base_helper::set_last_value(&idimr[PROPNAM], p.propnam.size());
	inumr[ISNODAL] = acf_base_helper::set_last_value(&idimr[ISNODAL],p.isnodal.size());
    inumr[GRIDNAM]= acf_base_helper::set_last_value(&idimr[GRIDNAM],p.gridnam.size()); //numgrds;

    // Here I'm not quite sure JT
    inumr[TIMESET] = acf_base_helper::set_last_value(&idimr[TIMESET],p.timeset.size());
    inumr[AGESET] = acf_base_helper::set_last_value(&idimr[AGESET],p.ageset.size());

	std::vector<size_t> arr_size{ p.indprop.layers(), //numprops
	p.indprop.rows(), //numtims
	p.indprop.cols() //numtops
	};
	inumr[INDPROP] = acf_base_helper::set_array_values(&idimr[INDPROP], arr_size);
	arr_size = { p.indprop_stats.layers(), p.indprop_stats.rows(), p.indprop_stats.cols()};
	inumr[INDPROP_STATS] = acf_base_helper::set_array_values(&idimr[INDPROP_STATS], arr_size);
	arr_size = { p.zgrid.layers() ,p.zgrid.rows(), p.zgrid.cols() };
	inumr[ZGRID] = acf_base_helper::set_array_values(&idimr[ZGRID], arr_size);
	arr_size = { p.prop_display_name.rows() , p.prop_display_name.cols() };
	inumr[PROP_DISPLAY_NAME] = acf_base_helper::set_array_values(&idimr[PROP_DISPLAY_NAME],arr_size);
}

void adjust_incremental_dimensions(const acf_base_writer::parameter_num_type& inumr_prev,
                                   const acf_base_writer::array_dim_size_type& idimr_prev,
                                   acf_base_writer::parameter_num_type& inumr,
                                   acf_base_writer::array_dim_size_type& idimr,
                                   const Tools::gpm_logger& logger)
{
	std::vector<int> items = { XCORNERS ,YCORNERS, PROPNAM, ISNODAL,PROP_DISPLAY_NAME, TITLE, NODORDER, ZNULL, NUMROWS, NUMCOLS, BASE_ERODABILITY, BASE_LAYER_MAP, XCORNERS, YCORNERS, SED_PROP_MAP, HISTORY };
	// Ensure property names and limits get written
    for (int id : items) {
        inumr[id] = 0;
        idimr[id] = {};
    }

	idimr[GRIDNAM].back() = idimr[GRIDNAM].back().left_subtract(idimr_prev[GRIDNAM].back());
	inumr[GRIDNAM] = acf_base_helper::get_size(idimr[GRIDNAM]); //numgrds;

	// Here I'm not quite sure JT
	idimr[TIMESET].back() = idimr[TIMESET].back().left_subtract(idimr_prev[TIMESET].back());
	inumr[TIMESET] = acf_base_helper::get_size(idimr[TIMESET]);
	idimr[AGESET].back() = idimr[AGESET].back().left_subtract(idimr_prev[AGESET].back());
	inumr[AGESET] = acf_base_helper::get_size(idimr[AGESET]);

	idimr[INDPROP][1] = idimr[INDPROP][1].left_subtract(idimr_prev[INDPROP][1]);
    inumr[INDPROP]= acf_base_helper::get_size(idimr[INDPROP]);
	idimr[INDPROP_STATS][1] = idimr[INDPROP_STATS][1].left_subtract(idimr_prev[INDPROP_STATS][1]);
	inumr[INDPROP_STATS] = acf_base_helper::get_size(idimr[INDPROP_STATS]);
	idimr[ZGRID].front() = idimr[ZGRID].front().left_subtract(idimr_prev[ZGRID].front());
	inumr[ZGRID] = acf_base_helper::get_size(idimr[ZGRID]);
}


}}}
