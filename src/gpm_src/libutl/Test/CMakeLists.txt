cmake_minimum_required(VERSION 3.1)

project ( libutl_test)

set( LIBUTL_TEST_FUNCS 
string_test  
iterator_test 
iterator_1_test 
polyline_test 
rectangle_union_test 
polyline_buffer_test
voronoi_test 
interval_union_test
multi_index_test
nextafter_test
logger_test
)

create_test_sourcelist(libutl_test libutl_test_main.cpp ${LIBUTL_TEST_FUNCS})

add_executable(libutl_test "")
target_sources( libutl_test
PRIVATE
 libutl_test_main.cpp
 extent_test.cxx 
 string_test.cxx 
 polyline_test.cxx 
 voronoi_test.cxx 
 multi_index_test.cxx
 nextafter_test.cxx
 logger_test.cxx
#calc_utree_ast_test.cxx
)
target_link_libraries(libutl_test libutl lib_adt Boost::boost)

foreach (loctest ${LIBUTL_TEST_FUNCS})
	add_test(${loctest} libutl_test ${loctest})
endforeach(loctest)
