// -- Schlumberger Private --

#include "gpm_h5_reader_impl.h"
#include "parm_prop_descr.h"
#include <memory>
#include <utility>
#include "gpm_h5cpp_wrapper.h"
#include "gpm_format_helper.h"

using namespace H5;

namespace Slb { namespace Exploration { namespace Gpm {

	gpm_h5_reader_impl::gpm_h5_reader_impl(std::shared_ptr<gpm_h5cpp_wrapper> file_p) : m_file_p(std::move(file_p))
	{
	}

    gpm_h5_reader_impl::~gpm_h5_reader_impl() = default;
    
	bool gpm_h5_reader_impl::initialize() {
		try {
			const auto propertiesRef = gpm_h5cpp_wrapper::PROPERTIESDATASET;
			const auto ref_dataset = m_file_p->get_dataset(propertiesRef);
			auto property_ids = m_file_p->read_ref_names(ref_dataset);
			const auto size = static_cast<int>(property_ids.size());
			for (auto i = 0; i < size; ++i) {
				const auto id = property_ids.at(i);
				m_indprop_map[id] = i;
			}

			m_num_tops = get_num_tops();
			return true;
		}
		catch (...)
		{
			return false;
		}		
	}

	std::vector<hobj_ref_t> gpm_h5_reader_impl::get_props_references(const std::shared_ptr<gpm_h5cpp_wrapper>& file_ptr)
	{
		const auto propertiesRef = gpm_h5cpp_wrapper::PROPERTIESDATASET;
		const auto ref_dataset = file_ptr->get_dataset(propertiesRef);
		return file_ptr->get_references(ref_dataset);
	}
	
	int gpm_h5_reader_impl::get_num_tops(const std::shared_ptr<gpm_h5cpp_wrapper>& file_ptr) const
	{
		auto references = get_props_references(file_ptr);
		const auto property_id = parm_descr_holder::get_grid_map_name(TOP);
		const auto property_index = get_property_index(property_id);
		const auto property_dataset = file_ptr->dereference_dataset(&references[property_index]);
		auto dataspace = property_dataset.getSpace();
		const int rank = dataspace.getSimpleExtentNdims();
		hsize_t sdim[64];
		int ndims = dataspace.getSimpleExtentDims(sdim);
		std::vector<hsize_t> dimensions;
		std::copy(sdim, sdim + rank, std::back_inserter(dimensions));
		return dimensions[1];
	}

	int gpm_h5_reader_impl::get_num_tops()
	{
		return get_num_tops(m_file_p);
	}

	void gpm_h5_reader_impl::get_timesteps(lin_span<double>* timesteps) const
	{
		const auto typedesc = parm_type_descr_holder::get_parm_description(TIMESET);
		const auto dataset = m_file_p->get_dataset(typedesc.name);
		m_file_p->read_dataset<double>(dataset, timesteps);
	}

	int gpm_h5_reader_impl::size_timesteps() const
	{
		const auto typedesc = parm_type_descr_holder::get_parm_description(TIMESET);
		return get_num_1d_array(typedesc.name);
	}
    
    void gpm_h5_reader_impl::get_ages(lin_span<double>* ages) const
    {
        const auto typedesc = parm_type_descr_holder::get_parm_description(AGESET);
        const auto dataset = m_file_p->get_dataset(typedesc.name);
        m_file_p->read_dataset<double>(dataset, ages);
    }

    int gpm_h5_reader_impl::size_ages() const
    {
        const auto typedesc = parm_type_descr_holder::get_parm_description(AGESET);
        return get_num_1d_array(typedesc.name);
    }

    void gpm_h5_reader_impl::get_world_corners(lin_span<float>* x_values, lin_span<float>* y_values) const
    {
        const auto typedesc_x = parm_type_descr_holder::get_parm_description(XCORNERS);
        const auto x_dataset = m_file_p->get_dataset(typedesc_x.name);
        m_file_p->read_dataset<float>(x_dataset, x_values);

        const auto typedesc_y = parm_type_descr_holder::get_parm_description(YCORNERS);
        const auto y_dataset = m_file_p->get_dataset(typedesc_y.name);
        m_file_p->read_dataset<float>(y_dataset, y_values);
    }

