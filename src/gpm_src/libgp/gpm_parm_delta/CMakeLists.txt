cmake_minimum_required(VERSION 3.1)


project (gpm_parm_delta)
  
add_executable(gpm_parm_delta gpm_parm_delta.cxx)
target_link_libraries(gpm_parm_delta PRIVATE libgp libacf libutl)
