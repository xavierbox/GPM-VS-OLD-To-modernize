cmake_minimum_required(VERSION 3.1)


project (vtk_top_model)

add_executable(vtk_top_model "")
target_sources(vtk_top_model
PRIVATE
   vtk_top_model.cxx
)
target_link_libraries(vtk_top_model PRIVATE libgp libacf libutl)

