cmake_minimum_required(VERSION 3.1)

project (hdf5_to_vtu)

set (SRCS 
    format_converter.cxx)

add_executable(hdf5_to_vtu ${SRCS})

target_link_libraries(hdf5_to_vtu libacf vtk_writer)
