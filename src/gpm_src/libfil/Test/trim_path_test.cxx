// -- Schlumberger Private --

#include "trim_path.h"
int trim_path_test( int argc, char *argv[]){
	// 
	std::string test1("E:/BuildAgent1/work/845a02ed1a4bac07/Port_64bit/src/../../testData");
	std::string test1_oracle("E:/BuildAgent1/work/845a02ed1a4bac07/testData");
	std::string test2= Slb::Exploration::Gpm::FileUtils::remove_climbs_in_path(test1);

	bool isOk=test2==test1_oracle;
	return isOk?0:1;
}


int find_directory_test( int argc, char *argv[]){
	// 
	std::string test1_linux("E:/BuildAgent1/work/845a02ed1a4bac07/Port_64bit/src/../../testData");
	std::string test1_linux_oracle("E:/BuildAgent1/work/845a02ed1a4bac07/Port_64bit/src/../../");
	std::string test1_win("E:\\BuildAgent1\\work\\845a02ed1a4bac07\\Port_64bit\\src\\..\\..\\testData");
	std::string test1_win_oracle("E:\\BuildAgent1\\work\\845a02ed1a4bac07\\Port_64bit\\src\\..\\..\\");
	std::string test1_linux_win("E:\\BuildAgent1\\work\\845a02ed1a4bac07\\Port_64bit\\src\\../../testData");
	std::string test1_linux_win_oracle("E:\\BuildAgent1\\work\\845a02ed1a4bac07\\Port_64bit\\src\\../../");
	std::string test2_linux= Slb::Exploration::Gpm::FileUtils::find_directory(test1_linux);
	std::string test2_win= Slb::Exploration::Gpm::FileUtils::find_directory(test1_win);
	std::string test2_linux_win= Slb::Exploration::Gpm::FileUtils::find_directory(test1_linux_win);

	bool isOk=test2_linux==test1_linux_oracle;
	isOk = isOk && test2_win==test1_win_oracle;
	isOk = isOk && test2_linux_win==test1_linux_win_oracle;
	return isOk?0:1;
}
