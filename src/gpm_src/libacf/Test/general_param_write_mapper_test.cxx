// -- Schlumberger Private --

#include "general_param_write_mapper.h"
#include "acf_json_writer.h"
#include "acf_json_reader.h"
#include "general_param_mapper.h"

#include <boost/algorithm/string.hpp>

namespace {
enum test {
    String,
    Int,
    Float,
    Double,
    StringList,
    IntList,
    FloatList,
    DoubleList,
    String2DArray,
    Int2DArray,
    Float2DArray,
    Double2DArray,
    String3DArray,
    Int3DArray,
    Float3DArray,
    Double3DArray
};

}

using namespace Slb::Exploration::Gpm;

std::vector<std::string> write_json_statement_list =
{ "{",
"\"String\": \"1\",",
"\"Int\":1,",
"\"Float\":1,",
"\"Double\":1,",
"\"StringList\":[\"0\",\"1\",\"2\"],",
"\"IntList\":[0,1,2],",
"\"FloatList\":[0,1,2],",
"\"DoubleList\":[0,1,2],",
"\"String2DArray\":[",
"    [\"0\",\"1\",\"2\"],",
"    [\"3\",\"4\",\"5\"]",
"],",
"\"Int2DArray\":[",
"    [0,1,2],",
"    [3,4,5]",
"],",
"\"Float2DArray\":[",
"    [0,1,2],",
"    [3,4,5]",
"],",
"\"Double2DArray\":[",
"    [0,1,2],",
"    [3,4,5]",
"],",
"\"String3DArray\":[",
"    [",
"        [\"0\",\"1\",\"2\",\"3\"],",
"        [\"4\",\"5\",\"6\",\"7\"],",
"        [\"8\",\"9\",\"10\",\"11\"]",
"    ],",
"    [",
"        [\"12\",\"13\",\"14\",\"15\"],",
"        [\"16\",\"17\",\"18\",\"19\"],",
"        [\"20\",\"21\",\"22\",\"23\"]",
"    ]",
"],",
"\"Int3DArray\":[",
"    [",
"        [0,1,2,3],",
"        [4,5,6,7],",
"        [8,9,10,11]",
"    ],",
"    [",
"        [12,13,14,15],",
"        [16,17,18,19],",
"        [20,21,22,23]",
"    ]",
"],",
"\"Float3DArray\":[",
"    [",
"        [0,1,2,3],",
"        [4,5,6,7],",
"        [8,9,10,11]",
"    ],",
"    [",
"        [12,13,14,15],",
"        [16,17,18,19],",
"        [20,21,22,23]",
"    ]",
"],",
"\"Double3DArray\":[",
"    [",
"        [0,1,2,3],",
"        [4,5,6,7],",
"        [8,9,10,11]",
"    ],",
"    [",
"        [12,13,14,15],",
"        [16,17,18,19],",
"        [20,21,22,23]",
"    ]",
"]",
"}" };


