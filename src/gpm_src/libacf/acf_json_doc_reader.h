// -- Schlumberger Private --

#ifndef acf_json_doc_reader_h
#define acf_json_doc_reader_h

// A wrapper for the acf parser currently used, but where we use signals to call back to the struct
#include "acf_base_reader.h"
#include <rapidjson/document.h>


namespace Slb { namespace Exploration { namespace Gpm{

class acf_json_doc_reader: public acf_base_reader
{
public:
	typedef acf_base_reader base;
	acf_json_doc_reader();
	acf_json_doc_reader(const acf_json_doc_reader& rhs) = delete;
	acf_json_doc_reader& operator=(const acf_json_doc_reader& rhs) = delete;
	~acf_json_doc_reader();
	
	// Returns the state of the stream
	bool parse_text(const std::string& text_to_parse) override;
 	bool parse_file(const std::string& file) override;
private:
	void setup_dimensions();
    int validate_document(const rapidjson::Document& doc);
    void signal_value(const std::vector<TypeDescr>::value_type& id,
                      const rapidjson::GenericValue<rapidjson::UTF8<>>& generic_value, const std::vector<index_type>& dummy);
    void signal_data(const rapidjson::Document& doc);
};
}}}
#endif
