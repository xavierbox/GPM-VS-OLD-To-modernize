cmake_minimum_required(VERSION 3.1)

project ( libfil_test)

set (LIBFIL_TEST_SRCS 
 trim_path_test.cxx)

set (LIBFIL_TEST_FUNCS trim_path_test find_directory_test)

create_test_sourcelist(libfil_test libfil_test_main.cpp ${LIBFIL_TEST_FUNCS})

add_executable(libfil_test ${LIBFIL_TEST_SRCS} libfil_test_main.cpp)
target_link_libraries(libfil_test libfil Boost::boost)

foreach (loctest ${LIBFIL_TEST_FUNCS})
	add_test(${loctest} libfil_test ${loctest})
endforeach(loctest)
