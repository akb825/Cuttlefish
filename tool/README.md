# Tool

The `cuttlefish` tool can be used to convert textures on the command line. This provides the functionality required to create most textures.

When running the `cuttlefish` tool, one or more input images are provided on the command line. Different arguments are provided for single images, multiple images for an array or 3D texture, or a cube map. The images may optionally be provided from a text file, with one image path per line.

Options are available for common operations on the images. The following operations are provided, and are executed in this order:

* Resize. Resizing may be an explicit size, next power of 2, or nearest power of 2.
* Rotate.
* Convert to grayscale.
* Convert from a bump map to a normal map.
* Flip along the X axis.
* Flip along the Y axis.
* Convert from sRGB to linear. This is performed for formats that cannot natively be sampled as sRGB from the GPU. (note that this may cause a noticeable loss of quality for lower precision formats)
* Swizzle.
* Pre-multiply alpha with color channels.

When converting a texture, the format is provided, which is either the color bits for each channel (e.g. R8G8B8A8, R5G6B5, B10G11R11\_UFloat) or the name of a compressed format. (e.g. BC3, ETC2\_R8G8B8) Some formats allow the type used for the channel to be provided. For example, R16G16B16A16 may be unorm, snorm, uint, int, or float. The final image may be saved as a DDS, KTX, or PVR file.

When running the tool, you may provide the `-j` parameter to use multiple threaded jobs. The number of jobs may be provided, otherwise it will use all available cores. This is recommended when a single instance of `cuttlefish` is run, but shouldn't be used if integrated into a build system that will run multiple instances in parallel. (e.g. `make` with `-j` provided)

For more detailed information about the command line arguments, run `cuttlefish -h`.
