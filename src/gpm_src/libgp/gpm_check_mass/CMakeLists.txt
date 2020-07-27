cmake_minimum_required(VERSION 3.1)


project (gpm_check_mass)
  
add_executable(gpm_check_mass gpm_check_mass.cxx)
target_link_libraries(gpm_check_mass PRIVATE libgp libacf libutl)
