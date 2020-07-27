#ifndef GPM_MODEL_CONVERTER_H
#define GPM_MODEL_CONVERTER_H
#include "gpm_model_definition.h"
#include "gpm_vbl_array_2d.h"
#include <boost/geometry.hpp>
#include <functional>
#include <vector>
#include "gpm_int_extent_3d.h"

namespace Slb { namespace Exploration { namespace Gpm {

class gpm_model_converter {
public:
    // Need a couple of changes here
    using size_type =  std::size_t;
    using index_type = std::ptrdiff_t;
    using long_index_type = long long;
    using long_size_type = unsigned long;
    using quad_index_type = std::array<long_size_type, 4>;

    using point = boost::geometry::model::point<double, 3, boost::geometry::cs::cartesian>;
    using vec_point = std::vector<point>;
    // We are writing it out in double, may be a bit extravagant?
    using function_2d_read_type = std::function<double(index_type, index_type)>;

    enum number_of_points_for_cell { n_tetra = 4, n_pyramid = 5, n_wedge = 6, n_hexahedron = 8 };

    static std::vector<std::vector<int>> check_cell_pointers(const vec_point& points,
                                                             const quad_index_type& bot_mapped,
                                                             const quad_index_type& top_mapped);
    void convert_to_unstructured(const std::vector<function_2d_read_type>& surface_stack,
                                 const gpm_model_definition& model);
    void clean_ugrid_points_and_cells();

    gpm_vbl_array_2d<long long> _top_model_index_map;
    int_extent_3d _stack_extent;
    std::vector<long long> _origin_to_compressed_map;
    std::vector<long long> _compressed_to_origin_map;
    std::vector<point> _grid_points;
    std::vector<point> _compressed_grid_points;
    std::vector<std::vector<int>> _cell_indexes;
    std::vector<std::vector<int>> _compressed_cell_indexes;
    bool _is_compressed = false;
};
}}}
#endif
