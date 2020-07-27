#ifndef gpm_acf_GitSHA1_h
#define gpm_acf_GitSHA1_h

#include <string>

namespace Slb {
	namespace Exploration {
		namespace Gpm {

			class gpm_acf_GitSHA1 {
			public:
				static std::string get_hash();
			};
		}
	}
}

#endif