cmake_minimum_required(VERSION 3.1)

project (gpm_model_data_io)
#add_subdirectory(Test)

    
add_library(gpm_model_data_io STATIC "")

target_sources(gpm_model_data_io
PRIVATE
gpm_surface_data_io.cxx
gpm_surface_data_io.h
gpm_surface_data_stc.h
gpm_surface_data_enum_def.h
)

target_link_libraries(gpm_model_data_io 
   PRIVATE libacf
   PUBLIC libutl lib_adt Boost::boost)

target_include_directories(gpm_model_data_io
  PUBLIC
     $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

set_property(TARGET gpm_model_data_io PROPERTY POSITION_INDEPENDENT_CODE ON)
