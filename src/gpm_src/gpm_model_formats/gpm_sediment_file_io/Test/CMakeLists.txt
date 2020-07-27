cmake_minimum_required(VERSION 3.1)

project (gpm_sediment_file_io_test)

set (gpm_sediment_file_io_test_SIMPLE_TEST_FUNCS 
  read_sed_litho_test 
  test_new_format
  )


create_test_sourcelist(gpm_sediment_file_io_test gpm_sediment_file_io_test_main.cpp ${gpm_sediment_file_io_test_SIMPLE_TEST_FUNCS})
add_executable(gpm_sediment_file_io_test "")
target_sources( gpm_sediment_file_io_test
PRIVATE
 gpm_sediment_file_io_test_main.cpp
 read_sed_litho_test.cxx 
 test_new_format.cxx
 )

target_link_libraries(gpm_sediment_file_io_test 
PUBLIC gpm_sediment_file_io libacf libutl 
PRIVATE libfil
)


foreach (loctest ${gpm_sediment_file_io_test_SIMPLE_TEST_FUNCS})
   add_test(${loctest} gpm_sediment_file_io_test ${loctest})
endforeach (loctest)

