// -- Schlumberger Private --

#ifndef gpm_sediment_file_io_h
#define gpm_sediment_file_io_h

#include "sedstc.h"
#include "gpm_param_type_mapper.h"
#include "gpm_json_doc_mapper.h"

#include "gpm_logger.h"
#include <vector>

namespace Slb { namespace Exploration { namespace Gpm {

class gpm_sediment_file_io {
public:
    gpm_sediment_file_io();
    virtual ~gpm_sediment_file_io();
    static std::string get_process_name();
    int read_parameters(const std::string& param_file, const Tools::gpm_logger& logger);
    bool are_parameters_ok(const Tools::gpm_logger& logger) const;
    bool is_erodability_props_ok(const Tools::gpm_logger& logger) const;
    int read_parameters_from_string(const std::string& param_string, const Tools::gpm_logger& logger);
    void get_parms(pars* sed_parms_p) const;
    std::vector<int> has_missing_indexes() const;
private:
    struct sediment {
		std::string sedname;
		std::string sediment_id;
		float diameter{};
		float grainden{};
		bool has_transp{};
		float transp{};
		float por0{};
		float por1{};
		float perm0{};
		float perm1{};
		float permanis{};
		float comp{};
		float erodability{};
		float_2darray_type erodability_function;
		//std::shared_ptr<gpm_vbl_vector_array_3d<float>> erodability_functions() const;
		//std::vector<std::string> transform_by_transport() const;
		std::string transform_by_transport; // id to transform to
		int do_inplace_transform{};
		float inplace_transform_coeff{};
    };
	struct time_data_holder
	{
		std::string version;
		std::string schema_version;
		std::vector< sediment> sediments;
        template <typename T> std::vector<T> make_vector_of_item(std::function<T(const sediment& a)> func) const
        {
			std::vector<T> res;
			std::transform(sediments.begin(), sediments.end(), std::back_inserter(res), func);
			return res;
        }
		template <typename T> std::vector<T> make_empty_vector() const
		{
			std::vector<T> res(sediments.size());
			return res;
		}
        std::vector<float> make_transportability() const
        {
			std::vector<float> res;
            if (std::any_of(sediments.begin(), sediments.end(), [](const sediment& in){return in.has_transp;})) {				
				std::transform(sediments.begin(), sediments.end(), std::back_inserter(res), 
					[](const sediment& in) {return in.has_transp ? in.transp : 0.0F; });
				return res;
            }
			return res;
        }
	};
	time_data_holder get_holder(const gpm_json_doc_mapper& mapper) const;
	bool are_sizes_ok() const;
    static bool are_diameters_consistent(std::vector<float>& diams);
    std::vector<std::string> sedname() const;
    std::vector<std::string> sediment_comment() const;
    std::vector<std::string> sediment_id() const;
    std::vector<float> diameter() const;
    std::vector<float> grainden() const;
    std::vector<float> fallvel() const;
    std::vector<float> diffcf() const;
    std::vector<float> transp() const;
    std::vector<float> por0() const;
    std::vector<float> por1() const;
    std::vector<float> perm0() const;
    std::vector<float> perm1() const;
    std::vector<float> permanis() const;
    std::vector<float> pfactor() const;
    std::vector<float> pexpon() const;
    std::vector<float> comp() const;
    std::vector<std::string> erodability_prop() const;
    std::vector<float> erodability() const;
    std::shared_ptr<gpm_vbl_vector_array_3d<float>> erodability_functions() const;
    std::vector<std::string> transform_by_transport() const;
    std::vector<int> do_inplace_transform() const;
    std::vector<float> inplace_transform_coeff() const;
    int numseds() const;
    std::string version() const;
    float no_val() const;

    bool _parameters_ok;
    bool _parsed;
    int _num_read;
    std::vector<int> _missing_indexes;
	gpm_param_type_mapper _mapper;
    bool _old_version{};
    time_data_holder _sed_holder;
};

}}}
#endif
