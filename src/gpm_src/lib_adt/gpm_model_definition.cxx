// -- Schlumberger Private --

#include "gpm_model_definition.h"
#include <cassert>
#include <boost/geometry/strategies/transform/matrix_transformers.hpp>
#include <boost/geometry/strategies/transform/inverse_transformer.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/algorithms/transform.hpp>

BOOST_GEOMETRY_REGISTER_POINT_2D(Slb::Exploration::Gpm::gpm_model_definition::world_pt, float, boost::geometry::cs::cartesian, x, y)
BOOST_GEOMETRY_REGISTER_POINT_2D(Slb::Exploration::Gpm::gpm_model_definition::index_pt, float, boost::geometry::cs::cartesian, i, j)

namespace Slb { namespace Exploration { namespace Gpm {
struct gpm_model_definition::impl {
    impl() : _column_width(-1), _row_height(-1)
    {
    }

    impl(int_extent_2d::size_type nr, int_extent_2d::size_type nc) : _model_extent(nr, nc), _column_width(-1), _row_height(-1)
    {
    }

    explicit impl(const int_extent_2d& ext) : _model_extent(ext), _column_width(0), _row_height(0)
    {
    }
    typedef boost::geometry::strategy::transform::matrix_transformer<double, 2, 2> matrix_type;
    float angle_from_x_axis() const;
    std::pair<matrix_type, matrix_type> world_to_index_and_inverse() const;
    int_extent_2d _model_extent;
    float _column_width;
    float _row_height;
    std::vector<float> _x_coords;
    std::vector<float> _y_coords;
    matrix_type _world_to_index;
    matrix_type _index_to_world;
};

gpm_model_definition::gpm_model_definition(): _impl(new impl)
{
}

gpm_model_definition::gpm_model_definition(int_extent_2d::size_type nr, int_extent_2d::size_type nc): _impl(new impl(nr, nc)) {
}

gpm_model_definition::gpm_model_definition(const int_extent_2d& ext): _impl(new impl(ext)) {

}

gpm_model_definition::gpm_model_definition(const gpm_model_definition& rhs):_impl(new impl())
{
    *_impl = *(rhs._impl);
}

gpm_model_definition& gpm_model_definition::operator=(const gpm_model_definition& rhs)
{
    if (&rhs == this) {
        return *this;
    }
    *_impl = *(rhs._impl);
    return *this;
}

gpm_model_definition::~gpm_model_definition() = default;

const int_extent_2d& gpm_model_definition::model_extent() const {
    return _impl->_model_extent;
}

void gpm_model_definition::set_model_size(int_extent_1d::size_type nr, int_extent_1d::size_type nc) {
    _impl->_model_extent = int_extent_2d(nr, nc);
}

void gpm_model_definition::set_model_size(const int_extent_2d& ext) {
    _impl->_model_extent = ext;
}

void gpm_model_definition::set_world_coordinates(const std::vector<float>& x, const std::vector<float>& y) {
    assert(x.size() > 3 && y.size() > 3);
    assert(!_impl->_model_extent.is_empty());
    _impl->_x_coords = x;
    _impl->_y_coords = y;
    auto dx1 = x[1] - x[0];
    auto dy1 = y[1] - y[0];
    auto num = _impl->_model_extent.col_extent().size();
    if (num == 1) { ++num; }
    _impl->_column_width = std::sqrt(dx1 * dx1 + dy1 * dy1) / (num - 1);
    dx1 = x[3] - x[0];
    dy1 = y[3] - y[0];
    num = _impl->_model_extent.row_extent().size();
    if (num == 1) { ++num; }
    _impl->_row_height = std::sqrt(dx1 * dx1 + dy1 * dy1) / (num - 1);
    auto trans = _impl->world_to_index_and_inverse();
    _impl->_world_to_index = trans.first;
    _impl->_index_to_world = trans.second;
}

std::vector<float> gpm_model_definition::get_x_coordinates() const {
    return  _impl->_x_coords;
}

std::vector<float> gpm_model_definition::get_y_coordinates() const {
    return  _impl->_y_coords;
}

gpm_model_definition::index_pt gpm_model_definition::convert_from_world_to_index(const world_pt& pt) const
{
    index_pt ind;
    boost::geometry::transform(pt, ind, _impl->_world_to_index);
    return ind;
}

gpm_model_definition::world_pt gpm_model_definition::convert_from_index_to_world(const index_pt& ind) const
{
    world_pt pt;
    boost::geometry::transform(ind, pt, _impl->_index_to_world);
    return pt;
}


float gpm_model_definition::impl::angle_from_x_axis() const {
    auto dx1 = _x_coords[1] - _x_coords[0];
    auto dy1 = _y_coords[1] - _y_coords[0];
    return atan2f(dy1, dx1);
}

std::pair<gpm_model_definition::impl::matrix_type, gpm_model_definition::impl::matrix_type> gpm_model_definition::impl::world_to_index_and_inverse() const {
    matrix_type trans1 = boost::geometry::strategy::transform::translate_transformer<double, 2, 2>(-_x_coords.front(), -_y_coords.front());
    auto ang = angle_from_x_axis();
    matrix_type rot1 = boost::geometry::strategy::transform::rotate_transformer<boost::geometry::radian, double, 2, 2>(ang);
    // Scale by cell size
    matrix_type scale1 = boost::geometry::strategy::transform::scale_transformer<double, 2, 2>(1 / _column_width, 1 / _row_height);
    matrix_type flip1 = boost::geometry::strategy::transform::matrix_transformer<double, 2, 2>(0, 1, 0, 1, 0, 0, 0, 0, 1);
    // Now lets move according to start index
    matrix_type offset1 = boost::geometry::strategy::transform::translate_transformer<double, 2, 2>(_model_extent.row_extent().lower(), _model_extent.col_extent().lower());

    auto tmp1 = matrix_type(rot1.matrix()* trans1.matrix());
    tmp1 = matrix_type(scale1.matrix() * tmp1.matrix());
    tmp1 = matrix_type(flip1.matrix()* tmp1.matrix());
    tmp1 = matrix_type(offset1.matrix() * tmp1.matrix());

    auto _world_to_index = tmp1;

    auto _index_to_world = boost::geometry::strategy::transform::inverse_transformer<double, 2, 2>(_world_to_index);
    return std::make_pair(_world_to_index, _index_to_world);
}

float gpm_model_definition::column_width_cell_size() const {
    return _impl->_column_width;
}

float gpm_model_definition::row_height_cell_size() const {
    return _impl->_row_height;
}

void gpm_model_definition::set_cell_size(float column_width, float row_height) {
    _impl->_column_width = column_width;
    _impl->_row_height = row_height;
}

float gpm_model_definition::column_width_cell_size_squared() const {
    return _impl->_column_width * _impl->_column_width;
}

float gpm_model_definition::row_height_cell_size_squared() const {
    return _impl->_row_height * _impl->_row_height;
}

float gpm_model_definition::cell_area() const {
    return _impl->_column_width * _impl->_row_height;
}

float gpm_model_definition::model_length_column_dir() const {
    int_extent_2d::extent_type col = _impl->_model_extent.col_extent();
    return _impl->_column_width * (col.upper() - col.lower() - 1);
}

float gpm_model_definition::model_length_row_dir() const {
    int_extent_2d::extent_type row = _impl->_model_extent.row_extent();
    return _impl->_row_height * (row.upper() - row.lower() - 1);
}
}}}
