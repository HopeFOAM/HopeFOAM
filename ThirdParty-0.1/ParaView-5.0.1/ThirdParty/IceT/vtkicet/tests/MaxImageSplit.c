/* -*- c -*- *****************************************************************
** Copyright (C) 2011 Sandia Corporation
** Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
** the U.S. Government retains certain rights in this software.
**
** This source code is released under the New BSD License.
**
** This tests the ICET_MAX_IMAGE_SPLIT option.  It sets the max image split to
** less than the total number of processes and makes sure that the image is
** still composited correctly.
*****************************************************************************/

#include <IceT.h>
#include "test_codes.h"
#include "test_util.h"

#include <IceTDevContext.h>
#include <IceTDevMatrix.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define PROC_REGION_WIDTH       10
#define PROC_REGION_HEIGHT      10

static void MaxImageSplitDraw(const IceTDouble *projection_matrix,
                              const IceTDouble *modelview_matrix,
                              const IceTFloat *background_color,
                              const IceTInt *readback_viewport,
                              IceTImage result)
{
    IceTDouble full_transform[16];
    IceTSizeType width;
    IceTSizeType height;
    IceTUInt *colors;
    IceTFloat *depths;

    /* Not using this. */
    (void)background_color;

    width = icetImageGetWidth(result);
    height = icetImageGetHeight(result);

    colors = icetImageGetColorui(result);
    depths = icetImageGetDepthf(result);

    /* Get full transform all the way to window coordinates (pixels). */ {
        IceTDouble scale_transform[16];
        IceTDouble translate_transform[16];
        icetMatrixScale(0.5*width, 0.5*height, 0.5, scale_transform);
        icetMatrixTranslate(1.0, 1.0, 1.0, translate_transform);
        icetMatrixMultiply(full_transform,scale_transform,translate_transform);
        icetMatrixPostMultiply(full_transform, projection_matrix);
        icetMatrixPostMultiply(full_transform, modelview_matrix);
    }

    /* Clear out the image (testing purposes only). */ {
        IceTSizeType pixel;
        memset(colors, 0xFC, width*height*sizeof(IceTUInt));
        for (pixel = 0; pixel < width*height; pixel++) {
            depths[pixel] = 1.0;
        }
    }

    /* Set my pixels. */ {
        IceTInt rank;
        IceTSizeType region_y_start;
        IceTSizeType region_x;
        IceTSizeType region_y;

        icetGetIntegerv(ICET_RANK, &rank);
        region_y_start = rank*PROC_REGION_HEIGHT;

        for (region_y = 0; region_y < PROC_REGION_HEIGHT; region_y++) {
            for (region_x = 0; region_x < PROC_REGION_WIDTH; region_x++) {
                IceTDouble object_coord[4];
                IceTDouble window_coord[4];
                IceTSizeType window_pixel[2];
                IceTSizeType readback_lower[2];
                IceTSizeType readback_upper[2];
                IceTBoolean in_readback;

                object_coord[0] = (IceTDouble)region_x;
                object_coord[1] = (IceTDouble)(region_y + region_y_start);
                object_coord[2] = 0.0;
                object_coord[3] = 1.0;

                icetMatrixVectorMultiply(window_coord,
                                         full_transform,
                                         object_coord);

                window_pixel[0]=(IceTSizeType)(window_coord[0]/window_coord[3]);
                window_pixel[1]=(IceTSizeType)(window_coord[1]/window_coord[3]);

                readback_lower[0] = readback_viewport[0];
                readback_lower[1] = readback_viewport[1];
                readback_upper[0] = readback_viewport[0] + readback_viewport[2];
                readback_upper[1] = readback_viewport[1] + readback_viewport[3];

                in_readback  = (readback_lower[0] <= window_pixel[0]);
                in_readback &= (readback_lower[1] <= window_pixel[1]);
                in_readback &= (window_pixel[0] < readback_upper[0]);
                in_readback &= (window_pixel[1] < readback_upper[1]);
                if (in_readback) {
                    IceTSizeType pixel_idx
                        = window_pixel[0] + window_pixel[1]*PROC_REGION_WIDTH;
                    colors[pixel_idx] = rank;
                    depths[pixel_idx] = 0.0;
                }
            }
        }
    }
}

static void MaxImageSplitSetupRender()
{
    IceTInt num_proc;

    icetGetIntegerv(ICET_NUM_PROCESSES, &num_proc);

    icetCompositeMode(ICET_COMPOSITE_MODE_Z_BUFFER);
    icetSetColorFormat(ICET_IMAGE_COLOR_RGBA_UBYTE);
    icetSetDepthFormat(ICET_IMAGE_DEPTH_FLOAT);
    icetDisable(ICET_ORDERED_COMPOSITE);

    icetDrawCallback(MaxImageSplitDraw);

    icetResetTiles();
    icetAddTile(0, 0, PROC_REGION_WIDTH, PROC_REGION_HEIGHT*num_proc, 0);
}

