// -- Schlumberger Private --

#include "gpm_string_utils.h"
//#include "format_string.h"
#include <boost/lexical_cast.hpp>
#include <boost/xpressive/xpressive.hpp>

#include <sstream>
#include <iomanip>

namespace Slb { namespace Exploration { namespace Gpm { namespace Tools {
using namespace boost::xpressive;

namespace {

template <class T>
std::string
simple_format_string_basic(T inp, int prec, bool dofill, char fill = '0') {
    std::ostringstream ss;
    ss << std::setw(prec);
    if (dofill) {
        ss << std::setfill(fill);
    }
    ss << inp;
    return ss.str();
}

template <class T>
std::string
simple_format_string(T inp, int prec) {
    return simple_format_string_basic(inp, prec, false);
}

template <class T>
std::string
simple_format_string_2(T inp) {
    return simple_format_string(inp, 2);
}

template <class T>
std::string
simple_format_string_3(T inp) {
    return simple_format_string(inp, 3);
}

template <class T>
std::string
simple_format_string_fill(T inp, int prec) {
    return simple_format_string_basic(inp, prec, true);
}

template <class T>
std::string
simple_format_string_2_fill(T inp) {
    return simple_format_string_fill(inp, 2);
}

template <class T>
std::string
simple_format_string_3_fill(T inp) {
    return simple_format_string_fill(inp, 3);
}

}

// These are typical strings of type
// ABCnn

bool split_into_alpha_number(const std::string& input, std::string& alpha_str, int& num) {
    sregex rex = (s1 = +alpha >> *('_'>>+alpha)) >> (s2 = +_d);
    smatch what;
    bool retval = false;
    if (regex_match(input, what, rex)) {
        alpha_str = what[1];
        num = boost::lexical_cast<int>(what[2]);
        retval = true;
    }
    return retval;
}


//ABCnn.nn
bool split_into_alpha_number_number(const std::string& input, std::string& alpha_str, int& num1, int& num2) {
    sregex rex = (s1 = +alpha) >> (s2 = +_d) >> "." >> (s3 = +_d);
    smatch what;
    bool retval = false;
    if (regex_match(input, what, rex)) {
        alpha_str = what[1];
        num1 = boost::lexical_cast<int>(what[2]);
        num2 = boost::lexical_cast<int>(what[3]);
        retval = true;
    }
    return retval;
}

std::string single_string_format(const std::string& alpha_str_1, const std::string& alpha_str_2) {
    return alpha_str_1 + alpha_str_2;
}

std::string single_number_format(const std::string& alpha_str, int num1) {
    return alpha_str + std::to_string(num1);
}

std::string two_number_format(const std::string& alpha_str, int num1, int num2) {
    return alpha_str + std::to_string(num1) + "." + std::to_string(num2);
}

std::string single_number_format_2_fill(const std::string& alpha_str, int num1) {
    return alpha_str + simple_format_string_2_fill(num1);
}

std::string two_number_format_2_fill(const std::string& alpha_str, int num1, int num2) {
    return alpha_str + simple_format_string_2_fill(num1) + "." + simple_format_string_2_fill(num2);
}

}}}}
