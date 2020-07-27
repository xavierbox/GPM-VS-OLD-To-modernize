// -- Schlumberger Private --

#include "acf_json_doc_reader.h"
#include "rapidjson/reader.h"
#include "rapidjson/error/en.h"
#include "rapidjson/istreamwrapper.h"
#include <iostream>
#include <fstream>
#include <streambuf>
#include <rapidjson/document.h>

namespace Slb { namespace Exploration { namespace Gpm {
namespace {
bool types_are_equal(const rapidjson::Type& found, ACL_PARSE_POD_TYPES wanted)
{
    rapidjson::Type expected = rapidjson::kFalseType;
    switch (wanted) {
    case STRING:
        expected = rapidjson::kStringType;
        break;
    case INTEGER:
    case FLOAT:
    case DOUBLE:
        expected = rapidjson::kNumberType;
        break;
	default:
		break;
	}
    return found == expected;
}
int check_array(const TypeDescr& type_def, const rapidjson::GenericValue<rapidjson::UTF8<>>::ConstArray& values)
{
	int ret = 0;
	for (auto& v : values) {
		if (!types_are_equal(v.GetType(), type_def.pod_type)) {
			ret += 1;
		}
	}
	return ret;
}
}

acf_json_doc_reader::acf_json_doc_reader() = default;

acf_json_doc_reader::~acf_json_doc_reader() = default;

bool acf_json_doc_reader::parse_text(const std::string& text_to_parse)
{
    rapidjson::Document doc;
	doc.Parse(text_to_parse.c_str());
    if (!doc.HasParseError()){
        // Check files
		signal_data(doc);
		setup_dimensions();
        m_errno = 0;
        return true;
    }
    const rapidjson::ParseErrorCode e = doc.GetParseError();
    const size_t o = doc.GetErrorOffset();
    m_errno = e;
    m_error = std::string(rapidjson::GetParseError_En(e)) + " at offset " + std::to_string(o);
    return false;
}

void acf_json_doc_reader::setup_dimensions()
{
	std::vector<TypeDescr> arrays;
	std::copy_if(m_type_info.begin(), m_type_info.end(), std::back_inserter(arrays), [](const TypeDescr& val) {return val.num_dimensions > 0; });
	auto keys = m_counter.arrays_found();
    for(auto id:keys) {
		m_idimr[id].fill(1);
        auto ranges = dimensions(id);
		std::vector<index_type> dims;
		std::transform(ranges.begin(), ranges.end(), std::back_inserter(dims), [](const index_pair_type& val) {return val.second; });
		std::copy(dims.rbegin(), dims.rend(), m_idimr[id].rbegin());
    }
}


int acf_json_doc_reader::validate_document(const rapidjson::Document& doc)
{
	int ret = 0;
	std::vector<std::string> ids;
	for (auto& m : doc.GetObject()) {
		ids.emplace_back(m.name.Get<std::string>());
	}
	std::sort(ids.begin(), ids.end());
	std::vector<std::string> type_ids;
	std::transform(m_type_info.begin(), m_type_info.end(), std::back_inserter(type_ids), [](const TypeDescr& item) {return item.name; });
	std::sort(type_ids.begin(), type_ids.end());
	std::vector<std::string> intersection;
	std::set_intersection(ids.begin(), ids.end(), type_ids.begin(), type_ids.end(), std::back_inserter(intersection));
	std::map<std::string, std::vector<TypeDescr>::const_iterator> the_types;
    for(auto it = m_type_info.begin(); it != m_type_info.end();++it){
		the_types[it->name]= it;
	};
    if (intersection.size()==ids.size()) {
        // Got all legal ones
		for (auto& m : doc.GetObject()) {
            const auto type_def = *the_types[m.name.Get<std::string>()];
            if (type_def.num_dimensions==0){
				if (!types_are_equal(m.value.GetType(), type_def.pod_type)) {
					ret += 1;
				}
            }
            if (type_def.num_dimensions==1 ) {
				ret += m.value.IsArray() ? check_array(type_def, m.value.GetArray()) : 1;
            }
			if (type_def.num_dimensions == 2) {
				ret += m.value.IsArray() ? 0 : 1;
				if (m.value.IsArray()) {
					for (auto& a1 : m.value.GetArray()) {
						ret += a1.IsArray() ? check_array(type_def, a1.GetArray()) : 1;
					}
				}
			}
			if (type_def.num_dimensions == 3) {
				ret += m.value.IsArray() ? 0 : 1;
				if (m.value.IsArray()) {
					for (auto& a1 : m.value.GetArray()) {
						ret += a1.IsArray() ? 0 : 1;
						if (a1.IsArray()) {
							for (auto& a2 : a1.GetArray())
							{
								ret += a2.IsArray() ? check_array(type_def, a2.GetArray()) : 1;
							}
						}
					}
				}
			}
		}
    }
	return ret==0;
}

void acf_json_doc_reader::signal_value(const std::vector<TypeDescr>::value_type& id, const rapidjson::GenericValue<rapidjson::UTF8<>>& generic_value, const std::vector<index_type>& dummy)
{
    if (id.pod_type == STRING && generic_value.IsString()) {
		std::string tmp(generic_value.GetString(), generic_value.GetStringLength());
        signal_string(id.id, tmp, dummy);
		++m_inumr[id.id];
    }
    else if (generic_value.IsNumber()) {
        if (id.pod_type == INTEGER) {
            signal_integer(id.id, generic_value.GetInt(), dummy);
            ++m_inumr[id.id];
        }
        else if (id.pod_type == FLOAT) {
            signal_float(id.id, generic_value.GetFloat(), dummy);
            ++m_inumr[id.id];
        }
        else if (id.pod_type == DOUBLE) {
            signal_double(id.id, generic_value.GetDouble(), dummy);
            ++m_inumr[id.id];
        }
        else {
            int i = 0;
        }
    }
}

void acf_json_doc_reader::signal_data(const rapidjson::Document& doc)
{
	std::vector<TypeDescr> arrays;
    for(const auto& id:m_type_info) {
        if(doc.HasMember(id.name.c_str())) {
            const rapidjson::GenericValue<rapidjson::UTF8<>>& generic_value = doc[id.name.c_str()];
			if (id.num_dimensions == 0) {
                // POD
				std::vector<index_type> dummy;
				signal_value(id, generic_value, dummy);
			}
			if (id.num_dimensions == 1 && generic_value.IsArray()) { // POD
				std::vector<index_type> dummy(1);
				int i = 0;
				for (auto& a1 : generic_value.GetArray()) {
					dummy[0] = i;
					signal_value(id, a1, dummy);
					++i;
				}
			}
			if (id.num_dimensions == 2 && generic_value.IsArray()) { // POD
				std::vector<index_type> dummy(2);
				int i = 0;
				for (auto& a1 : generic_value.GetArray()) {
					dummy[0] = i;
					if (a1.IsArray()) {
						int j = 0;
						for (auto& a2 : a1.GetArray()) {
							dummy[1] = j;
							signal_value(id, a2, dummy);
							++j;
						}
					}
					++i;
				}
			}
			if (id.num_dimensions == 3 && generic_value.IsArray()) { // POD
				std::vector<index_type> dummy(3);
				int i = 0;
				for (auto& a1 : generic_value.GetArray()) {
					dummy[0] = i;
					if (a1.IsArray()) {
						int j = 0;
						for (auto& a2 : a1.GetArray()) {
							dummy[1] = j;
							if (a2.IsArray()) {
								int k = 0;
								for (auto& a3 : a2.GetArray()) {
									dummy[2] = k;
									signal_value(id, a3, dummy);
									++k;
								}
							}
							++j;
						}
					}
					++i;
				}
			}
        }
    }
}

bool acf_json_doc_reader::parse_file(const std::string& file) {
    rapidjson::Reader reader;
    std::ifstream t(file, std::ios_base::in);
	rapidjson::Document d;
    if (t.good()) {
        rapidjson::IStreamWrapper is(t);
		if (!d.ParseStream(is).HasParseError()) {
			signal_data(d);
			setup_dimensions();
            m_errno = 0;
            return true;
        }
        const rapidjson::ParseErrorCode e = reader.GetParseErrorCode();
        const size_t o = reader.GetErrorOffset();
        m_errno = e;
        m_error = std::string(rapidjson::GetParseError_En(e)) + " at offset " + std::to_string(o);
    }
    return false;

}

}}}
