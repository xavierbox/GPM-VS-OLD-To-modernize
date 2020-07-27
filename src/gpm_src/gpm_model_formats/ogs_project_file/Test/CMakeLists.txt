cmake_minimum_required(VERSION 3.1)

project (ogs_project_writer_test)

set (ogs_project_writer_test_HDRS 

)
set (ogs_project_writer_test_SRCS 
 ogs_project_writer_test.cxx
 )

set(ogs_project_writer_test_FUNCS 
Test_ogs_project_writer_simple
)

create_test_sourcelist(ogs_project_writer_test ogs_project_writer_test_main.cpp ${ogs_project_writer_test_FUNCS})
add_executable(ogs_project_writer_test  ${ogs_project_writer_test_SRCS} ${ogs_project_writer_test_HDRS} ogs_project_writer_test_main.cpp)
target_link_libraries(ogs_project_writer_test   PRIVATE ogs_project_writer Boost::boost)

foreach (loctest ${ogs_project_writer_test_FUNCS})
	add_test(${loctest} ogs_project_writer_test ${loctest})
endforeach(loctest)

if(TARGET Tbb::tbb)
    export_tbb_dll( ogs_project_writer_test) 
endif()