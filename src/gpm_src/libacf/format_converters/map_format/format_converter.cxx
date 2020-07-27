// -- Schlumberger Private --

#include "mapstc_descr.h"
#include "acf_format_selector.h"
#include <boost/algorithm/string.hpp>


using namespace Slb::Exploration::Gpm;


int main( int argc, char *argv[])
{
	// We need to find the old name, the new name 
	std::vector<std::string> program_options(argv, argv+argc);
	if (program_options.size() < 3){
		printf("The format is: old file name new_file_name\n");
	}

  // This may be tricky, lets atrt with diffusion
 //
	auto is_hdf5 = boost::iends_with(program_options[1], ".hdf5");
    const auto in_format = acf_format::find_format_according_to_suffix(program_options[1]);
    const auto out_format = acf_format::find_format_according_to_suffix(program_options[2]);;
    if (in_format != out_format) {
        acf_format converter(in_format, out_format);
        converter.setup_format(parm_descr_holder::get_parm_descr());
        converter.convert(program_options[1], program_options[2]);
    }
}
