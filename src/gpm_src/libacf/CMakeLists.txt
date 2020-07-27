cmake_minimum_required(VERSION 3.1)

project ( libacf)
add_subdirectory(Test)

find_package(RapidJSON REQUIRED)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/gpm_acf_GitSHA1.cxx.in" "${CMAKE_CURRENT_BINARY_DIR}/gpm_acf_GitSHA1.cxx" @ONLY)

set( HDF5_USE_STATIC_LIBRARIES 1)
MESSAGE(STATUS "HDF5 directory: " ${HDF5_DIR})
find_package(HDF5 REQUIRED)

set (LIBACF_SRCS 
    acf_base_reader.cxx acf_json_reader.cxx acf_json_doc_reader.cxx
	acf_base_writer.cxx acf_json_writer.cxx acf_format_selector.cxx  
    acf_base_ascii_writer.cxx
	grid_param_mapper.cxx 
	grid_param_reader.cxx  param_wrapper_helper.cxx index_counter.cxx
    general_param_mapper.cxx general_param_string_reader.cxx general_param_file_reader.cxx general_param_write_mapper.cxx
	gpm_format_helper.cxx typedescr_to_funcmapper.cxx 
	mapstc_descr.cxx 
	gpm_model_reader.cxx
	gpm_h5cpp_wrapper.cxx
	gpm_h5_reader_impl.cxx
	gpm_h5_reader_impl_v1.cxx
	gpm_h5_reader_impl_v2.cxx
	gpm_h5_incremental_reader_v1.cxx
	gpm_hdf5_writer_impl.cxx
	gpm_hdf5_writer.cxx
	gpm_json_doc_mapper.cxx
	${CMAKE_CURRENT_BINARY_DIR}/gpm_acf_GitSHA1.cxx
	gpm_param_type_mapper.cxx
	gpm_json_keywords.cxx
	)
	
set (LIBACF_HDRS
    acf_base_io_types.h
    acf_base_reader.h acf_json_reader.h acf_json_doc_reader.h
	acf_base_writer.h acf_json_writer.h acf_format_selector.h  
	acf_base_ascii_writer.h
	acf_base_helper.h
	grid_param_mapper.h 
	grid_param_reader.h param_wrapper_helper.h index_counter.h
    param_utils.h general_param_mapper.h base_param_holder.h general_param_string_reader.h general_param_file_reader.h general_param_write_mapper.h
	gpm_format_helper.h typedescr_to_funcmapper.h
    acl_parse_enum_def.h acl_parse_enum_m.h gpm_type_descriptor.h
    map_enum_def.h mapstc_descr.h
	gpm_model_reader.h
	gpm_h5_reader_impl.h
	gpm_h5_reader_impl_v1.h
	gpm_h5_reader_impl_v2.h
	gpm_h5_incremental_reader_v1.h
	gpm_hdf5_writer_impl.h
	gpm_hdf5_writer.h
	gpm_json_doc_mapper.h
	gpm_acf_GitSHA1.h
	gpm_param_type_mapper.h
	gpm_json_keywords.h
)

set (LIBACF_HDRS ${LIBACF_HDRS} gpm_hdf5_reader.h acf_hdf5_handler.h gpm_h5cpp_wrapper.h gpm_hdf5_writer_impl.h gpm_hdf5_writer.h)
set (LIBACF_SRCS ${LIBACF_SRCS} gpm_hdf5_reader.cxx gpm_h5cpp_wrapper.cxx gpm_hdf5_writer_impl.cxx gpm_hdf5_writer.cxx)
    
add_library(libacf STATIC ${LIBACF_SRCS} ${LIBACF_HDRS})

target_link_libraries(libacf 
   PRIVATE  libutl hdf5::hdf5_hl_cpp-static hdf5::hdf5_cpp-static hdf5::hdf5_hl-static hdf5::hdf5-static
   PUBLIC gpm_data_model lib_adt Boost::boost
   PRIVATE $<$<PLATFORM_ID:UNIX>:"-ldl">
 )

target_include_directories(libacf 
  PUBLIC
     $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	 $<BUILD_INTERFACE:${RAPIDJSON_INCLUDE_DIRS}>
)

set_property(TARGET libacf PROPERTY POSITION_INDEPENDENT_CODE ON)
