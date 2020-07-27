cmake_minimum_required(VERSION 3.1)

project ( libgp)
add_subdirectory(Test)

  
add_library(libgp STATIC "")
target_sources(libgp
PRIVATE
  dirgrad.cxx
  gpm_compute_interval.h 
  gpm_find_sediment_intervals.h
  gpm_water_vectors.h
  gradn.cxx
  gridmult.cxx
  mann.cxx
  nod2tie.cxx
  normvec.cxx
  gpm_compute_interval.cxx
  gpm_diffusion_vector.cxx
  gpm_diffusion_vector.h
  gpm_find_sediment_intervals.cxx
  gpm_parm_helper_methods.h
  gpm_parm_helper_methods.cxx
  gpm_source_indexes.cxx
  gpm_source_indexes.h 
  gpm_water_vectors.cxx
  gpm_write_vtk_file.cxx
  gpm_write_vtk_file.h
  libgp.h
  lit2pet.cxx
  mann.h 
  gpm_program_options.cxx
  gpm_program_options.h
  gpm_pars_helper_methods.cxx
  gpm_pars_helper_methods.h
  gpm_codes.h
  gpm_water_transport.h
  gpm_water_transport.cxx
)

if(TARGET Armadillo)
    target_sources(libgp PRIVATE gpm_arma.h)
endif()

target_include_directories(libgp 
  PUBLIC
     $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
#Move gpm_data_model to private when pars, parc and parm is moved into data_model JT
target_link_libraries(libgp 
PRIVATE libacf
PUBLIC gpm_data_model lib_adt libutl Boost::boost)

if(TARGET Armadillo)
    target_link_libraries(libgp PUBLIC Armadillo)
endif()

if(TARGET Tbb::tbb)
    target_link_libraries(libgp PUBLIC Tbb::tbb)
endif()



