/* -*- c -*- *****************************************************************
** Copyright (C) 2011 Sandia Corporation
** Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
** the U.S. Government retains certain rights in this software.
**
** This source code is released under the New BSD License.
**
** This test check to make sure that when an image is interlaced, split,
** and then combined back together, all the pixels are reconstructed
** correctly.
*****************************************************************************/

#include "test_codes.h"
#include "test_util.h"

#include <IceTDevImage.h>

#include <stdlib.h>
#include <stdio.h>

/* Encode image position in color. */
#define ACTIVE_COLOR(x, y) \
    ((((x) & 0xFFFF) | 0x8000) | ((((y) & 0xFFFF) | 0x8000) << 16))

/* Just something to set in the depth. */
#define ACTIVE_DEPTH(x, y) \
    (((x)+(y)+1.0)/10000.0)

/* Fills the given image to have data in the lower triangle like this:
 *
 * +-------------+
 * |  \          |
 * |    \        |
 * |      \      |
 * |        \    |
 * |          \  |
 * +-------------+
 *
 * Where the lower half is filled with data and the upper half is background. */
static void LowerTriangleImage(IceTImage image)
{
    IceTSizeType width = icetImageGetWidth(image);
    IceTSizeType height = icetImageGetHeight(image);
    IceTSizeType x, y;

    if (icetImageGetColorFormat(image) == ICET_IMAGE_COLOR_RGBA_UBYTE) {
        IceTUInt *data = icetImageGetColorui(image);
        for (y = 0; y < height; y++) {
            for (x = 0; x < width; x++) {
                if ((height-y) < x) {
                    data[0] = ACTIVE_COLOR(x, y);
                } else {
                    data[0] = 0;
                }
                data++;
            }
        }
    } else if (icetImageGetColorFormat(image) == ICET_IMAGE_COLOR_RGBA_FLOAT) {
        IceTFloat *data = icetImageGetColorf(image);
        for (y = 0; y < height; y++) {
            for (x = 0; x < width; x++) {
                if ((height-y) < x) {
                    data[0] = (float)x;
                    data[1] = (float)y;
                    data[2] = 0.0;
                    data[3] = 1.0;
                } else {
                    data[0] = data[1] = data[2] = data[3] = 0.0;
                }
                data += 4;
            }
        }
    } else if (icetImageGetColorFormat(image) == ICET_IMAGE_COLOR_NONE) {
        /* Do nothing. */
    } else {
        printrank("ERROR: Encountered unknown color format.");
    }

    if (icetImageGetDepthFormat(image) == ICET_IMAGE_DEPTH_FLOAT) {
        IceTFloat *data = icetImageGetDepthf(image);
        for (y = 0; y < height; y++) {
            for (x = 0; x < width; x++) {
                if ((height-y) < x) {
                    data[0] = ACTIVE_DEPTH(x, y);
                } else {
                    data[0] = 0.0;
                }
                data++;
            }
        }
    } else if (icetImageGetDepthFormat(image) == ICET_IMAGE_DEPTH_NONE) {
        /* Do nothing. */
    } else {
        printrank("ERROR: Encountered unknown depth format.");
    }
}

/* Fills the given to be completely full. */
static void FullImage(IceTImage image)
{
    IceTSizeType width = icetImageGetWidth(image);
    IceTSizeType height = icetImageGetHeight(image);
    IceTSizeType x, y;

    if (icetImageGetColorFormat(image) == ICET_IMAGE_COLOR_RGBA_UBYTE) {
        IceTUInt *data = icetImageGetColorui(image);
        for (y = 0; y < height; y++) {
            for (x = 0; x < width; x++) {
                data[0] = ACTIVE_COLOR(x, y);
                data++;
            }
        }
    } else if (icetImageGetColorFormat(image) == ICET_IMAGE_COLOR_RGBA_FLOAT) {
        IceTFloat *data = icetImageGetColorf(image);
        for (y = 0; y < height; y++) {
            for (x = 0; x < width; x++) {
                data[0] = (float)x;
                data[1] = (float)y;
                data[2] = 0.0;
                data[3] = 1.0;
                data += 4;
            }
        }
    } else if (icetImageGetColorFormat(image) == ICET_IMAGE_COLOR_NONE) {
        /* Do nothing. */
    } else {
        printrank("ERROR: Encountered unknown color format.");
    }

    if (icetImageGetDepthFormat(image) == ICET_IMAGE_DEPTH_FLOAT) {
        IceTFloat *data = icetImageGetDepthf(image);
        for (y = 0; y < height; y++) {
            for (x = 0; x < width; x++) {
                data[0] = ACTIVE_DEPTH(x, y);
                data++;
            }
        }
    } else if (icetImageGetDepthFormat(image) == ICET_IMAGE_DEPTH_NONE) {
        /* Do nothing. */
    } else {
        printrank("ERROR: Encountered unknown depth format.");
    }
}

