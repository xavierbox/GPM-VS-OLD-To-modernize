// -- Schlumberger Private --

#include "gpm_sediment_file_io.h"
#include "sedstc_descr.h"

#include "gpm_format_helper.h"
#include "gpm_version_number.h"
#include "general_param_file_reader.h"
#include "gpm_numerics.h"
#include "gpm_sediment_file_io_sc.h"
#include "gpm_json_doc_mapper.h"
#include "gpm_sediment_file_io_sc_old.h"
#include "gpm_json_keywords.h"
#include <rapidjson/pointer.h>


namespace Slb { namespace Exploration { namespace Gpm {namespace {

template <class T>
void reshuffle_vectors(std::vector<T>& to_array, const std::vector<T>& from_array, const std::vector<int>& indexes) {
    for (std::size_t i = 0; i < indexes.size(); ++i) {
        to_array[i] = from_array[indexes[i]];
    }
}


typedef std::vector<float>::const_iterator myiter;
typedef std::pair<int, myiter> mypair;

struct my_ordering {
    bool operator()(mypair const& a, mypair const& b) const {
        return (*(a.second) > *(b.second));
    };
};

void sort_diameters(pars* sed_parms_p) {
    auto& m_param(*sed_parms_p);
    // Here we want the diameters to come in decreasing order, and we need to make sure that all the others are shuffled accordingly
    // Not supported in VS 2008, so we'll do it anyway
    //if (!std::is_sorted(m_param.diameter.begin(), m_param.diameter.end(), std::greater_equal<float>())){
    // Check if we have id's
    if (m_param.sed_id.empty()) {
        // Lets generate them for now
        for (size_t i = 0; i < m_param.diameter.size(); ++i) {
            m_param.sed_id.push_back(std::to_string(i));
        }
    }
    sed_parameter_type tmp = m_param;
    std::vector<mypair> order(tmp.diameter.size());
    int n = 0;
    for (myiter it = tmp.diameter.begin(); it != tmp.diameter.end(); ++it , ++n)
        order[n] = std::make_pair(n, it);

    std::sort(order.begin(), order.end(), my_ordering());
    std::vector<int> indexes;
    indexes.reserve(order.size());
    for (auto& i : order) {
        indexes.push_back(i.first);
    }
    // Here we assume that we have same length of diameter and sedname
    reshuffle_vectors(m_param.diameter, tmp.diameter, indexes);
    reshuffle_vectors(m_param.sedname, tmp.sedname, indexes);
    reshuffle_vectors(m_param.sed_id, tmp.sed_id, indexes);
    if (m_param.grainden.size() == indexes.size()) {
        reshuffle_vectors(m_param.grainden, tmp.grainden, indexes);
    }
    if (m_param.fallvel.size() == indexes.size()) {
        reshuffle_vectors(m_param.fallvel, tmp.fallvel, indexes);
    }
    if (m_param.diffcf.size() == indexes.size()) {
        reshuffle_vectors(m_param.diffcf, tmp.diffcf, indexes);
    }
    if (m_param.transp.size() == indexes.size()) {
        reshuffle_vectors(m_param.transp, tmp.transp, indexes);
    }
    if (m_param.por0.size() == indexes.size()) {
        reshuffle_vectors(m_param.por0, tmp.por0, indexes);
    }
    if (m_param.por1.size() == indexes.size()) {
        reshuffle_vectors(m_param.por1, tmp.por1, indexes);
    }
    if (m_param.perm0.size() == indexes.size()) {
        reshuffle_vectors(m_param.perm0, tmp.perm0, indexes);
    }
    if (m_param.perm1.size() == indexes.size()) {
        reshuffle_vectors(m_param.perm1, tmp.perm1, indexes);
    }
    if (m_param.permanis.size() == indexes.size()) {
        reshuffle_vectors(m_param.permanis, tmp.permanis, indexes);
    }
    if (m_param.pfactor.size() == indexes.size()) {
        reshuffle_vectors(m_param.pfactor, tmp.pfactor, indexes);
    }
    if (m_param.pexpon.size() == indexes.size()) {
        reshuffle_vectors(m_param.pexpon, tmp.pexpon, indexes);
    }
    if (m_param.comp.size() == indexes.size()) {
        reshuffle_vectors(m_param.comp, tmp.comp, indexes);
    }
    if (m_param.erode_funcs.size() == indexes.size()) {
        reshuffle_vectors(m_param.erode_funcs, tmp.erode_funcs, indexes);
    }
    if (m_param.erodability.size() == indexes.size()) {
        reshuffle_vectors(m_param.erodability, tmp.erodability, indexes);
    }
    if (m_param.transform_by_transport.size() == indexes.size()) {
        reshuffle_vectors(m_param.transform_by_transport, tmp.transform_by_transport, indexes);
    }
    //} //end of test for sorting
}

std::vector<std::string> get_string_array(const gpm_param_type_mapper& mapper, PARS_MEMBERS id) {
    const auto it = mapper.get_array_ptr<STRING, ARRAY_1d>(id);
    return it != nullptr ? *it : std::vector<std::string>();
}

std::vector<float> get_float_array(const gpm_param_type_mapper& mapper, PARS_MEMBERS id) {
    const auto it = mapper.get_array_ptr<FLOAT, ARRAY_1d>(id);
    return it != nullptr ? *it : std::vector<float>();
}

std::vector<int> get_int_array(const gpm_param_type_mapper& mapper, PARS_MEMBERS id) {
    const auto it = mapper.get_array_ptr<INTEGER, ARRAY_1d>(id);
    return it != nullptr ? *it : std::vector<int>();
}
}

gpm_sediment_file_io::gpm_sediment_file_io(): _parameters_ok(false), _parsed(false), _num_read(0) {
}

gpm_sediment_file_io::~gpm_sediment_file_io() = default;

std::string gpm_sediment_file_io::get_process_name() {
    return "Sediment file io";
}

int gpm_sediment_file_io::read_parameters(const std::string& param_file, const Tools::gpm_logger& logger) {
    _parsed = false;
	gpm_json_doc_mapper mapper(get_process_name(), gpm_sediment_file_io_schema_old, gpm_sediment_file_io_schema);
    const int ret_val = mapper.parse_file(param_file, logger);
	_parsed = ret_val == 0;
    if (_parsed) {
		_old_version = mapper.is_old_format();
		if (_old_version) {
			_mapper.initialize_parameters(pars_descr_holder::get_pars_descr());
			mapper.find_mapping(&_mapper);
		}
		else {
			_sed_holder = get_holder(mapper);
		}
		_parameters_ok = are_parameters_ok(logger);
        return _parameters_ok?0:1;
    }
    return 1;
}

// This is for unit testing really
int gpm_sediment_file_io::read_parameters_from_string(const std::string& param_string, const Tools::gpm_logger& logger) {
    _parsed = false;
	gpm_json_doc_mapper mapper(get_process_name(), gpm_sediment_file_io_schema_old, gpm_sediment_file_io_schema);
	const int ret_val = mapper.parse_text(param_string, logger);
	_parsed = ret_val == 0;
	if (_parsed) {
		_old_version = mapper.is_old_format();
		if (_old_version) {
			_mapper.initialize_parameters(pars_descr_holder::get_pars_descr());
			mapper.find_mapping(&_mapper);
		}
		else {
			_sed_holder = get_holder(mapper);
		}
		_parameters_ok = are_parameters_ok(logger);
		return _parameters_ok ? 0 : 1;
    }
    return 1;
}


bool gpm_sediment_file_io::are_parameters_ok(const Tools::gpm_logger& logger) const {
    // Here we check
    Tools::gpm_version_number found(version());
    if (!found.is_number_correct()) {
        logger.print(Tools::LOG_NORMAL, "SED file file version number wrong\n");
        return false;
    }
	if (_old_version) {
		const Tools::gpm_version_number supported(pars_descr_holder::get_current_version());
		if (!found.version_compatible(supported)) {
			logger.print(Tools::LOG_IMPORTANT, "Sediment file versions not compatible\n");
			return false;
		}
	}
    if (numseds() > 0 && are_sizes_ok()) {
        auto dia = diameter();
        if (are_diameters_consistent(dia)){
            if( is_erodability_props_ok(logger)){
                return true;
            }
            logger.print(Tools::LOG_NORMAL, "Sediment erodability properties are wrong\n");
        }
        else {
            logger.print(Tools::LOG_NORMAL, "Sediment diameters are inconsistent\n");
        }
    }
    return false;
}

bool gpm_sediment_file_io::is_erodability_props_ok(const Tools::gpm_logger& logger) const {
    bool is_erod_ok = true;
	if (_old_version) {
		if (!erodability().empty()) {
			const auto numseds1 = numseds();
			const auto funcs = erodability_functions();
			auto props = erodability_prop();
			is_erod_ok = erodability().size() == props.size() && numseds1 == erodability().size();
			if (is_erod_ok && gpm_format_helper::has_function_indexes(props)) {
				is_erod_ok = gpm_format_helper::check_function_indexes(props, *funcs, "Sediments", logger) == 0;
			}
		}
	}
    return is_erod_ok;
}

void gpm_sediment_file_io::get_parms(pars* sed_parms_p) const {
    auto& sed(*sed_parms_p);
    sed.numseds = numseds();
    sed.version = version();
    sed.sedname = sedname();
    sed.sedcomnt = sediment_comment();
    sed.sed_id = sediment_id();
    sed.diameter = diameter();
    sed.grainden = grainden();
    sed.fallvel = fallvel();
    sed.diffcf = diffcf();
    sed.transp = transp();
    sed.por0 = por0();
    sed.por1 = por1();
    sed.perm0 = perm0();
    sed.perm1 = perm1();
    sed.permanis = permanis();
    sed.pfactor = pfactor();
    sed.pexpon = pexpon();
    sed.comp = comp();
    sed.erodability = erodability();
    // Check if we have them right
    if (!sed.erodability.empty()) {
		if (_old_version) {
			auto props = erodability_prop();
			if (!props.empty()) {
				const auto funcs = erodability_functions();
				sed.erode_funcs = gpm_format_helper::make_functions(props, funcs != nullptr ? *funcs : gpm_vbl_vector_array_3d<float>(), no_val());
			}
			else {
				sed.erode_funcs = std::vector<std::shared_ptr<gpm_1d_interpolator>>(sed.numseds, std::shared_ptr<gpm_1d_interpolator>(new gpm_1d_constant(1.0F)));
			}
		}
		else {
			std::transform(_sed_holder.sediments.begin(), _sed_holder.sediments.end(), std::back_inserter(sed.erode_funcs),
				[](const sediment& a)
				{
                    if (a.erodability_function.has_data()) {
						return std::shared_ptr<gpm_1d_interpolator>(new gpm_1d_linear_array2d(a.erodability_function, -1e12F));
                    }
					return std::shared_ptr<gpm_1d_interpolator>(new gpm_1d_constant(1.0F));
				});
		}
    }
    else {
        sed.erodability = std::vector<float>(sed.numseds, 1.0F);
        sed.erode_funcs = std::vector<std::shared_ptr<gpm_1d_interpolator>>(sed.numseds, std::shared_ptr<gpm_1d_interpolator>(new gpm_1d_constant(1.0F)));
    }
    auto transform = transform_by_transport();
    sed.transform_by_transport = transform.size() == sed.numseds ? transform : std::vector<std::string>(sed.numseds);
    auto an_inplace_transform = do_inplace_transform();
    sed.do_inplace_transform = an_inplace_transform.size() == sed.numseds ? an_inplace_transform : std::vector<int>(sed.numseds);
    auto an_inplace_tranform_coeff = inplace_transform_coeff();
    sed.inplace_transform_coeff = an_inplace_tranform_coeff.size() == sed.numseds ? an_inplace_tranform_coeff: std::vector<float>(sed.numseds);
    sort_diameters(sed_parms_p);
}

std::vector<int> gpm_sediment_file_io::has_missing_indexes() const {
    return _missing_indexes;
}

gpm_sediment_file_io::time_data_holder gpm_sediment_file_io::get_holder(const gpm_json_doc_mapper& mapper) const
{
	 time_data_holder res;
	 const auto parameters = mapper.doc[gpm_json_keywords::PARAMETERS].GetObject();
	 res.version = mapper.doc[gpm_json_keywords::VERSION].GetString();
	 res.schema_version = mapper.doc[gpm_json_keywords::SCHEMA_VERSION].GetString();
	 auto items = parameters["Sediments"].GetArray();
	 for (rapidjson::Value::ConstValueIterator sed_iter = items.Begin(); sed_iter != items.End(); ++sed_iter) {
		 const auto sed_ptr = rapidjson::Pointer(sed_iter->Get<std::string>());
		 const auto sed = sed_ptr.Get(mapper.doc)->GetObject();
		 sediment tmp;
		 tmp.sedname = sed["Name"].Get<std::string>();
         tmp.sediment_id = sed["Id"].Get<std::string>();
		 tmp.diameter = sed["Diameter"].GetFloat();
		 tmp.grainden = sed["Density"].GetFloat();
		 if (sed.HasMember("Transportability")) {
			 tmp.has_transp = true;
			 tmp.transp = sed["Transportability"].GetFloat();
		 }
		 tmp.por0=sed["InitialPorosity"].GetFloat();
		 tmp.por1 = sed["CompactedPorosity"].GetFloat();;
		 tmp.perm0 = sed["InitialPermeability"].GetFloat();
		 tmp.perm1 = sed["CompactedPermeability"].GetFloat();
		 tmp.permanis= sed["PermeabilityAnisotropy"].GetFloat();
		 tmp.comp = sed["Compaction"].GetFloat();
		 tmp.erodability = sed["ErodabilityCoefficient"].GetFloat();
         if (sed.HasMember("ErodabilityFunction")) {
			 const auto erod_func_ptr = rapidjson::Pointer(sed["ErodabilityFunction"].Get<std::string>());
			 const auto erod_func_arr = erod_func_ptr.Get(mapper.doc)->GetObject()[gpm_json_keywords::ARRAY_2D_VALUES].GetArray();
			 gpm_json_doc_mapper::resize_rect_2darray(erod_func_arr, &tmp.erodability_function);
			 gpm_json_doc_mapper::fill_rect_2darray(erod_func_arr, &tmp.erodability_function);
         }
		 //tmp.erodability_function;
         if (sed.HasMember("SedimentTransformation")) {
			 //tmp.transform_by_transport
			 const auto sed_id_ptr = rapidjson::Pointer(sed["SedimentTransformation"].Get<std::string>());
			 const auto sed_id = sed_id_ptr.Get(mapper.doc)->GetObject();
			 tmp.transform_by_transport= sed_id["Id"].Get<std::string>();
		 }
		 tmp.do_inplace_transform = sed["InplaceTransformation"].GetInt();
		 tmp.inplace_transform_coeff = sed["InplaceTransformationCoefficient"].GetFloat();
		 res.sediments.push_back(tmp);
	 }
	 std::sort(res.sediments.begin(), res.sediments.end(), [](const sediment& a, const sediment& b) {return a.diameter > b.diameter; });
	 return res;
}

bool gpm_sediment_file_io::are_sizes_ok() const {
	if (_old_version) {
		const bool por_isOk = (por0().size() == perm0().size()) &&
			(por1().size() == por0().size()) && (perm0().size() == perm1().size());
		const auto numseds1 = numseds();
		const bool sed_is_ok = (numseds1 == por0().size()) && (numseds1 == sedname().size() && numseds1 == diameter().size());
		return por_isOk && sed_is_ok;
	}
	return true;
}

bool gpm_sediment_file_io::are_diameters_consistent(std::vector<float>& diams)//const std::vector<float>& diams) // DT
{
    // Need to check if sizes are right
    bool isOk = true;
    if (diams.empty()) {
        //m_param.diameter=diams;
        isOk = false;
    }
    else {
        isOk = std::all_of(diams.begin(), diams.end(), [](float val){return val > 0.0F;});
    }
    if (isOk) {
        // Clip them
        for (auto& val : diams) {
            val = gpm_clip_value(val, 0.00001f, 15.0f);
        }
    }
        //m_param.transp[i] = (float)(-log(m_param.diameter[i]/20.0)/20.0);
    return isOk;
}

std::vector<std::string> gpm_sediment_file_io::sedname() const {
	if (_old_version) {
		return get_string_array(_mapper, SEDNAME);
	}
	return _sed_holder.make_vector_of_item<std::string>([](const sediment& a) {return a.sedname; });
}

std::vector<std::string> gpm_sediment_file_io::sediment_comment() const {
	if (_old_version) {
		return get_string_array(_mapper, SEDCOMNT);
	}
	return std::vector<std::string>(_sed_holder.sediments.size());
}

std::vector<std::string> gpm_sediment_file_io::sediment_id() const {
	if (_old_version) {
		return get_string_array(_mapper, ID);
	}
	return _sed_holder.make_vector_of_item<std::string>([](const sediment& a) {return a.sediment_id; });
}

std::vector<std::string> gpm_sediment_file_io::erodability_prop() const {
    return get_string_array(_mapper, ERODABILITY_PROP);
}

std::vector<float> gpm_sediment_file_io::diameter() const {
	if (_old_version) {
		return get_float_array(_mapper, DIAMETER);
	}
	return _sed_holder.make_vector_of_item<float>([](const sediment& a) {return a.diameter; });
}

std::vector<float> gpm_sediment_file_io::grainden() const {
	if (_old_version) {
		return get_float_array(_mapper, GRAINDEN);
	}
	return _sed_holder.make_vector_of_item<float>([](const sediment& a) {return a.grainden; });
}

std::vector<float> gpm_sediment_file_io::fallvel() const {
	if (_old_version) {
		return get_float_array(_mapper, FALLVEL);
	}
	return _sed_holder.make_empty_vector<float>();
}

std::vector<float> gpm_sediment_file_io::diffcf() const {
	if (_old_version) {
		return get_float_array(_mapper, DIFFCF);
	}
	return _sed_holder.make_empty_vector<float>();
}

std::vector<float> gpm_sediment_file_io::transp() const {
	if (_old_version) {
		return get_float_array(_mapper, TRANSP);
	}
	return _sed_holder.make_transportability();
}

std::vector<float> gpm_sediment_file_io::por0() const {
	if (_old_version) {
		return get_float_array(_mapper, POR0);
	}
	return _sed_holder.make_vector_of_item<float>([](const sediment& a) {return a.por0; });
}

std::vector<float> gpm_sediment_file_io::por1() const {
	if (_old_version) {
		return get_float_array(_mapper, POR1);
	}
	return _sed_holder.make_vector_of_item<float>([](const sediment& a) {return a.por1; });

}

std::vector<float> gpm_sediment_file_io::perm0() const {
	if (_old_version) {
		return get_float_array(_mapper, PERM0);
	}
	return _sed_holder.make_vector_of_item<float>([](const sediment& a) {return a.perm0; });

}

std::vector<float> gpm_sediment_file_io::perm1() const {
	if (_old_version) {
		return get_float_array(_mapper, PERM1);
	}
	return _sed_holder.make_vector_of_item<float>([](const sediment& a) {return a.perm1; });
}

std::vector<float> gpm_sediment_file_io::permanis() const {
	if (_old_version) {
		return get_float_array(_mapper, PERMANIS);
	}
	return _sed_holder.make_vector_of_item<float>([](const sediment& a) {return a.permanis; });
}

std::vector<float> gpm_sediment_file_io::pfactor() const {
	if (_old_version) {
		return get_float_array(_mapper, PFACTOR);
	}
	return _sed_holder.make_empty_vector<float>();
}

std::vector<float> gpm_sediment_file_io::pexpon() const {
	if (_old_version) {
		return get_float_array(_mapper, PEXPON);
	}
	return _sed_holder.make_empty_vector<float>();
}

std::vector<float> gpm_sediment_file_io::comp() const {
	if (_old_version) {
		return get_float_array(_mapper, COMP);
	}
	return _sed_holder.make_vector_of_item<float>([](const sediment& a) {return a.comp; });
}

std::vector<float> gpm_sediment_file_io::erodability() const {
	if (_old_version) {
		return get_float_array(_mapper, ERODABILITY_COEFF);
	}
	return _sed_holder.make_vector_of_item<float>([](const sediment& a) {return a.erodability; });
}

std::shared_ptr<gpm_vbl_vector_array_3d<float>> gpm_sediment_file_io::erodability_functions() const {
    return _mapper.get_array_ptr<FLOAT, ARRAY_3d>(ERODABILITY_FUNC);
}

std::vector<std::string> gpm_sediment_file_io::transform_by_transport() const {
	if (_old_version) {
		return get_string_array(_mapper, TRANSFORM_BY_TRANSPORT);
	}
	return _sed_holder.make_vector_of_item<std::string>([](const sediment& a) {return a.transform_by_transport; });
}
std::vector<int> gpm_sediment_file_io::do_inplace_transform() const {
	if (_old_version) {
		return get_int_array(_mapper, DO_INPLACE_TRANSFORM);
	}
	return _sed_holder.make_vector_of_item<int>([](const sediment& a) {return a.do_inplace_transform; });
}

std::vector<float> gpm_sediment_file_io::inplace_transform_coeff() const {
	if (_old_version) {
		return get_float_array(_mapper, INPLACE_TRANSFORM_COEFF);
	}
	return _sed_holder.make_vector_of_item<float>([](const sediment& a) {return a.inplace_transform_coeff; });
}


int gpm_sediment_file_io::numseds() const
{
	if (_old_version) {
		return _mapper.get_value<INTEGER>(NUMSEDSS, 0);
	}
	return _sed_holder.sediments.size();
}
std::string gpm_sediment_file_io::version() const
{
	if (_old_version) {
		return _mapper.get_value<STRING>(PARS_VERSION, "1.0.0.0");
	}
	return _sed_holder.version;
}
float gpm_sediment_file_io::no_val() const
{
	if (_old_version) {
		return _mapper.get_value<FLOAT>(NO_VAL, FLOAT_IO_NO_VALUE);
	}
	return -1e12F;
}


}}}
