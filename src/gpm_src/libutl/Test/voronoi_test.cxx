// -- Schlumberger Private --

#include <boost/polygon/voronoi.hpp>
using boost::polygon::voronoi_builder;
using boost::polygon::voronoi_diagram;

int iterate_primary_edges2(const voronoi_diagram<double>& vd) {
    int result = 0;
    for (voronoi_diagram<double>::const_cell_iterator it = vd.cells().begin();
         it != vd.cells().end(); ++it) {
        const voronoi_diagram<double>::cell_type& cell = *it;
        const voronoi_diagram<double>::edge_type* edge = cell.incident_edge();
        // This is convenient way to iterate edges around Voronoi cell.
        do {
            if (edge->is_primary()) {
                ++result;
            }
            edge = edge->next();
        }
        while (edge != cell.incident_edge());
    }
    return result;
}

int iterate_primary_edges3(const voronoi_diagram<double>& vd) {
    int result = 0;
    for (voronoi_diagram<double>::const_vertex_iterator it = vd.vertices().begin();
         it != vd.vertices().end(); ++it) {
        const voronoi_diagram<double>::vertex_type& vertex = *it;
        const voronoi_diagram<double>::edge_type* edge = vertex.incident_edge();
        // This is convenient way to iterate edges around Voronoi vertex.
        do {
            if (edge->is_primary())
                ++result;
            edge = edge->rot_next();
        }
        while (edge != vertex.incident_edge());
    }
    return result;
}

int voronoi_test(int argc, char* argv[]) {
    std::vector<boost::polygon::point_data<int>> points;
    points.push_back(boost::polygon::point_data<int>(0, 0));
    points.push_back(boost::polygon::point_data<int>(1, 0));
    points.push_back(boost::polygon::point_data<int>(2, 0));
    points.push_back(boost::polygon::point_data<int>(0, 1));
    points.push_back(boost::polygon::point_data<int>(1, 1));
    points.push_back(boost::polygon::point_data<int>(2, 1));

    voronoi_diagram<double> vd;
    construct_voronoi(points.begin(), points.end(), &vd);
    iterate_primary_edges3(vd);
    return 0;
}
