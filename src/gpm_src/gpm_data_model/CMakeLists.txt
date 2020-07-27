cmake_minimum_required(VERSION 3.1)

project(gpm_data_model)

add_library(gpm_data_model STATIC "")
target_sources(gpm_data_model 
PRIVATE
mapstc.h
sedstc.h
ssmstc.h
gpm_indprop_indexes.cxx
gpm_indprop_indexes.h
gpm_pars_methods.cxx
gpm_pars_methods.h
gpm_parm_methods.cxx
gpm_parm_methods.h
findind.cxx
findint.cxx
gpm_sediment_model.cxx
gpm_sediment_model.h
parm_prop_descr.cxx
parm_prop_descr.h
parm_prop_enum_def.h
gpm_grid_creation.cxx
gpm_indprop_indexes_helper.cxx
gpm_indprop_indexes_helper.h
gpm_parc_helper_methods.h
gpm_parc_helper_methods.cxx
)
target_include_directories(gpm_data_model 
PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_link_libraries(gpm_data_model 
   PUBLIC lib_adt libutl Boost::boost)

set_property(TARGET gpm_data_model PROPERTY POSITION_INDEPENDENT_CODE ON)
