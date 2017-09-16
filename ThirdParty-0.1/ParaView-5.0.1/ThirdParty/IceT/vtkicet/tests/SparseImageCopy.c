/* -*- c -*- *****************************************************************
** Copyright (C) 2011 Sandia Corporation
** Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
** the U.S. Government retains certain rights in this software.
**
** This source code is released under the New BSD License.
**
** This test checks the behavior of various ways to copy blocks of pixels
** in sparse images.
*****************************************************************************/

#include "test_codes.h"
#include "test_util.h"

#include <IceTDevImage.h>

#include <stdlib.h>
#include <stdio.h>

/* Encode image position in color. */
#define ACTIVE_COLOR(x, y) \
    ((((x) & 0xFFFF) | 0x8000) | ((((y) & 0xFFFF) | 0x8000) << 16))

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
    IceTUInt *data = icetImageGetColorui(image);
    IceTSizeType width = icetImageGetWidth(image);
    IceTSizeType height = icetImageGetHeight(image);
    IceTSizeType x, y;

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            if (x < (height-y)) {
                data[0] = ACTIVE_COLOR(x, y);
            } else {
                data[0] = 0;
            }
            data++;
        }
    }
}

/* Fills the given image to have data in the upper triangle like this:
 *
 * +-------------+
 * |  \          |
 * |    \        |
 * |      \      |
 * |        \    |
 * |          \  |
 * +-------------+
 *
 * Where the upper half is filled with data and the upper half is background. */
static void UpperTriangleImage(IceTImage image)
{
    IceTUInt *data = icetImageGetColorui(image);
    IceTSizeType width = icetImageGetWidth(image);
    IceTSizeType height = icetImageGetHeight(image);
    IceTSizeType x, y;

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
}

static int CompareSparseImages(const IceTSparseImage image0,
                               const IceTSparseImage image1)
{
    IceTSizeType width;
    IceTSizeType height;
    IceTVoid *image_buffer[2];
    IceTImage image[2];
    IceTUInt *color_buffer[2];
    IceTSizeType i;

    if (   icetSparseImageGetCompressedBufferSize(image0)
        != icetSparseImageGetCompressedBufferSize(image1) ) {
        printrank("Buffer sizes do not match: %d vs %d!\n",
                  icetSparseImageGetCompressedBufferSize(image0),
                  icetSparseImageGetCompressedBufferSize(image1));
        return TEST_FAILED;
    }

    width = icetSparseImageGetWidth(image1);
    height = icetSparseImageGetHeight(image1);

    image_buffer[0] = malloc(icetImageBufferSize(width, height));
    image[0] = icetImageAssignBuffer(image_buffer[0], width, height);
    icetDecompressImage(image0, image[0]);
    color_buffer[0] = icetImageGetColorui(image[0]);

    image_buffer[1] = malloc(icetImageBufferSize(width, height));
    image[1] = icetImageAssignBuffer(image_buffer[1], width, height);
    icetDecompressImage(image1, image[1]);
    color_buffer[1] = icetImageGetColorui(image[1]);

    for (i = 0; i < width*height; i++) {
        if (color_buffer[0][i] != color_buffer[1][i]) {
            printrank("Buffer mismatch at uint %d\n", i);
            printrank("0x%x vs 0x%x\n", color_buffer[0][i], color_buffer[1][i]);
            return TEST_FAILED;
        }
    }

    return TEST_PASSED;
}

static int TrySparseImageCopyPixels(const IceTImage image,
                                    IceTSizeType start,
                                    IceTSizeType end)
{
    IceTVoid *full_sparse_buffer;
    IceTSparseImage full_sparse;
    IceTVoid *compress_sub_buffer;
    IceTSparseImage compress_sub;
    IceTVoid *sparse_copy_buffer;
    IceTSparseImage sparse_copy;

    IceTSizeType width = icetImageGetWidth(image);
    IceTSizeType height = icetImageGetHeight(image);
    IceTSizeType sub_size = end - start;

    int result;

    printstat("Trying sparse image copy from %d to %d\n", start, end);

    full_sparse_buffer = malloc(icetSparseImageBufferSize(width, height));
    full_sparse = icetSparseImageAssignBuffer(full_sparse_buffer,width,height);

    compress_sub_buffer = malloc(icetSparseImageBufferSize(sub_size, 1));
    compress_sub = icetSparseImageAssignBuffer(compress_sub_buffer,
                                               sub_size, 1);

    sparse_copy_buffer = malloc(icetSparseImageBufferSize(sub_size, 1));
    sparse_copy = icetSparseImageAssignBuffer(sparse_copy_buffer,
                                              sub_size, 1);

    icetCompressSubImage(image, start, sub_size, compress_sub);

    icetCompressImage(image, full_sparse);
    icetSparseImageCopyPixels(full_sparse, start, sub_size, sparse_copy);

    result = CompareSparseImages(compress_sub, sparse_copy);

    free(full_sparse_buffer);
    free(compress_sub_buffer);
    free(sparse_copy_buffer);

    return result;
}

