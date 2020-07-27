cmake_minimum_required(VERSION 3.1)


project (gpm_parm_tester)
set (GPM_PARM_TESTER_SRCS 
gpm_semantic_parm_tester.cxx
gpm_test_parameters.cxx

 )

 set (GPM_PARM_TESTER_HDRS 
gpm_semantic_parm_tester.h
gpm_test_parameters.h
 )
  
add_library(gpm_parm_tester STATIC ${GPM_PARM_TESTER_SRCS} ${GPM_PARM_TESTER_HDRS})
target_link_libraries(gpm_parm_tester PRIVATE gpm_control_file_io libgp libacf libutl)
target_include_directories(gpm_parm_tester 
  PUBLIC
     $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
