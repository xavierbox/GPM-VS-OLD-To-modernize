cmake_minimum_required(VERSION 3.1)

project(gpm_plugin_api_handler)
add_library(gpm_plugin_api_handler STATIC "")
target_sources(gpm_plugin_api_handler
PRIVATE
   gpm_plugin_holder.cxx
   gpm_plugin_holder.h
   gpm_plugin_mapper.cxx
   gpm_plugin_mapper.h
 )

target_include_directories(gpm_plugin_api_handler 
PUBLIC
     $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

# Thsi depenedency to libgp is on sediments only, and should be broken JT
target_link_libraries(gpm_plugin_api_handler
PUBLIC gpm_plugin_description libgp lib_adt libutl
)

