cmake_minimum_required(VERSION 2.8.7 FATAL_ERROR)

project (vtk_writer)
add_subdirectory(Test)


find_package(VTK COMPONENTS vtkIOXML vtkFiltersCore NO_MODULE CONFIG REQUIRED)


add_library(vtk_writer STATIC "" )

target_sources(vtk_writer
PRIVATE
    vtk_reader_vtu.cxx
    vtk_writer_legacy.cxx
    vtk_writer_vtu.cxx
    vtk_reader_vtu.h
    vtk_writer_legacy.h
    vtk_writer_vtu.h
    gpm_model_converter.h
    gpm_model_converter.cxx
)


set_property(TARGET vtk_writer APPEND
		PROPERTY COMPILE_DEFINITIONS "${VTK_DEFINITIONS}")
set_property(TARGET vtk_writer APPEND
		PROPERTY INCLUDE_DIRECTORIES ${VTK_INCLUDE_DIRS})


target_include_directories(vtk_writer
  PUBLIC
     $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_link_libraries(vtk_writer PRIVATE vtkIOXML vtkFiltersCore libgp libacf libfil libutl Boost::boost)


