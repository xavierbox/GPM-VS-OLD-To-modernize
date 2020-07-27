// -- Schlumberger Private --

#include "gpm_hdf5_writer.h"
#include "gpm_hdf5_writer_impl.h"
#include "acf_base_writer.h"
#include <algorithm>
#include "acf_base_helper.h"

namespace Slb { namespace Exploration { namespace Gpm {
gpm_hdf5_writer::gpm_hdf5_writer()
{
    m_impl.reset(new gpm_hdf5_writer_impl());
}

gpm_hdf5_writer::~gpm_hdf5_writer()
= default;

bool gpm_hdf5_writer::check_file_access(const std::string& file_name, bool create_new)
{
    return m_impl->open_file(file_name, create_new) && m_impl->close_file();
}

bool gpm_hdf5_writer::open_file(const std::string& file_name, bool create_new)
{
	m_file_name = file_name;
	const auto file_ok = check_file_access(m_file_name, create_new);
	if (!file_ok) {
		m_error = "gpm_hdf5_writer: invalid output file state\n";
		m_errno = FILE_NOT_OPENED;
	}

	return file_ok;
}
void gpm_hdf5_writer::set_write_info(const std::vector<TypeDescr>& type_info,
                                     const parameter_num_type& inumr, const array_dim_size_type& idimr)
{
    m_type_info = type_info;
    m_inumr = inumr;
    m_idimr = idimr;

    set_signals_to_impl();
}

void gpm_hdf5_writer::set_signals_to_impl()
{
    signals signals{};
    signals.string_sig = &m_string_sig;
    signals.int_sig = &m_int_sig;
    signals.float_sig = &m_float_sig;
    signals.double_sig = &m_double_sig;
    signals.string_array_sig = &m_string_array_sig;
    signals.int_array_sig = &m_int_array_sig;
    signals.float_array_sig = &m_float_array_sig;
    signals.double_array_sig = &m_double_array_sig;
    m_impl->set_signals(&signals);
}

void gpm_hdf5_writer::write()
{
    if (!m_impl->open_file(m_file_name, true)) {
        m_errno = FILE_NOT_OPENED;
        return;
    }

    writeacf(m_type_info, m_inumr, m_idimr);

    if (!m_impl->close_file()) {
        m_errno = FILE_NOT_CLOSED;
    }
}

void gpm_hdf5_writer::append(const std::vector<TypeDescr>& type_info, const parameter_num_type& inumr,
                             const array_dim_size_type& idimr)
{
    int status = m_impl->open_file(m_file_name, false);
    if (status != 1) {
        m_errno = FILE_NOT_OPENED;
        return;
    }

    m_type_info = type_info;
    m_inumr = inumr;
    m_idimr = idimr;
    m_errno = writeacf(m_type_info, m_inumr, m_idimr);

    status = m_impl->close_file();
    if (status < 0) {
        m_errno = FILE_NOT_CLOSED;
        return;
    }
}

void gpm_hdf5_writer::close_file()
{
	m_impl->close_file();
}

int gpm_hdf5_writer::writeacf(const std::vector<TypeDescr>& type_info, const parameter_num_type& inumr,
                              const array_dim_size_type& idimr)
{
    int iret = 0;
    try {
        std::vector<TypeDescr> filtered;
        std::copy_if(type_info.begin(), type_info.end(), std::back_inserter(filtered),
                     [&idimr, &inumr](const TypeDescr& item_desc)
                     {
                         io_array_dim_type item_data = idimr[item_desc.id];
                         const bool valid = inumr[item_desc.id] > 0 || acf_base_helper::get_size(item_data) > 0;
                         return valid;
                     }
        );
        std::sort(filtered.begin(), filtered.end(), [](const TypeDescr& a, const TypeDescr& b)
        {
            return a.num_dimensions < b.num_dimensions;
        });

        for (const auto& item_desc : filtered) {
            const io_array_dim_type item_data = idimr[item_desc.id];
            m_impl->write(item_desc, item_data);
        }
    }
    catch (H5::Exception& err) {
		m_error = err.getDetailMsg();
		m_errno = ERROR;
        iret = ERROR;
    }

    return iret;
}
}}}
