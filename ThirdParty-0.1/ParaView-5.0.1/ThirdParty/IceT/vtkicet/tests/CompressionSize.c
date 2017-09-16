/* -*- c -*- *****************************************************************
** Copyright (C) 2003 Sandia Corporation
** Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
** the U.S. Government retains certain rights in this software.
**
** This source code is released under the New BSD License.
**
** This test checks to make sure the size of compressed images never
** exceeds the advertised maximum buffer size.
*****************************************************************************/

#include "test_codes.h"
#include "test_util.h"

#include <IceTDevImage.h>
#include <IceTDevState.h>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

static IceTDouble IdentityMatrix[16] = {
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0
};
static IceTFloat Black[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

static void InitPathologicalImage(IceTImage image)
{
  /* Create a worst case possible for image with respect to compression.
     Every other pixel is active so the run lengths are all 1. */
    IceTEnum format;
    IceTSizeType num_pixels;

    num_pixels = icetImageGetNumPixels(image);

    format = icetImageGetColorFormat(image);
    if (format == ICET_IMAGE_COLOR_RGBA_UBYTE) {
        IceTUByte *buffer = icetImageGetColorub(image);
        IceTSizeType i;
        for (i = 0; i < num_pixels; i++) {
            buffer[4*i + 0] = 255*(IceTUByte)(i%2);
            buffer[4*i + 1] = 255*(IceTUByte)(i%2);
            buffer[4*i + 2] = 255*(IceTUByte)(i%2);
            buffer[4*i + 3] = 255*(IceTUByte)(i%2);
        }
    } else if (format == ICET_IMAGE_COLOR_RGBA_FLOAT) {
        IceTFloat *buffer = icetImageGetColorf(image);
        IceTSizeType i;
        for (i = 0; i < num_pixels; i++) {
            buffer[4*i + 0] = (IceTFloat)(i%2);
            buffer[4*i + 1] = (IceTFloat)(i%2);
            buffer[4*i + 2] = (IceTFloat)(i%2);
            buffer[4*i + 3] = (IceTFloat)(i%2);
        }
    } else if (format != ICET_IMAGE_COLOR_NONE) {
        printrank("*** Unknown color format? ***\n");
    }

    format = icetImageGetDepthFormat(image);
    if (format == ICET_IMAGE_DEPTH_FLOAT) {
        IceTFloat *buffer = icetImageGetDepthf(image);
        IceTSizeType i;
        for (i = 0; i < num_pixels; i++) {
            buffer[i] = (IceTFloat)(i%2);
        }
    } else if (format != ICET_IMAGE_DEPTH_NONE) {
        printrank("*** Unknown depth format? ***\n");
    }
}

static void InitActiveImage(IceTImage image)
{
  /* Create a worst case possible for image with respect to compression.
     All the pixels are active, so no data can be removed. */
    IceTEnum format;
    IceTSizeType num_pixels;
    int seed;

    seed = (int)time(NULL);
    srand(seed);

    num_pixels = icetImageGetNumPixels(image);

    format = icetImageGetColorFormat(image);
    if (format == ICET_IMAGE_COLOR_RGBA_UBYTE) {
        IceTUByte *buffer = icetImageGetColorub(image);
        IceTSizeType i;
        for (i = 0; i < num_pixels; i++) {
            buffer[4*i + 0] = (IceTUByte)(rand()%255 + 1);
            buffer[4*i + 1] = (IceTUByte)(rand()%255 + 1);
            buffer[4*i + 2] = (IceTUByte)(rand()%255 + 1);
            buffer[4*i + 3] = (IceTUByte)(rand()%255 + 1);
        }
    } else if (format == ICET_IMAGE_COLOR_RGBA_FLOAT) {
        IceTFloat *buffer = icetImageGetColorf(image);
        IceTSizeType i;
        for (i = 0; i < num_pixels; i++) {
            buffer[4*i + 0] = ((IceTFloat)(rand()%255 + 1))/255;
            buffer[4*i + 1] = ((IceTFloat)(rand()%255 + 1))/255;
            buffer[4*i + 2] = ((IceTFloat)(rand()%255 + 1))/255;
            buffer[4*i + 3] = ((IceTFloat)(rand()%255 + 1))/255;
        }
    } else if (format != ICET_IMAGE_COLOR_NONE) {
        printrank("*** Unknown color format? ***\n");
    }

    format = icetImageGetDepthFormat(image);
    if (format == ICET_IMAGE_DEPTH_FLOAT) {
        IceTFloat *buffer = icetImageGetDepthf(image);
        IceTSizeType i;
        for (i = 0; i < num_pixels; i++) {
            buffer[i] = ((IceTFloat)(rand()%255))/255;
        }
    } else if (format != ICET_IMAGE_DEPTH_NONE) {
        printrank("*** Unknown depth format? ***\n");
    }
}

static void drawCallback(const IceTDouble *projection_matrix,
                         const IceTDouble *modelview_matrix,
                         const IceTFloat *background_color,
                         const IceTInt *readback_viewport,
                         IceTImage result)
{
  /* Don't care about this information. */
    (void)projection_matrix;
    (void)modelview_matrix;
    (void)background_color;
    (void)readback_viewport;

    InitActiveImage(result);
}

static int DoCompressionTest(IceTEnum color_format, IceTEnum depth_format,
                             IceTEnum composite_mode)
{
    IceTInt viewport[4];
    IceTSizeType pixels;
    IceTImage image;
    IceTVoid *imagebuffer;
    IceTSizeType imagesize;
    IceTSparseImage compressedimage;
    IceTVoid *compressedbuffer;
    IceTSparseImage interlacedimage;
    IceTVoid *interlacedbuffer;
    IceTSizeType compressedsize;
    IceTSizeType color_pixel_size;
    IceTSizeType depth_pixel_size;
    IceTSizeType pixel_size;
    IceTSizeType size;
    int result;

    result = TEST_PASSED;

    printstat("Using color format of 0x%x\n", (int)color_format);
    printstat("Using depth format of 0x%x\n", (int)depth_format);
    printstat("Using composite mode of 0x%x\n", (int)composite_mode);

    icetSetColorFormat(color_format);
    icetSetDepthFormat(depth_format);
    icetCompositeMode(composite_mode);

    pixels = SCREEN_WIDTH*SCREEN_HEIGHT;

    printstat("Allocating memory for %dx%d pixel image.\n",
              (int)SCREEN_WIDTH, (int)SCREEN_HEIGHT);
    imagesize = icetImageBufferSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    imagebuffer = malloc(imagesize);
    image = icetImageAssignBuffer(imagebuffer, SCREEN_WIDTH, SCREEN_HEIGHT);

    compressedsize = icetSparseImageBufferSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    compressedbuffer = malloc(compressedsize);
    compressedimage = icetSparseImageAssignBuffer(compressedbuffer,
                                                  SCREEN_WIDTH,
                                                  SCREEN_HEIGHT);

    interlacedbuffer = malloc(compressedsize);
    interlacedimage = icetSparseImageAssignBuffer(interlacedbuffer,
                                                  SCREEN_WIDTH,
                                                  SCREEN_HEIGHT);

  /* Get the number of bytes per pixel.  This is used in checking the
     size of compressed images. */
    icetImageGetColorVoid(image, &color_pixel_size);
    icetImageGetDepthVoid(image, &depth_pixel_size);
    pixel_size = color_pixel_size + depth_pixel_size;
    printstat("Pixel size: color=%d, depth=%d, total=%d\n",
              (int)color_pixel_size, (int)depth_pixel_size, (int)pixel_size);

    printstat("\nCreating worst possible image"
              " (with respect to compression).\n");
    InitPathologicalImage(image);

    printstat("Compressing image.\n");
    icetCompressImage(image, compressedimage);
    size = icetSparseImageGetCompressedBufferSize(compressedimage);
    printstat("Expected size: %d.  Actual size: %d\n",
              (int)(pixel_size*(pixels/2) + 2*sizeof(IceTUShort)*(pixels/2)),
              (int)size);
    if (   (size > compressedsize)
        || (size < pixel_size*(pixels/2)) ) {
        printrank("*** Size differs from expected size!\n");
        result = TEST_FAILED;
    }

    printstat("Interlacing image.\n");
    icetSparseImageInterlace(compressedimage,
                             97,
                             ICET_SI_STRATEGY_BUFFER_0,
                             interlacedimage);
    size = icetSparseImageGetCompressedBufferSize(compressedimage);
    printstat("Expected size: %d.  Actual size: %d\n",
              (int)(pixel_size*(pixels/2) + 2*sizeof(IceTUShort)*(pixels/2)),
              (int)size);
    if (   (size > compressedsize)
        || (size < pixel_size*(pixels/2)) ) {
        printrank("*** Size differs from expected size!\n");
        result = TEST_FAILED;
    }

    printstat("\nCreating a different worst possible image.\n");
    InitActiveImage(image);
    printstat("Compressing image.\n");
    icetCompressImage(image, compressedimage);
    size = icetSparseImageGetCompressedBufferSize(compressedimage);
    printstat("Expected size: %d.  Actual size: %d\n",
           (int)compressedsize, (int)size);
    if ((size > compressedsize) || (size < pixel_size*pixels)) {
        printrank("*** Size differs from expected size!\n");
        result = TEST_FAILED;
    }

    printstat("Interlacing image.\n");
    icetSparseImageInterlace(compressedimage,
                             97,
                             ICET_SI_STRATEGY_BUFFER_0,
                             interlacedimage);
    size = icetSparseImageGetCompressedBufferSize(compressedimage);
    printstat("Expected size: %d.  Actual size: %d\n",
           (int)(pixel_size*(pixels/2) + 2*sizeof(IceTUShort)*(pixels/2)),
           (int)size);
    if (   (size > compressedsize)
        || (size < pixel_size*(pixels/2)) ) {
        printrank("*** Size differs from expected size!\n");
        result = TEST_FAILED;
    }

    printstat("\nCompressing zero size image.\n");
    icetImageSetDimensions(image, 0, 0);
    icetCompressImage(image, compressedimage);
    size = icetSparseImageGetCompressedBufferSize(compressedimage);
    printstat("Expected size: %d.  Actual size: %d\n",
           (int)icetSparseImageBufferSize(0, 0), (int)size);
    if (size > icetSparseImageBufferSize(0, 0)) {
        printrank("*** Size differs from expected size!\n");
        result = TEST_FAILED;
    }

  /* This test can be a little volatile.  The icetGetCompressedTileImage
   * expects certain things to be set correctly by the icetDrawFrame
   * function.  Since we want to call icetGetCompressedTileImage directly,
   * we try to set up these parameters by hand.  It is possible for this
   * test to incorrectly fail if the two functions are mutually changed and
   * this scaffolding is not updated correctly. */
    printstat("\nSetup for actual render.\n");
    icetResetTiles();
    icetAddTile(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    icetDrawCallback(drawCallback);
  /* Do a perfunctory draw to set other state variables. */
    icetDrawFrame(IdentityMatrix, IdentityMatrix, Black);
    viewport[0] = viewport[1] = 0;
    viewport[2] = (IceTInt)SCREEN_WIDTH;  viewport[3] = (IceTInt)SCREEN_HEIGHT;
    icetStateSetIntegerv(ICET_CONTAINED_VIEWPORT, 4, viewport);
    printstat("Now render and get compressed image.\n");
    icetGetCompressedTileImage(0, compressedimage);
    size = icetSparseImageGetCompressedBufferSize(compressedimage);
    printstat("Expected size: %d.  Actual size: %d\n",
              (int)compressedsize, (int)size);
    if ((size > compressedsize) || (size < pixel_size*pixels)) {
        printrank("*** Size differs from expected size!\n");
        result = TEST_FAILED;
    }

    printstat("Cleaning up.\n");
    free(imagebuffer);
    free(compressedbuffer);
    free(interlacedbuffer);
    return result;
}

static int CompressionSizeRun()
{
    int result;

    icetStrategy(ICET_STRATEGY_REDUCE);

    printstat("Compress depth only.\n");
    result = DoCompressionTest(ICET_IMAGE_COLOR_NONE, ICET_IMAGE_DEPTH_FLOAT,
                               ICET_COMPOSITE_MODE_Z_BUFFER);

    printstat("\n\nCompress 8-bit color only.\n");
    if (result == TEST_PASSED) {
        result = DoCompressionTest(ICET_IMAGE_COLOR_RGBA_UBYTE,
                                   ICET_IMAGE_DEPTH_NONE,
                                   ICET_COMPOSITE_MODE_BLEND);
    } else {
        DoCompressionTest(ICET_IMAGE_COLOR_RGBA_UBYTE,
                          ICET_IMAGE_DEPTH_NONE,
                          ICET_COMPOSITE_MODE_BLEND);
    }

    printstat("\n\nCompress 32-bit color only.\n");
    if (result == TEST_PASSED) {
        result = DoCompressionTest(ICET_IMAGE_COLOR_RGBA_FLOAT,
                                   ICET_IMAGE_DEPTH_NONE,
                                   ICET_COMPOSITE_MODE_BLEND);
    } else {
        DoCompressionTest(ICET_IMAGE_COLOR_RGBA_FLOAT,
                          ICET_IMAGE_DEPTH_NONE,
                          ICET_COMPOSITE_MODE_BLEND);
    }

    printstat("\n\nCompress depth and 8-bit color.\n");
    if (result == TEST_PASSED) {
        result = DoCompressionTest(ICET_IMAGE_COLOR_RGBA_UBYTE,
                                   ICET_IMAGE_DEPTH_FLOAT,
                                   ICET_COMPOSITE_MODE_Z_BUFFER);
    } else {
        DoCompressionTest(ICET_IMAGE_COLOR_RGBA_UBYTE,
                          ICET_IMAGE_DEPTH_FLOAT,
                          ICET_COMPOSITE_MODE_Z_BUFFER);
    }

    printstat("\n\nCompress depth and 32-bit color.\n");
    if (result == TEST_PASSED) {
        result = DoCompressionTest(ICET_IMAGE_COLOR_RGBA_FLOAT,
                                   ICET_IMAGE_DEPTH_FLOAT,
                                   ICET_COMPOSITE_MODE_Z_BUFFER);
    } else {
        DoCompressionTest(ICET_IMAGE_COLOR_RGBA_FLOAT,
                          ICET_IMAGE_DEPTH_FLOAT,
                          ICET_COMPOSITE_MODE_Z_BUFFER);
    }

    return result;
}

int CompressionSize(int argc, char *argv[])
{
    /* To remove warning */
    (void)argc;
    (void)argv;

    return run_test(CompressionSizeRun);
}
