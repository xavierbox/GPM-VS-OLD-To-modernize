// -- Schlumberger Private --


#include "gpm_param_type_mapper.h"
#include "gpm_type_descriptor.h"
#include "gpm_vbl_array_2d.h"
#include <boost/any.hpp>
#include <map>
#include <memory>

namespace Slb { namespace Exploration { namespace Gpm {
    // Remember that [] will create items when used, so check properly
    // Class to hold data that is mapped in through the parser normally

    void gpm_param_type_mapper::initialize_parameters(const std::vector<TypeDescr>& descr)
    {
        for (auto it : descr) {
            _param_descrs.insert(std::make_pair(it.id, it));
        }
        make_boost_types();
    }

    void gpm_param_type_mapper::add_id_holder(TypeDescr::id_type id)
    {
		const auto pod_type = _param_descrs.find(id)->second.pod_type;
		const auto dims = _param_descrs.find(id)->second.num_dimensions;
		boost::any tmp;
		switch (dims) {
		case POD_VAL:
			switch (pod_type) {
			case INTEGER:
				tmp = TypeDescr::Pod_Promotion<INTEGER>::pod_type();
				break;
			case FLOAT:
				tmp = TypeDescr::Pod_Promotion<FLOAT>::pod_type();
				break;
			case DOUBLE:
				tmp = TypeDescr::Pod_Promotion<DOUBLE>::pod_type();
				break;
			case STRING:
				tmp = TypeDescr::Pod_Promotion<STRING>::pod_type();
				break;
            default:
                break;
            }
			break;
		case ARRAY_1d:
			switch (pod_type) {
			case INTEGER:
				tmp = std::make_shared<Promotion<INTEGER, ARRAY_1d>::return_type>();
				break;
			case FLOAT:
				tmp = std::make_shared<Promotion<FLOAT, ARRAY_1d>::return_type>();
				break;
			case DOUBLE:
				tmp = std::make_shared<Promotion<DOUBLE, ARRAY_1d>::return_type>();
				break;
			case STRING:
				tmp = std::make_shared<Promotion<STRING, ARRAY_1d>::return_type>();
				break;
            default:
                break;
            }

			break;
		case ARRAY_2d:
			switch (pod_type) {
			case INTEGER:
				tmp = std::make_shared<Promotion<INTEGER, ARRAY_2d>::return_type>();
				break;
			case FLOAT:
				tmp = std::make_shared<Promotion<FLOAT, ARRAY_2d>::return_type>();
				break;
			case DOUBLE:
				tmp = std::make_shared<Promotion<DOUBLE, ARRAY_2d>::return_type>();
				break;
			case STRING:
				tmp = std::make_shared<Promotion<STRING, ARRAY_2d>::return_type>();
				break;
            default:
                break;
            }
			break;
		case ARRAY_3d:
			switch (pod_type) {
			case INTEGER:
				tmp = std::make_shared<Promotion<INTEGER, ARRAY_3d>::return_type>();
				break;
			case FLOAT:
				tmp = std::make_shared<Promotion<FLOAT, ARRAY_3d>::return_type>();
				break;
			case DOUBLE:
				tmp = std::make_shared<Promotion<DOUBLE, ARRAY_3d>::return_type>();
				break;
			case STRING:
				tmp = std::make_shared<Promotion<STRING, ARRAY_3d>::return_type>();
				break;
            default:
                break;
            }
			break;
        default:
            break;
        }
		_params[id] = tmp;
    }

    void gpm_param_type_mapper::make_boost_types()
    {
        for (const auto& it : _param_descrs) {
            auto key = it.first;
            auto val = it.second;
            const auto pod_type = _param_descrs.find(key)->second.pod_type;
            const auto dims = _param_descrs.find(key)->second.num_dimensions;
            boost::any tmp;
            switch (dims) {
            case POD_VAL:
                switch (pod_type) {
                case INTEGER:
                    tmp = TypeDescr::Pod_Promotion<INTEGER>::pod_type();
                    break;
                case FLOAT:
                    tmp = TypeDescr::Pod_Promotion<FLOAT>::pod_type();
                    break;
                case DOUBLE:
                    tmp = TypeDescr::Pod_Promotion<DOUBLE>::pod_type();
                    break;
                case STRING:
                    tmp = TypeDescr::Pod_Promotion<STRING>::pod_type();
                    break;
                }
                break;
            case ARRAY_1d:
                switch (pod_type) {
                case INTEGER:
                    tmp = std::shared_ptr<Promotion<INTEGER, ARRAY_1d>::return_type>();
                    break;
                case FLOAT:
                    tmp = std::shared_ptr<Promotion<FLOAT, ARRAY_1d>::return_type>();
                    break;
                case DOUBLE:
                    tmp = std::shared_ptr<Promotion<DOUBLE, ARRAY_1d>::return_type>();
                    break;
                case STRING:
                    tmp = std::shared_ptr<Promotion<STRING, ARRAY_1d>::return_type>();
                    break;
                }

                break;
            case ARRAY_2d:
                switch (pod_type) {
                case INTEGER:
                    tmp = std::shared_ptr<Promotion<INTEGER, ARRAY_2d>::return_type>();
                    break;
                case FLOAT:
                    tmp = std::shared_ptr<Promotion<FLOAT, ARRAY_2d>::return_type>();
                    break;
                case DOUBLE:
                    tmp = std::shared_ptr<Promotion<DOUBLE, ARRAY_2d>::return_type>();
                    break;
                case STRING:
                    tmp = std::shared_ptr<Promotion<STRING, ARRAY_2d>::return_type>();
                    break;
                }
                break;
            case ARRAY_3d:
                switch (pod_type) {
                case INTEGER:
                    tmp = std::shared_ptr<Promotion<INTEGER, ARRAY_3d>::return_type>();
                    break;
                case FLOAT:
                    tmp = std::shared_ptr<Promotion<FLOAT, ARRAY_3d>::return_type>();
                    break;
                case DOUBLE:
                    tmp = std::shared_ptr<Promotion<DOUBLE, ARRAY_3d>::return_type>();
                    break;
                case STRING:
                    tmp = std::shared_ptr<Promotion<STRING, ARRAY_3d>::return_type>();
                    break;
                }
                break;
            }
            _param_storage_types[key] = tmp;
        }
    }

}}}

