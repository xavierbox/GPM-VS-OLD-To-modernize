// -- Schlumberger Private --

#include "gpm_expires.h"

#include <ctime>
using namespace std;


namespace Slb { namespace Exploration { namespace Gpm { namespace Tools {

bool gpm_expires(int year, int month, int day) {
    time_t t = time(nullptr); // get time now
    struct tm* now = localtime(& t);
    int year_now = now->tm_year + 1900;
    int month_now = now->tm_mon + 1;
    int day_now = now->tm_mday;
    bool valid = year_now <= year && month_now <= month && day_now <= day;
    return !valid;
}
}}}}
