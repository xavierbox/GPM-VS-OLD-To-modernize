cmake_minimum_required(VERSION 3.1)

project (gpm_control_file_io_test)


set (LIBACF_SIMPLE_TEST_FUNCS 
  read_control_file_test 
  read_control_file_test_iso 
  read_control_file_test_sim
  )



create_test_sourcelist(gpm_control_file_io_test gpm_control_file_io_test_main.cpp ${LIBACF_SIMPLE_TEST_FUNCS})
add_executable(gpm_control_file_io_test "")
target_sources( gpm_control_file_io_test
PRIVATE
 gpm_control_file_io_test_main.cpp
 control_file_test.cxx 
 )

target_link_libraries(gpm_control_file_io_test 
PUBLIC gpm_control_file_io libacf libutl 
PRIVATE libfil
)

foreach (loctest ${LIBACF_SIMPLE_TEST_FUNCS})
   add_test(${loctest} gpm_control_file_io_test ${loctest})
endforeach (loctest)