static void MaxImageSplitGetMatrices(IceTDouble *projection_matrix,
                                     IceTDouble *modelview_matrix)
{
    IceTDouble scale_matrix[16];
    IceTDouble transform_matrix[16];
    IceTInt num_proc;
    IceTInt rank;

    icetGetIntegerv(ICET_NUM_PROCESSES, &num_proc);
    icetGetIntegerv(ICET_RANK, &rank);

    /* Make the projection really project pixel positions to normalized clipping
       coordinates. */
    icetMatrixScale(2.0/PROC_REGION_WIDTH,
                    2.0/(PROC_REGION_HEIGHT*num_proc),
                    2.0,
                    scale_matrix);
    icetMatrixTranslate(-1.0, -1.0, -1.0, transform_matrix);
    icetMatrixMultiply(projection_matrix, transform_matrix, scale_matrix);

    /* The modelview just passes pixel positions. */
    icetMatrixIdentity(modelview_matrix);

    /* Based on these matrices, set the region we want the local process to draw
       pixels.  The region is defined in pixels.  The previous matrices
       transform the pixels to screen coordinates, which should then implicitly
       be transformed back to pixel coordinates in the draw function. */
    icetBoundingBoxd(0.0,
                     (IceTDouble)PROC_REGION_WIDTH,
                     (IceTDouble)rank*PROC_REGION_HEIGHT,
                     (IceTDouble)(rank+1)*PROC_REGION_HEIGHT,
                     0.0,
                     1.0);
}

static int MaxImageSplitCheckImage(const IceTImage image)
{
    IceTInt rank;

    icetGetIntegerv(ICET_RANK, &rank);
    if (rank == 0) {
        IceTInt num_proc;
        IceTInt proc;
        const IceTUInt *pixel;

        icetGetIntegerv(ICET_NUM_PROCESSES, &num_proc);

        pixel = icetImageGetColorcui(image);

        for (proc = 0; proc < num_proc; proc++) {
            IceTInt x, y;
            for (y = 0; y < PROC_REGION_HEIGHT; y++) {
                for (x = 0; x < PROC_REGION_WIDTH; x++) {
                    if (*pixel != (IceTUInt)proc) {
                        printrank("**** Found bad pixel!!!! ****\n");
                        printrank("Region for process %d, x = %d, y = %d\n",
                                  proc, x, y);
                        printrank("Reported %d\n", *pixel);
                        return TEST_FAILED;
                    }
                    pixel++;
                }
            }
        }
    }

    return TEST_PASSED;
}

static int MaxImageSplitTryRender()
{
    IceTDouble projection_matrix[16];
    IceTDouble modelview_matrix[16];
    IceTFloat white[4];
    IceTImage image;

    MaxImageSplitSetupRender();
    MaxImageSplitGetMatrices(projection_matrix, modelview_matrix);

    white[0] = white[1] = white[2] = white[3] = 1.0f;

    image = icetDrawFrame(projection_matrix, modelview_matrix, white);

    return MaxImageSplitCheckImage(image);
}

static int MaxImageSplitTryStrategy()
{
    int result = TEST_PASSED;
    int si_strategy_idx;

    icetStrategy(ICET_STRATEGY_SEQUENTIAL);

    for (si_strategy_idx = 0;
         si_strategy_idx < SINGLE_IMAGE_STRATEGY_LIST_SIZE;
         si_strategy_idx++) {
        icetSingleImageStrategy(single_image_strategy_list[si_strategy_idx]);
        printstat("  Trying single image strategy %s\n",
                  icetGetSingleImageStrategyName());
        result += MaxImageSplitTryRender();
    }

    return result;
}

static int MaxImageSplitTryMax()
{
    IceTContext original_context = icetGetContext();
    IceTInt num_proc;
    IceTInt max_image_split;
    IceTInt result = TEST_PASSED;

    icetGetIntegerv(ICET_NUM_PROCESSES, &num_proc);

    for (max_image_split = 1;
         max_image_split/2 < num_proc;
         max_image_split *= 2) {
        char image_split_string[64];
        IceTInt reported_image_split;

        printstat("Trying max image split of %d\n", max_image_split);

#ifdef _WIN32
        sprintf(image_split_string, "ICET_MAX_IMAGE_SPLIT=%d", max_image_split);
        putenv(image_split_string);
#else
        sprintf(image_split_string, "%d", max_image_split);
        setenv("ICET_MAX_IMAGE_SPLIT", image_split_string, ICET_TRUE);
#endif

        /* This is a bit hackish.  The max image split value is set when the
           IceT context is initialized.  Thus, for the environment to take
           effect, we need to make a new context.  To make a new context, we
           need to get the communiator. */
        {
            IceTCommunicator comm = icetGetCommunicator();
            icetCreateContext(comm);
        }

        icetGetIntegerv(ICET_MAX_IMAGE_SPLIT, &reported_image_split);
        if (max_image_split != reported_image_split) {
            printrank("**** Max image split not set correctly!!!! ****\n");
            return TEST_FAILED;
        }

        result += MaxImageSplitTryStrategy();

        /* We no longer need the context we just created. */
        icetDestroyContext(icetGetContext());
        icetSetContext(original_context);
    }

    return result;
}

static int MaxImageSplitRun(void)
{
    return MaxImageSplitTryMax();
}

int MaxImageSplit(int argc, char *argv[])
{
    /* To remove warning. */
    (void)argc;
    (void)argv;

    return run_test(MaxImageSplitRun);
}
