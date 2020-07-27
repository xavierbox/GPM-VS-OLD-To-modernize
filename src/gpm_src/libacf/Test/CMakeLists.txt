cmake_minimum_required(VERSION 3.1)

project (libacf_test)

ADD_DEFINITIONS(-D_USEHDF5=1)

# add multi test in later 
set (LIBACF_TEST_FUNCS 
	 read_hdf5_test
     read_hdf5_incremental_mode_api_test
#	 write_hdf5_multiple_cycles_test
	 read_hdf5_api_test)

set(LIBACF_TEST_FUNC_DATA 
	sfsed.ctl.out
	seqstrat7beb00d2-ac0e-4829-931a-b93f3f0e48ca.hdf5
#	Case1.out
	singlecol_read.hdf5)

set (LIBACF_SIMPLE_TEST_FUNCS 
  base_param_holder_test 
  general_param_write_json_mapper_test 
  general_param_write_read_json_mapper_test 
  test_json_pretty
  test_json_history
  json_pointer_test
  json_doc_mapper_test
  test_memptr_wrapper
  )



create_test_sourcelist(libacf_test libacf_test_main.cpp ${LIBACF_TEST_FUNCS} ${LIBACF_SIMPLE_TEST_FUNCS})
add_executable(libacf_test "")
target_sources( libacf_test
PRIVATE
 libacf_test_main.cpp
 base_param_holder_test.cxx 
 general_param_write_mapper_test.cxx 
 read_hdf5_test.cxx 
 write_hdf5_multiple_cycles_test.cxx  
 test_utils.cxx 
 test_json.cxx
 test_memptr_wrapper.cxx
 read_hdf5_incremental_read_mode.cxx
 )

target_link_libraries(libacf_test 
PUBLIC libacf libutl 
PRIVATE libfil
)

set(OUTPUT_TEST_DIR ${libacf_test_BINARY_DIR}/output)
file(MAKE_DIRECTORY ${OUTPUT_TEST_DIR})
list(LENGTH LIBACF_TEST_FUNCS count)
math(EXPR count "${count}-1")
foreach(i RANGE ${count})
    list(GET LIBACF_TEST_FUNCS ${i} s1)
	list(GET LIBACF_TEST_FUNC_DATA ${i} s2)
    add_test(${s1} libacf_test ${s1} ${s2} ${PROJECT_SOURCE_DIR}/TestData ${OUTPUT_TEST_DIR})
endforeach()

foreach (loctest ${LIBACF_SIMPLE_TEST_FUNCS})
   add_test(${loctest} libacf_test ${loctest})
endforeach (loctest)

