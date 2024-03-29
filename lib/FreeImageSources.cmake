# Copied from flipMakefile.srcs withthe following substitutions:
# .s/\.\//\${FREEIMAGE_DIR}\//g
# .s/ Source\// \${FREEIMAGE_DIR}\/Source\//g
# .s/ /\r/g
# Delete the "Wrapper" entries
# Remove b44ExpLogTable.cpp (may need to check for other files with main function)
set(freeImageSources
	${FREEIMAGE_DIR}/Source/FreeImage/BitmapAccess.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/ColorLookup.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/ConversionRGBA16.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/ConversionRGBAF.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/FreeImage.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/FreeImageC.c
	${FREEIMAGE_DIR}/Source/FreeImage/FreeImageIO.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/GetType.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/LFPQuantizer.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/MemoryIO.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/PixelAccess.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/J2KHelper.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/MNGHelper.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/Plugin.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/PluginBMP.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/PluginCUT.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/PluginDDS.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/PluginEXR.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/PluginG3.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/PluginGIF.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/PluginHDR.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/PluginICO.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/PluginIFF.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/PluginJ2K.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/PluginJNG.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/PluginJP2.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/PluginJPEG.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/PluginJXR.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/PluginKOALA.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/PluginMNG.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/PluginPCD.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/PluginPCX.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/PluginPFM.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/PluginPICT.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/PluginPNG.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/PluginPNM.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/PluginPSD.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/PluginRAS.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/PluginRAW.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/PluginSGI.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/PluginTARGA.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/PluginTIFF.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/PluginWBMP.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/PluginWebP.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/PluginXBM.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/PluginXPM.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/PSDParser.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/TIFFLogLuv.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/Conversion.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/Conversion16_555.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/Conversion16_565.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/Conversion24.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/Conversion32.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/Conversion4.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/Conversion8.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/ConversionFloat.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/ConversionRGB16.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/ConversionRGBF.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/ConversionType.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/ConversionUINT16.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/Halftoning.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/tmoColorConvert.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/tmoDrago03.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/tmoFattal02.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/tmoReinhard05.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/ToneMapping.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/NNQuantizer.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/WuQuantizer.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/CacheFile.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/MultiPage.cpp
	${FREEIMAGE_DIR}/Source/FreeImage/ZLibInterface.cpp
	${FREEIMAGE_DIR}/Source/Metadata/Exif.cpp
	${FREEIMAGE_DIR}/Source/Metadata/FIRational.cpp
	${FREEIMAGE_DIR}/Source/Metadata/FreeImageTag.cpp
	${FREEIMAGE_DIR}/Source/Metadata/IPTC.cpp
	${FREEIMAGE_DIR}/Source/Metadata/TagConversion.cpp
	${FREEIMAGE_DIR}/Source/Metadata/TagLib.cpp
	${FREEIMAGE_DIR}/Source/Metadata/XTIFF.cpp
	${FREEIMAGE_DIR}/Source/FreeImageToolkit/Background.cpp
	${FREEIMAGE_DIR}/Source/FreeImageToolkit/BSplineRotate.cpp
	${FREEIMAGE_DIR}/Source/FreeImageToolkit/Channels.cpp
	${FREEIMAGE_DIR}/Source/FreeImageToolkit/ClassicRotate.cpp
	${FREEIMAGE_DIR}/Source/FreeImageToolkit/Colors.cpp
	${FREEIMAGE_DIR}/Source/FreeImageToolkit/CopyPaste.cpp
	${FREEIMAGE_DIR}/Source/FreeImageToolkit/Display.cpp
	${FREEIMAGE_DIR}/Source/FreeImageToolkit/Flip.cpp
	${FREEIMAGE_DIR}/Source/FreeImageToolkit/JPEGTransform.cpp
	${FREEIMAGE_DIR}/Source/FreeImageToolkit/MultigridPoissonSolver.cpp
	${FREEIMAGE_DIR}/Source/FreeImageToolkit/Rescale.cpp
	${FREEIMAGE_DIR}/Source/FreeImageToolkit/Resize.cpp
	${FREEIMAGE_DIR}/Source/LibJPEG/jaricom.c
	${FREEIMAGE_DIR}/Source/LibJPEG/jcapimin.c
	${FREEIMAGE_DIR}/Source/LibJPEG/jcapistd.c
	${FREEIMAGE_DIR}/Source/LibJPEG/jcarith.c
	${FREEIMAGE_DIR}/Source/LibJPEG/jccoefct.c
	${FREEIMAGE_DIR}/Source/LibJPEG/jccolor.c
	${FREEIMAGE_DIR}/Source/LibJPEG/jcdctmgr.c
	${FREEIMAGE_DIR}/Source/LibJPEG/jchuff.c
	${FREEIMAGE_DIR}/Source/LibJPEG/jcinit.c
	${FREEIMAGE_DIR}/Source/LibJPEG/jcmainct.c
	${FREEIMAGE_DIR}/Source/LibJPEG/jcmarker.c
	${FREEIMAGE_DIR}/Source/LibJPEG/jcmaster.c
	${FREEIMAGE_DIR}/Source/LibJPEG/jcomapi.c
	${FREEIMAGE_DIR}/Source/LibJPEG/jcparam.c
	${FREEIMAGE_DIR}/Source/LibJPEG/jcprepct.c
	${FREEIMAGE_DIR}/Source/LibJPEG/jcsample.c
	${FREEIMAGE_DIR}/Source/LibJPEG/jctrans.c
	${FREEIMAGE_DIR}/Source/LibJPEG/jdapimin.c
	${FREEIMAGE_DIR}/Source/LibJPEG/jdapistd.c
	${FREEIMAGE_DIR}/Source/LibJPEG/jdarith.c
	${FREEIMAGE_DIR}/Source/LibJPEG/jdatadst.c
	${FREEIMAGE_DIR}/Source/LibJPEG/jdatasrc.c
	${FREEIMAGE_DIR}/Source/LibJPEG/jdcoefct.c
	${FREEIMAGE_DIR}/Source/LibJPEG/jdcolor.c
	${FREEIMAGE_DIR}/Source/LibJPEG/jddctmgr.c
	${FREEIMAGE_DIR}/Source/LibJPEG/jdhuff.c
	${FREEIMAGE_DIR}/Source/LibJPEG/jdinput.c
	${FREEIMAGE_DIR}/Source/LibJPEG/jdmainct.c
	${FREEIMAGE_DIR}/Source/LibJPEG/jdmarker.c
	${FREEIMAGE_DIR}/Source/LibJPEG/jdmaster.c
	${FREEIMAGE_DIR}/Source/LibJPEG/jdmerge.c
	${FREEIMAGE_DIR}/Source/LibJPEG/jdpostct.c
	${FREEIMAGE_DIR}/Source/LibJPEG/jdsample.c
	${FREEIMAGE_DIR}/Source/LibJPEG/jdtrans.c
	${FREEIMAGE_DIR}/Source/LibJPEG/jerror.c
	${FREEIMAGE_DIR}/Source/LibJPEG/jfdctflt.c
	${FREEIMAGE_DIR}/Source/LibJPEG/jfdctfst.c
	${FREEIMAGE_DIR}/Source/LibJPEG/jfdctint.c
	${FREEIMAGE_DIR}/Source/LibJPEG/jidctflt.c
	${FREEIMAGE_DIR}/Source/LibJPEG/jidctfst.c
	${FREEIMAGE_DIR}/Source/LibJPEG/jidctint.c
	${FREEIMAGE_DIR}/Source/LibJPEG/jmemmgr.c
	${FREEIMAGE_DIR}/Source/LibJPEG/jmemnobs.c
	${FREEIMAGE_DIR}/Source/LibJPEG/jquant1.c
	${FREEIMAGE_DIR}/Source/LibJPEG/jquant2.c
	${FREEIMAGE_DIR}/Source/LibJPEG/jutils.c
	${FREEIMAGE_DIR}/Source/LibJPEG/transupp.c
	${FREEIMAGE_DIR}/Source/LibPNG/png.c
	${FREEIMAGE_DIR}/Source/LibPNG/pngerror.c
	${FREEIMAGE_DIR}/Source/LibPNG/pngget.c
	${FREEIMAGE_DIR}/Source/LibPNG/pngmem.c
	${FREEIMAGE_DIR}/Source/LibPNG/pngpread.c
	${FREEIMAGE_DIR}/Source/LibPNG/pngread.c
	${FREEIMAGE_DIR}/Source/LibPNG/pngrio.c
	${FREEIMAGE_DIR}/Source/LibPNG/pngrtran.c
	${FREEIMAGE_DIR}/Source/LibPNG/pngrutil.c
	${FREEIMAGE_DIR}/Source/LibPNG/pngset.c
	${FREEIMAGE_DIR}/Source/LibPNG/pngtrans.c
	${FREEIMAGE_DIR}/Source/LibPNG/pngwio.c
	${FREEIMAGE_DIR}/Source/LibPNG/pngwrite.c
	${FREEIMAGE_DIR}/Source/LibPNG/pngwtran.c
	${FREEIMAGE_DIR}/Source/LibPNG/pngwutil.c
	${FREEIMAGE_DIR}/Source/LibTIFF4/tif_aux.c
	${FREEIMAGE_DIR}/Source/LibTIFF4/tif_close.c
	${FREEIMAGE_DIR}/Source/LibTIFF4/tif_codec.c
	${FREEIMAGE_DIR}/Source/LibTIFF4/tif_color.c
	${FREEIMAGE_DIR}/Source/LibTIFF4/tif_compress.c
	${FREEIMAGE_DIR}/Source/LibTIFF4/tif_dir.c
	${FREEIMAGE_DIR}/Source/LibTIFF4/tif_dirinfo.c
	${FREEIMAGE_DIR}/Source/LibTIFF4/tif_dirread.c
	${FREEIMAGE_DIR}/Source/LibTIFF4/tif_dirwrite.c
	${FREEIMAGE_DIR}/Source/LibTIFF4/tif_dumpmode.c
	${FREEIMAGE_DIR}/Source/LibTIFF4/tif_error.c
	${FREEIMAGE_DIR}/Source/LibTIFF4/tif_extension.c
	${FREEIMAGE_DIR}/Source/LibTIFF4/tif_fax3.c
	${FREEIMAGE_DIR}/Source/LibTIFF4/tif_fax3sm.c
	${FREEIMAGE_DIR}/Source/LibTIFF4/tif_flush.c
	${FREEIMAGE_DIR}/Source/LibTIFF4/tif_getimage.c
	${FREEIMAGE_DIR}/Source/LibTIFF4/tif_hash_set.c
	${FREEIMAGE_DIR}/Source/LibTIFF4/tif_jpeg.c
	${FREEIMAGE_DIR}/Source/LibTIFF4/tif_lerc.c
	${FREEIMAGE_DIR}/Source/LibTIFF4/tif_luv.c
	${FREEIMAGE_DIR}/Source/LibTIFF4/tif_lzw.c
	${FREEIMAGE_DIR}/Source/LibTIFF4/tif_next.c
	${FREEIMAGE_DIR}/Source/LibTIFF4/tif_ojpeg.c
	${FREEIMAGE_DIR}/Source/LibTIFF4/tif_open.c
	${FREEIMAGE_DIR}/Source/LibTIFF4/tif_packbits.c
	${FREEIMAGE_DIR}/Source/LibTIFF4/tif_pixarlog.c
	${FREEIMAGE_DIR}/Source/LibTIFF4/tif_predict.c
	${FREEIMAGE_DIR}/Source/LibTIFF4/tif_print.c
	${FREEIMAGE_DIR}/Source/LibTIFF4/tif_read.c
	${FREEIMAGE_DIR}/Source/LibTIFF4/tif_strip.c
	${FREEIMAGE_DIR}/Source/LibTIFF4/tif_swab.c
	${FREEIMAGE_DIR}/Source/LibTIFF4/tif_thunder.c
	${FREEIMAGE_DIR}/Source/LibTIFF4/tif_tile.c
	${FREEIMAGE_DIR}/Source/LibTIFF4/tif_version.c
	${FREEIMAGE_DIR}/Source/LibTIFF4/tif_warning.c
	${FREEIMAGE_DIR}/Source/LibTIFF4/tif_webp.c
	${FREEIMAGE_DIR}/Source/LibTIFF4/tif_write.c
	${FREEIMAGE_DIR}/Source/LibTIFF4/tif_zip.c
	${FREEIMAGE_DIR}/Source/ZLib/adler32.c
	${FREEIMAGE_DIR}/Source/ZLib/compress.c
	${FREEIMAGE_DIR}/Source/ZLib/crc32.c
	${FREEIMAGE_DIR}/Source/ZLib/deflate.c
	${FREEIMAGE_DIR}/Source/ZLib/gzclose.c
	${FREEIMAGE_DIR}/Source/ZLib/gzlib.c
	${FREEIMAGE_DIR}/Source/ZLib/gzread.c
	${FREEIMAGE_DIR}/Source/ZLib/gzwrite.c
	${FREEIMAGE_DIR}/Source/ZLib/infback.c
	${FREEIMAGE_DIR}/Source/ZLib/inffast.c
	${FREEIMAGE_DIR}/Source/ZLib/inflate.c
	${FREEIMAGE_DIR}/Source/ZLib/inftrees.c
	${FREEIMAGE_DIR}/Source/ZLib/trees.c
	${FREEIMAGE_DIR}/Source/ZLib/uncompr.c
	${FREEIMAGE_DIR}/Source/ZLib/zutil.c
	${FREEIMAGE_DIR}/Source/LibOpenJPEG/bio.c
	${FREEIMAGE_DIR}/Source/LibOpenJPEG/cio.c
	${FREEIMAGE_DIR}/Source/LibOpenJPEG/dwt.c
	${FREEIMAGE_DIR}/Source/LibOpenJPEG/event.c
	${FREEIMAGE_DIR}/Source/LibOpenJPEG/function_list.c
	${FREEIMAGE_DIR}/Source/LibOpenJPEG/image.c
	${FREEIMAGE_DIR}/Source/LibOpenJPEG/invert.c
	${FREEIMAGE_DIR}/Source/LibOpenJPEG/j2k.c
	${FREEIMAGE_DIR}/Source/LibOpenJPEG/jp2.c
	${FREEIMAGE_DIR}/Source/LibOpenJPEG/mct.c
	${FREEIMAGE_DIR}/Source/LibOpenJPEG/mqc.c
	${FREEIMAGE_DIR}/Source/LibOpenJPEG/openjpeg.c
	${FREEIMAGE_DIR}/Source/LibOpenJPEG/opj_clock.c
	${FREEIMAGE_DIR}/Source/LibOpenJPEG/pi.c
	${FREEIMAGE_DIR}/Source/LibOpenJPEG/raw.c
	${FREEIMAGE_DIR}/Source/LibOpenJPEG/t1.c
	${FREEIMAGE_DIR}/Source/LibOpenJPEG/t2.c
	${FREEIMAGE_DIR}/Source/LibOpenJPEG/tcd.c
	${FREEIMAGE_DIR}/Source/LibOpenJPEG/tgt.c
	${FREEIMAGE_DIR}/Source/OpenEXR/IexMath/IexMathFpu.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfAcesFile.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfAttribute.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfB44Compressor.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfBoxAttribute.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfChannelList.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfChannelListAttribute.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfChromaticities.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfChromaticitiesAttribute.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfCompositeDeepScanLine.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfCompressionAttribute.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfCompressor.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfConvert.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfCRgbaFile.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfDeepCompositing.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfDeepFrameBuffer.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfDeepImageStateAttribute.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfDeepScanLineInputFile.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfDeepScanLineInputPart.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfDeepScanLineOutputFile.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfDeepScanLineOutputPart.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfDeepTiledInputFile.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfDeepTiledInputPart.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfDeepTiledOutputFile.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfDeepTiledOutputPart.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfDoubleAttribute.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfDwaCompressor.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfEnvmap.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfEnvmapAttribute.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfFastHuf.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfFloatAttribute.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfFloatVectorAttribute.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfFrameBuffer.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfFramesPerSecond.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfGenericInputFile.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfGenericOutputFile.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfHeader.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfHuf.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfInputFile.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfInputPart.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfInputPartData.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfIntAttribute.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfIO.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfKeyCode.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfKeyCodeAttribute.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfLineOrderAttribute.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfLut.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfMatrixAttribute.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfMisc.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfMultiPartInputFile.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfMultiPartOutputFile.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfMultiView.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfOpaqueAttribute.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfOutputFile.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfOutputPart.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfOutputPartData.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfPartType.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfPizCompressor.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfPreviewImage.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfPreviewImageAttribute.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfPxr24Compressor.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfRational.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfRationalAttribute.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfRgbaFile.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfRgbaYca.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfRle.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfRleCompressor.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfScanLineInputFile.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfStandardAttributes.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfStdIO.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfStringAttribute.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfStringVectorAttribute.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfSystemSpecific.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfTestFile.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfThreading.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfTileDescriptionAttribute.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfTiledInputFile.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfTiledInputPart.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfTiledMisc.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfTiledOutputFile.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfTiledOutputPart.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfTiledRgbaFile.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfTileOffsets.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfTimeCode.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfTimeCodeAttribute.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfVecAttribute.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfVersion.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfWav.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfZip.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf/ImfZipCompressor.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/Imath/ImathBox.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/Imath/ImathColorAlgo.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/Imath/ImathFun.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/Imath/ImathMatrixAlgo.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/Imath/ImathRandom.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/Imath/ImathShear.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/Imath/ImathVec.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/Iex/IexBaseExc.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/Iex/IexThrowErrnoExc.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/Half/half.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmThread/IlmThread.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmThread/IlmThreadMutex.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmThread/IlmThreadPool.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmThread/IlmThreadSemaphore.cpp
	${FREEIMAGE_DIR}/Source/OpenEXR/IexMath/IexMathFloatExc.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/decoders/canon_600.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/decoders/crx.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/decoders/decoders_dcraw.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/decoders/decoders_libraw.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/decoders/decoders_libraw_dcrdefs.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/decoders/dng.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/decoders/fp_dng.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/decoders/fuji_compressed.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/decoders/generic.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/decoders/kodak_decoders.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/decoders/load_mfbacks.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/decoders/smal.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/decoders/unpack.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/decoders/unpack_thumb.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/demosaic/aahd_demosaic.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/demosaic/ahd_demosaic.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/demosaic/dcb_demosaic.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/demosaic/dht_demosaic.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/demosaic/misc_demosaic.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/demosaic/xtrans_demosaic.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/integration/dngsdk_glue.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/integration/rawspeed_glue.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/libraw_datastream.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/metadata/adobepano.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/metadata/canon.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/metadata/ciff.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/metadata/cr3_parser.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/metadata/epson.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/metadata/exif_gps.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/metadata/fuji.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/metadata/hasselblad_model.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/metadata/identify.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/metadata/identify_tools.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/metadata/kodak.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/metadata/leica.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/metadata/makernotes.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/metadata/mediumformat.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/metadata/minolta.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/metadata/misc_parsers.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/metadata/nikon.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/metadata/normalize_model.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/metadata/olympus.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/metadata/p1.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/metadata/pentax.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/metadata/samsung.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/metadata/sony.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/metadata/tiff.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/postprocessing/aspect_ratio.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/postprocessing/dcraw_process.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/postprocessing/mem_image.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/postprocessing/postprocessing_aux.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/postprocessing/postprocessing_utils.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/postprocessing/postprocessing_utils_dcrdefs.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/preprocessing/ext_preprocess.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/preprocessing/raw2image.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/preprocessing/subtract_black.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/tables/cameralist.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/tables/colorconst.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/tables/colordata.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/tables/wblists.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/utils/curves.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/utils/decoder_info.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/utils/init_close_utils.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/utils/open.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/utils/phaseone_processing.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/utils/read_utils.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/utils/thumb_utils.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/utils/utils_dcraw.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/utils/utils_libraw.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/write/file_write.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/x3f/x3f_parse_process.cpp
	${FREEIMAGE_DIR}/Source/LibRawLite/src/x3f/x3f_utils_patched.cpp
	${FREEIMAGE_DIR}/Source/LibWebP/src/dec/alpha_dec.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dec/buffer_dec.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dec/frame_dec.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dec/idec_dec.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dec/io_dec.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dec/quant_dec.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dec/tree_dec.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dec/vp8l_dec.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dec/vp8_dec.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dec/webp_dec.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/demux/anim_decode.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/demux/demux.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/alpha_processing.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/alpha_processing_mips_dsp_r2.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/alpha_processing_neon.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/alpha_processing_sse2.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/alpha_processing_sse41.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/cost.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/cost_mips32.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/cost_mips_dsp_r2.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/cost_neon.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/cost_sse2.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/cpu.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/dec.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/dec_clip_tables.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/dec_mips32.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/dec_mips_dsp_r2.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/dec_msa.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/dec_neon.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/dec_sse2.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/dec_sse41.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/enc.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/enc_avx2.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/enc_mips32.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/enc_mips_dsp_r2.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/enc_msa.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/enc_neon.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/enc_sse2.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/enc_sse41.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/filters.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/filters_mips_dsp_r2.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/filters_msa.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/filters_neon.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/filters_sse2.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/lossless.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/lossless_enc.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/lossless_enc_mips32.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/lossless_enc_mips_dsp_r2.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/lossless_enc_msa.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/lossless_enc_neon.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/lossless_enc_sse2.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/lossless_enc_sse41.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/lossless_mips_dsp_r2.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/lossless_msa.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/lossless_neon.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/lossless_sse2.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/lossless_sse41.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/rescaler.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/rescaler_mips32.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/rescaler_mips_dsp_r2.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/rescaler_msa.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/rescaler_neon.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/rescaler_sse2.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/ssim.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/ssim_sse2.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/upsampling.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/upsampling_mips_dsp_r2.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/upsampling_msa.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/upsampling_neon.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/upsampling_sse2.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/upsampling_sse41.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/yuv.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/yuv_mips32.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/yuv_mips_dsp_r2.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/yuv_neon.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/yuv_sse2.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/dsp/yuv_sse41.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/enc/alpha_enc.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/enc/analysis_enc.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/enc/backward_references_cost_enc.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/enc/backward_references_enc.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/enc/config_enc.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/enc/cost_enc.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/enc/filter_enc.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/enc/frame_enc.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/enc/histogram_enc.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/enc/iterator_enc.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/enc/near_lossless_enc.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/enc/picture_csp_enc.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/enc/picture_enc.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/enc/picture_psnr_enc.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/enc/picture_rescale_enc.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/enc/picture_tools_enc.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/enc/predictor_enc.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/enc/quant_enc.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/enc/syntax_enc.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/enc/token_enc.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/enc/tree_enc.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/enc/vp8l_enc.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/enc/webp_enc.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/mux/anim_encode.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/mux/muxedit.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/mux/muxinternal.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/mux/muxread.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/utils/bit_reader_utils.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/utils/bit_writer_utils.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/utils/color_cache_utils.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/utils/filters_utils.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/utils/huffman_encode_utils.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/utils/huffman_utils.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/utils/quant_levels_dec_utils.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/utils/quant_levels_utils.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/utils/random_utils.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/utils/rescaler_utils.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/utils/thread_utils.c
	${FREEIMAGE_DIR}/Source/LibWebP/src/utils/utils.c
	${FREEIMAGE_DIR}/Source/LibJXR/image/decode/decode.c
	${FREEIMAGE_DIR}/Source/LibJXR/image/decode/JXRTranscode.c
	${FREEIMAGE_DIR}/Source/LibJXR/image/decode/postprocess.c
	${FREEIMAGE_DIR}/Source/LibJXR/image/decode/segdec.c
	${FREEIMAGE_DIR}/Source/LibJXR/image/decode/strdec.c
	${FREEIMAGE_DIR}/Source/LibJXR/image/decode/strdec_x86.c
	${FREEIMAGE_DIR}/Source/LibJXR/image/decode/strInvTransform.c
	${FREEIMAGE_DIR}/Source/LibJXR/image/decode/strPredQuantDec.c
	${FREEIMAGE_DIR}/Source/LibJXR/image/encode/encode.c
	${FREEIMAGE_DIR}/Source/LibJXR/image/encode/segenc.c
	${FREEIMAGE_DIR}/Source/LibJXR/image/encode/strenc.c
	${FREEIMAGE_DIR}/Source/LibJXR/image/encode/strenc_x86.c
	${FREEIMAGE_DIR}/Source/LibJXR/image/encode/strFwdTransform.c
	${FREEIMAGE_DIR}/Source/LibJXR/image/encode/strPredQuantEnc.c
	${FREEIMAGE_DIR}/Source/LibJXR/image/sys/adapthuff.c
	${FREEIMAGE_DIR}/Source/LibJXR/image/sys/image.c
	${FREEIMAGE_DIR}/Source/LibJXR/image/sys/strcodec.c
	${FREEIMAGE_DIR}/Source/LibJXR/image/sys/strPredQuant.c
	${FREEIMAGE_DIR}/Source/LibJXR/image/sys/strTransform.c
	${FREEIMAGE_DIR}/Source/LibJXR/jxrgluelib/JXRGlue.c
	${FREEIMAGE_DIR}/Source/LibJXR/jxrgluelib/JXRGlueJxr.c
	${FREEIMAGE_DIR}/Source/LibJXR/jxrgluelib/JXRGluePFC.c
	${FREEIMAGE_DIR}/Source/LibJXR/jxrgluelib/JXRMeta.c
)

