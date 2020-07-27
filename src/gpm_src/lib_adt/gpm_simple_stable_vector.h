#ifndef gpm_simple_stable_vector_h_
#define gpm_simple_stable_vector_h_
#include <forward_list>
#include <vector>

// This is a simple version of the boost stable_vector
// The implementation is using a linked_list and a vector to provide 
// The ability to have a vector interface, but keeping each iterator stable unless it is deleted
// It is not optimally implemented like stable_vector, but the access use is hopefully good enough
namespace Slb { namespace Exploration { namespace Gpm {
template <typename T>
class gpm_simple_stable_vector {
public:
    typedef T value_type;
    typedef std::size_t size_type;
    gpm_simple_stable_vector():_items(),_accessors(){}
    gpm_simple_stable_vector(const gpm_simple_stable_vector& rhs) { _items = rhs._items; assign_iterators(rhs.size()); }
    gpm_simple_stable_vector& operator=(const gpm_simple_stable_vector& rhs)
    {
        if (&rhs == this) {
            return *this;
        }
        _items = rhs._items; assign_iterators(rhs.size()); return *this;
    }
    bool empty()const { return _items.empty(); }
    T& operator[](size_type i) { return *_accessors[i]; }
    const T& operator[](size_type i) const { return *_accessors[i]; }
    size_type size() const { return _accessors.size(); }
    void clear() { _items.clear();_accessors.clear(); }
    void resize(size_type size)
    {
        if (size != _accessors.size()) {
            _items.resize(size);
            assign_iterators(size);
        }
    }
    void resize(size_type size, const T& item)
    {
        // Need to add some default stuff
        if (size != _accessors.size()) {
            _items.resize(size, item);
            assign_iterators(size);
        }
    }
private:
    void assign_iterators(size_type size)
    {
        _accessors.resize(size);
        int i = 0;
        for (auto it = _items.begin(); it != _items.end();++it) {
            _accessors[i] = it;
            ++i;
        }
    }
    std::forward_list<T> _items;
    std::vector<typename std::forward_list<T>::iterator> _accessors;
};
}}}

#endif
