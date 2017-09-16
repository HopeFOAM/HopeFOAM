/* -*- c -*- *******************************************************/
/*
 * Copyright (C) 2003 Sandia Corporation
 * Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 * the U.S. Government retains certain rights in this software.
 *
 * This source code is released under the New BSD License.
 */

#ifndef __IceTDevImage_h
#define __IceTDevImage_h

#include <IceT.h>
#include <IceTDevState.h>

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

#define ICET_SRC_ON_TOP         ICET_TRUE
#define ICET_DEST_ON_TOP        ICET_FALSE

ICET_EXPORT IceTImage icetGetStateBufferImage(IceTEnum pname,
                                              IceTSizeType width,
                                              IceTSizeType height);
ICET_EXPORT IceTImage icetRetrieveStateImage(IceTEnum pname);
ICET_EXPORT IceTSizeType icetImageBufferSize(IceTSizeType width,
                                             IceTSizeType height);
ICET_EXPORT IceTSizeType icetImageBufferSizeType(IceTEnum color_format,
                                                 IceTEnum depth_format,
                                                 IceTSizeType width,
                                                 IceTSizeType height);
ICET_EXPORT IceTImage icetImageAssignBuffer(IceTVoid *buffer,
                                            IceTSizeType width,
                                            IceTSizeType height);
ICET_EXPORT IceTImage icetGetStatePointerImage(IceTEnum pname,
                                               IceTSizeType width,
                                               IceTSizeType height,
                                               const IceTVoid *color_buffer,
                                               const IceTVoid *depth_buffer);
ICET_EXPORT IceTSizeType icetImagePointerBufferSize(void);
ICET_EXPORT IceTImage icetImagePointerAssignBuffer(IceTVoid *buffer,
                                                   IceTSizeType width,
                                                   IceTSizeType height,
                                                   const IceTVoid *color_buf,
                                                   const IceTVoid *depth_buf);
ICET_EXPORT void icetImageAdjustForOutput(IceTImage image);
ICET_EXPORT void icetImageAdjustForInput(IceTImage image);
ICET_EXPORT void icetImageSetDimensions(IceTImage image,
                                        IceTSizeType width,
                                        IceTSizeType height);
ICET_EXPORT IceTVoid *icetImageGetColorVoid(IceTImage image,
                                            IceTSizeType *pixel_size);
ICET_EXPORT const IceTVoid *icetImageGetColorConstVoid(
                                                      const IceTImage image,
                                                      IceTSizeType *pixel_size);
ICET_EXPORT IceTVoid *icetImageGetDepthVoid(IceTImage image,
                                            IceTSizeType *pixel_size);
ICET_EXPORT const IceTVoid *icetImageGetDepthConstVoid(
                                                      const IceTImage image,
                                                      IceTSizeType *pixel_size);
ICET_EXPORT IceTBoolean icetImageEqual(const IceTImage image1,
                                       const IceTImage image2);
ICET_EXPORT void icetImageCopyPixels(const IceTImage in_image,
                                     IceTSizeType in_offset,
                                     IceTImage out_image,
                                     IceTSizeType out_offset,
                                     IceTSizeType num_pixels);
ICET_EXPORT void icetImageCopyRegion(const IceTImage in_image,
                                     const IceTInt *in_viewport,
                                     IceTImage out_image,
                                     const IceTInt *out_viewport);
ICET_EXPORT void icetImageClearAroundRegion(IceTImage image,
                                            const IceTInt *region);
ICET_EXPORT void icetImagePackageForSend(IceTImage image,
                                         IceTVoid **buffer,
                                         IceTSizeType *size);
ICET_EXPORT IceTImage icetImageUnpackageFromReceive(IceTVoid *buffer);

typedef struct { IceTVoid *opaque_internals; } IceTSparseImage;

ICET_EXPORT IceTSizeType icetSparseImageBufferSize(IceTSizeType width,
                                                   IceTSizeType height);
ICET_EXPORT IceTSizeType icetSparseImageBufferSizeType(IceTEnum color_format,
                                                       IceTEnum depth_format,
                                                       IceTSizeType width,
                                                       IceTSizeType height);
ICET_EXPORT IceTSparseImage icetGetStateBufferSparseImage(IceTEnum pname,
                                                          IceTSizeType width,
                                                          IceTSizeType height);
ICET_EXPORT IceTSparseImage icetSparseImageAssignBuffer(IceTVoid *buffer,
                                                        IceTSizeType width,
                                                        IceTSizeType height);
ICET_EXPORT IceTSparseImage icetSparseImageNull(void);
ICET_EXPORT IceTBoolean icetSparseImageIsNull(const IceTSparseImage image);
ICET_EXPORT IceTEnum icetSparseImageGetColorFormat(const IceTSparseImage image);
ICET_EXPORT IceTEnum icetSparseImageGetDepthFormat(const IceTSparseImage image);
ICET_EXPORT IceTSizeType icetSparseImageGetWidth(const IceTSparseImage image);
ICET_EXPORT IceTSizeType icetSparseImageGetHeight(const IceTSparseImage image);
ICET_EXPORT IceTSizeType icetSparseImageGetNumPixels(
                                                   const IceTSparseImage image);
ICET_EXPORT void icetSparseImageSetDimensions(IceTSparseImage image,
                                              IceTSizeType width,
                                              IceTSizeType height);