    void gpm_h5_reader_impl::get_model_size(int* num_rows, int* num_cols) const
    {
        const auto numrows = parm_type_descr_holder::get_parm_description(NUMROWS);
        *num_rows = m_file_p->read_scalar_attribute<int>(numrows.name);
        const auto numcols = parm_type_descr_holder::get_parm_description(NUMCOLS);
        *num_cols = m_file_p->read_scalar_attribute<int>(numcols.name);
    }

	float gpm_h5_reader_impl::get_base_erodability() const
	{
		const auto erodability = parm_type_descr_holder::get_parm_description(BASE_ERODABILITY);
		if (m_file_p->attribute_exists(erodability.name)) {
			return m_file_p->read_scalar_attribute<float>(erodability.name);
		}

		return 1.0F;
	}

    float gpm_h5_reader_impl::get_novalue() const
    {
        const auto novalue = parm_type_descr_holder::get_parm_description(ZNULL);
        return m_file_p->read_scalar_attribute<float>(novalue.name);
    }

    void gpm_h5_reader_impl::get_property_statistics(const std::string& prop_id, int time_index, float& min, float& max)
    {
		return get_property_statistics(m_file_p, prop_id, time_index, min, max);
    }

	void gpm_h5_reader_impl::get_property_statistics(const std::shared_ptr<gpm_h5cpp_wrapper>& file_ptr, const std::string& prop_id, int time_index, float& min, float& max)
	{
		const auto prop_index = get_property_index(prop_id);
		const auto dataset_name = gpm_h5cpp_wrapper::STATISTICSDATASET;
		if (file_ptr->dataset_exists(dataset_name)) {
			const auto stats_dataset = file_ptr->get_dataset(dataset_name);
			std::vector<hobj_ref_t> references = file_ptr->get_references(stats_dataset);
			const auto dataset = file_ptr->dereference_dataset(stats_dataset, &references[prop_index]);
			std::vector<hsize_t> dimensions;
			std::vector<float> values = file_ptr->read_dataset<float>(dataset, dimensions);
			if (values.size() > time_index * 2 + 1) {
				min = values[time_index * 2];
				max = values[time_index * 2 + 1];
			}
			else {
				min = 0.0F;
				max = 0.0F;
			}
		}
		else if (m_indprop_stats_cache.count(prop_id) 
			&& m_indprop_stats_cache[prop_id][time_index].size() == 2) {
			auto values = m_indprop_stats_cache[prop_id][time_index];
			min = values[0];
			max = values[1];
		}
		else {
			calculate_statistics(time_index, prop_id, min, max);
			const auto time_num = size_timesteps();
			std::vector<std::vector<float>> statistics(time_num);
			std::vector<float> statistics_item(2);
			statistics_item[0] = min;
			statistics_item[1] = max;
			statistics[time_index] = statistics_item;
			m_indprop_stats_cache[prop_id] = statistics;
		}
	}

    int gpm_h5_reader_impl::size_sediment_info_in_model() const
    {
        int num = 0;
        const auto typedesc = parm_type_descr_holder::get_parm_description(SED_PROP_MAP);
        const auto dims = get_dataset_dims(typedesc.name);
        if (!dims.empty()) {
            num = dims[1];
        }
        return num;
    }

    void gpm_h5_reader_impl::get_sediment_info_in_model(std::vector<std::string>* sed_ids, std::vector<std::string>* sed_names, std::vector<std::string>* sed_prop) const
    {
        const auto typedesc = parm_type_descr_holder::get_parm_description(SED_PROP_MAP);
        const std::string dataset_name = typedesc.name;
        if (m_file_p->dataset_exists(dataset_name)) {
            H5::DataSet dataset = m_file_p->get_dataset(dataset_name);
            std::vector<hsize_t> dimensions;
            std::vector<std::string> res_data = m_file_p->read_string_dataset(dataset, dimensions);
            int colsize = dimensions[1];
            std::copy(res_data.begin(), res_data.begin() + colsize, sed_ids->begin());
            std::copy(colsize + res_data.begin(), res_data.begin() + 2 * colsize, sed_names->begin());
            std::copy(res_data.begin() + 2 * colsize, res_data.end(), sed_prop->begin());
        }
    }

