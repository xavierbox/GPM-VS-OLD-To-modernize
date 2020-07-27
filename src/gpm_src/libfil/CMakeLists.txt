cmake_minimum_required(VERSION 3.1)

project ( libfil)
add_subdirectory(Test)

set (LIBFIL_SRCS 
# chkfil.cxx
# deslash.cxx
# getdir.cxx
# isdots.cxx
# isspaces.cxx
trim_path.cxx
# sprintf8.cxx
# strup.cxx
 )

set(LIBFIL_HDRS trim_path.h)

add_library(libfil STATIC ${LIBFIL_SRCS} ${LIBFIL_HDRS})
target_link_libraries(libfil 
   PRIVATE Boost::boost)

target_include_directories(libfil 
  PUBLIC
     $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)