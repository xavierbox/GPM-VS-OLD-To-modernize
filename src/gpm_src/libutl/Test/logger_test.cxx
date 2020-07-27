#include "gpm_logger.h"
#include <sstream>
using namespace Slb::Exploration::Gpm::Tools;
int logger_test(int argc, char* argv[]) {
	std::stringstream buf;
	gpm_strstream_logger logger(LOG_NORMAL, true, buf);
	logger.print(LOG_NORMAL, "%s \n", "This is a test line 1");
	logger.print(LOG_NORMAL, "%s \n", "This is a test line 2");
	auto res = buf.str();
	printf("Number of chars in logger test: %d\n", static_cast<int>(res.size()));
	return (res.size()==48)?0:1;
}
