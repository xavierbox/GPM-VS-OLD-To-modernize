// -- Schlumberger Private --

#ifndef acf_json_reader_h
#define acf_json_reader_h

// A wrapper for the acf parser currently used, but where we use signals to call back to the struct
#include "acf_base_reader.h"


namespace Slb { namespace Exploration { namespace Gpm{

class acf_json_reader: public acf_base_reader
{
public:
	typedef acf_base_reader base;
	acf_json_reader();
	~acf_json_reader();
	
	// Returns the state of the stream
	bool parse_text(const std::string& text_to_parse) override;
 	bool parse_file(const std::string& file) override;
private:
	typedef acf_base_reader::index_type index_type;
	typedef acf_base_reader::size_type size_type;

	void setup_dimensions();
	void signal_double_and_count(const key_type& key_name, double val, const std::vector<index_type>& indexes);
	void signal_float_and_count(const key_type& key_name, float val, const std::vector<index_type>& indexes);
	void signal_integer_and_count(const key_type& key_name, int val, const std::vector<index_type>& indexes);
	void signal_string_and_count(const key_type& key_name, const std::string& val, const std::vector<index_type>& indexes);

};
}}}
#endif
