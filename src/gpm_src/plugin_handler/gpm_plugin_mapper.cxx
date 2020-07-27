
#include "gpm_plugin_mapper.h"
#include "gpm_model_definition.h"
#include "gpm_function_2d.h"
#include <vector>

namespace Slb { namespace Exploration { namespace Gpm {
namespace {
Tools::LogLevel get_log_level(int log_level)
{
    auto level = static_cast<gpm_plugin_api_loglevel>(log_level);
    auto res = Tools::LogLevel::LOG_NORMAL;
    switch (level) {
    case gpm_plugin_api_log_important:
        res = Tools::LogLevel::LOG_IMPORTANT;
        break;
    case gpm_plugin_api_log_normal:
        break;
    case gpm_plugin_api_log_verbose:
        res = Tools::LogLevel::LOG_VERBOSE;
        break;
    case gpm_plugin_api_log_trace:
        res = Tools::LogLevel::LOG_TRACE;
        break;
    default: ;
    }
	return res;
}
#if defined(USE_ARMA)
gpm_plugin_api_3d_memory_layout
get_memory_layout(const sediment_layer_base_type& arr)
{
	gpm_plugin_api_3d_memory_layout res{};
	res.num_rows = arr.n_rows();
	res.num_cols = arr.n_cols();
	res.num_samples = arr.n_depth();
	res.sample_stride = 1;
	res.col_stride = arr.n_depth();
	res.row_stride = arr.n_depth()*arr.n_cols();
	return res;
}
#else
gpm_plugin_api_3d_memory_layout
get_memory_layout(const sediment_layer_base_type& arr)
{
	gpm_plugin_api_3d_memory_layout res{};
	res.num_rows = arr.layers();
	res.num_cols = arr.rows();
	res.num_samples = arr.cols();
	res.sample_stride = 1;
	res.col_stride = arr.cols();
	res.row_stride = arr.rows()*arr.cols();
	return res;
}
#endif

gpm_plugin_api_2d_memory_layout
get_memory_layout(const float_2darray_base_type& arr)
{
	gpm_plugin_api_2d_memory_layout res{};
	res.num_rows = arr.rows();
	res.num_cols = arr.cols();
	res.col_stride = 1;
	res.row_stride = arr.cols();
	return res;
}

gpm_plugin_api_2d_memory_layout
get_memory_layout(const int_extent_2d& exts)
{
	gpm_plugin_api_2d_memory_layout res{};
	res.num_rows = exts.row_extent().size();
	res.num_cols = exts.col_extent().size();
	res.col_stride = 1;
	res.row_stride = res.num_cols;
	return res;
}

gpm_plugin_api_2d_memory_layout
get_memory_layout_for_constant(const int_extent_2d& exts)
{
	gpm_plugin_api_2d_memory_layout res{};
	res.num_rows = exts.row_extent().size();
	res.num_cols = exts.col_extent().size();
	res.col_stride = 0;
	res.row_stride = 0;
	return res;
}


gpm_plugin_api_message_definition
get_message_definition(std::vector<char>* vec)
{
	gpm_plugin_api_message_definition def{};
	def.message = vec->data();
	def.message_array_length = vec->size();
	def.message_length = 0;
	return def;
}
void print_message(const gpm_plugin_api_message_definition& def, const Tools::gpm_logger& logger)
{
	if (def.message_length > 0 && def.log_level != gpm_plugin_api_log_none) {
		std::string mess(def.message, def.message + def.message_length);
		logger.print(get_log_level(def.log_level), "%s", mess.c_str());
	}
}
}
gpm_plugin_mapper::gpm_plugin_mapper(const std::shared_ptr<ppm_plugin_holder>& holder):_holder(holder)
{
    _plugin_handle= _holder->_factory(); // Create the plugin
}

gpm_plugin_mapper::~gpm_plugin_mapper()
{
	_holder->_deleter(_plugin_handle);
}
std::string gpm_plugin_mapper::id()
{
	return _holder->id;
}

bool gpm_plugin_mapper::has_valid_handle()
{
	return _plugin_handle != nullptr;
}

void gpm_plugin_mapper::set_model_extents(const gpm_model_definition& in_model)
{
	gpm_plugin_api_model_definition model{};
	model.num_rows = in_model.model_extent().row_extent().size();
	model.num_columns = in_model.model_extent().col_extent().size();;
	std::vector<float> x = in_model.get_x_coordinates();
	std::vector<float> y = in_model.get_y_coordinates();
	std::copy(x.begin(), x.end(), model.x_coordinates);
	std::copy(y.begin(), y.end(), model.y_coordinates);
	_holder->_set_model_extents_func(_plugin_handle, &model);
	_extents = in_model.model_extent();
}
void gpm_plugin_mapper::set_current_install_dir(const std::string& curr_dir)
{
	if (_holder->_current_install_dir_func) {
		_holder->_current_install_dir_func(_plugin_handle, curr_dir.c_str(), curr_dir.size());
	}
}

int gpm_plugin_mapper::get_multiples_of_timestep(double display_time) const
{
	int ret = _holder->_process_multiple_of_timestep_func(_plugin_handle, display_time);
	return ret;

}

int gpm_plugin_mapper::read_parameters(const std::string& file_name, const Tools::gpm_logger& logger)
{
	std::vector<char> message_holder(6000);
	gpm_plugin_api_message_definition def = get_message_definition(&message_holder);
	int ret = _holder->_read_parameters_func(_plugin_handle, file_name.c_str(), file_name.size(), &def);
	print_message(def, logger);
	return ret;
}

int gpm_plugin_mapper::read_needed_properties(const std::vector<std::string>& prop_ids, const Tools::gpm_logger& logger)
{
	std::vector<gpm_plugin_api_string_layout> holder(prop_ids.size());
	std::vector<int> use_attr(prop_ids.size());
    for(auto i = 0; i < prop_ids.size();++i) {
		holder[i].str = const_cast<char*>(prop_ids[i].data());
		holder[i].str_length = prop_ids[i].size();
    }
	std::vector<char> message_holder(6000);
	gpm_plugin_api_message_definition def = get_message_definition(&message_holder);
	int ret = _holder->_get_needed_model_attributes_func(_plugin_handle, prop_ids.size(), holder.data(), use_attr.data(), &def);
    if (ret == 0) {
        for(auto i =0UL; i < prop_ids.size();++i) {
            if (use_attr[i]) {
				_model_properties.push_back(prop_ids[i]);
            }
        }
        // Get the write attributes
        const int num_attributes = _holder->_get_write_model_attribute_num_func(_plugin_handle);
        if (num_attributes > 0 ) {
			std::vector<int> attr_sizes(num_attributes);
			_holder->_get_write_model_attribute_sizes_func(_plugin_handle, attr_sizes.data(), num_attributes);
			std::vector<std::string> attribs(num_attributes);
			std::vector<gpm_plugin_api_string_layout> write_attrib(num_attributes);
			std::vector<int> top_layer_only(num_attributes);
            for(int j = 0; j < num_attributes;++j) {
				attribs[j].resize(attr_sizes[j]);
				write_attrib[j].str = const_cast<char*>(attribs[j].data());
				write_attrib[j].str_length = attribs[j].size();
            }
			_holder->_get_write_model_attributes_func(_plugin_handle, write_attrib.data(), top_layer_only.data(), num_attributes);
			std::vector<attribute_type> out_types;
            std::transform(attribs.begin(), attribs.end(), top_layer_only.begin(), std::back_inserter(out_types),
                           [](const std::string& item, int top_only)
                           {
                               return attribute_type{item, top_only == 0 ? false : true};
                           });
            _write_properties = out_types;
        }
    }
	print_message(def, logger);
	return ret;
}



void gpm_plugin_mapper::set_sediments(const std::vector< sediment_props>& in_seds) const
{
	std::vector<gpm_plugin_api_sediment_definition> seds(in_seds.size());
	for (auto i = 0u; i < in_seds.size(); ++i) {
		seds[i].id = in_seds[i].id.c_str();
		seds[i].id_length = in_seds[i].id.size() + 1;
		seds[i].name = in_seds[i].name.c_str();
		seds[i].name_length = in_seds[i].name.size() + 1;
		seds[i].index_in_sed_array = in_seds[i].sed_index;
	}
	_holder->_set_sediment_func(_plugin_handle, seds.data(), seds.size());
}

void gpm_plugin_mapper::initialize_display_time(double time)
{
	_holder->_initialize_display_func(_plugin_handle, time);
}

int gpm_plugin_mapper::process_sediments(double time_prev, double time_now, const float_2darray_base_type& top,
                                         float sea_level, sediment_layer_base_type* seds_p,
                                         const Tools::gpm_logger& logger) const
{
	gpm_plugin_api_process_with_top_sediment_sea_parms parms{};
	parms.time.start = time_prev;
	parms.time.end = time_now;
	parms.top = top.begin();
	parms.top_layout = get_memory_layout(top);
	parms.sealevel = &sea_level;
	parms.sealevel_layout = get_memory_layout_for_constant(top.extents());
	parms.sediments = seds_p->data();
	parms.sediment_layout = get_memory_layout(*seds_p);
	std::vector<char> message_holder(6000);
	parms.error = get_message_definition(&message_holder);
	(void) _holder->_process_top_sediment_sea_timestep_func(_plugin_handle, &parms);
	print_message(parms.error, logger);
    return 0;
}

int gpm_plugin_mapper::process_model_timestep(double time_prev, double time_now,
                                              const std::vector<attribute_const_descriptor>& arrs,
                                              const Tools::gpm_logger& logger) const
{
    // Start packing
	std::vector<std::vector<float*>> attr_holders;
	std::vector<std::vector<uint8_t>> attr_type;
	std::vector<size_t> attr_nums;
	std::forward_list<float> vals;

    for(auto& attr:arrs) {
		std::vector<float*> tmp1;
		std::vector<uint8_t> is_const;
		attr_nums.push_back(attr.funcs.size());
        for(const auto& func_2d : attr.funcs) {
            if (func_2d.is_constant()) {
                // Need to map to a constant
				vals.push_front(func_2d.constant_value());
				tmp1.push_back(&(*vals.begin()));
				is_const.push_back(1);
            }
			else {
				tmp1.push_back(func_2d.array_holder().begin());
				is_const.push_back(0);
			}
        }
		attr_holders.push_back(tmp1);
		attr_type.push_back(is_const);
    }
    // All set up, map them in
	std::vector<float**> holders;
	std::transform(attr_holders.begin(), attr_holders.end(), std::back_inserter(holders), [](std::vector<float*>& data) {return data.data(); });
	std::vector<uint8_t*> constant_holder;
	std::transform(attr_type.begin(), attr_type.end(), std::back_inserter(constant_holder), [](std::vector<uint8_t>& data) {return data.data(); });
	std::vector<gpm_plugin_api_string_layout> names;
	std::transform(arrs.begin(), arrs.end(), std::back_inserter(names), [](const attribute_const_descriptor& data) {return gpm_plugin_api_string_layout{ const_cast<char*>(data.id.data()), data.id.size() }; });

    gpm_plugin_api_process_attribute_parms parms{};
	parms.attributes = holders.data();
	parms.is_constant = constant_holder.data();
	parms.num_attr_array = attr_nums.data();
	parms.num_attributes = arrs.size();
	std::vector<char> message_holder(6000);
	parms.error = get_message_definition(&message_holder);
	parms.time.start = time_prev;
	parms.time.end = time_now;
	parms.surface_layout = get_memory_layout(_extents);
	parms.attr_names = names.data();
	const auto ret = _holder->_process_model_timestep_func(_plugin_handle, &parms);
	print_message(parms.error, logger);
	return ret;
}

int gpm_plugin_mapper::update_attributes_timestep(double time_prev, double time_now,
                                                  const std::vector<attribute_descriptor>& arrs,
                                                  const Tools::gpm_logger& logger)
{
	// Start packing
	std::vector<std::vector<float*>> attr_holders;
	std::vector<std::vector<uint8_t>> attr_type;
	std::vector<size_t> attr_nums;
	std::forward_list<float> vals;

	for (auto& attr : arrs) {
		std::vector<float*> tmp1;
		std::vector<uint8_t> is_const(attr.funcs.size());
		attr_nums.push_back(attr.funcs.size());
		for (const auto& func_2d : attr.funcs) {
				tmp1.push_back(func_2d->begin());
		}
		attr_holders.push_back(tmp1);
		attr_type.push_back(is_const);
	}
	// All set up, map them in
	std::vector<float**> holders;
	std::transform(attr_holders.begin(), attr_holders.end(), std::back_inserter(holders), [](std::vector<float*>& data) {return data.data(); });
	std::vector<uint8_t*> constant_holder;
	std::transform(attr_type.begin(), attr_type.end(), std::back_inserter(constant_holder), [](std::vector<uint8_t>& data) {return data.data(); });
	std::vector<gpm_plugin_api_string_layout> names;
	std::transform(arrs.begin(), arrs.end(), std::back_inserter(names), [](const attribute_descriptor& data) {return gpm_plugin_api_string_layout{ const_cast<char*>(data.id.data()), data.id.size() }; });

	gpm_plugin_api_process_attribute_parms parms{};
	parms.attributes = holders.data();
	parms.is_constant = constant_holder.data();
	parms.num_attr_array = attr_nums.data();
	parms.num_attributes = arrs.size();
	std::vector<char> message_holder(6000);
	parms.error = get_message_definition(&message_holder);
	parms.time.start = time_prev;
	parms.time.end = time_now;
	parms.surface_layout = get_memory_layout(_extents);
	parms.attr_names = names.data();
	const auto ret = _holder->_update_attributes_timestep_func(_plugin_handle, &parms);
	print_message(parms.error, logger);
	return ret;
}

std::vector<std::string> gpm_plugin_mapper::find_input_properties() const
{
	return _model_properties;
}

std::vector<gpm_plugin_mapper::attribute_type> gpm_plugin_mapper::find_output_properties() const
{
	return _write_properties;
}


}}}
