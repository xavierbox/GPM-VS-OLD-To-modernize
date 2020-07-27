cmake_minimum_required(VERSION 3.1)

project ( lib_adt_test)

#set (LIB_ADT_TEST_HDRS 
#)
set (LIB_ADT_TEST_SRCS 
 int_extent_2d_test.cxx 
 int_extent_1d_test.cxx 
 array_2d_test.cxx 
 array_3d_test.cxx
 array_3d_vector_test.cxx 
 gpm_1d_interpolator_test.cxx
 model_transformation_test.cxx
 gpm_2dvector_tie_test.cxx
 function_2d_test.cxx
 property_tree_test.cxx
)

set( LIB_ADT_TEST_FUNCS 
int_extent_2d_test
int_extent_1d_test
int_extent_2d_difference_test
int_extent_1d_iterator_test
extent_test
array_2d_test 
array_3d_test
array_3d_vector_test 
gpm_1d_ordering_test_failed gpm_1d_ordering_test
gpm_1d_integration_test gpm_1d_integration_test_clipped
gpm_1d_interpolator_diff_test
model_transformation_test
gpm_2dvector_tie_origin_test
gpm_2dvector_tie_off_origin_test
function_2d_test
array_2d_multi_span_test
model_transformation_qvm_test
property_tree_simple_test
)

create_test_sourcelist(lib_adt_test lib_adt_test_main.cpp ${LIB_ADT_TEST_FUNCS})
add_executable(lib_adt_test ${LIB_ADT_TEST_SRCS} lib_adt_test_main.cpp)
target_link_libraries(lib_adt_test lib_adt Boost::boost)

foreach (loctest ${LIB_ADT_TEST_FUNCS}) 
	add_test(${loctest} lib_adt_test ${loctest})
endforeach(loctest)
