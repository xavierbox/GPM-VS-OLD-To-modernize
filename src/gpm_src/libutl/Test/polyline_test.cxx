// -- Schlumberger Private --

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/icl/interval_set.hpp>

typedef boost::geometry::model::d2::point_xy<double> boost_point;
typedef boost::geometry::model::polygon<boost_point, false> boost_polygon;
typedef boost::geometry::model::linestring<boost_point> boost_line;
typedef boost::geometry::model::multi_linestring<boost_point> boost_multiline;

int line_test(int argc, char* argv[]) {

    boost_line b1;
    b1.push_back(boost_point(0, 0));
    b1.push_back(boost_point(1, 0));
    boost_line b2;
    b2.push_back(boost_point(0.5, 0));
    b2.push_back(boost_point(1.5, 0));
    std::vector<boost_line> output;
    boost::geometry::intersection(b1, b2, output);
    return 0;
}

int polyline_test(int argc, char* argv[]) {

    boost_polygon b1;
    b1.outer().push_back(boost_point(0, 0));
    b1.outer().push_back(boost_point(1, 1));
    boost::geometry::append(b1.outer(), boost_point(2, 0));
    boost::geometry::append(b1.outer(), boost_point(2, 2));
    boost::geometry::append(b1.outer(), boost_point(0, 2));
    boost::geometry::append(b1.outer(), boost_point(0, 0));
    boost_line b2;
    b2.push_back(boost_point(-1, 0.5));
    b2.push_back(boost_point(3, 0.5));
    std::vector<boost_line> output;
    boost::geometry::intersection(b1, b2, output);
    return 0;
}

int polyline_buffer_test(int argc, char* argv[]) {

	boost_polygon b1;
	b1.outer().push_back(boost_point(0, 0));
	b1.outer().push_back(boost_point(1, 1));
	boost::geometry::append(b1.outer(), boost_point(2, 0));
	boost::geometry::append(b1.outer(), boost_point(2, 2));
	boost::geometry::append(b1.outer(), boost_point(0, 2));
	boost::geometry::append(b1.outer(), boost_point(0, 0));
	boost_polygon b2;
	// Not implemented yet
	//boost::geometry::buffer(b1, b2, 1.0);
	return 0;
}

int rectangle_union_test(int argc, char* argv[]) {
    boost_polygon ba, bb, bc;
    boost::geometry::append(ba.outer(), boost_point(0, 0));
    boost::geometry::append(ba.outer(), boost_point(1, 0));
    boost::geometry::append(ba.outer(), boost_point(1, 1));
    boost::geometry::append(ba.outer(), boost_point(0, 1));
    boost::geometry::append(ba.outer(), boost_point(0, 0));
    boost::geometry::append(bb.outer(), boost_point(0 + 2, 0));
    boost::geometry::append(bb.outer(), boost_point(1 + 2, 0));
    boost::geometry::append(bb.outer(), boost_point(1 + 2, 1));
    boost::geometry::append(bb.outer(), boost_point(0 + 2, 1));
    boost::geometry::append(bb.outer(), boost_point(0 + 2, 0));
    boost::geometry::append(bc.outer(), boost_point(0 + 1, 0));
    boost::geometry::append(bc.outer(), boost_point(1 + 1, 0));
    boost::geometry::append(bc.outer(), boost_point(1 + 1, 1));
    boost::geometry::append(bc.outer(), boost_point(0 + 1, 1));
    boost::geometry::append(bc.outer(), boost_point(0 + 1, 0));
    std::vector<boost_polygon> output1, output2;
    boost::geometry::union_(ba, bb, output1);
    auto res = boost::geometry::distance(ba, bb);
    boost::geometry::union_(bb, bc, output2);
    int test = 0;
    return test;
}

int interval_union_test(int argc, char* argv[])
{
	boost::icl::interval_set<int> t1;
	t1 += boost::icl::discrete_interval<int>::right_open(0, 1);
    t1 += boost::icl::discrete_interval<int>::right_open(1, 2);
    t1 += boost::icl::discrete_interval<int>::right_open(10, 11);
    t1 += boost::icl::discrete_interval<int>::right_open(2, 10);
	return 0;
}