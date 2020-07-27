// -- Schlumberger Private --

#include "gpm_json_doc_mapper.h"
#include "gpm_json_keywords.h"
#include "gpm_vbl_array_2d.h"
#include "gpm_vbl_array_3d.h"
#include "gpm_logger.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>
#include "rapidjson/schema.h"
#include <rapidjson/stringbuffer.h>
#include <fstream>
#include <sstream>
#include "gpm_version_number.h"

namespace Slb { namespace Exploration { namespace Gpm {



// These are assumed to have a schema that will test that they are jagged arrays
// Assume they are non empty
bool gpm_json_doc_mapper::is_rect2darray(const const_json_array& the_array)
{
    const auto num_rows = the_array.Size();
	std::vector<int> col_size;
	std::vector<bool> is_col_value;
	if (std::all_of(the_array.begin(), the_array.end(), [](const const_json_array::PlainType& arr) {return arr.IsArray(); })) {
		for (auto& v : the_array) {
			auto col_arr=v.GetArray();
			is_col_value.push_back(std::all_of(col_arr.begin(), col_arr.end(), [](const const_json_array::PlainType& item) {return item.IsNumber(); }));
			col_size.push_back(col_arr.Size());
		}
		auto col_size1 = !col_size.empty() ? col_size.front() : 0;
		return num_rows > 0 && col_size1 > 0 && std::all_of(col_size.begin(), col_size.end(), [col_size1](int val) {return val == col_size1; })
			&& std::all_of(is_col_value.begin(), is_col_value.end(),[](bool val) {return val; });
	}
	return false;
}

std::pair<int, int> gpm_json_doc_mapper::find_2drect_size(const const_json_array & the_array)
{
    auto num_rows = the_array.Size();
    if (num_rows > 0 && the_array[0].IsArray()) {
        auto num_cols = the_array[0].GetArray().Size();
        return std::make_pair(num_rows, num_cols);
    }
    return std::pair<int, int>();
}

bool gpm_json_doc_mapper::array_has_size(const TypeDescr& it, const const_json_array& the_array)
{
    if (it.num_dimensions == 1) {
        return the_array.Size() > 0;
    }
    if (it.num_dimensions == 2) {
        const auto num_rows = the_array.Size();
        if (num_rows > 0 && the_array[0].IsArray()) {
            const auto num_cols = the_array[0].GetArray().Size();
            return num_cols > 0;
        }
    }
    const auto num_layers = the_array.Size();
    if (num_layers > 0 && the_array[0].IsArray()) {
        auto row_array = the_array[0].GetArray();
        const auto num_rows = row_array.Size();
        if (num_rows > 0 && row_array[0].IsArray()) {
            const auto num_cols = row_array[0].GetArray().Size();
            return num_cols > 0;
        }
    }
	return false;
}

template <typename T>
void gpm_json_doc_mapper::fill_array(const TypeDescr& it, const const_json_array& the_array, boost::any& mapper)
{
	if (array_has_size(it, the_array)) {
		if (it.num_dimensions == 1) {
			auto tmp(boost::any_cast<std::shared_ptr<typename gpm_param_type_mapper::Array_Promotion<ARRAY_1d, T>::array_type>>(mapper));
			for (auto& v : the_array) {
				tmp->push_back(v.Get<T>());
			}
		}
		else if (it.num_dimensions == 2) {
			auto tmp(boost::any_cast<std::shared_ptr<typename gpm_param_type_mapper::Array_Promotion<ARRAY_2d, T>::array_type>>(mapper));
			resize_rect_2darray(the_array, tmp.get());
			fill_rect_2darray(the_array, tmp.get());
		}
		else {
			auto tmp(boost::any_cast<std::shared_ptr<typename gpm_param_type_mapper::Array_Promotion<ARRAY_3d, T>::array_type>>(mapper));
			const auto num_layers = the_array.Size();
			if (num_layers > 0 && the_array[0].IsArray()) {
				auto row_array = the_array[0].GetArray();
				auto num_rows = row_array.Size();
				if (num_rows > 0 && row_array[0].IsArray()) {
					auto num_cols = row_array[0].GetArray().Size();
					tmp->resize(num_layers, num_rows, num_cols);
					int i = 0;
					for (auto& vi : the_array) {
						fill_rect_2darray(vi.GetArray(), &((*tmp)(i)));
						++i;
					}
				}
			}
		}
	}
}

gpm_json_doc_mapper::gpm_json_doc_mapper(const std::string& process_name, const std::string& old_schema,
                                         const std::string& new_schema): _process_name(process_name),
                                                                         _old_schema(old_schema),
                                                                         _new_schema(new_schema)
{
}

int gpm_json_doc_mapper::parse_text(const std::string& string, const Tools::gpm_logger& logger)
{
	if (string.empty()) {
		return -rapidjson::ParseErrorCode::kParseErrorDocumentEmpty;
	}
	std::stringstream s(string);
    int ret_val = parse(s, logger);
	if (ret_val != 0) {
		logger.print(Tools::LOG_IMPORTANT, "%s file text Error message :%s\n", _process_name.c_str(), doc_error.c_str());
	}
	return ret_val;
}

int gpm_json_doc_mapper::parse(std::istream& istream, const Tools::gpm_logger& logger)
{
	int ret_val = 0;
	doc_error.clear();
	if (istream.good()) {
		rapidjson::IStreamWrapper is(istream);
		if (doc.ParseStream<rapidjson::kParseNanAndInfFlag>(is).HasParseError()) {
			ret_val = doc.GetParseError();
			doc_error = get_document_parse_error();
		}
        else {
            auto schema = has_version_info() && !is_old_format() ? _new_schema : _old_schema;
            if (!schema.empty()) {
                ret_val = check_against_schema_text(schema, logger);
                if (ret_val == 0) {
                    ret_val = validate_schema();
                }
            }
        }
    }
	else {
		ret_val = rapidjson::ParseErrorCode::kParseErrorDocumentEmpty;
		doc_error = std::string("Input stream not valid");
	}
	if (ret_val != 0) {
		logger.print(Tools::LOG_IMPORTANT, "%s file %s\n Error message :%s\n", _process_name.c_str(), _file_name.c_str(), doc_error.c_str());
		ret_val = -ret_val;
	}
	return ret_val;
}
std::string gpm_json_doc_mapper::get_document_parse_error() const
{
	if (doc.HasParseError()) {
		return std::string("Error(offset ") + std::to_string(static_cast<unsigned>(doc.GetErrorOffset())) +
			rapidjson::GetParseError_En(doc.GetParseError());
	}
	return std::string();
}
int gpm_json_doc_mapper::parse_file(const std::string& file_name, const Tools::gpm_logger& logger)
{
    if(file_name.empty()) {
		return -rapidjson::ParseErrorCode::kParseErrorDocumentEmpty;
    }
	rapidjson::Reader reader;
	std::ifstream t(file_name, std::ios_base::in);
	_file_name = file_name;
	int ret_val = parse(t, logger);
    return ret_val;
}
bool gpm_json_doc_mapper::has_version_info() const
{
	return doc.HasMember(gpm_json_keywords::VERSION);
}
bool gpm_json_doc_mapper::is_old_format() const
{
    std::string key(gpm_json_keywords::VERSION);
    const auto major_version = Tools::gpm_version_number(doc[gpm_json_keywords::VERSION].Get<std::string>()).get_major();
    return major_version < 2;
}
int gpm_json_doc_mapper::check_against_schema_text(const std::string& file_name, const Tools::gpm_logger& logger)
{
    const int ret_val = check_schema_text(file_name);
	if (ret_val != 0) {
		logger.print(Tools::LOG_IMPORTANT, "%s text %s schema parse failed \n Error message :%s\n", _process_name.c_str(), file_name.c_str(), schema_error.c_str());
        for(const auto& it:validation_error) {
			logger.print(Tools::LOG_IMPORTANT, "%s: %s\n", _process_name.c_str(), it.c_str());
        }
	}
	return ret_val;
}

int gpm_json_doc_mapper::validate_schema( )
{
    rapidjson::SchemaDocument schema(schema_doc); // Compile a Document to SchemaDocument
    rapidjson::SchemaValidator validator(schema);
	validation_error.clear();
	int ret_val = 0;
    if (!doc.Accept(validator)) {
        // Input JSON is invalid according to the schema
        // Output diagnostic information
        rapidjson::StringBuffer sb;
        validator.GetInvalidSchemaPointer().StringifyUriFragment(sb);
		validation_error.push_back(std::string("Invalid schema: ") + std::string(sb.GetString()));
		validation_error.push_back(std::string("Invalid keyword: ")+ std::string(validator.GetInvalidSchemaKeyword()));
        sb.Clear();
        validator.GetInvalidDocumentPointer().StringifyUriFragment(sb);
		validation_error.push_back( std::string("Invalid document: ")+ std::string(sb.GetString()));
        ret_val = 64; // Schema error
    }
    return ret_val;
}

int gpm_json_doc_mapper::check_schema_text(const std::string& loc_schema)
{
	int ret_val = 0;
	schema_error.clear();
	validation_error.clear();
	if (!loc_schema.empty()) {
		std::stringstream s(loc_schema);
		return check_schema(s);
	}
	return rapidjson::ParseErrorCode::kParseErrorDocumentEmpty + 32;
}
int gpm_json_doc_mapper::check_schema(std::istream& istream)
{
	int ret_val = 0;
	if (istream.good()) {
		rapidjson::IStreamWrapper is(istream);
		if (schema_doc.ParseStream<rapidjson::kParseNanAndInfFlag>(is).HasParseError()) {
			// the schema is not a valid JSON.
			ret_val = schema_doc.GetParseError() + 32;
			schema_error = "Schema is not valid json";
			return ret_val;
		}
        return validate_schema();
    }
    ret_val = 32 + rapidjson::ParseErrorCode::kParseErrorDocumentEmpty;
    return ret_val;
}

void gpm_json_doc_mapper::find_mapping(gpm_param_type_mapper* mapper_p)
{
	auto& mapper(*mapper_p);
    for(const auto& it: mapper._param_descrs) {
		const auto& type = it.second;
		auto& map_holder(mapper._params);
        if(doc.HasMember(type.name.c_str())) {
			mapper.add_id_holder(type.id);
			const auto& val = doc[type.name.c_str()];
            if ((val.IsNumber() || val.IsString()) && type.num_dimensions==0 && mapper.is_value_type(type.id)) {
                // Basic types
				boost::any& tmp(map_holder[type.id]);
                if (val.IsString()) {
                    tmp = boost::any(std::string(val.GetString(), val.GetStringLength()));
                }
                else {
                    if (type.pod_type == INTEGER) {
                        tmp = val.GetInt();
                    }
                    else if (type.pod_type == FLOAT) {
                        tmp = val.GetFloat();
                    }
                    else {
                        tmp =val.GetDouble();
                    }
                }
            }
			else if (val.IsArray() && mapper.is_array_type(type.id)) {
				const const_json_array& the_array(val.GetArray());
				if (array_has_size(type, the_array)) {
					if (type.pod_type == FLOAT) {
						fill_array<float>(type, the_array, map_holder[type.id]);
					}
					else if (type.pod_type == DOUBLE) {
						fill_array<double>(type, the_array, map_holder[type.id]);
					}
					else if (type.pod_type == INTEGER) {
						fill_array<int>(type, the_array, map_holder[type.id]);
					}
					else if (type.pod_type == STRING) {
						fill_array<std::string>(type, the_array, map_holder[type.id]);
					}
				}
                else {
                    // Remove the empty ones, that the prev parser just ignored
					mapper._params.erase(type.id);
                }
			}
        }
    }

}
}}}
