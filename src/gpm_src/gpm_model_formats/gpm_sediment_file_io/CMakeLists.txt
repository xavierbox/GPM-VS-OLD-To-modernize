cmake_minimum_required(VERSION 3.1)

project (gpm_sediment_file_io)
add_subdirectory(Test)

file(READ ${CMAKE_CURRENT_SOURCE_DIR}/gpm_sediment_file_io_schema.json GPM_SEDIMENT_FILE_IO_SCHEMA)
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/gpm_sediment_file_io_schema_old.json GPM_SEDIMENT_FILE_IO_SCHEMA_OLD)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/gpm_sediment_file_io_sc.cxx.in" "${CMAKE_CURRENT_BINARY_DIR}/gpm_sediment_file_io_sc.cxx" @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/gpm_sediment_file_io_sc_old.cxx.in" "${CMAKE_CURRENT_BINARY_DIR}/gpm_sediment_file_io_sc_old.cxx" @ONLY)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/gpm_sediment_file_io_schema.json)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/gpm_sediment_file_io_schema_old.json)

    
add_library(gpm_sediment_file_io STATIC "")

target_sources(gpm_sediment_file_io
PRIVATE
  gpm_sediment_file_io.cxx
gpm_sediment_file_io.h
sed_enum_def.h
sedstc_descr.cxx
sedstc_descr.h
${CMAKE_CURRENT_BINARY_DIR}/gpm_sediment_file_io_sc.cxx
gpm_sediment_file_io_sc.h
${CMAKE_CURRENT_BINARY_DIR}/gpm_sediment_file_io_sc_old.cxx
gpm_sediment_file_io_sc_old.h
)

target_link_libraries(gpm_sediment_file_io 
   PRIVATE libacf
   PUBLIC libutl lib_adt Boost::boost)

target_include_directories(gpm_sediment_file_io
  PUBLIC
     $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

set_property(TARGET gpm_sediment_file_io PROPERTY POSITION_INDEPENDENT_CODE ON)