static int TestSparseImageCopyPixels(const IceTImage image)
{
#define NUM_OFFSETS 7
    IceTSizeType interesting_offset[NUM_OFFSETS];
    IceTSizeType width = icetImageGetWidth(image);
    IceTSizeType height = icetImageGetHeight(image);
    int start, end;

    if (height <= 20) {
        printstat("Need image height greater than 20.\n");
        return TEST_NOT_RUN;
    }

    interesting_offset[0] = 0;  /* First pixel. */
    interesting_offset[1] = 10*width; /* Some pixels up at left. */
    interesting_offset[2] = 10*width + width/2; /* A bit up in the middle. */
    interesting_offset[3] = width*(height/2) + height/2; /* Middle at triangle diagonal. */
    interesting_offset[4] = width*(height-10); /* Some pixels from top at left. */
    interesting_offset[5] = width*(height-10) + width/2; /* Some pixels from top in middle. */
    interesting_offset[6] = width*height; /* Last pixel. */

    for (start = 0; start < NUM_OFFSETS; start++) {
        for (end = start+1; end < NUM_OFFSETS; end++) {
            int result;
            result = TrySparseImageCopyPixels(image,
                                              interesting_offset[start],
                                              interesting_offset[end]);
            if (result != TEST_PASSED) return result;
        }
    }

    return TEST_PASSED;
#undef NUM_OFFSETS
}

static int TestSparseImageSplit(const IceTImage image)
{
#define NUM_PARTITIONS 7
    IceTVoid *full_sparse_buffer;
    IceTSparseImage full_sparse;
    IceTVoid *sparse_partition_buffer[NUM_PARTITIONS];
    IceTSparseImage sparse_partition[NUM_PARTITIONS];
    IceTSizeType offsets[NUM_PARTITIONS];
    IceTVoid *compare_sparse_buffer;
    IceTSparseImage compare_sparse;

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

    full_sparse_buffer = malloc(icetSparseImageBufferSize(width, height));
    full_sparse = icetSparseImageAssignBuffer(full_sparse_buffer,width,height);

    for (partition = 0; partition < NUM_PARTITIONS; partition++) {
        sparse_partition_buffer[partition]
            = malloc(icetSparseImageBufferSize(num_partition_pixels, 1));
        sparse_partition[partition]
            = icetSparseImageAssignBuffer(sparse_partition_buffer[partition],
                                          num_partition_pixels, 1);
    }

    compare_sparse_buffer
        = malloc(icetSparseImageBufferSize(num_partition_pixels, 1));
    compare_sparse
        = icetSparseImageAssignBuffer(compare_sparse_buffer,
                                      num_partition_pixels, 1);

    icetCompressImage(image, full_sparse);

    printstat("Spliting image %d times\n", NUM_PARTITIONS);
    icetSparseImageSplit(full_sparse,
                         0,
                         NUM_PARTITIONS,
                         NUM_PARTITIONS,
                         sparse_partition,
                         offsets);
    for (partition = 0; partition < NUM_PARTITIONS; partition++) {
        IceTInt result;
        icetCompressSubImage(image,
                             offsets[partition],
                             icetSparseImageGetNumPixels(
                                                   sparse_partition[partition]),
                             compare_sparse);
        printstat("    Comparing partition %d\n", partition);
        result = CompareSparseImages(compare_sparse,
                                     sparse_partition[partition]);
        if (result != TEST_PASSED) return result;
    }

    printstat("Spliting image %d times with first partition in place.\n",
              NUM_PARTITIONS);
    sparse_partition[0] = full_sparse;
    icetSparseImageSplit(full_sparse,
                         0,
                         NUM_PARTITIONS,
                         NUM_PARTITIONS,
                         sparse_partition,
                         offsets);
    for (partition = 0; partition < NUM_PARTITIONS; partition++) {
        IceTInt result;
        icetCompressSubImage(image,
                             offsets[partition],
                             icetSparseImageGetNumPixels(
                                                   sparse_partition[partition]),
                             compare_sparse);
        printstat("    Comparing partition %d\n", partition);
        result = CompareSparseImages(compare_sparse,
                                     sparse_partition[partition]);
        if (result != TEST_PASSED) return result;
    }

    free(full_sparse_buffer);
    for (partition = 0; partition < NUM_PARTITIONS; partition++) {
        free(sparse_partition_buffer[partition]);
    }
    free(compare_sparse_buffer);

    return TEST_PASSED;
#undef NUM_PARTITIONS
}

static int SparseImageCopyRun()
{
    IceTVoid *imagebuffer;
    IceTImage image;

    icetSetColorFormat(ICET_IMAGE_COLOR_RGBA_UBYTE);
    icetSetDepthFormat(ICET_IMAGE_DEPTH_NONE);
    icetCompositeMode(ICET_COMPOSITE_MODE_BLEND);

    imagebuffer = malloc(icetImageBufferSize(SCREEN_WIDTH, SCREEN_HEIGHT));
    image = icetImageAssignBuffer(imagebuffer, SCREEN_WIDTH, SCREEN_HEIGHT);

    printstat("\n********* Creating lower triangle image\n");
    LowerTriangleImage(image);

    if (TestSparseImageCopyPixels(image) != TEST_PASSED) {
        return TEST_FAILED;
    }
    if (TestSparseImageSplit(image) != TEST_PASSED) {
        return TEST_FAILED;
    }

    printstat("\n********* Creating upper triangle image\n");
    UpperTriangleImage(image);

    if (TestSparseImageCopyPixels(image) != TEST_PASSED) {
        return TEST_FAILED;
    }
    if (TestSparseImageSplit(image) != TEST_PASSED) {
        return TEST_FAILED;
    }

    free(imagebuffer);

    return TEST_PASSED;
}

int SparseImageCopy(int argc, char *argv[])
{
    /* To remove warning */
    (void)argc;
    (void)argv;

    return run_test(SparseImageCopyRun);
}
