// -- Schlumberger Private --

#ifndef TRIM_PATH_H
#define TRIM_PATH_H
#include <string>

namespace Slb { namespace Exploration { namespace Gpm { namespace FileUtils { 
// we need to remove /dir/../ patterns in the string

std::string remove_climbs_in_path(const std::string& path);
std::string find_directory(std::string const & filename);

}}}}

#endif
