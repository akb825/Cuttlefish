if (NOT GTEST_FOUND OR NOT CUTTLEFISH_BUILD_TESTS)
	return()
endif()

find_package(Threads)

file(GLOB_RECURSE sources *.cpp *.h)
add_executable(cuttlefish_lib_test ${sources})

target_include_directories(cuttlefish_lib_test PRIVATE ${GTEST_INCLUDE_DIRS})
target_link_libraries(cuttlefish_lib_test PRIVATE
	Cuttlefish::lib	
	${GTEST_BOTH_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

cfs_set_folder(cuttlefish_lib_test)
add_test(NAME CuttlefishLibTest COMMAND cuttlefish_lib_test)