    int gpm_h5_reader_impl::size_sediment_info_in_baselayer() const
    {
        int num = 0;
        const auto typedesc = parm_type_descr_holder::get_parm_description(BASE_LAYER_MAP);
        const auto dims = get_dataset_dims(typedesc.name);
        if (!dims.empty()) {
            num = dims.front();
        }
        return num;
    }

    int gpm_h5_reader_impl::get_num_1d_array(const std::string& dataset_name) const
    {
        std::vector<hsize_t> dims = get_dataset_dims(dataset_name);
        if (dims.empty()) {
            return 0;
        }

        return dims.front();
    }

    void gpm_h5_reader_impl::get_sediment_ids_in_baselayer(std::vector<std::string>* sed_ids) const
    {
        const auto typedesc = parm_type_descr_holder::get_parm_description(BASE_LAYER_MAP);
        const std::string dataset_name = typedesc.name;
        if (m_file_p->dataset_exists(dataset_name)) {
            H5::DataSet dataset = m_file_p->get_dataset(dataset_name);
            std::vector<hsize_t> dimensions;
            std::vector<std::string> res_data = m_file_p->read_string_dataset(dataset, dimensions);
            std::copy(res_data.begin(), res_data.end(), sed_ids->begin());
        }
    }

	void gpm_h5_reader_impl::get_property(const std::string& prop_id, int time_index, std::vector<lin_span<float>>* surfaces, std::vector<int>* diagonal_indexes)
	{
		assert(surfaces->size() == diagonal_indexes->size());

		int top_right_most = m_diagonal_indexes[time_index];
		for (int i = surfaces->size() - 1; i >= 0; --i) {
			diagonal_indexes->at(i) = top_right_most;
			top_right_most--;
		}

		get_property_values(time_index, prop_id, surfaces);
	}
    
    void gpm_h5_reader_impl::get_isnodal(lin_span<int>* nodal) const
    {
        const auto typedesc = parm_type_descr_holder::get_parm_description(ISNODAL);
        const auto dataset_name = typedesc.name;
        H5::DataSet dataset = m_file_p->get_dataset(dataset_name);
        std::vector<hsize_t> dimensions;
        std::vector<int> values = m_file_p->read_dataset<int>(dataset, dimensions);
        std::copy(values.begin(), values.end(), nodal->begin());
    }

    int gpm_h5_reader_impl::size_isnodal() const
    {
        const auto typedesc = parm_type_descr_holder::get_parm_description(ISNODAL);
        return get_num_1d_array(typedesc.name);
    }

	int gpm_h5_reader_impl::get_num_grid_names() const
	{
		const auto gridRef = gpm_h5cpp_wrapper::GRIDSDATASET;
		const auto ref_dataset = m_file_p->get_dataset(gridRef);
		auto names = m_file_p->read_ref_names(ref_dataset);
		return names.size();
	}

	void gpm_h5_reader_impl::get_grid_names(std::vector<std::string>* grid_names) const
    {
		const auto gridRef = gpm_h5cpp_wrapper::GRIDSDATASET;
		const auto ref_dataset = m_file_p->get_dataset(gridRef);
        auto names = m_file_p->read_ref_names(ref_dataset);
		std::copy(names.begin(), names.end(), grid_names->begin());
	}

	void gpm_h5_reader_impl::get_grid(const int grid_index, lin_span<float>* z_nodes) const
    {
		const auto gridRef = gpm_h5cpp_wrapper::GRIDSDATASET;
		const auto ref_dataset = m_file_p->get_dataset(gridRef);
		const auto references = gpm_h5cpp_wrapper::get_references(ref_dataset);
		const auto dataset = gpm_h5cpp_wrapper::dereference_dataset(ref_dataset, &references[grid_index]);

		std::vector<hsize_t> dimensions;
		std::vector<float> values = m_file_p->read_dataset<float>(dataset, dimensions);
		std::copy(values.begin(), values.end(), z_nodes->begin());
	}
    
    void gpm_h5_reader_impl::close() const
    {
        m_file_p->close_file();
    }

    std::vector<hsize_t> gpm_h5_reader_impl::get_dataset_dims(const std::string& dataset_name) const
    {
        std::vector<hsize_t> dims;
        if (!m_file_p->dataset_exists(dataset_name)) {
            return dims;
        }

        DataSet dataset = m_file_p->get_dataset(dataset_name);
        DataSpace dataspace = dataset.getSpace();
        const int rank = dataspace.getSimpleExtentNdims();
        if (rank > 0) {
            dims.resize(rank);
            dataspace.getSimpleExtentDims(&dims[0]);
        }

        return dims;
    }

