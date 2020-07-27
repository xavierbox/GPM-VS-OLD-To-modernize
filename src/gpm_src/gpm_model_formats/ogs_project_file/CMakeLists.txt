cmake_minimum_required(VERSION 2.8.7 FATAL_ERROR)

project (ogs_project_writer)
add_subdirectory(Test)


find_package(Boost REQUIRED)


add_library(ogs_project_writer STATIC "" )

target_sources(ogs_project_writer
PRIVATE
    ogs_project_writer.cxx
	ogs_project_writer.h
)
target_include_directories(ogs_project_writer
  PUBLIC
     $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_link_libraries(ogs_project_writer PRIVATE Boost::boost)