set(freeImageIncludeDirs
	${FREEIMAGE_DIR}/Source
	${FREEIMAGE_DIR}/Source/LibJPEG
	${FREEIMAGE_DIR}/Source/ZLib
	${FREEIMAGE_DIR}/Source/OpenEXR
	${FREEIMAGE_DIR}/Source/OpenEXR/Half
	${FREEIMAGE_DIR}/Source/OpenEXR/Iex
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmImf
	${FREEIMAGE_DIR}/Source/OpenEXR/IlmThread
	${FREEIMAGE_DIR}/Source/OpenEXR/Imath
	${FREEIMAGE_DIR}/Source/OpenEXR/IexMath
	${FREEIMAGE_DIR}/Source/LibRawLite
	${FREEIMAGE_DIR}/Source/LibRawLite/dcraw
	${FREEIMAGE_DIR}/Source/LibRawLite/internal
	${FREEIMAGE_DIR}/Source/LibRawLite/libraw
	${FREEIMAGE_DIR}/Source/LibRawLite/src
	${FREEIMAGE_DIR}/Source/LibWebP
	${FREEIMAGE_DIR}/Source/LibJXR
	${FREEIMAGE_DIR}/Source/LibJXR/common/include
	${FREEIMAGE_DIR}/Source/LibJXR/image/sys
	${FREEIMAGE_DIR}/Source/LibJXR/jxrgluelib
)
