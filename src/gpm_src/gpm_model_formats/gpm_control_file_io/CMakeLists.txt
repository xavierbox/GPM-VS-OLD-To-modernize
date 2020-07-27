cmake_minimum_required(VERSION 3.1)

project (gpm_control_file_io)

add_subdirectory(Test)

file(READ ${CMAKE_CURRENT_SOURCE_DIR}/gpm_control_file_io_schema.json GPM_CONTROL_FILE_IO_SCHEMA)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/gpm_control_file_io_sc.cxx.in" "${CMAKE_CURRENT_BINARY_DIR}/gpm_control_file_io_sc.cxx" @ONLY)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/gpm_control_file_io_schema.json)

    
add_library(gpm_control_file_io STATIC "")

target_sources(gpm_control_file_io
PRIVATE
  gpm_control_file_io.cxx
gpm_control_file_io.h
ssm_enum_def.h
ssmstc_descr.cxx
ssmstc_descr.h
${CMAKE_CURRENT_BINARY_DIR}/gpm_control_file_io_sc.cxx
)

target_link_libraries(gpm_control_file_io 
   PUBLIC libgp libacf libutl lib_adt Boost::boost)

target_include_directories(gpm_control_file_io
  PUBLIC
     $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

set_property(TARGET gpm_control_file_io PROPERTY POSITION_INDEPENDENT_CODE ON)
