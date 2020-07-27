// -- Schlumberger Private --

#include "gpm_timebomb.h"
#include <utility>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#define BOOST_DATE_TIME_NO_LIB
#include <boost/date_time/gregorian/gregorian_types.hpp>


namespace Slb { namespace Exploration { namespace Gpm{

using namespace Tools;

namespace
{   
    // "private" function
    boost::gregorian::date get_build_date(const std::string& compile_date) {
        std::vector<std::string> split_vec;
        split(split_vec, compile_date, boost::is_any_of(" "), boost::token_compress_on);
        const int year = boost::lexical_cast<int>(split_vec[2]);
        const int day = boost::lexical_cast<int>(split_vec[1]);
        std::map<std::string, boost::date_time::months_of_year> mapper = {
            {"Jan", boost::gregorian::Jan},
            {"Feb", boost::gregorian::Feb}, {"Mar", boost::gregorian::Mar}, {"Apr", boost::gregorian::Apr},
            {"May", boost::gregorian::May},
         {"Jun", boost::gregorian::Jun}, {"Jul", boost::gregorian::Jul}, {"Aug", boost::gregorian::Aug},
         {"Sep", boost::gregorian::Sep},
         {"Oct", boost::gregorian::Oct}, {"Nov", boost::gregorian::Nov}, {"Dec", boost::gregorian::Dec}
        };
        const boost::date_time::months_of_year month = mapper[split_vec[0]];
        const boost::gregorian::date build_date(year, month, day);
        return build_date;
    }

    // "private" member
    boost::gregorian::date expire_date;
}

// Implement the params class

void set_expire_date(const std::string& compile_date,int time_bomb_offset_years) {
    expire_date = get_build_date(compile_date) + boost::gregorian::years(time_bomb_offset_years);
}

std::string get_expire_date()
{
	return std::to_string(expire_date.year()) + "-"+ std::to_string(expire_date.month())+"-"+
        std::to_string(expire_date.day());

}


bool check_time_bomb(const gpm_logger& logger, const std::string& compile_date)
{
    const boost::gregorian::date build_date = get_build_date(compile_date);
    const boost::gregorian::date warn_date=expire_date-boost::gregorian::months(1);
    const boost::gregorian::date today = boost::gregorian::day_clock::local_day();

	//************************   VERSION   **************************/

	if(today < build_date){
		logger.print(LOG_NORMAL,"ERROR in computer date. If program has not expired, set date correctly and rerun\n");
		return true;
	}
	else if(today > expire_date){
		logger.print(LOG_NORMAL,"ERROR: Program has expired\n");
		return true;
	}
	else if(today > warn_date){
		logger.print(LOG_NORMAL,"WARNING: Program will expire at the end of this month\n");
	}
	return false;
}


}}}
           
         