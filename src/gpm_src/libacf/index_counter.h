// -- Schlumberger Private --

#ifndef index_counter_h
#define index_counter_h

#include "acf_base_io_types.h"
// Statically bounded intervals
#include <boost/icl/interval_set.hpp>

#include <map>
#include <vector>
#include <utility>
#include <string>

// A system to count indexes of a 1,2 or 3d array
// Will calculate the bounding box and return the indexes as they come out of the system
// Each array will be identified by a string, and the number of indexes will be counted

namespace Slb { namespace Exploration { namespace Gpm {

class index_counter :public acf_base_io_types {
public:
    typedef boost::icl::interval_set<index_type> interval_set_type;

    index_counter() = default;

    ~index_counter() = default;

    void set_string_id_mapper(const std::map<std::string, key_type>& tmp_map);
    void add_index(const std::string& key, const std::vector<index_type>& indexes);
    void add_index(const std::string& key, index_type i);
    void add_index(const std::string& key, index_type i, index_type j);
    void add_index(const std::string& key, index_type i, index_type j, index_type k);
    void add_index(const key_type& key, const std::vector<index_type>& indexes);
    void add_index(const key_type& key, index_type i);
    void add_index(const key_type& key, index_type i, index_type j);
    void add_index(const key_type& key, index_type i, index_type j, index_type k);

    void clear();
    // The dimesions of the indexes read
    // These are also half open intervals for index i, j and k
	std::vector< key_type> arrays_found();

    std::vector<min_max_type> dimensions(const std::string& key) const;
    std::vector<std::pair<index_pair_type, interval_set_type>> missing_indexes(const std::string& key) const;
    std::vector<std::string> has_missing_indexes() const;
    std::vector<min_max_type> dimensions(const key_type& key) const;
    std::vector<std::pair<index_pair_type, interval_set_type>> missing_indexes(const key_type& key) const;
    std::vector<key_type> has_missing_indexes(key_type fake) const;
private:
    typedef std::map<key_type, interval_set_type> map_1d_type;
    typedef std::map<key_type, std::map<index_type, interval_set_type>> map_2d_type;
    typedef std::map<key_type, std::map<index_pair_type, interval_set_type>> map_3d_type;
    map_1d_type _map_1d;
    map_2d_type _map_2d;
    map_3d_type _map_3d;
    std::map<std::string, key_type> _id_mapper;
    std::map<key_type, std::string> _inv_id_mapper;
};

}}}
#endif
