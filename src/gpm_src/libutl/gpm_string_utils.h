// -- Schlumberger Private --

#ifndef gpm_string_utils_h
#define gpm_string_utils_h

#include <string>

namespace Slb { namespace Exploration { namespace Gpm { namespace Tools {

// These are typical strings of type
// ABCnn
bool split_into_alpha_number(const std::string& input, std::string& alpha_str, int& num);

bool split_into_alpha_number_number(const std::string& input, std::string& alpha_str, int& num1, int& num2);

std::string single_number_format(const std::string& alpha_str, int num1);
std::string single_string_format(const std::string& alpha_str_1, const std::string& alpha_str_2);
std::string two_number_format(const std::string& alpha_str, int num1, int num2);
std::string single_number_format_2_fill(const std::string& alpha_str, int num1);
std::string two_number_format_2_fill(const std::string& alpha_str, int num1, int num2);

}}}}
#endif
