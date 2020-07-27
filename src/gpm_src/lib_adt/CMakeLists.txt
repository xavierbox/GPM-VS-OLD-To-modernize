cmake_minimum_required(VERSION 3.1)

project ( lib_adt)
#add_subdirectory(Test)

set (LIB_ADT_SRCS 
gpm_int_extent_1d.cxx
gpm_int_extent_2d.cxx
gpm_int_extent_3d.cxx
 gpm_1d_interpolator.cxx
 gpm_numerics.cxx
 gpm_model_definition.cxx
 gpm_2dvector_tie_array.cxx
 gpm_extent_tie_helper.cxx
 )

 set (LIB_ADT_HDRS 
 gpm_int_extent_1d.h
gpm_int_extent_2d.h
gpm_int_extent_3d.h
gpm_index_2d.h
#gpm_vbl_array_1d.h
gpm_vbl_array_2d.h
gpm_vbl_array_3d.h
gpm_vbl_vector_array_3d.h
gpm_array_common_defs.h
gpm_function_2d.h
gpm_function_3d.h
gpm_basic_defs.h
 gpm_1d_interpolator.h
 gpm_numerics.h
 gpm_model_definition.h
 gpm_active_indices.h
 gpm_2darray_functions.h
 gpm_simple_stable_vector.h
 gpm_2dvector_tie_array.h
 gpm_extent_tie_helper.h
 gpm_lin_span.h
 gpm_lin_multi_span.h
 )
  

add_library(lib_adt STATIC ${LIB_ADT_SRCS} ${LIB_ADT_HDRS})

#target_link_libraries(lib_adt 
#   PRIVATE Boost::boost)

get_property(COMPILE_DIRS DIRECTORY PROPERTY COMPILE_DEFINITIONS)
#get_property(COMPILE_OPTS DIRECTORY PROPERTY COMPILE_OPTIONS)
#MESSAGE( STATUS "COMPILE_DIRS: " ${COMPILE_DIRS})
#MESSAGE( STATUS "COMPILE_OPTS: " ${COMPILE_OPTS})

target_include_directories(lib_adt 
  PUBLIC
     $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  PRIVATE
    $<BUILD_INTERFACE:${Boost_INCLUDE_DIR}>
)
target_compile_definitions(lib_adt
  PUBLIC
     ${COMPILE_DIRS}
	 )
set_property(TARGET lib_adt PROPERTY POSITION_INDEPENDENT_CODE ON)

export(TARGETS lib_adt FILE lib_adt-exports.cmake)