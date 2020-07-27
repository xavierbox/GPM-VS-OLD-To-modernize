// -- Schlumberger Private --

#include "general_param_mapper.h"
#include "gpm_format_helper.h"
#include "gpm_logger.h"
#include "acf_json_reader.h"
#include <rapidjson/document.h>
#include "gpm_json_doc_mapper.h"

using namespace Slb::Exploration::Gpm;
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
		(make_typedescription<DOUBLE>(Double2DArray, "Double2DArray", ARRAY_2d)) };

	std::vector<std::string> file =
	{ "{\"String\":\"123\",",
		"\"Int\":123,",
		"\"Float\":123,",
		"\"Double\":123,",
		"\"StringList\":[\"123\", \"456\"],",
		"\"IntList\":[123, 456],",
		"\"FloatList\":[123, 456],",
		"\"DoubleList\":[123, 456],",
		"\"String2DArray\":[[\"123\", \"456\"],",
		"[\"789\", \"100\"],",
		"[\"A\", \"B\"]],",
		"\"Int2DArray\":[[123, 456],",
		"[789, 100]],",
		"\"Float2DArray\":[[123, 456],",
		"[789, 100]],",
		"\"Double2DArray\":[[123, 456],",
		"[789, 100]]}" };
}

int base_param_holder_test(int /*argc*/, char * /*argv*/[]) {

    acf_json_reader parse;
    parse.set_parse_type_info(vals);
    general_param_read_mapper tmp(parse);
    std::string buf;
    // Lets make the parser thing properly
    for (int i = 0; i < file.size(); ++i) {
        buf += file[i];
        buf += "\n";
    }
    parse.parse_text(buf);
    gpm_vbl_array_2d<int> tmp1;
    auto res = tmp.get_array_ptr<INTEGER, ARRAY_2d>(Int2DArray);
    auto res1 = tmp.get_array_ptr<STRING, ARRAY_2d>(String2DArray);
    std::vector<std::string> ids;
    ids.push_back("A");
    ids.push_back("B");
    Slb::Exploration::Gpm::Tools::gpm_default_logger logger(Slb::Exploration::Gpm::Tools::LOG_DEBUG, true);
    int num = gpm_format_helper::check_for_valid_entries(*res1, ids, "tull", logger);
    bool has_it = tmp.has_entry(Int2DArray);
    return 0;
}

int json_doc_mapper_test(int /*argc*/, char * /*argv*/[])
{
	std::string buf;
	// Lets make the parser thing properly
	for (int i = 0; i < file.size(); ++i) {
		buf += file[i];
		buf += "\n";
	}
	Tools::gpm_default_logger logger(Tools::LOG_DEBUG, true);
	gpm_json_doc_mapper test("json_doc_mapper_test", std::string());
	if (test.parse_text(buf, logger)==0) {
		gpm_param_type_mapper mapper;
		mapper.initialize_parameters(vals);
		test.find_mapping(&mapper);
		return 0;
	}
    return 1;
}
