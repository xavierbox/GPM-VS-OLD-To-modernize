// -- Schlumberger Private --

#include "gpm_string_utils.h"

using namespace Slb::Exploration::Gpm::Tools;

bool check_right_alpha_number() {
    std::string test1("ABF_TEST34");
    std::string alpha_part;
    int int_part;
    bool res = split_into_alpha_number(test1, alpha_part, int_part);
    res = res && alpha_part == "ABF_TEST" && int_part == 34;
    return res;
}

bool check_wrong_alpha_number() {
    std::string test1("ABF3.4");
    std::string alpha_part;
    int int_part;
    bool res = split_into_alpha_number(test1, alpha_part, int_part);

    return res ? false : true;
}

bool check_right_alpha_number_number() {
    std::string test1("ABF3.4");
    std::string alpha_part;
    int int_part1;
    int int_part2;
    bool res = split_into_alpha_number_number(test1, alpha_part, int_part1, int_part2);
    res = res && alpha_part == "ABF" && int_part1 == 3 && int_part2 == 4;
    return res;
}

bool check_wrong_alpha_number_number() {
    std::string test1("ABF3.4.4");
    std::string alpha_part;
    int int_part1;
    int int_part2;
    bool res = split_into_alpha_number_number(test1, alpha_part, int_part1, int_part2);

    return res ? false : true;
}

bool check_right_simple_fill_number() {
    std::string res("AB01");
    std::string actual = single_number_format_2_fill("AB", 1);
    bool is_ok = res == actual;
    return is_ok;
}

bool check_right_simple_fill_number_overflow() {
    std::string res("AB100");
    std::string actual = single_number_format_2_fill("AB", 100);
    bool is_ok = res == actual;
    return is_ok;
}

bool check_right_two_numbers() {
    std::string res("AB1.1");
    std::string actual = two_number_format("AB", 1, 1);
    bool is_ok = res == actual;
    return is_ok;
}

bool check_right_simple_two_fill_numbers() {
    std::string res("AB01.01");
    std::string actual = two_number_format_2_fill("AB", 1, 1);
    bool is_ok = res == actual;
    return is_ok;
}


int string_test(int argc, char* argv[]) {
    bool res = check_right_alpha_number() && check_wrong_alpha_number();
    res = res && check_right_alpha_number_number() && check_wrong_alpha_number_number();
    res = res && check_right_simple_fill_number() && check_right_simple_fill_number_overflow();
    res = res && check_right_two_numbers() && check_right_simple_two_fill_numbers();
    return res ? 0 : 1;
}
