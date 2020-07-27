// -- Schlumberger Private --

#include "mapstc_descr.h"
#include "gpm_string_utils.h"
#include "gpm_acf_GitSHA1.h"
#include <boost/algorithm/string/case_conv.hpp>
#include <rapidjson/stringbuffer.h>
#include <sstream>
#include <rapidjson/writer.h>
#include <chrono>
#include <iomanip>

namespace Slb { namespace Exploration { namespace Gpm {

// TODO downgrade the version to 1.1 so 2017 and 2018 commercial can read the results of the simulation
// Has to do with the statistics that is added
std::string parm_type_descr_holder::get_current_version() { return ("1.1.0.0"); }

const parameter_descr_type& parm_type_descr_holder::get_parm_descr()
{
    static parameter_descr_type parm_descr =
        {
            make_typedescription<STRING>(TITLE, "TITLE"), // 0			   
            make_typedescription<INTEGER>(NODORDER, "NODORDER"),
            make_typedescription<FLOAT>(XMIN_OBSOLETE, "XMIN"),
            make_typedescription<FLOAT>(XMAX_OBSOLETE, "XMAX"),
            make_typedescription<FLOAT>(YMIN_OBSOLETE, "YMIN"),
            make_typedescription<FLOAT>(YMAX_OBSOLETE, "YMAX"),
            make_typedescription<FLOAT>(XCORNERS, "XCORNERS", ARRAY_1d),
            // The x coordinates of the corners of rectangle
            make_typedescription<FLOAT>(YCORNERS, "YCORNERS", ARRAY_1d), // the y coordinates of the corners
            make_typedescription<FLOAT>(ZNULL, "ZNULL"),
            make_typedescription<INTEGER>(NUMROWS, "NUMROWS"),
            make_typedescription<INTEGER>(NUMCOLS, "NUMCOLS"),
            make_typedescription<STRING>(GRIDNAM, "GRIDNAM", ARRAY_1d),
            make_typedescription<FLOAT>(ZGRID, "ZGRID", ARRAY_3d),
            make_typedescription<DOUBLE>(TIMESET, "TIMESET", ARRAY_1d),
            make_typedescription<DOUBLE>(AGESET, "AGESET", ARRAY_1d),
            make_typedescription<STRING>(PROPNAM, "PROPNAM", ARRAY_1d), // PROPNAM		   	   
            make_typedescription<INTEGER>(ISNODAL, "ISNODAL", ARRAY_1d), // ISNODAL			   			       
            make_typedescription<STRING>(INDPROP, "INDPROP", ARRAY_3d),

            make_typedescription<FLOAT>(ELEMDEP, "ELEMDEP"),
            make_typedescription<INTEGER>(NUMELEM, "NUMELEM"),
            make_typedescription<FLOAT>(ELEMX, "ELEMX", ARRAY_1d),
            make_typedescription<FLOAT>(ELEMY, "ELEMY", ARRAY_1d),
            make_typedescription<FLOAT>(ELEMVX, "ELEMVX", ARRAY_1d),
            make_typedescription<FLOAT>(ELEMVY, "ELEMVY", ARRAY_1d),
            make_typedescription<STRING>(PARM_VERSION, "VERSION"),
            make_typedescription<STRING>(BASE_LAYER_MAP, "BASE_LAYER_MAP", ARRAY_1d),
            // First row is ids, second row is sediment name, third row is sed1 sed2 etc
            make_typedescription<FLOAT>(BASE_ERODABILITY, "BASE_ERODABILITY"), // The erodability of the base sediments
            // The sediment ids that are part of the base layer
            make_typedescription<STRING>(SED_PROP_MAP, "SED_PROP_MAP", ARRAY_2d),
            make_typedescription<STRING>(PROP_DISPLAY_NAME, "PROP_DISPLAY_NAME", ARRAY_2d),
            // First row is propnam, the second is the display name
            make_typedescription<FLOAT>(INDPROP_STATS, "INDPROP_STATS", ARRAY_3d),
            make_typedescription<STRING>(HISTORY, "HISTORY", ARRAY_1d),
            make_typedescription<INTEGER>(GREF_START_INDEX, "GREF_START_INDEX"),
            make_typedescription<INTEGER>(GREF_END_INDEX, "GREF_END_INDEX"),
        } //Layer index equality with PROPNAM, Row index equality with Timeset, col 0 is min, col 1 is max
        // Insert display names for the types
        ;
    return parm_descr;
}

const TypeDescr& parm_type_descr_holder::get_parm_description(PARM_MEMBERS id)
{
    return get_parm_descr().at(id);
}

std::string get_local_time()
{
    const auto start = std::chrono::system_clock::now();
    std::time_t end_time = std::chrono::system_clock::to_time_t(start);
    std::tm tm = *std::localtime(&end_time);
#ifdef  __linux__
				char tmp[128];
				if (0 < strftime(tmp, sizeof(tmp), "%FT%T", &tm)) {
					return std::string(tmp);
				}
				return std::string();
#else
    std::stringstream ss;
    ss << std::put_time(&tm, "%FT%T");
    return ss.str();
#endif
}

std::string parm_type_descr_holder::make_json_history_item(const std::string& process, const std::string& git_hash)
{
    rapidjson::StringBuffer s;
    rapidjson::Writer<rapidjson::StringBuffer> writer(s);
    //writer.SetFormatOptions(rapidjson::PrettyFormatOptions::kFormatSingleLineArray);
    writer.StartObject();
    writer.Key("Process");
    writer.String(process);
    writer.Key("GitSHA1");
    writer.String(git_hash);
    writer.Key("Date");
    writer.String(get_local_time());
    writer.EndObject();
    std::string t1 = s.GetString();

    return t1;
}

std::string parm_type_descr_holder::make_json_history_item(const std::string& process)
{
    return parm_type_descr_holder::make_json_history_item(process, gpm_acf_GitSHA1::get_hash());
}



}}}
