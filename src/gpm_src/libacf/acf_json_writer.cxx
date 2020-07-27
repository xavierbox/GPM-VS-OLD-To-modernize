// -- Schlumberger Private --

#include "acf_json_writer.h"
#include "acf_base_helper.h"
#include <iomanip>

namespace Slb { namespace Exploration { namespace Gpm {

acf_json_writer::acf_json_writer() = default;

acf_json_writer::~acf_json_writer() {
    disconnect_slots();
}

void acf_json_writer::write() {
    // Filter out the ones that don't make it
    std::vector<TypeDescr> write_types;
    for (const auto& obj : m_type_info) {
        const int ipar = obj.id;
        /* Check that there are elements to write for this parameter */
		if (m_inumr[ipar] > 0 || acf_base_helper::get_size(m_idimr[ipar]) > 0) {
			write_types.push_back(obj);
		}
    }

    m_errno = writeacf(m_filin, write_types, m_inumr, m_idimr, m_error);
}


int acf_json_writer::writeacf(std::ostream& filou,
                              const std::vector<TypeDescr>& type_info,
                              const parameter_num_type& inumr,
                              const array_dim_size_type& idimr, std::string& strerr) {
    // Writes selected parameters
    // Parameters are selected with inumr (total number not to exceed, to be implemented)
    // and idimr (number in each dimension)

    if (!filou.good()) {
        strerr = "writacf: invalid output file handle\n";
        return -1;
    }

    strerr.clear(); /* initialize to no errors */
    int iret = 0;

    // Write the first items
    filou << "{ \n";
    /* Write */
    int ifirs = 0; // First line not done /* First param has been written */
    const std::size_t npar = type_info.size();
    auto par_count = 0;
    for (const auto& obj : type_info) {
        const int ipar = obj.id;

        /* If first line done, new line */
        if (ifirs) filou << "\n";
        ifirs = 1;
        filou << "\"" << obj.name << "\":";

        std::vector<index_type> indexes;
        key_type loc_id(obj.id);
        indexes.clear();

        if (obj.num_dimensions > 0) {
            filou << "[";
            auto& arr_size = idimr[ipar];
            const int m = arr_size.size() - obj.num_dimensions;
            int_extent_1d first_ext( arr_size[m]);
            for (auto i = first_ext.lower(), num_i = first_ext.upper(); i != num_i; i++) {
                indexes.push_back(i);
                if (obj.num_dimensions > 1) {
                    if (i > 0) filou << ",";
                    filou << "\n    [";
                    int_extent_1d sec_ext(arr_size[m + 1]);
                    for (auto j = sec_ext.lower(), num_j = sec_ext.upper(); j != num_j; j++) {
                        indexes.push_back(j);
                        if (obj.num_dimensions > 2) {
                            if (j > 0) filou << ",";
                            filou << "\n        [";
                            int_extent_1d third_ext(arr_size[m + 2]);
                            for (auto k = third_ext.lower(), num_k = third_ext.upper(); k < num_k; k++) {
                                indexes.push_back(k);
                                add_array_item_to_file(loc_id, indexes, obj.pod_type, filou, k != num_k - 1);
                                indexes.pop_back();
                            }
                            filou << "]";
                        }
                        else {
                            add_array_item_to_file(loc_id, indexes, obj.pod_type, filou, j != num_j - 1);
                        }
                        indexes.pop_back();
                    }
                    if (obj.num_dimensions > 2) filou << "\n    ";
                    filou << "]";
                }
                else {
                    add_array_item_to_file(loc_id, indexes, obj.pod_type, filou, i != num_i - 1);
                }
                indexes.pop_back();
            }
            if (obj.num_dimensions > 1) filou << "\n";
            filou << "]";
        }
        else {
            add_item_to_file(loc_id, obj.pod_type, filou);
        }
        if (par_count < npar - 1) {
            filou << ",";
        }
        ++par_count;
    } // for(ipar=0;ipar<npar;i++)

    // Put CR before end of file
    filou << "\n}\n";
    if (!filou.good()) {
        iret = -1;
        m_error = "writacf: invalid output file state\n";
    }
    else {
        filou.flush();
    }
    return iret;
}


void acf_json_writer::add_array_item_to_file(const key_type& loc_id, const std::vector<index_type>& indexes, ACL_PARSE_POD_TYPES pod_type, std::ostream& filou, bool add_comma) const
{
    if (pod_type == DOUBLE) {
        const int precision = 10;
        const double vald = m_double_array_sig(loc_id, indexes);
        filou << std::setprecision(precision) << vald;
    }
    else if (pod_type == FLOAT) {
        const int precision = 8;
        const float valf = m_float_array_sig(loc_id, indexes);
        filou << std::setprecision(precision) << valf;
    }
    else if (pod_type == INTEGER) {
        const int vali = m_int_array_sig(loc_id, indexes);
        filou << vali;
    }
    else if (pod_type == STRING) {
        const std::string vals = m_string_array_sig(loc_id, indexes);
        filou << "\"" << vals << "\"";
    }
    if (add_comma) {
        filou << ",";
    }
}

void acf_json_writer::add_item_to_file(const key_type& loc_id, ACL_PARSE_POD_TYPES pod_type, std::ostream& filou) const
{
    if (pod_type == DOUBLE) {
        const int precision = 10;
        const double vald = m_double_sig(loc_id);
        filou << std::setprecision(precision) << vald;
    }
    else if (pod_type == FLOAT) {
        const int precision = 8;
        const float valf = m_float_sig(loc_id);
        filou << std::setprecision(precision) << valf;
    }
    else if (pod_type == INTEGER) {
        const int vali = m_int_sig(loc_id);
        filou << vali;
    }
    else if (pod_type == STRING) {
        const std::string vals = m_string_sig(loc_id);
        filou << " \"" << vals << "\"";
    }
}
}}}
