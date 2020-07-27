cmake_minimum_required(VERSION 3.1)

project ( libutl)
add_subdirectory(Test)

add_library(libutl STATIC "")
target_sources( libutl
PRIVATE
 gpm_logger.cxx
 gpm_expires.cxx
 gpm_string_utils.cxx
 gpm_version_number.cxx
 gpm_constants.cxx
 gpm_timebomb.cxx
 gpm_logger.h
 gpm_logging_scope.h
 gpm_constants.h
 gpm_expires.h
 gpm_string_utils.h
 gpm_version_number.h
 gpm_timebomb.h
)
target_link_libraries(libutl 
   PRIVATE Boost::boost)

target_include_directories(libutl 
  PUBLIC
     $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
set_property(TARGET libutl PROPERTY POSITION_INDEPENDENT_CODE ON)
