cmake_minimum_required(VERSION 3.1)

project (gpm_time_file_io)
add_subdirectory(Test)

file(READ ${CMAKE_CURRENT_SOURCE_DIR}/gpm_time_file_io_schema.json GPM_TIME_FILE_IO_SCHEMA)
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/gpm_time_file_io_schema_old.json GPM_TIME_FILE_IO_SCHEMA_OLD)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/gpm_time_file_io_sc.cxx.in" "${CMAKE_CURRENT_BINARY_DIR}/gpm_time_file_io_sc.cxx" @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/gpm_time_file_io_sc_old.cxx.in" "${CMAKE_CURRENT_BINARY_DIR}/gpm_time_file_io_sc_old.cxx" @ONLY)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/gpm_time_file_io_schema.json)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/gpm_time_file_io_schema_old.json)

add_library(gpm_time_file_io STATIC "")

target_sources(gpm_time_file_io
PRIVATE
  gpm_time_function_data_io.cxx
gpm_time_function_data_io.h
gpm_time_function_data_enum_def.h
gpm_time_function_data_stc.h
gpm_time_file_io_sc.h
${CMAKE_CURRENT_BINARY_DIR}/gpm_time_file_io_sc.cxx
gpm_time_file_io_sc_old.h
${CMAKE_CURRENT_BINARY_DIR}/gpm_time_file_io_sc_old.cxx
)

target_link_libraries(gpm_time_file_io 
   PRIVATE libacf
   PUBLIC libutl lib_adt Boost::boost)

target_include_directories(gpm_time_file_io
  PUBLIC
     $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

set_property(TARGET gpm_time_file_io PROPERTY POSITION_INDEPENDENT_CODE ON)
