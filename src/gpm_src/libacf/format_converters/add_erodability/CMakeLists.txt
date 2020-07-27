cmake_minimum_required(VERSION 3.1)

project (map_add_erodability)

add_executable(map_add_erodability add_erodability.cxx)

target_link_libraries(map_add_erodability gpm_control_file_io libacf libgp)
