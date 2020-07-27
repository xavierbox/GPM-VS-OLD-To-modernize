cmake_minimum_required(VERSION 3.1)
project( gpm_base_input_reader)

add_subdirectory(gpm_control_file_io)
add_subdirectory(gpm_sediment_file_io)
add_subdirectory(gpm_model_data_io)
add_subdirectory(gpm_time_file_io)
add_subdirectory(vtk_writer)
add_subdirectory(ogs_project_file)

add_library(gpm_base_input_reader STATIC "")

target_sources(gpm_base_input_reader
PRIVATE
  gpm_base_input_reader.cxx
gpm_base_input_reader.h
)

target_link_libraries(gpm_base_input_reader 
   PRIVATE gpm_control_file_io gpm_time_file_io gpm_sediment_file_io libacf libgp
   PUBLIC libutl lib_adt Boost::boost)

target_include_directories(gpm_base_input_reader
  PUBLIC
     $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

set_property(TARGET gpm_base_input_reader PROPERTY POSITION_INDEPENDENT_CODE ON)