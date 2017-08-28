if (NOT GTEST_FOUND OR NOT CUTTLEFISH_BUILD_TESTS)
	return()
endif()

file(GLOB_RECURSE sources *.cpp *.h)
add_executable(cuttlefish_test ${sources})

target_include_directories(cuttlefish_test PRIVATE ${GTEST_INCLUDE_DIRS})
target_link_libraries(cuttlefish_test PRIVATE
	cuttlefish_lib
	${GTEST_BOTH_LIBRARIES})

cfs_set_folder(cuttlefish_test tests)
add_test(NAME CuttlefishTest COMMAND cuttlefish_test)