TypeDescrToFunctionMapper make_description_proxy() {
    std::vector<TypeDescr> vals = { (make_typedescription<STRING>(String, "String")),
        (make_typedescription<INTEGER>(Int, "Int")),
        (make_typedescription<FLOAT>(Float, "Float")),
        (make_typedescription<DOUBLE>(Double, "Double")),
        (make_typedescription<STRING>(StringList, "StringList", ARRAY_1d)),
        (make_typedescription<INTEGER>(IntList, "IntList", ARRAY_1d)),
        (make_typedescription<FLOAT>(FloatList, "FloatList", ARRAY_1d)),
        (make_typedescription<DOUBLE>(DoubleList, "DoubleList", ARRAY_1d)),
        (make_typedescription<STRING>(String2DArray, "String2DArray", ARRAY_2d)),
        (make_typedescription<INTEGER>(Int2DArray, "Int2DArray", ARRAY_2d)),
        (make_typedescription<FLOAT>(Float2DArray, "Float2DArray", ARRAY_2d)),
        (make_typedescription<DOUBLE>(Double2DArray, "Double2DArray", ARRAY_2d)),
        (make_typedescription<STRING>(String3DArray, "String3DArray", ARRAY_3d)),
        (make_typedescription<INTEGER>(Int3DArray, "Int3DArray", ARRAY_3d)),
        (make_typedescription<FLOAT>(Float3DArray, "Float3DArray", ARRAY_3d)),
        (make_typedescription<DOUBLE>(Double3DArray, "Double3DArray", ARRAY_3d)) };
    TypeDescrToFunctionMapper tmp(vals);
    std::vector<std::pair<acf_base_writer::key_type, FunctionConstant<int>>> int_val = { (std::make_pair(Int, FunctionConstant<int>([]() { return 1; }))) };
    std::vector<std::pair<acf_base_writer::key_type, FunctionConstant<float>>> float_val = { (std::make_pair(Float, FunctionConstant<float>([]() { return 1.0F; }))) };
    std::vector<std::pair<acf_base_writer::key_type, FunctionConstant<double>>> double_val = { (std::make_pair(Double, FunctionConstant<double>([]() { return 1.0; }))) };
    std::vector<std::pair<acf_base_writer::key_type, FunctionConstant<std::string>>> string_val = { (std::make_pair(String, FunctionConstant<std::string>([]() { return "1"; }))) };
    tmp.set_pod_functions(int_val);
    tmp.set_pod_functions(float_val);
    tmp.set_pod_functions(double_val);
    tmp.set_pod_functions(string_val);
    std::vector<std::pair<acf_base_writer::key_type, Function1d<std::string>>> string_list = { (std::make_pair(StringList, Function1d<std::string>(3, [](FunctionBase::index_type i) { return std::to_string(i); }))) };
    std::vector<std::pair<acf_base_writer::key_type, Function1d<int>>> int_list = { (std::make_pair(IntList, Function1d<int>(3, [](FunctionBase::index_type i) { return static_cast<int>(i); }))) };
    std::vector<std::pair<acf_base_writer::key_type, Function1d<float>>> float_list = { (std::make_pair(FloatList, Function1d<float>(3, [](FunctionBase::index_type i) { return static_cast<float>(i); }))) };
    std::vector<std::pair<acf_base_writer::key_type, Function1d<double>>> double_list = { (std::make_pair(DoubleList, Function1d<double>(3, [](FunctionBase::index_type i) { return static_cast<double>(i); }))) };
    tmp.set_array1d_functions(string_list);
    tmp.set_array1d_functions(int_list);
    tmp.set_array1d_functions(float_list);
    tmp.set_array1d_functions(double_list);
    std::vector<std::pair<acf_base_writer::key_type, Function2d<std::string>>> string_2d = { (std::make_pair(String2DArray, Function2d<std::string>(2, 3, [](FunctionBase::index_type i, FunctionBase::index_type j) { return std::to_string(i * 3 + j); }))) };
    std::vector<std::pair<acf_base_writer::key_type, Function2d<int>>> int_2d = { (std::make_pair(Int2DArray, Function2d<int>(2, 3, [](FunctionBase::index_type i, FunctionBase::index_type j) { return static_cast<int>(i * 3 + j); }))) };
    std::vector<std::pair<acf_base_writer::key_type, Function2d<float>>> float_2d = { (std::make_pair(Float2DArray, Function2d<float>(2, 3, [](FunctionBase::index_type i, FunctionBase::index_type j) { return static_cast<float>(i * 3 + j); }))) };
    std::vector<std::pair<acf_base_writer::key_type, Function2d<double>>> double_2d = { (std::make_pair(Double2DArray, Function2d<double>(2, 3, [](FunctionBase::index_type i, FunctionBase::index_type j) { return static_cast<double>(i * 3 + j); }))) };
    tmp.set_array2d_functions(string_2d);
    tmp.set_array2d_functions(int_2d);
    tmp.set_array2d_functions(float_2d);
    tmp.set_array2d_functions(double_2d);
    std::vector<std::pair<acf_base_writer::key_type, Function3d<std::string>>> string_3d = { (std::make_pair(String3DArray, Function3d<std::string>(2, 3, 4, [](FunctionBase::index_type i, FunctionBase::index_type j, FunctionBase::index_type k) { return std::to_string(i * 12 + j * 4 + k); }))) };
    std::vector<std::pair<acf_base_writer::key_type, Function3d<int>>> int_3d = { (std::make_pair(Int3DArray, Function3d<int>(2, 3, 4, [](FunctionBase::index_type i, FunctionBase::index_type j, FunctionBase::index_type k) { return static_cast<int>(i * 12 + j * 4 + k); }))) };
    std::vector<std::pair<acf_base_writer::key_type, Function3d<float>>> float_3d = { (std::make_pair(Float3DArray, Function3d<float>(2, 3, 4, [](FunctionBase::index_type i, FunctionBase::index_type j, FunctionBase::index_type k) { return static_cast<float>(i * 12 + j * 4 + k); }))) };
    std::vector<std::pair<acf_base_writer::key_type, Function3d<double>>> double_3d = { (std::make_pair(Double3DArray, Function3d<double>(2, 3, 4, [](FunctionBase::index_type i, FunctionBase::index_type j, FunctionBase::index_type k) { return static_cast<double>(i * 12 + j * 4 + k); }))) };
    tmp.set_array3d_functions(string_3d);
    tmp.set_array3d_functions(int_3d);
    tmp.set_array3d_functions(float_3d);
    tmp.set_array3d_functions(double_3d);
    return tmp;
}


