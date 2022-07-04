# Introduction

Cuttlefish is a texture conversion library and tool. A command line tool is provided for most texture conversion needs, such as running manually or part of an asset conversion pipeline. The library may be used to integrate more advanced texture generation within other software.

Features include:

* Load almost any image format. ([all supported by FreeImage](http://freeimage.sourceforge.net/features.html))
* Perform simple operations on input images such as resizing, flipping and rotating, and generating normalmaps.
* Create 1D, 2D, 3D, and cube textures, as well as texture arrays.
* Generate mipmaps.
* Convert to most formats supported by the GPU, including:
	* Most standard uncompressed formats. (normalized integers, non-normalized integers, floating point, etc.)
	* S3TC formats (BC/DXT)
	* ETC
	* ASTC
	* PVRTC
* Save the output texture in DDS, KTX, or PVR format.

[![Build Status](https://dev.azure.com/akb825/DeepSea/_apis/build/status/akb825.Cuttlefish?branchName=master)](https://dev.azure.com/akb825/DeepSea/_build/latest?definitionId=1&branchName=master)

# Dependencies

The following software is required to build Cuttlefish:

* [CMake](https://cmake.org/) 3.1 or later
* [FreeImage](http://freeimage.sourceforge.net/) (required, included as a submodule)
* [GLM](https://glm.g-truc.net/0.9.9/index.html) (required, included as a submodule)
* [squish](https://sourceforge.net/projects/libsquish/) (optional, included as a submodule)
* [Compressonator](https://github.com/GPUOpen-Tools/compressonator) (optional, included as a submodule)
* [bc7enc_rdo](https://github.com/richgel999/bc7enc_rdo) (optional, included as a submodule)
* [ispc_texcomp](https://github.com/GameTechDev/ISPCTextureCompressor) (optional, included as a submodule)
* [etc2comp](https://github.com/google/etc2comp) (optional, included as a submodule)
* [astc-encoder](https://github.com/ARM-software/astc-encoder) (optional, included as a submodule)
* [PVRTexTool](https://developer.imaginationtech.com/pvrtextool/) (optional, included as a submodule)
* [doxygen](https://doxygen.nl/) (optional)
* [gtest](https://github.com/google/googletest) (optional)

The submodules can be downloaded by running the command

	Cuttlefish$ git submodule update --init

When using the BC6H and BC7 encoders, it's highly recommended to install the [ISPC](https://ispc.github.io) compiler. This will use higher quality encoders that are also faster compared to the fallback used when ISP isn't available.

> **Note:** Use the `CUTTLEFISH_ISPC_PATH` CMake variable when ISPC isn't visible on the system `PATH`.

# Platforms

Cuttlefish has been built for and tested on the following platforms:

* Linux (GCC and LLVM clang)
* Windows (requires Visual Studio 2015 or later)
* macOS

# Building and Installing

[CMake](https://cmake.org/) is used as the build system. The way to invoke CMake differs for different platforms.

## Linux/macOS

To create a release build, execute the following commands:

	Cuttlefish$ mkdir build
	Cuttlefish$ cd build
	Cuttlefish/build$ cmake .. -DCMAKE_BUILD_TYPE=Release
	Cuttlefish/build$ make

The tests can be run by running the command:

	Cuttlefish/build$ ctest

The library and tool may then be installed by running the command:

	Cuttlefish/build$ sudo make install

### macOS Universal Binary

To create a universal binary on macOS, set the `CMAKE_OSX_ARCHITECTURES` variable. For example, for an x86_64/arm64 universal binary, add the argument `-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64`. Building a universal binary requires creating an Xcode project with the `-GXcode` argument to CMake.

> **Note:** when installing ISPC through homebrew not all targets are supported. For example, when installing on an ARM CPU (e.g. M1), only ARM targets are supported. Download from the [ISPC website](https://ispc.github.io) to get a version that supports both x86 and ARM targets.

> **Note:** when creating an install package using shared libraries with Xcode, add the `-DCMAKE_BUILD_WITH_INSTALL_RPATH=ON` option when running CMake. This avoids CMake breaking the code signature, which will make the executable unrunnable. The executables won't run in the build directory, however.

## Windows

Building is generally performed through Visual Studio. This can either be done through the CMake GUI tool or on the command line. To generate Visual Studio 2017 projects from the command line, you can run the commands:

	Cuttlefish$ mkdir build
	Cuttlefish$ cd build
	Cuttlefish\build$ cmake .. -G "Visual Studio 15 2017 Win64"

Once generated, the project may be opened through Visual Studio and built as normal. The `RUN_TESTS` project may be built in order to run the tests.

In order to install the libraries and tool, run Visual Studio as administrator, perform a release build, and run the `INSTALL` project. The default installation location is `C:\Program Files\Cuttlefish`. After installation, it's recommended to place the `C:\Program Files\Cuttlefish\bin` folder on your `PATH` environment variable to run the `cuttlefish` tool from the command line.

## Options

The following options may be used when running cmake:

### Compile Options:

* `-DCMAKE_BUILD_TYPE=Debug|Release`: Building in `Debug` or `Release`. This should always be specified.
* `-DCMAKE_INSTALL_PREFIX=path`: Sets the path to install to when running `make install`.
* `-DCUTTLEFISH_SHARED=ON|OFF`: Set to `ON` to build with shared libraries, `OFF` to build with static libraries. Default is `OFF`.
* `-DCUTTLEFISH_ISPC_PATH=path`: The path to the ISPC compiler. If unset, ispc will be searched in the `PATH` or default instal location.

### Enabled Builds

* `-DCUTTLEFISH_BUILD_TESTS=ON|OFF`: Set to `ON` to build the unit tests. `gtest` must also be found in order to build the unit tests. Defaults to `ON`.
* `-DCUTTLEFISH_BUILD_DOCS=ON|OFF`: Set to `ON` to build the documentation. `doxygen` must also be found in order to build the documentation. Defaults to `ON`.
* `-DCUTTLEFISH_BUILD_TOOL=ON|OFF`: Set to `ON` to build the tool. Defaults to `ON`.
* `-DCUTTLEFISH_FORCE_INTERNAL_FREEIMAGE=ON|OFF`: Set to `ON` to force building FreeImage internally even if a system version is found. Defaults to `OFF`.
* `-DCUTTLEFISH_BUILD_S3TC=ON|OFF`: Set to `ON` to build S3TC texture compression support. Defaults to `ON`.
* `-DCUTTLEFISH_BUILD_ETC=ON|OFF`: Set to `ON` to build ETC texture compression support. Defaults to `ON`.
* `-DCUTTLEFISH_BUILD_ASTC=ON|OFF`: Set to `ON` to build ASTC texture compression support. Defaults to `ON`.
* `-DCUTTLEFISH_BUILD_PVRTC=ON|OFF`: Set to `ON` to build PVRTC texture compression support. Defaults to `ON`. If the PVRTexTool library isn't found, support will be disabled.

### Miscellaneous Options:

* `-DCUTTLEFISH_OUTPUT_DIR=directory`: The folder to place the output files. This may be explicitly left empty in order to output to the defaults used by cmake, but this may prevent tests and executables from running properly when `CUTTLEFISH_SHARED` is set to `ON`. Defaults to `${CMAKE_BINARY_DIR}/output`.
* `-DCUTTLEFISH_EXPORTS_DIR=directory`: The folder to place the cmake exports when building. This directory can be added to the module path when embedding in other projects to be able to use the `library_find()` cmake function. Defaults to `${CMAKE_BINARY_DIR}/cmake`.
* `-DCUTTLEFISH_ROOT_FOLDER=folder`: The root folder for the projects in IDEs that support them. (e.g. Visual Studio or XCode) This is useful if embedding Cuttlefish in another project. Defaults to Cuttlefish.
* `-DCUTTLEFISH_INSTALL=ON|OFF`: Allow installation for Cuttlefish components. This can be useful when embedding in other projects to prevent installations from including Cuttlefish. For example, when statically linking into a shared library. Default is `ON`.
* `-DCUTTLEFISH_INSTALL_PVRTEXLIB=ON|OFF`: Include the PVRTextTool library with the installation. This allows the installation to be used for machines that don't have PVRTexTool installed, and can avoid adjusting the `PATH` environment variable on some platforms. Default is `ON`.
* `-DCUTTLEFISH_INSTALL_SET_RPATH=ON|OFF`: Set rpath during install for the library and tool on installation. Set to `OFF` if including in another project that wants to control the rpath. Default is `ON`.
* `-DPVRTEXLIB_ROOT=directory`: The location of the PVRTexTool library platform subdirectories. If the PVRTexTool library is not installed to the standard location on this machine, this variable can be set to tell CMake where to look for the library. The given folder must contain a subdirectory for the current platform (one of `OSX_x86`, `Linux_x86_64`, `Linux_x86_32`, `Windows_x86_64`, or `Windows_x86_32`) that contains the library files.
* `-DCMAKE_OSX_DEPLOYMENT_TARGET=version`: Minimum version of macOS to target when building for Mac. Defaults to 10.14, which is the minimum required for the PVRTexTool library.

Once you have built and installed Cuttlefish, you can find the library by calling `find_package(Cuttlefish CONFIG)` within your CMake files. Libraries and include directories can be accessed through the `Cuttlefish_LIBRARIES` and `Cuttlefish_INCLUDE_DIRS` CMake variables.

> **Note:** In order for `find_package()` to succeed, on Windows you will need to add the path to `INSTALL_DIR/lib/cmake` to `CMAKE_PREFIX_PATH`. (e.g. `C:/Program Files/Cuttlefish/lib/cmake`) On other systems, if you don't install to a standard location, you will need to add the base installation path to `CMAKE_PREFIX_PATH`.

# Limitations

## Texture file format limitations

Some texture file formats have limitations for what texture formats are used. The following formats are *not* supported:

* DDS
	* R4G4B4A4
	* B4G4R4A4
	* B5G6R5
	* R5G5B5A1
	* B5G5R5A1
	* R8G8B8
	* B8G8R8
	* A8B8G8R8
	* A2R10G10B10
	* R16G16B16
	* ETC/EAC compressed formats
	* ASTC compressed formats
	* PVRTC compressed formats
* KTX
	* R4G4
	* A4R4G4B4
	* B8G8R8
* PVR
	* All formats supported

## Custom PVR metadata

Metadata is used to enhance the PVR file format to provide information beyond what is supported by the base format. The metadata values are:

* **FourCC:** FOURCC('C', 'T', 'F', 'S')
* **Key:** one of the following:
	* FOURCC('B', 'C', '1', 'A'): set for BC1_RGBA format. (i.e. BC1 with 1-bit alpha)
	* FOURCC('B', 'C', '1', 0): set for BC1_RGB format. (i.e. BC1 with no alpha)
	* FOURCC('A', 'R', 'R', 'Y'): set for texture arrays. This can be used to differentiate between a non-array texture and an array with 1 element.
	* FOURCC('D', 'I', 'M', '1'): set for 1D textures.
* **Data Size:**: 4
* **Data:** `(uint32_t)0`

A dummy data field of 4 bytes is used because the PVRTexTool GUI tool will crash with a data size of 0.

# Further Documentation

* [Library](lib/README.md)
* [Tool](tool/README.md)
