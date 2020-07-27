cmake_minimum_required(VERSION 3.1)


project (gpm_parm_differ)
set (GPM_PARM_TESTER_SRCS 
gpm_parm_tester.cxx
gpm_semantic_parm_tester.cxx
gpm_test_parameters.cxx

 )

 set (GPM_PARM_TESTER_HDRS 
gpm_parm_tester.h
gpm_semantic_parm_tester.h
gpm_test_parameters.h
 )
  
add_executable(gpm_parm_differ gpm_parm_differ.cxx)
target_link_libraries(gpm_parm_differ PRIVATE gpm_parm_tester libgp libacf libutl)
