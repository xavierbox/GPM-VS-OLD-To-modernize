// -- Schlumberger Private --

#ifndef GPM_TIMEBOMB_H
#define GPM_TIMEBOMB_H
#include "gpm_logger.h"
#include <string>
#include <vector>


namespace Slb { namespace Exploration { namespace Gpm{

void set_expire_date(const std::string& compile_date, int time_bomb_offset_years);
std::string get_expire_date();
bool check_time_bomb(const Tools::gpm_logger& logger, const std::string& compile_date);

}}}
#endif
