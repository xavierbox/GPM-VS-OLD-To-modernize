cmake_minimum_required(VERSION 3.1)

project (vtk_writer_vtu_test)

find_package(VTK COMPONENTS vtkIOXML vtkFiltersCore NO_MODULE CONFIG REQUIRED)

set (VTK_WRITER_VTU_TEST_HDRS 

)
set (VTK_WRITER_VTU_TEST_SRCS 
 vtk_writer_vtu_test.cxx
 vtk_writer_vtu_unit_tests.cxx
 )

set(VTK_WRITER_VTU_TEST_FUNCS 
Test_vtu_writer_simple_no_props
Test_vtu_writer_simple_with_props
Test_vtu_writer_reader_consistency
Test_vtu_writer_simple_wedge_pyramid
Test_vtu_writer_wedge_pyramid
Test_vtu_writer_all_wedges
Test_vtu_configs
Test_wedge_configs
Test_tetra_configs
)

SET(VTK_WRITER_VTU_TEST_DEP_LIBS vtk_writer vtkIOXML vtkFiltersCore libacf libfil libutl libgp Boost::boost)


create_test_sourcelist(vtk_writer_vtu_test vtk_writer_vtu_test_main.cpp ${VTK_WRITER_VTU_TEST_FUNCS})
add_executable(vtk_writer_vtu_test ${VTK_WRITER_VTU_TEST_SRCS} ${VTK_WRITER_VTU_TEST_HDRS} vtk_writer_vtu_test_main.cpp)
#target_link_libraries(vtk_writer_vtu_test ${VTK_WRITER_VTU_TEST_DEP_LIBS})
target_link_libraries(vtk_writer_vtu_test  PRIVATE vtk_writer vtkIOXML vtkFiltersCore libacf libfil libutl libgp Boost::boost)

foreach (loctest ${VTK_WRITER_VTU_TEST_FUNCS})
	add_test(${loctest} vtk_writer_vtu_test ${loctest})
endforeach(loctest)

if(TARGET Tbb::tbb)
    export_tbb_dll( vtk_writer_vtu_test) 
endif()