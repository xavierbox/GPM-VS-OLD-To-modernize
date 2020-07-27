// -- Schlumberger Private --

#include "acf_base_ascii_writer.h"

namespace Slb { namespace Exploration { namespace Gpm {

acf_base_ascii_writer::acf_base_ascii_writer() = default;

acf_base_ascii_writer::~acf_base_ascii_writer() {
    disconnect_slots();
}

bool acf_base_ascii_writer::open_file(const std::string& file_name, bool create_new)
{
	m_filin.open(file_name.c_str(), std::ios_base::out);

	if (!m_filin.good()) {
		m_error = "writacf: invalid output file state\n";
		m_errno = FILE_NOT_OPENED;
	}
	return m_filin.good();

}
void acf_base_ascii_writer::set_write_info(const std::vector<TypeDescr>& type_info,
                                     const parameter_num_type& inumr,
                                     const array_dim_size_type& idimr) {
    m_type_info = type_info;
    m_inumr = inumr;
    m_idimr = idimr;
}

void acf_base_ascii_writer::close_file() {
    m_filin.close();
}

}}}
