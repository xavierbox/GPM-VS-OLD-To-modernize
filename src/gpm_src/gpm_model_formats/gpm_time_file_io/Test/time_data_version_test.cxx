#include <string>
#include "gpm_time_function_data_io.h"

std::string old_input = R"(
{ 
"ZNULL":-1e+12,
"GRIDNAM":["SEA0"],
"ZGRID":[
    [
        [10,9.5100002,8.0900002,5.8800001,3.0899999,0,-3.0899999,-5.8800001,-8.0900002,-9.5100002,-10,-9.5100002,-8.0900002,-5.8800001,-3.0899999,0,3.0899999,5.8800001,8.0900002,9.5100002,10,9.5100002,8.0900002,5.8800001,3.0899999,0,-3.0899999,-5.8800001,-8.0900002,-9.5100002,-10,-9.5100002,-8.0900002,-5.8800001,-3.0899999,0,3.0899999,5.8800001,8.0900002,9.5100002,10],
        [-20014,-19513.699,-19013.301,-18513,-18012.6,-17512.301,-17011.9,-16511.6,-16011.2,-15510.9,-15010.5,-14510.2,-14009.8,-13509.5,-13009.1,-12508.8,-12008.4,-11508.1,-11007.7,-10507.4,-10007,-9506.6699,-9006.3203,-8505.9697,-8005.6201,-7505.27,-7004.9199,-6504.5698,-6004.21,-5503.8599,-5003.5098,-4503.1602,-4002.8101,-3502.46,-3002.1101,-2501.76,-2001.4,-1501.05,-1000.7,-500.35101,-0]
    ]
],
"OUT_RELATIVE_SEALEVEL_RATE":1,
"VERSION": "1.0.0.0"
}
)";


std::string new_input = R"(
{
  "VERSION": "2.0.0.0",
  "SCHEMA_VERSION": "1.0.0.0",
  "ARRAY_2D": [
    {
      "NAME": "Sea level",
      "VALUES": [
        [
          10.0,
          9.51,
          8.09,
          5.88,
          3.09,
          0.0,
          -3.09,
          -5.88,
          -8.09,
          -9.51,
          -10.0,
          -9.51,
          -8.09,
          -5.88,
          -3.09,
          0.0,
          3.09,
          5.88,
          8.09,
          9.51,
          10.0,
          9.51,
          8.09,
          5.88,
          3.09,
          0.0,
          -3.09,
          -5.88,
          -8.09,
          -9.51,
          -10.0,
          -9.51,
          -8.09,
          -5.88,
          -3.09,
          0.0,
          3.09,
          5.88,
          8.09,
          9.51,
          10.0
        ],
        [
          -20014.0,
          -19513.7,
          -19013.3,
          -18513.0,
          -18012.6,
          -17512.3,
          -17011.9,
          -16511.6,
          -16011.2,
          -15510.9,
          -15010.5,
          -14510.2,
          -14009.8,
          -13509.5,
          -13009.1,
          -12508.8,
          -12008.4,
          -11508.1,
          -11007.7,
          -10507.4,
          -10007.0,
          -9506.67,
          -9006.32,
          -8505.97,
          -8005.62,
          -7505.27,
          -7004.92,
          -6504.57,
          -6004.21,
          -5503.86,
          -5003.51,
          -4503.16,
          -4002.81,
          -3502.46,
          -3002.11,
          -2501.76,
          -2001.4,
          -1501.05,
          -1000.7,
          -500.351,
          0.0
        ]
      ]
    }
  ],
  "PARAMETERS": {
    "SeaLevel": "/ARRAY_2D/0",
    "OutputRelativeSealevelRate": true
  }
}
)";

std::string constant_input = R"(
{
  "VERSION": "2.0.0.0",
  "SCHEMA_VERSION": "1.0.0.0",
  "PARAMETERS": {
    "SeaLevel": 11,
    "OutputRelativeSealevelRate": true
  }
}
)";
using namespace Slb::Exploration::Gpm;

bool check_arrays_equal(const float_2darray_base_type& v1, const float_2darray_base_type& v2)
{
	if (v1.extents() == v2.extents()) {
        const auto res = std::equal(v1.begin(), v1.end(), v2.begin(), [](float a, float b) {return std::abs(a - b) < 1e-5; });
		return res;
	}
	return false;
}
int check_time_data_versions(int argc, char* argv[])
{
    Tools::gpm_default_logger my_logger(Tools::LOG_VERBOSE, true);
    gpm_time_function_data_io old_tester;
	auto iret = old_tester.read_parameters_from_string(old_input, my_logger);
	if (iret == 0) {
		gpm_time_function_data_io new_tester;
		iret = new_tester.read_parameters_from_string(new_input, my_logger);
		if (iret == 0) {
			auto ok1 = old_tester.do_output_relative_sealevel_rate() == new_tester.do_output_relative_sealevel_rate();
			auto ok2 = check_arrays_equal(old_tester.get_sealevel_array(), new_tester.get_sealevel_array());
			gpm_time_function_data_io constant_tester;
			iret = constant_tester.read_parameters_from_string(constant_input, my_logger);
			if (iret == 0) {
				auto test = constant_tester.get_sealevel_array();

				return ok1 && ok2 && test.cols()==1 ? 0 : 1;
			}
		}
	}
	return 1;
}
