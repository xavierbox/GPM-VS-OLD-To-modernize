// -- Schlumberger Private --

#ifndef acf_json_writer_h
#define acf_json_writer_h

// A wrapper for the acf writer currently used, but where we use signals to call back to the struct

#include "acf_base_ascii_writer.h"

namespace Slb { namespace Exploration { namespace Gpm {

class acf_json_writer:public acf_base_ascii_writer {
public:
    // Perhaps we should do it all by array?

    acf_json_writer();
    ~acf_json_writer();
    void write() override;

private:
    int writeacf(std::ostream& filou,
                 const std::vector<TypeDescr>& type_info, const parameter_num_type& inumr, const array_dim_size_type& idimr, std::string& strerr);
    void add_array_item_to_file(const key_type& loc_id, const std::vector<index_type>& indexes, ACL_PARSE_POD_TYPES pod_type,
                                std::ostream& filou, bool add_comma) const;
    void add_item_to_file(const key_type& loc_id, ACL_PARSE_POD_TYPES pod_type, std::ostream& filou) const;

};

}}}
#endif
