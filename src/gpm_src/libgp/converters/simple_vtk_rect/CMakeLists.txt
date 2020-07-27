cmake_minimum_required(VERSION 3.1)


project (simple_vtk_rect)

add_executable(simple_vtk_rect "")
target_sources(simple_vtk_rect
PRIVATE
   simple_vtk_rect.cxx
)
target_link_libraries(simple_vtk_rect PRIVATE libgp libacf libutl)

