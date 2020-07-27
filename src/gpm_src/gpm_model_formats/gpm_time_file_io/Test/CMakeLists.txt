cmake_minimum_required(VERSION 3.1)

project (gpm_time_file_io_test)

set (gpm_time_file_io_test_SIMPLE_TEST_FUNCS 
  read_time_data_file_test 
  check_time_data_versions
  )



create_test_sourcelist(gpm_time_file_io_test gpm_time_file_io_test_main.cpp ${gpm_time_file_io_test_SIMPLE_TEST_FUNCS})
add_executable(gpm_time_file_io_test "")
target_sources( gpm_time_file_io_test
PRIVATE
 gpm_time_file_io_test_main.cpp
 time_data_file_test.cxx 
 time_data_version_test.cxx
 )

target_link_libraries(gpm_time_file_io_test 
PUBLIC gpm_time_file_io libacf libutl 
PRIVATE libfil
)


foreach (loctest ${gpm_time_file_io_test_SIMPLE_TEST_FUNCS})
   add_test(${loctest} gpm_time_file_io_test ${loctest})
endforeach (loctest)

