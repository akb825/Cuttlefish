file(GLOB_RECURSE sources *.cpp *.h)
add_executable(cuttlefish ${sources})

target_link_libraries(cuttlefish PRIVATE cuttlefish_lib)

cfs_set_folder(cuttlefish tools)

if (CUTTLEFISH_INSTALL)
	install(TARGETS cuttlefish RUNTIME DESTINATION bin COMPONENT tool)
endif()
