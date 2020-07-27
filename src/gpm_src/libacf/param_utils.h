// -- Schlumberger Private --

#ifndef param_utils_h
#define param_utils_h
#include "acf_base_io_types.h"
#include <vector>


namespace Slb { namespace Exploration { namespace Gpm {

class param_helper {
public:
	// TODO anchor the others then come back JT
	using index_type = acf_base_io_types::index_type;
    template <typename T>
    static void add_val_to_array(std::vector<T>& arr, index_type index, T val) {
        auto arr_size = static_cast<index_type>(arr.size());
        if (index > arr_size) {
            arr.resize(index + 1);
            arr[index] = val;
        }
        else if (index < arr_size) {
            arr[index] = val;
        }
        else {
            // index == arr.size
            arr.push_back(val);
        }
    }

    template <typename T>
    static void add_val_to_array(gpm_vbl_array_2d<T>& arr, index_type i, index_type j, T val) {
        auto row_ext = arr.row_extent();
        auto col_ext = arr.col_extent();
        if (!arr.is_valid_index(i, j)) {
            if (!row_ext.contains(i)) {
                row_ext = row_ext | int_extent_1d::make_index(i);
            }

            if (!col_ext.contains(j)) {
                col_ext = col_ext | int_extent_1d::make_index(j);
            }

            arr.resize(row_ext, col_ext);
        }

        arr(i, j) = val;
    }

    template <class T>
    static void add_val_to_array(gpm_vbl_array_2d<T>& arr, const std::vector<index_type>& indexes, T val) {
        auto i = indexes[0];
        auto j = indexes[1];

        const bool is_ok = arr.extents().contains(i, j);
        if (!is_ok) {
            int_extent_2d res = int_extent_2d::make_index(i, j) | arr.extents();
            arr.resize(res);
        }
        arr(i, j) = val;
    }

    template <class T>
    static void add_val_to_array(gpm_vbl_vector_array_3d<T>& arr, const std::vector<index_type>& indexes, T val) {
        auto i = indexes[0];
        auto j = indexes[1];
        auto k = indexes[2];

        const bool is_ok = arr.extents().contains(i, j, k);
        if (!is_ok) {
            int_extent_3d res = int_extent_3d::make_index(i, j, k) | arr.extents();
            arr.resize(res);
        }
        arr(i, j, k) = val;
    }
};

}}}
#endif
