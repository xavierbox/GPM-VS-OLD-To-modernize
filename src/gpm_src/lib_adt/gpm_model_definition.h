// -- Schlumberger Private --

#ifndef GPM_MODEL_DEFINITIONS_H
#define GPM_MODEL_DEFINITIONS_H

#include "gpm_int_extent_2d.h"

namespace Slb { namespace Exploration { namespace Gpm {

class gpm_model_definition // Put the model definitions in one place
{
public:
    // The world and index points we need to work with
    struct world_pt {
        float x;
        float y;
    };

    struct index_pt {
        float i;
        float j;
    };

    struct index {
        int_extent_1d::index_type i;
        int_extent_1d::index_type j;
    };
    gpm_model_definition();
    // Rows are in y direction, columns are in x direction
    gpm_model_definition(int_extent_2d::size_type nr, int_extent_2d::size_type nc);
    explicit gpm_model_definition(const int_extent_2d& ext);
    gpm_model_definition(const gpm_model_definition& rhs);
    gpm_model_definition& operator=(const gpm_model_definition& rhs);

    virtual ~gpm_model_definition();
    void set_model_size(int_extent_1d::size_type nr, int_extent_1d::size_type nc);
    void set_model_size(const int_extent_2d& ext);
    // Model size must be set first, so the numbers in x and y are known
    // Y axis goes along the row direction
    // X axis goes along the column direction
    // the vector contains 4 points, starting in origo and moved in a counter clockwise direction
    // X axis is thus from index 0 to index 1
    // Y axis is from index 0 to index 3
    void set_world_coordinates(const std::vector<float>& x, const std::vector<float>& y);
    std::vector<float> get_x_coordinates() const;
    std::vector<float> get_y_coordinates() const;
    index_pt convert_from_world_to_index(const world_pt& pt) const;
    world_pt convert_from_index_to_world(const index_pt& ind) const;
    float column_width_cell_size() const;
    float row_height_cell_size() const;
    float column_width_cell_size_squared() const;
    float row_height_cell_size_squared() const;
    float cell_area() const;
    float model_length_column_dir() const;
    float model_length_row_dir() const;
    const int_extent_2d& model_extent() const;
    void set_cell_size(float column_width, float row_height);
private:
    struct impl;
    std::unique_ptr<impl> _impl;
};
}}}

#endif