static IceTBoolean CompareImageColors(const IceTImage image_a,
                                      const IceTImage image_b)
{
    IceTSizeType width;
    IceTSizeType height;
    IceTUByte *data_a;
    IceTUByte *data_b;
    IceTUByte *data_a_start;
    IceTUByte *data_b_start;
    IceTSizeType x;
    IceTSizeType y;

    if (icetImageGetColorFormat(image_a) != icetImageGetColorFormat(image_b)) {
        printrank("ERROR: Image formats do not match.\n");
        return ICET_FALSE;
    }

    if (   (icetImageGetWidth(image_a) != icetImageGetWidth(image_b))
        || (icetImageGetHeight(image_a) != icetImageGetHeight(image_b)) ) {
        printrank("ERROR: Images have different dimensions.\n");
        return ICET_FALSE;
    }

    if (icetImageGetColorFormat(image_a) == ICET_IMAGE_COLOR_NONE) {
        /* No color data, trivially the same. */
        return ICET_TRUE;
    }

    width = icetImageGetWidth(image_a);
    height = icetImageGetHeight(image_a);

    data_a = data_a_start = malloc(width*height*4);
    icetImageCopyColorub(image_a, data_a, ICET_IMAGE_COLOR_RGBA_UBYTE);

    data_b = data_b_start = malloc(width*height*4);
    icetImageCopyColorub(image_b, data_b, ICET_IMAGE_COLOR_RGBA_UBYTE);

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            if (   (data_a[0] != data_b[0])
                || (data_a[1] != data_b[1])
                || (data_a[2] != data_b[2])
                || (data_a[3] != data_b[3]) ) {
                printrank("ERROR: Encountered bad pixel @ (%d,%d).\n", x, y);
                printrank("Expected (%d, %d, %d, %d)\n",
                          data_a[0], data_a[1], data_a[2], data_a[3]);
                printrank("Got (%d, %d, %d, %d)\n",
                          data_b[0], data_b[1], data_b[2], data_b[3]);
                return ICET_FALSE;
            }
            data_a += 4;
            data_b += 4;
        }
    }

    free(data_a_start);
    free(data_b_start);

    return ICET_TRUE;
}

static IceTBoolean CompareImageDepths(const IceTImage image_a,
                                      const IceTImage image_b)
{
    IceTSizeType width;
    IceTSizeType height;
    IceTFloat *data_a;
    IceTFloat *data_b;
    IceTFloat *data_a_start;
    IceTFloat *data_b_start;
    IceTSizeType x;
    IceTSizeType y;

    if (icetImageGetDepthFormat(image_a) != icetImageGetDepthFormat(image_b)) {
        printrank("ERROR: Image formats do not match.\n");
        return ICET_FALSE;
    }

    if (   (icetImageGetWidth(image_a) != icetImageGetWidth(image_b))
        || (icetImageGetHeight(image_a) != icetImageGetHeight(image_b)) ) {
        printrank("ERROR: Images have different dimensions.\n");
        return ICET_FALSE;
    }

    if (icetImageGetDepthFormat(image_a) == ICET_IMAGE_DEPTH_NONE) {
        /* No depth data, trivially the same. */
        return ICET_TRUE;
    }

    width = icetImageGetWidth(image_a);
    height = icetImageGetHeight(image_a);

    data_a = data_a_start = malloc(width*height*4);
    icetImageCopyDepthf(image_a, data_a, ICET_IMAGE_DEPTH_FLOAT);

    data_b = data_b_start = malloc(width*height*4);
    icetImageCopyDepthf(image_b, data_b, ICET_IMAGE_DEPTH_FLOAT);

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            if (data_a[0] != data_b[0]) {
                printrank("ERROR: Encountered bad pixel @ (%d,%d).\n", x, y);
                printrank("Expected %f, got %f\n", data_a[0], data_b[0]);
                return ICET_FALSE;
            }
            data_a++;
            data_b++;
        }
    }

    free(data_a_start);
    free(data_b_start);

    return ICET_TRUE;
}

