// -- Schlumberger Private --

#ifndef ACF_FORMAT_SELECTOR_H
#define ACF_FORMAT_SELECTOR_H


#include "acf_base_reader.h"
#include "acf_base_writer.h"
#include <string>
#include <memory>


namespace Slb { namespace Exploration { namespace Gpm {
class acf_format {
public:
    enum class Formats {JSON=1, HDF5=2};

    acf_format(Formats from, Formats to);
	acf_format(const acf_format&) = delete;
	acf_format& operator=(const acf_format&) = delete;
	static std::shared_ptr<acf_base_reader> find_reader(Formats format_type);
    static std::shared_ptr<acf_base_writer> find_writer(Formats format_type);
    static std::shared_ptr<acf_base_reader> find_reader_according_to_suffix(const std::string& file_name);

    static std::shared_ptr<acf_base_writer> find_writer_according_to_suffix(const std::string& file_name);

    static Formats find_format_according_to_suffix(const std::string& file_name);

    void setup_reader_writer(Formats from, Formats to);
    void setup_format(const std::vector<TypeDescr>& type_info);
    void convert(const std::string& from_file, const std::string& to_file);
private:
    std::unique_ptr<acf_base_reader> read_acf;
    std::unique_ptr<acf_base_writer> write_acf;
};

}}}
#endif