    void gpm_h5_reader_impl::calculate_statistics(int time_index, const std::string& prop_id, float& min, float& max)
    {
        int numRows = 0;
        int numCols = 0;
        get_model_size(&numRows, &numCols);
        const auto arrayLength = numRows * numCols;

        const int propNum = size_property(time_index, prop_id);
        std::vector<lin_span<float>> loc_arrays(propNum);
        const std::vector<float> holder(arrayLength);
        std::vector<std::vector<float>> loc_array_holder(propNum, holder);
        //Allocate holder to avoid new/delete, could have used 2d array also
        for (int i = 0; i < propNum; i++) {
            loc_arrays[i] = lin_span<float>(loc_array_holder[i].data(), arrayLength);
        }

        get_property_values(time_index, prop_id, &loc_arrays);

        const float znull = get_novalue();
        float min_val = znull;
        float max_val = znull;
        for (int l = 0; l < propNum; ++l) {
            auto layer = loc_arrays[l];
            for (int i = 0, nl = numRows * numCols; i < nl; ++i) {
                auto val = layer[i];
                if (val != znull) {
                    min_val = min_val != znull ? std::min(val, min_val) : val;
                    max_val = max_val != znull ? std::max(val, max_val) : val;
                }
            }
        }
        min = min_val;
        max = max_val;
    }

	void gpm_h5_reader_impl::get_property_display_names(std::vector<std::string>* ids, std::vector<std::string>* names) const
	{
		const auto typedesc = parm_type_descr_holder::get_parm_description(PROP_DISPLAY_NAME);
		const auto dataset_name = typedesc.name;
		const auto dataset = m_file_p->get_dataset(dataset_name);
		std::vector<hsize_t> dimensions;
		std::vector<std::string> res_data = m_file_p->read_string_dataset(dataset, dimensions);
		int colsize = dimensions[1];
		std::copy(res_data.begin(), res_data.begin() + colsize, ids->begin());
		std::copy(res_data.begin() + colsize, res_data.end(), names->begin());
	}

	int gpm_h5_reader_impl::size_property_display_names() const
	{
		int num = 0;
		const auto typedesc = parm_type_descr_holder::get_parm_description(PROP_DISPLAY_NAME);
		const auto dataset_name = typedesc.name;
		const auto dims = get_dataset_dims(dataset_name);
		if (!dims.empty()) {
			num = dims[1];
		}
		return num;
	}

	void gpm_h5_reader_impl::get_property_ids(std::vector<std::string>* property_ids) {
		std::vector<std::string> ids;
		for (auto it = m_indprop_map.begin(); it != m_indprop_map.end(); ++it) {
			ids.push_back(it->first);
		}

		std::copy(ids.begin(), ids.end(), property_ids->begin());
	}

	int gpm_h5_reader_impl::size_property_ids() const {
		return m_indprop_map.size();
	}


	int gpm_h5_reader_impl::get_property_index(const std::string& prop_id) const {
		if (m_indprop_map.count(prop_id) > 0) {
			return m_indprop_map.at(prop_id);
		}

		return -1;
	}

	void gpm_h5_reader_impl::get_property_values(const int time_index, const std::string& prop_id, std::vector<lin_span<float>>* surfaces) {
		const auto size = static_cast<int>(surfaces->size());
		for (auto j = 0; j < size; ++j) {
			get_property_at_index(prop_id, time_index, j, &surfaces->at(j));
		}
	}

	std::string gpm_h5_reader_impl::validate_surface_index(const int& surface_index) const
    {
		if (surface_index >= m_num_tops || surface_index < 0) {
			return "Surface index is incorrect.";
		}

		return std::string("");
	}	

	std::string gpm_h5_reader_impl::validate_time_index(const int& time_index) const
    {
		if (time_index > size_timesteps() || time_index < 0) {
			return "Timestep index is incorrect.";
		}

		return std::string("");
	}

	std::string gpm_h5_reader_impl::validate_property_id(const std::string& property_id) const
    {
		const auto property_index = get_property_index(property_id);
		if (property_index == -1) {
			return "Property ID is incorrect.";
		}

		return std::string("");
	}
}}}