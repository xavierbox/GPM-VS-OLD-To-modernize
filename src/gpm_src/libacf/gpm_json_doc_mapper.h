// -- Schlumberger Private --
#ifndef GPM_JSON_DOC_MAPPER_H
#define GPM_JSON_DOC_MAPPER_H
#include "gpm_param_type_mapper.h"
#include "gpm_logger.h"
#include <rapidjson/document.h>
#include <boost/any.hpp>
#include <map>

namespace Slb { namespace Exploration { namespace Gpm {
class gpm_json_doc_mapper {
public:
    // Int is the key value in the old description
    // Value is a shared_ptr to the array types if they are found
    using mapper_type = std::map<int, boost::any>;
    using const_json_array = rapidjson::Document::ConstArray;

    gpm_json_doc_mapper(const std::string& process_name, const std::string& old_schema=std::string(), const std::string& new_schema=std::string());
    int parse_text(const std::string& string, const Tools::gpm_logger& logger);
    int parse(std::istream& istream, const Tools::gpm_logger& logger);
    int parse_file(const std::string& file_name, const Tools::gpm_logger& logger);
    bool has_version_info() const;
    bool is_old_format() const;
	static bool is_rect2darray(const const_json_array& the_array);
    static std::pair<int, int> find_2drect_size(const const_json_array& the_array);
	template <typename T>
	static void fill_rect_2darray(const const_json_array& the_array, gpm_array_2d_base<T>* arr_p)
	{
		const auto num_rows = the_array.Size();
		if (num_rows > 0 && the_array[0].IsArray()) {
			auto num_cols = the_array[0].GetArray().Size();
			auto i = 0;
			for (auto& vi : the_array) {
				auto j = 0;
				for (auto& v : vi.GetArray()) {
					(*arr_p)(i, j) = v.Get<T>();
					++j;
				}
				++i;
			}
		}
	}

	template <typename T>
	static void resize_rect_2darray(const const_json_array& the_array, gpm_vbl_array_2d<T>* arr_p)
	{
		auto num_rows = the_array.Size();
		if (num_rows > 0 && the_array[0].IsArray()) {
			auto num_cols = the_array[0].GetArray().Size();
			arr_p->resize(num_rows, num_cols);
		}
	}

    void find_mapping(gpm_param_type_mapper* mapper_p);
	rapidjson::Document doc;
	rapidjson::Document schema_doc;
	std::vector<std::string> validation_error;
	std::string schema_error;
	std::string doc_error;
private:
	static bool array_has_size(const TypeDescr & it, const const_json_array & the_array);
	template <typename T>
	static void fill_array(const TypeDescr& it, const const_json_array& the_array, boost::any& mapper);

	int check_against_schema_text(const std::string& file_name, const Tools::gpm_logger& logger);

	std::string get_document_parse_error() const;
	int validate_schema();
	int check_schema_text(const std::string& loc_schema);
    int check_schema(std::istream& istream);

	std::string _process_name;
	std::string _file_name;
	std::string _old_schema;
	std::string _new_schema;
};
}}}
#endif // GPM_JSON_DOC_MAPPER_H
