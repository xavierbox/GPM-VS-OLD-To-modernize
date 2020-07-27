cmake_minimum_required(VERSION 2.8)

project ( libgp_test)

set (LIBGP_TEST_HDRS 
)
set (LIBGP_TEST_SRCS 
 grid_operations_test.cxx timeline_test.cxx
 gpm_porosity_test.cxx 
 mannings_test.cxx
 )

set( LIBGP_TEST_FUNCS 
grid_filter_test grid_mult_functor_test grid_mult_test grid_mult2_test timeline_test erodif_test 
#timeline_extremal_test 
gpm_porosity_test gpm_permeability_test 
gpm_porosity_test_full
timeline_infinite_test
time_event_tests
blas_invert_test
mannings_test
)
create_test_sourcelist(libgp_test libgp_test_main.cpp ${LIBGP_TEST_FUNCS})
add_executable(libgp_test ${LIBGP_TEST_SRCS} ${LIBGP_TEST_HDRS} libgp_test_main.cpp)
target_link_libraries(libgp_test libgp lib_adt)

foreach (loctest ${LIBGP_TEST_FUNCS})
	add_test(${loctest} libgp_test ${loctest})
endforeach(loctest)