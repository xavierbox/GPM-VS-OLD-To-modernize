// -- Schlumberger Private --

#include "index_counter.h"


namespace Slb { namespace Exploration { namespace Gpm {

using namespace boost;
using namespace boost::icl;


void index_counter::set_string_id_mapper(const std::map<std::string, int>& tmp_map) {
    _id_mapper = tmp_map;
    for (std::map<std::string, int>::const_iterator it = _id_mapper.begin(); it != _id_mapper.end(); ++it) {
        _inv_id_mapper[(*it).second] = (*it).first;
    }
}

void index_counter::add_index(const std::string& key, const std::vector<index_type>& indexes) {
    add_index(_id_mapper[key], indexes);
}

void index_counter::add_index(const std::string& key, index_type i) {
    add_index(_id_mapper[key], i);
}

void index_counter::add_index(const std::string& key, index_type i, index_type j) {
    add_index(_id_mapper[key], i, j);
}

void index_counter::add_index(const std::string& key, index_type i, index_type j, index_type k) {
    add_index(_id_mapper[key], i, j, k);
}

void index_counter::add_index(const key_type& key, const std::vector<index_type>& indexes) {
    switch (indexes.size()) {
    case 0:
        break;
    case 1:
        add_index(key, indexes[0]);
        break;
    case 2:
        add_index(key, indexes[0], indexes[1]);
        break;
    case 3:
        add_index(key, indexes[0], indexes[1], indexes[2]);
        break;
    default:
        break;
    }
}

void index_counter::add_index(const key_type& key, index_type i) {
    _map_1d[key] += discrete_interval<index_type>::right_open(i, i + 1);
}

void index_counter::add_index(const key_type& key, index_type i, index_type j) {
    _map_2d[key][i] += discrete_interval<index_type>::right_open(j, j + 1);
}

void index_counter::add_index(const key_type& key, index_type i, index_type j, index_type k) {
    _map_3d[key][index_pair_type(i, j)] += discrete_interval<index_type>::right_open(k, k + 1);
}

void index_counter::clear()
{
    _map_1d.clear();
    _map_2d.clear();
    _map_3d.clear();
    _id_mapper.clear();
    _inv_id_mapper.clear();
}

std::vector<index_counter::key_type> index_counter::arrays_found()
{
	std::vector<index_counter::key_type> found_keys;
    for(const auto& item: _map_1d) {
		found_keys.push_back(item.first);
    }
	for (const auto& item : _map_2d) {
		found_keys.push_back(item.first);
	}
	for (const auto& item : _map_3d) {
		found_keys.push_back(item.first);
	}
	return found_keys;
}

static index_counter::min_max_type find_min_max(const index_counter::interval_set_type& set) {
	index_counter::min_max_type res;
    if (!set.empty()) {
		index_counter::min_max_type min_max((*set.begin()).lower(), (*set.begin()).upper());
        for (const auto& item : set) {
            min_max.first = std::min(item.lower(), min_max.first);
            min_max.second = std::max(item.upper(), min_max.second);
        }
        res = min_max;
    }
    return res;
}

std::vector<index_counter::min_max_type> index_counter::dimensions(const std::string& key) const {
    const std::map<std::string, key_type>::const_iterator it = _id_mapper.find(key);
    if (it != _id_mapper.end()) {
        return dimensions((*it).second);
    }
    return std::vector<index_counter::min_max_type>();
}

std::vector<index_counter::min_max_type> index_counter::dimensions(const key_type& key) const {
    std::vector<min_max_type> res;
    if (_map_1d.find(key) != _map_1d.end()) {
        const interval_set_type& tmp = (*_map_1d.find(key)).second;
        min_max_type min_max_k = find_min_max(tmp);
        res.push_back(min_max_k);
    }
    else if (_map_2d.find(key) != _map_2d.end()) {
        const std::map<index_type, interval_set_type>& map = (*_map_2d.find(key)).second;
        if (!map.empty()) {
            // Lets find the maps
			interval_set_type i_range;
			interval_set_type j_range;
            for(const auto& item:map) {
                i_range += discrete_interval<index_type>::right_open(item.first, item.first + 1);
				j_range += item.second;
            }
			res.push_back(find_min_max(i_range));
			res.push_back(find_min_max(j_range));
        }
    }
    else if (_map_3d.find(key) != _map_3d.end()) {
        const std::map<index_pair_type, interval_set_type>& map = (*_map_3d.find(key)).second;
        if (!map.empty()) {
			interval_set_type i_range;
			interval_set_type j_range;
			interval_set_type k_range;
			for (const auto& item : map) {
				i_range += discrete_interval<index_type>::right_open(item.first.first, item.first.first + 1);
				j_range += discrete_interval<index_type>::right_open(item.first.second, item.first.second + 1);
				k_range += item.second;
			}
            res.push_back(find_min_max(i_range));
            res.push_back(find_min_max(j_range));
            res.push_back(find_min_max(k_range));
        }
    }
    return res;
}

std::vector<std::string>
index_counter::has_missing_indexes() const {
    std::vector<key_type> tmp = has_missing_indexes(0);
    std::vector<std::string> res;
    for (std::size_t i = 0; i < tmp.size(); ++i) {
        const std::map<key_type, std::string>::const_iterator it = _inv_id_mapper.find(tmp[i]);
        if (it != _inv_id_mapper.end()) {
            res.push_back((*it).second);
        }
    }
    return res;
}

std::vector<std::pair<index_counter::index_pair_type, index_counter::interval_set_type>>
index_counter::missing_indexes(const std::string& key) const {
    const std::map<std::string, key_type>::const_iterator it = _id_mapper.find(key);
    if (it != _id_mapper.end()) {
        return missing_indexes((*it).second);
    }
    return std::vector<std::pair<index_counter::index_pair_type, index_counter::interval_set_type>>();
}

std::vector<index_counter::key_type>
index_counter::has_missing_indexes(key_type fake) const {
    std::vector<key_type> res;
    for (map_1d_type::const_iterator it = _map_1d.begin(); it != _map_1d.end(); ++it) {
        if (!missing_indexes((*it).first).empty()) {
            res.push_back((*it).first);
        }
    }
    for (map_2d_type::const_iterator it = _map_2d.begin(); it != _map_2d.end(); ++it) {
        if (!missing_indexes((*it).first).empty()) {
            res.push_back((*it).first);
        }
    }
    for (map_3d_type::const_iterator it = _map_3d.begin(); it != _map_3d.end(); ++it) {
        if (!missing_indexes((*it).first).empty()) {
            res.push_back((*it).first);
        }
    }
    return res;
}

// Todo implement the splitting logic here
std::vector<std::pair<index_counter::index_pair_type, index_counter::interval_set_type>>
index_counter::missing_indexes(const key_type& key) const {
    std::vector<std::pair<index_pair_type, interval_set_type>> res;
    std::vector<index_pair_type> dims = dimensions(key);
    if (_map_1d.find(key) != _map_1d.end()) {
        interval_set_type tmp = (*_map_1d.find(key)).second;
        auto lower = dims[0].first;
        auto upper = dims[0].second;
        interval_set_type loc_max(discrete_interval<index_type>::right_open(lower, upper));
		interval_set_type diff = loc_max - tmp;
        if (!diff.empty()) {
            res.push_back(std::pair<index_pair_type, interval_set_type>(index_pair_type(0, 0), tmp));
        }
    }
    else if (_map_2d.find(key) != _map_2d.end()) {
		std::map<index_type, interval_set_type> map = (*_map_2d.find(key)).second;
        auto lower = dims[1].first;
        auto upper = dims[1].second;
        interval_set_type loc_max(discrete_interval<index_type>::right_open(lower, upper));
        if (!map.empty()) {
            for (auto i = dims[0].first; i < dims[0].second; ++i) {
				interval_set_type tmp = map[i];
                index_pair_type pos(0, i);
                interval_set_type diff = loc_max - tmp;
                if (!diff.empty()) {
                    res.push_back(std::pair<index_pair_type, interval_set_type>(pos, diff));
                }
            }
        }
    }
    else if (_map_3d.find(key) != _map_3d.end()) {
        std::map<index_pair_type, interval_set_type> map = (*_map_3d.find(key)).second;
        auto lower = dims[2].first;
        auto upper = dims[2].second;
        interval_set_type loc_max(discrete_interval<index_type>::right_open(lower, upper));
        if (!map.empty()) {
            for (auto i = dims[0].first; i < dims[0].second; ++i) {
                for (auto j = dims[1].first; j < dims[1].second; ++j) {
					index_pair_type pos(i, j);
					interval_set_type tmp = map[pos];
					interval_set_type diff = loc_max - tmp;
                    if (!diff.empty()) {
                        res.push_back(std::pair<index_pair_type, interval_set_type>(pos, diff));
                    }
                }
            }
        }
    }

    return res;
}

}}}
