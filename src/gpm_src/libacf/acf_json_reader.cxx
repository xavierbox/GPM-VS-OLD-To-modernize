// -- Schlumberger Private --

#include "acf_json_reader.h"
#include "rapidjson/reader.h"
#include "rapidjson/error/en.h"
#include "rapidjson/istreamwrapper.h"
#include <iostream>
#include <fstream>
#include <streambuf>

namespace Slb { namespace Exploration { namespace Gpm {


// We need to keep the key value pairs
// We will have the start object first

struct MyHandler {
	typedef acf_base_reader::index_type index_type;
    MyHandler(): type_index(-1) {
    }

    std::vector<index_type> array_indexes;
    std::vector<TypeDescr> all_types;
    int type_index; // If -1, skip parsing until new key shows up
    std::function<void(const acf_base_reader::key_type& key_name, float val, const std::vector<index_type>& indexes)> signal_float;
    std::function<void(const acf_base_reader::key_type& key_name, double val, const std::vector<index_type>& indexes)> signal_double;
    std::function<void(const acf_base_reader::key_type& key_name, int val, const std::vector<index_type>& indexes)> signal_int;
    std::function<void(const acf_base_reader::key_type& key_name, const std::string& val, const std::vector<index_type>& indexes)> signal_string;

    bool has_key() const {
        return type_index != -1;
    }

    bool is_array() const {
        return !array_indexes.empty();
    }

    void increment_last_index() {
        if (!array_indexes.empty()) {
            ++array_indexes.back();
        }
    }

    template <class T>
    void signal_number(const TypeDescr& item, T numb) {
        if (item.pod_type == FLOAT) {
            signal_float(item.id, static_cast<TypeDescr::Pod_Promotion<FLOAT>::pod_type>(numb), array_indexes);
        }
        if (item.pod_type == DOUBLE) {
            signal_double(item.id, static_cast<TypeDescr::Pod_Promotion<DOUBLE>::pod_type>(numb), array_indexes);
        }
        if (item.pod_type == INTEGER) {
            signal_int(item.id, static_cast<TypeDescr::Pod_Promotion<INTEGER>::pod_type>(numb), array_indexes);
        }
        increment_last_index();
    }

    bool Null() {
        return true;
    }

    bool Bool(bool b) {
        return true;
    }

    bool Int(int i) {
        if (has_key()) {
            signal_number(all_types[type_index], i);
        }
        return true;
    }

    bool Uint(unsigned u) {
        if (has_key()) {
            signal_number(all_types[type_index], u);
        }
        return true;
    }

    bool Int64(int64_t i) {
        if (has_key()) {
            signal_number(all_types[type_index], i);
        }
        return true;
    }

    bool Uint64(uint64_t u) {
        if (has_key()) {
            signal_number(all_types[type_index], u);
        }
        return true;
    }

    bool Double(double d) {
        if (has_key()) {
            signal_number(all_types[type_index], d);
        }
        return true;
    }

    /// enabled via kParseNumbersAsStringsFlag, string is not null-terminated (use length)
    bool RawNumber(const char* str, rapidjson::SizeType length, bool copy) {
        // No op for us
        return true;
    }

    bool String(const char* str, rapidjson::SizeType length, bool copy) {
        if (has_key()) {
            std::string res;
            res.assign(str, length);
            signal_string(all_types[type_index].id, res, array_indexes);
            increment_last_index();
        }
        return true;
    }

    bool StartObject() {
        return true;
    }

    bool Key(const char* str, rapidjson::SizeType length, bool copy) {
        std::string res;
        res.assign(str, length);
        type_index = -1;
        array_indexes.clear();
        for (auto i = 0; i < all_types.size(); ++i) {
            if (all_types[i].name == res) {
                type_index = i;
                break;
            }
        }
        return true;
    }

    bool EndObject(rapidjson::SizeType memberCount) {
        return true;
    }

    bool StartArray() {
        if (has_key()) {
            array_indexes.push_back(0);
        }
        return true;
    }

    bool EndArray(rapidjson::SizeType elementCount) {
        if (has_key()) {
            array_indexes.pop_back();
            increment_last_index();
        }
        return true;
    }
};

acf_json_reader::acf_json_reader() = default;

acf_json_reader::~acf_json_reader() = default;

bool acf_json_reader::parse_text(const std::string& text_to_parse) {
    MyHandler handler;
    handler.all_types = type_descriptors();
    handler.signal_double = std::bind(&acf_json_reader::signal_double_and_count, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    handler.signal_float = std::bind(&acf_json_reader::signal_float_and_count, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    handler.signal_int = std::bind(&acf_json_reader::signal_integer_and_count, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    handler.signal_string = std::bind(&acf_json_reader::signal_string_and_count, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    rapidjson::Reader reader;
    rapidjson::StringStream ss(text_to_parse.c_str());
    if (reader.Parse(ss, handler)) {
        m_errno = 0;
        return true;
    }
    const rapidjson::ParseErrorCode e = reader.GetParseErrorCode();
    const size_t o = reader.GetErrorOffset();
    m_errno = e;
    m_error = std::string(rapidjson::GetParseError_En(e)) + " at offset " + std::to_string(o);
    return false;
}

void acf_json_reader::setup_dimensions()
{
	std::vector<TypeDescr> arrays;
	std::copy_if(m_type_info.begin(), m_type_info.end(), std::back_inserter(arrays), [](const TypeDescr& val) {return val.num_dimensions > 0; });
	auto keys = m_counter.arrays_found();
    for(auto id:keys) {
		m_idimr[id].fill(1);
        auto ranges = dimensions(id);
		std::vector<int> dims;
		std::transform(ranges.begin(), ranges.end(), std::back_inserter(dims), [](const std::pair<int, int>& val) {return val.second; });
		std::copy(dims.rbegin(), dims.rend(), m_idimr[id].rbegin());
    }
}

bool acf_json_reader::parse_file(const std::string& file) {
    MyHandler handler;
    handler.all_types = type_descriptors();
    handler.signal_double = std::bind(&acf_json_reader::signal_double_and_count, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    handler.signal_float = std::bind(&acf_json_reader::signal_float_and_count, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    handler.signal_int = std::bind(&acf_json_reader::signal_integer_and_count, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    handler.signal_string = std::bind(&acf_json_reader::signal_string_and_count, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    rapidjson::Reader reader;
    std::ifstream t(file, std::ios_base::in);
    if (t.good()) {
        rapidjson::IStreamWrapper is(t);
        if (reader.Parse(is, handler)) {
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

void acf_json_reader::signal_double_and_count(const key_type& key_name, double val, const std::vector<index_type>& indexes) {
    signal_double(key_name, val, indexes);
    ++m_inumr[key_name];
}

void acf_json_reader::signal_float_and_count(const key_type& key_name, float val, const std::vector<index_type>& indexes) {
    signal_float(key_name, val, indexes);
    ++m_inumr[key_name];
}

void acf_json_reader::signal_integer_and_count(const key_type& key_name, int val, const std::vector<index_type>& indexes) {
    signal_integer(key_name, val, indexes);
    ++m_inumr[key_name];
}

void acf_json_reader::signal_string_and_count(const key_type& key_name, const std::string& val, const std::vector<index_type>& indexes) {
    signal_string(key_name, val, indexes);
    ++m_inumr[key_name];
}
}}}
