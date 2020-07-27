// -- Schlumberger Private --

#include "gpm_model_definition.h"
#include <boost/geometry/algorithms/transform.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/arithmetic/dot_product.hpp>
#include <boost/geometry/geometries/register/point.hpp>

#include <vector>

BOOST_GEOMETRY_REGISTER_POINT_2D(Slb::Exploration::Gpm::gpm_model_definition::world_pt, float, boost::geometry::cs::cartesian, x, y)

int model_transformation_test(int /*argc*/, char* /*argv*/[]) {
    float x[] = {2,4,3,1};
    float y[] = {1,2,4,3};
    std::vector<float> xf(&x[0], &x[0] + 4), yf(&y[0], &y[0] + 4);
    Slb::Exploration::Gpm::gpm_model_definition def(Slb::Exploration::Gpm::int_extent_2d(1, 6, 2, 9));
    def.set_world_coordinates(xf, yf);
    std::vector<Slb::Exploration::Gpm::gpm_model_definition::world_pt> w, wi;
    std::transform(xf.begin(), xf.end(), yf.begin(), std::back_inserter(w), [](float lx, float ly) { return Slb::Exploration::Gpm::gpm_model_definition::world_pt{ lx, ly }; });
    std::vector<Slb::Exploration::Gpm::gpm_model_definition::index_pt> ind;
    std::transform(w.begin(), w.end(), std::back_inserter(ind), [&def](const Slb::Exploration::Gpm::gpm_model_definition::world_pt& pt) {
                       return def.convert_from_world_to_index(pt);
                   });
    std::transform(ind.begin(), ind.end(), std::back_inserter(wi), [&def](const Slb::Exploration::Gpm::gpm_model_definition::index_pt& pt) {
                       return def.convert_from_index_to_world(pt);
                   });


    auto is_equal = std::equal(w.begin(), w.end(), wi.begin(), [](const Slb::Exploration::Gpm::gpm_model_definition::world_pt& a, const Slb::Exploration::Gpm::gpm_model_definition::world_pt& b) {
                               Slb::Exploration::Gpm::gpm_model_definition::world_pt t1(a);
                                   boost::geometry::subtract_point(t1, b);
                                   auto diff = boost::geometry::dot_product(t1, t1);
                                   return diff < 1e-5;
                               });
    return is_equal ? 0 : 1;
}

#include "boost/qvm/all.hpp"

int model_transformation_qvm_test(int /*argc*/, char* /*argv*/[]) {
	std::vector<float> xf= { 2,4,3,1 };
	std::vector<float> yf= { 1,2,4,3 };;
	typedef boost::qvm::vec<float, 2> vec_2d;
	const vec_2d p0 = { xf.front(),yf.front() };
	const vec_2d p1 = { xf[1],yf[1] };
	const vec_2d p3 = { xf.back(),yf.back() };
	vec_2d dp10 = p1 - p0;
	const vec_2d dp30 = p3 - p0;
	const auto angle= atan2f(boost::qvm::Y(dp10), boost::qvm::X(dp10));
	auto ext = Slb::Exploration::Gpm::int_extent_2d(1, 6, 2, 9);
	// Translate back to origin
	const boost::qvm::mat<float, 3, 3> tr = boost::qvm::translation_mat(-p0);
	// rotate back to x axis
	const boost::qvm::mat<float, 3, 3> rot = boost::qvm::rotz_mat<3,float>(-angle);
	// Scale to index space
    auto col_index = ext.col_extent().size() > 1 ? ext.col_extent().size() - 1 : 1;
    auto row_index = ext.row_extent().size() > 1 ? ext.row_extent().size() - 1 : 1;
	vec_2d ext_size = { static_cast<float>(col_index), static_cast<float>(row_index) };
	boost::qvm::vec<float, 3> scale_vec = { boost::qvm::X(ext_size) / boost::qvm::mag(dp10), boost::qvm::Y(ext_size) / boost::qvm::mag(dp30),1 };
	const boost::qvm::mat<float, 3, 3> scale = boost::qvm::diag_mat(scale_vec);
    // Flip coords to i,j
	const boost::qvm::mat<float, 3, 3> flip = { 0, 1, 0, 1, 0, 0, 0, 0, 1 };
	vec_2d offset_index = { static_cast<float>(ext.row_extent().lower()), static_cast<float>(ext.col_extent().lower()) };
	// Offset back to index start
	const boost::qvm::mat<float, 3, 3> btr = boost::qvm::translation_mat(offset_index);
	boost::qvm::mat<float, 3, 3> w_to_i = btr*flip*scale*rot*tr;
	boost::qvm::mat<float, 3, 3> i_to_w = boost::qvm::inverse(w_to_i);

	std::vector<vec_2d> w, wi;
	std::transform(xf.begin(), xf.end(), yf.begin(), std::back_inserter(w), [](float lx, float ly) { return vec_2d{ lx, ly }; });
	std::vector<vec_2d> ind;
	std::transform(w.begin(), w.end(), std::back_inserter(ind), [&w_to_i](const vec_2d& pt)
	{
		const vec_2d loc_pt = boost::qvm::XY(w_to_i * boost::qvm::XY1(pt));
		return loc_pt;
	});

	std::transform(ind.begin(), ind.end(), std::back_inserter(wi), [&i_to_w](const vec_2d& pt)
	{
		const vec_2d loc_pt = boost::qvm::XY(i_to_w * boost::qvm::XY1(pt));
		return loc_pt;
	});

	const auto is_equal = std::equal(w.begin(), w.end(), wi.begin(), [](const vec_2d& a, const vec_2d& b)
	{
		const vec_2d diff_vec = a -b;
		const auto diff = boost::qvm::dot(diff_vec, diff_vec);
		return diff < 1e-5;
	});
	return is_equal ? 0 : 1;
}