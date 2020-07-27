// -- Schlumberger Private --

#include "trim_path.h"
#include <boost/algorithm/string.hpp>
#include <string>
#include <vector>

namespace Slb {
	namespace Exploration {
		namespace Gpm {
			namespace FileUtils {
				// we need to remove /dir/../ patterns in the string
				std::string remove_climbs_in_path(const std::string& path)
				{
					std::string res(path);
					if (boost::find_first(path, "/..")){
						std::vector<std::string> items;
						boost::split(items, path, boost::is_any_of("/"));
						std::string up("..");
						std::vector<std::string>::iterator it = std::find(items.begin(), items.end(), up);
						while (it != items.end()){
							items.erase(it - 1, it + 1);
							it = std::find(items.begin(), items.end(), up);
						}
						res.clear();
						for (it = items.begin(); it != items.end(); ++it){
							res.append(*it).append("/");
						}
						if (*path.rbegin() != '/'){
							res.erase(res.end() - 1, res.end());
						}
					}
					return res;
				}

				std::string find_directory(std::string const & filename)
				{
					std::string res;
					std::string dir_sign_linux("/");
					std::string dir_sign_win("\\");
					bool has_linux_dir = boost::find_first(filename, dir_sign_linux);
					bool has_win_dir = boost::find_first(filename, dir_sign_win);

					if (has_linux_dir && has_win_dir){
						std::string::const_iterator it1 = std::find_end(filename.begin(), filename.end(), dir_sign_linux.begin(), dir_sign_linux.end());
						std::string::const_iterator it2 = std::find_end(filename.begin(), filename.end(), dir_sign_win.begin(), dir_sign_win.end());
						std::string::const_iterator it = it1 > it2 ? it1 : it2;
						std::copy(filename.begin(), it + 1, back_inserter(res));
					}
					else if (has_linux_dir)
					{
						std::string::const_iterator it = std::find_end(filename.begin(), filename.end(), dir_sign_linux.begin(), dir_sign_linux.end());
						std::copy(filename.begin(), it + 1, back_inserter(res));
					}
					else if (has_win_dir)
					{
						std::string::const_iterator it = std::find_end(filename.begin(), filename.end(), dir_sign_win.begin(), dir_sign_win.end());
						std::copy(filename.begin(), it + 1, back_inserter(res));
					}
					return res;
				}
			}
		}
	}
}