static int TestInterlaceSplit(const IceTImage image)
{
#define NUM_PARTITIONS 13
    IceTVoid *original_sparse_buffer;
    IceTSparseImage original_sparse;
    IceTVoid *interlaced_sparse_buffer;
    IceTSparseImage interlaced_sparse;
    IceTVoid *sparse_partition_buffer[NUM_PARTITIONS];
    IceTSparseImage sparse_partition[NUM_PARTITIONS];
    IceTSizeType offsets[NUM_PARTITIONS];
    IceTVoid *reconstruction_buffer;
    IceTImage reconstruction;

    IceTSizeType width;
    IceTSizeType height;
    IceTSizeType num_partition_pixels;

    IceTInt partition;

    width = icetImageGetWidth(image);
    height = icetImageGetHeight(image);
    num_partition_pixels
        = icetSparseImageSplitPartitionNumPixels(width*height,
                                                 NUM_PARTITIONS,
                                                 NUM_PARTITIONS);

    original_sparse_buffer = malloc(icetSparseImageBufferSize(width, height));
    original_sparse = icetSparseImageAssignBuffer(original_sparse_buffer,
                                                  width,
                                                  height);

    interlaced_sparse_buffer = malloc(icetSparseImageBufferSize(width, height));
    interlaced_sparse = icetSparseImageAssignBuffer(interlaced_sparse_buffer,
                                                    width,
                                                    height);

    for (partition = 0; partition < NUM_PARTITIONS; partition++) {
        sparse_partition_buffer[partition]
            = malloc(icetSparseImageBufferSize(num_partition_pixels, 1));
        sparse_partition[partition]
            = icetSparseImageAssignBuffer(sparse_partition_buffer[partition],
                                          num_partition_pixels, 1);
    }

    reconstruction_buffer = malloc(icetImageBufferSize(width, height));
    reconstruction = icetImageAssignBuffer(reconstruction_buffer,width,height);

    icetCompressImage(image, original_sparse);

    printstat("Interlacing image for %d pieces\n", NUM_PARTITIONS);
    icetSparseImageInterlace(original_sparse,
                             NUM_PARTITIONS,
                             ICET_SI_STRATEGY_BUFFER_0,
                             interlaced_sparse);

    printstat("Splitting image %d times\n", NUM_PARTITIONS);
    icetSparseImageSplit(interlaced_sparse,
                         0,
                         NUM_PARTITIONS,
                         NUM_PARTITIONS,
                         sparse_partition,
                         offsets);

    printstat("Reconstructing image.\n");
    for (partition = 0; partition < NUM_PARTITIONS; partition++) {
        IceTSizeType real_offset = icetGetInterlaceOffset(partition,
                                                          NUM_PARTITIONS,
                                                          width*height);
        icetDecompressSubImage(sparse_partition[partition],
                               real_offset,
                               reconstruction);
    }

    if (!CompareImageColors(image, reconstruction)) { return TEST_FAILED; }
    if (!CompareImageDepths(image, reconstruction)) { return TEST_FAILED; }

    free(original_sparse_buffer);
    free(interlaced_sparse_buffer);
    for (partition = 0; partition < NUM_PARTITIONS; partition++) {
        free(sparse_partition_buffer[partition]);
    }
    free(reconstruction_buffer);

    return TEST_PASSED;
}

static int InterlaceRunFormat()
{
    IceTVoid *imagebuffer;
    IceTImage image;
    int result;

    imagebuffer = malloc(icetImageBufferSize(SCREEN_WIDTH, SCREEN_HEIGHT));
    image = icetImageAssignBuffer(imagebuffer, SCREEN_WIDTH, SCREEN_HEIGHT);

    printstat("\n********* Creating lower triangle image\n");
    LowerTriangleImage(image);

    result = TestInterlaceSplit(image);
    if (result != TEST_PASSED) { return result; }

    printstat("\n********* Creating full image\n");
    FullImage(image);

    result = TestInterlaceSplit(image);
    if (result != TEST_PASSED) { return result; }

    free(imagebuffer);

    return TEST_PASSED;
}

static int InterlaceRun()
{
    int result;

    icetSetDepthFormat(ICET_IMAGE_DEPTH_NONE);
    icetCompositeMode(ICET_COMPOSITE_MODE_BLEND);

    icetSetColorFormat(ICET_IMAGE_COLOR_RGBA_UBYTE);
    result = InterlaceRunFormat();
    if (result != TEST_PASSED) { return result; }

    icetSetColorFormat(ICET_IMAGE_COLOR_RGBA_FLOAT);
    result = InterlaceRunFormat();
    if (result != TEST_PASSED) { return result; }

    icetSetDepthFormat(ICET_IMAGE_DEPTH_FLOAT);
    icetCompositeMode(ICET_COMPOSITE_MODE_Z_BUFFER);

    icetSetColorFormat(ICET_IMAGE_COLOR_RGBA_UBYTE);
    result = InterlaceRunFormat();
    if (result != TEST_PASSED) { return result; }

    icetSetColorFormat(ICET_IMAGE_COLOR_RGBA_FLOAT);
    result = InterlaceRunFormat();
    if (result != TEST_PASSED) { return result; }

    icetSetColorFormat(ICET_IMAGE_COLOR_NONE);
    result = InterlaceRunFormat();
    if (result != TEST_PASSED) { return result; }

    return TEST_PASSED;
}

int Interlace(int argc, char *argv[])
{
    /* To remove warning */
    (void)argc;
    (void)argv;

    return run_test(InterlaceRun);
}
