cmake_minimum_required(VERSION 3.1)

project (map_fix_slope)

add_executable(map_fix_slope fix_slope.cxx)

target_link_libraries(map_fix_slope gpm_control_file_io libacf libgp)
