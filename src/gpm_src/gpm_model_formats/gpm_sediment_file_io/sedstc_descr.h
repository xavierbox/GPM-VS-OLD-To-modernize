// -- Schlumberger Private --

#ifndef _sedstc_descr_h_
#define _sedstc_descr_h_

#include "gpm_type_descriptor.h"

#include <vector>
#include <string>

namespace Slb { namespace Exploration { namespace Gpm{
// Include the enum def
#include "sed_enum_def.h"

  // Our stuff
	class pars_descr_holder{
	public:
		static const std::vector<TypeDescr>& get_pars_descr();  
		static std::string get_current_version();
	};
}}}


#endif