ICET_EXPORT IceTSizeType icetSparseImageGetCompressedBufferSize(
                                                   const IceTSparseImage image);
ICET_EXPORT void icetSparseImagePackageForSend(IceTSparseImage image,
                                               IceTVoid **buffer,
                                               IceTSizeType *size);
ICET_EXPORT IceTSparseImage icetSparseImageUnpackageFromReceive(
                                                              IceTVoid *buffer);

ICET_EXPORT IceTBoolean icetSparseImageEqual(const IceTSparseImage image1,
                                             const IceTSparseImage image2);

ICET_EXPORT void icetSparseImageCopyPixels(const IceTSparseImage in_image,
                                           IceTSizeType in_offset,
                                           IceTSizeType num_pixels,
                                           IceTSparseImage out_image);

ICET_EXPORT void icetSparseImageSplit(const IceTSparseImage in_image,
                                      IceTSizeType in_image_offset,
                                      IceTInt num_partitions,
                                      IceTInt eventual_num_partitions,
                                      IceTSparseImage *out_images,
                                      IceTSizeType *offsets);
ICET_EXPORT IceTSizeType icetSparseImageSplitPartitionNumPixels(
                                               IceTSizeType input_num_pixels,
                                               IceTInt num_partitions,
                                               IceTInt eventual_num_partitions);

ICET_EXPORT void icetSparseImageInterlace(const IceTSparseImage in_image,
                                          IceTInt eventual_num_partitions,
                                          IceTEnum scratch_state_buffer,
                                          IceTSparseImage out_image);

ICET_EXPORT IceTSizeType icetGetInterlaceOffset(
                                              IceTInt partition_index,
                                              IceTInt eventual_num_partitions,
                                              IceTSizeType original_image_size);

ICET_EXPORT void icetClearImage(IceTImage image);
ICET_EXPORT void icetClearSparseImage(IceTSparseImage image);

ICET_EXPORT void icetGetTileImage(IceTInt tile, IceTImage image);

ICET_EXPORT void icetGetCompressedTileImage(IceTInt tile,
                                            IceTSparseImage compressed_image);

ICET_EXPORT void icetCompressImage(const IceTImage image,
                                   IceTSparseImage compressed_image);

ICET_EXPORT void icetCompressSubImage(const IceTImage image,
                                      IceTSizeType offset,
                                      IceTSizeType pixels,
                                      IceTSparseImage compressed_image);

ICET_EXPORT void icetDecompressImage(const IceTSparseImage compressed_image,
                                     IceTImage image);

ICET_EXPORT void icetDecompressSubImage(const IceTSparseImage compressed_image,
                                        IceTSizeType offset,
                                        IceTImage image);

ICET_EXPORT void icetDecompressImageCorrectBackground(
                                         const IceTSparseImage compressed_image,
                                         IceTImage image);

ICET_EXPORT void icetDecompressSubImageCorrectBackground(
                                         const IceTSparseImage compressed_image,
                                         IceTSizeType offset,
                                         IceTImage image);

ICET_EXPORT void icetComposite(IceTImage destBuffer,
                               const IceTImage srcBuffer,
                               int srcOnTop);

ICET_EXPORT void icetCompressedComposite(IceTImage destBuffer,
                                         const IceTSparseImage srcBuffer,
                                         int srcOnTop);

ICET_EXPORT void icetCompressedSubComposite(IceTImage destBuffer,
                                            IceTSizeType offset,
                                            const IceTSparseImage srcBuffer,
                                            int srcOnTop);

ICET_EXPORT void icetCompressedCompressedComposite(
                                             const IceTSparseImage front_buffer,
                                             const IceTSparseImage back_buffer,
                                             IceTSparseImage dest_buffer);

ICET_EXPORT void icetImageCorrectBackground(IceTImage image);
ICET_EXPORT void icetClearImageTrueBackground(IceTImage image);

#define ICET_BLEND_UBYTE(front, back, dest)                             \
{                                                                       \
    IceTUInt afactor = 255 - (front)[3];                                \
    (dest)[0] = (IceTUByte)(((back)[0]*afactor)/255 + (front)[0]);      \
    (dest)[1] = (IceTUByte)(((back)[1]*afactor)/255 + (front)[1]);      \
    (dest)[2] = (IceTUByte)(((back)[2]*afactor)/255 + (front)[2]);      \
    (dest)[3] = (IceTUByte)(((back)[3]*afactor)/255 + (front)[3]);      \
}

#define ICET_OVER_UBYTE(src, dest)  ICET_BLEND_UBYTE(src, dest, dest)
#define ICET_UNDER_UBYTE(src, dest) ICET_BLEND_UBYTE(dest, src, dest)

#define ICET_BLEND_FLOAT(front, back, dest)                             \
{                                                                       \
    IceTFloat afactor = 1.0f - (front)[3];                              \
    (dest)[0] = (back)[0]*afactor + (front)[0];                         \
    (dest)[1] = (back)[1]*afactor + (front)[1];                         \
    (dest)[2] = (back)[2]*afactor + (front)[2];                         \
    (dest)[3] = (back)[3]*afactor + (front)[3];                         \
}

#define ICET_OVER_FLOAT(src, dest)  ICET_BLEND_FLOAT(src, dest, dest)
#define ICET_UNDER_FLOAT(src, dest) ICET_BLEND_FLOAT(dest, src, dest)

#ifdef __cplusplus
}
#endif

#endif /* __IceTDevImage_h */
