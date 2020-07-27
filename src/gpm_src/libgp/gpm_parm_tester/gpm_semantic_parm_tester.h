// -- Schlumberger Private --

#ifndef GPM_SEMANTIC_PARM_TESTER_H
#define GPM_SEMANTIC_PARM_TESTER_H
#include <string>
int semantic_diff_parm_hdf5_format(const std::string& output_file, const std::string& org_file_ver, bool check_same_ages=true, float rel_diff=5e-3f, float abs_diff=5e-3f);
int diff_parm_mass_balance_in_hdf5_format(const std::string& output_file, const std::string& org_file_ver, bool check_same_ages = true, float rel_diff = 5e-3f, float abs_diff = 5e-5f);
#endif