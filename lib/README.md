# Library

The Cuttlefish library provides utilities to load images, perform operations on the images, convert to a texture, and save the final result. The texture may be saved in a common format (DDS, KTX, or PVR) or the raw data may be extracted to be used in another manner.

# Image

The `Image` class represents an input image. It may either be loaded from a file, a data buffer, or generated programatically. The data may be accessed directly to be manipulated by the program. Some common operations are provided by the `Image` class. See the `Image` class for a full list of operations that are supported.

# Texture

Once the source images have been created, the `Texture` class is used to create the final texture. After initialization, the images are set through `Texture::setImage()`. This must be called for all surfaces, such as cube faces, depth slices, and array indices.

When generating a mipmapped texture, you can avoid having to set each mip level manually by calling `Texture::generateMipmaps()`. When calling this, you should initialize the texture with one mip level and set the images for mip level 0 before calling `Texture::generateMipmaps()`.

Once all images have been set on the texture, `Texture::convert()` may be called to convert to a texture usable by the GPU. The parameters include:

* The format. This is either the bit depth per channel a compressed format.
* The type of data stored for each channel.
* The quality used for texture compression, which also affects conversion speed.
* The color space of the image. This may be used to flag the GPU to convert from sRGB to linear during texture sampling.
* The alpha type and color mask. This may be used for compressed formats to adjust pixel weights.
* The number of threads to use during conversion. If 1 is provided, no threads will be spawned.

After conversion, `Texture::save()` may be called to save to a DDS, KTX, or PVR texture. Alternatively, the `Texture::data()` and `Texture::dataSize()` accessors may be used to access the raw data for each surface.
