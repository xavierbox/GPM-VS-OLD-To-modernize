cmake_minimum_required(VERSION 3.1)

project (map_io_cnvrt)

set (SRCS 
    format_converter.cxx)

add_executable(map_io_cnvrt ${SRCS})

target_link_libraries(map_io_cnvrt libacf)
