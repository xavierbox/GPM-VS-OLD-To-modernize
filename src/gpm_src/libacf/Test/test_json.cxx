#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#include <iostream>
#include "mapstc_descr.h"
#include <rapidjson/document.h>
#include <rapidjson/pointer.h>

int test_json_pretty(int argc, char* argv[]) {
	rapidjson::StringBuffer s;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(s);
	//writer.SetFormatOptions(rapidjson::PrettyFormatOptions::kFormatSingleLineArray);
	writer.StartObject();
	writer.Key("hello");
	writer.String("world");
	writer.Key("t");
	writer.Bool(true);
	writer.Key("f");
	writer.Bool(false);
	writer.Key("n");
	writer.Null();
	writer.Key("i");
	writer.Uint(123);
	writer.Key("pi");
	writer.Double(3.1416);
	writer.Key("a");
	writer.StartArray();
	for (auto l = 0; l < 10; ++l) {
		writer.StartArray();
		for (unsigned i = 0; i < 10; i++) {
			writer.Uint(i);
		}
		writer.EndArray();
	}
	writer.EndArray();
	writer.EndObject();
	std::cout << s.GetString() << std::endl;
	return 0;
}

int test_json_history(int argc, char* argv[]) {
	auto test = Slb::Exploration::Gpm::parm_type_descr_holder::make_json_history_item("GPM", "RRFF");
	auto test2 = Slb::Exploration::Gpm::parm_type_descr_holder::make_json_history_item("GPM");
	return 0;
}

int json_pointer_test(int argc, char* argv[])
{
	std::string json_string = std::string("{ \"project\" : \"RapidJSON\", \"stars\" : 10, \"refe\":\"/stars\" }");

	// ...
	rapidjson::Document document;
	document.Parse(json_string.c_str());
	int i = 0;
	auto pointer = document["refe"].Get<std::string>();
	rapidjson::Value* at = rapidjson::Pointer(pointer).Get(document);
	auto num = at->GetInt();
	std::string json_string1 = std::string(R"({ "a":"/test" })");
	rapidjson::Pointer p("/test");
	rapidjson::StringBuffer sb;
	p.Stringify(sb);
	std::cout << sb.GetString() << std::endl;
	return 0;
}