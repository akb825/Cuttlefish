# Introduction

Cuttlefish is a texture conversion library and tool. It is designed to be able to handle nearly any kind of input image and output texture encoding, includig S3TC, ETC, and ASTC compression formats.

# Dependencies

The following software is required to build Cuttlefish:

* [cmake](https://cmake.org/) 3.0.2 or later
* [FreeImage](http://freeimage.sourceforge.net/) (required, included as a submodule)
* [nvidia-texture-tools](https://github.com/castano/nvidia-texture-tools) (optional, included as a submodule)
* [etc2comp](https://github.com/google/etc2comp) (optional, included as a submodule)
* [astc-encoder](https://github.com/ARM-software/astc-encoder) (optional, included as a submodule)
* [PVRTexTools](https://community.imgtec.com/developers/powervr/tools/pvrtextool/) (optional)
* [doxygen](http://www.stack.nl/~dimitri/doxygen/) (optional)
* [gtest](https://github.com/google/googletest) (optional)

The submodules can be downloaded by running the commands

	Cuttlefish$ git submodule init
	Cuttlefish$ git submodule update

# Platforms

Cuttlefish has been built for and tested on the following platforms:

* Linux (GCC and LLVM clang)
* Windows (requires Visual Studio 2015 or later)
* Mac OS X

# Building

Building is done with CMake. To build a release build, execute the following commands:

	Cuttlefish$ mkdir build
	Cuttlefish$ cd build
	Cuttlefish/build$ cmake .. -DCMAKE_BUILD_TYPE=Release
	Cuttlefish/build$ make

The tests can be run by running the command:

	Cuttlefish/build$ ctest

The following options may be used when running cmake:

## Compile Options:

* `-DCMAKE_BUILD_TYPE=Debug|Release`: Building in `Debug` or `Release`. This should always be specified.
* `-DCMAKE_INSTALL_PREFIX=path`: Sets the path to install to when running make install.
* `-DCUTTLEFISH_SHARED=ON|OFF`: Set to `ON` to build with shared libraries, `OFF` to build with static libraries. Default is `ON`.

## Enabled Builds

* `-DCUTTLEFISH_BUILD_TESTS=ON|OFF`: Set to `ON` to build the unit tests. `gtest` must also be found in order to build the unit tests. Defaults to `ON`.
* `-DCUTTLEFISH_BUILD_TOOL=ON|OFF`: Set to `ON` to build the tool. Defaults to `ON`.
* `-DCUTTLEFISH_FORCE_INTERNAL_FREEIMAGE=ON|OFF`: Set to `ON` to force building FreeImage internally even if a system version is found. Defaults to `OFF`.
* `-DCUTTLEFISH_BUILD_S3TC=ON|OFF`: Set to `ON` to build S3TC texture compression support. Defaults to `ON`.
* `-DCUTTLEFISH_BUILD_ETC=ON|OFF`: Set to `ON` to build ETC texture compression support. Defaults to `ON`.
* `-DCUTTLEFISH_BUILD_ASTC=ON|OFF`: Set to `ON` to build ASTC texture compression support. Defaults to `ON`.
* `-DCUTTLEFISH_BUILD_PVRTC=ON|OFF`: Set to `ON` to build PVRTC texture compression support. Defaults to `ON`. If the PVRTexTool library isn't found, support will be disabled.

## Miscellaneous Options:

* `-DCUTTLEFISH_OUTPUT_DIR=directory`: The folder to place the output files. This may be explicitly left empty in order to output to the defaults used by cmake, but this may prevent tests and executables from running properly when `CUTTLEFISH_SHARED` is set to `ON`. Defaults to `${CMAKE_BINARY_DIR}/output`.
* `-DCUTTLEFISH_EXPORTS_DIR=directory`: The folder to place the cmake exports when building. This directory can be added to the module path when embedding in other projects to be able to use the `library_find()` cmake function. Defaults to `${CMAKE_BINARY_DIR}/cmake`.
* `-DCUTTLEFISH_ROOT_FOLDER=folder`: The root folder for the projects in IDEs that support them. (e.g. Visual Studio or XCode) This is useful if embedding MSL in another project. Defaults to Cuttlefish.
* `-DCUTTLEFISH_INSTALL=ON|OFF`: Allow installation for Cuttlefish components. This can be useful when embedding in other projects to prevent installations from including Cuttlefish. For example, when statically linking into a shared library. Default is `ON`.

Once you have built and installed Cuttlefish, and have added the `lib/cmake/cuttlefish` directory to `CMAKE_PREFIX_PATH`, you can find the various modules with the `find_package()` CMake function. For example:

    find_package(Cuttlefish)

Libraries and include directories can be found through the `Cuttlefish_LIBRARIES` and `Cuttlefish_INCLUDE_DIRS` CMake variables.

# Further Documentation

* [Library](lib/README.md)
* [Tool](tool/README.md)
