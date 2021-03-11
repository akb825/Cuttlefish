if (NOT GTEST_FOUND OR NOT CUTTLEFISH_BUILD_TESTS)
	return()
endif()

find_package(Threads)

file(GLOB_RECURSE sources *.cpp *.h)
add_executable(cuttlefish_test ${sources})

target_include_directories(cuttlefish_test PRIVATE ${GTEST_INCLUDE_DIRS})
target_link_libraries(cuttlefish_test PRIVATE
	cuttlefish_lib
	${GTEST_BOTH_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

if (APPLE AND CUTTLEFISH_BUILD_PVRTC AND NOT CUTTLEFISH_SHARED AND PVRTEXLIB_FOUND)
	cfs_fixup_mac_dep(cuttlefish_test "@executable_path/../lib/libPVRTexLib.dylib"
		"@rpath/libPVRTexLib.dylib")
endif()

cfs_set_folder(cuttlefish_test tests)
add_test(NAME CuttlefishTest COMMAND cuttlefish_test)
