#ifndef GPM_BASE_INPUT_READER_H
#define GPM_BASE_INPUT_READER_H

#include "gpm_program_options.h"
#include "gpm_logger.h"
#include "ssmstc.h"
#include "mapstc.h"
#include "sedstc.h"
#include "acf_base_reader.h"
#include "acf_base_writer.h"
#include "gpm_model_definition.h"

namespace Slb { namespace Exploration { namespace Gpm {
class gpm_base_input_reader {
public:
    gpm_base_input_reader() = default;
    gpm_base_input_reader(const gpm_base_input_reader& rhs) = delete;
    gpm_base_input_reader& operator=(const gpm_base_input_reader& rhs) = delete;
    int initialize(const program_option_params& params, const Tools::gpm_logger& logger);
    struct parc c; // control file */
    struct parm p; // map file
    acf_base_reader::parameter_num_type inumr;
    acf_base_writer::array_dim_size_type idimr;
	gpm_model_definition model_def;
    struct pars s; // Sed file
    std::map<std::string, int> sediment_id_index;
    std::map<std::string, std::pair<std::string, int>> sediment_id_parm_index;
    std::shared_ptr<gpm_1d_interpolator> sea;
	bool do_output_relative_sealevel_rate = false;
};
}}}
#endif