int general_param_write_json_mapper_test(int /*argc*/, char * /*argv*/[]) {
    auto tmp = make_description_proxy();
    auto file_name = "test_json_write"; {
        general_param_write_mapper writer;
        acf_json_writer writer_acf;
        writer.attach_to_writer(file_name, tmp, &writer_acf);
        writer_acf.write();
        writer_acf.disconnect_slots();
    }
    std::ifstream fpctl((file_name), std::ios_base::in);
    std::vector<std::string> res;
    std::string line;
    while (std::getline(fpctl, line)) {
        res.push_back(line);
    }
    auto res_count = 0;
    if (res.size() == write_json_statement_list.size()) {
        for (auto i = 0; i < res.size(); ++i) {
            auto t1 = res[i];
            auto t2 = write_json_statement_list[i];
            boost::trim(t1);
            boost::trim(t2);
            if (t1 != t2) {
                ++res_count;
            }
        }
    }
    else {
        ++res_count;
    }
    return res_count;
}

int general_param_write_read_json_mapper_test(int /*argc*/, char * /*argv*/[]) {
    auto tmp = make_description_proxy();
    auto file_name = "test_json_write_read"; {
        general_param_write_mapper writer;
        acf_json_writer writer_acf;
        writer.attach_to_writer(file_name, tmp, &writer_acf);
        writer_acf.write();
        writer_acf.disconnect_slots();
    } 
	{
        acf_json_reader read_acf;
        general_param_read_mapper reader;
        read_acf.set_parse_type_info(tmp.types);
        reader.attach_to_parser(read_acf);
        read_acf.parse_file(file_name);
    }
    std::ifstream fpctl((file_name), std::ios_base::in);
    std::vector<std::string> res;
    std::string line;
    while (std::getline(fpctl, line)) {
        res.push_back(line);
    }
    auto res_count = 0;
    if (res.size() == write_json_statement_list.size()) {
        for (auto i = 0; i < res.size(); ++i) {
            auto t1 = res[i];
            auto t2 = write_json_statement_list[i];
            boost::trim(t1);
            boost::trim(t2);
            if (t1 != t2) {
                ++res_count;
            }
        }
    }
    else {
        ++res_count;
    }
    return res_count;
}

