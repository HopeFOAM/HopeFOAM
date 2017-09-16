/* -*- c -*- *****************************************************************
** Copyright (C) 2011 Sandia Corporation
** Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
** the U.S. Government retains certain rights in this software.
**
** This source code is released under the New BSD License.
**
** This tests the ICET_CORRECT_COLORED_BACKGROUND option.
** This tests the ICET_MAX_IMAGE_SPLIT option.  It sets the max image split to
** less than the total number of processes and makes sure that the image is
** still composited correctly.
*****************************************************************************/

#include <IceT.h>
#include "test_codes.h"
#include "test_util.h"

#include <IceTDevContext.h>
#include <IceTDevImage.h>
#include <IceTDevMatrix.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define PROC_REGION_WIDTH       10
#define PROC_REGION_HEIGHT      10

static const IceTFloat g_background_color[4] = { 0.5, 0.5, 0.5, 1.0 };
static const IceTFloat g_foreground_color[4] = { 0.0, 0.25, 0.5, 0.5 };
static const IceTFloat g_blended_color[4] = { 0.25, 0.5, 0.75, 1.0 };

static IceTBoolean ColorsEqual(const IceTFloat *color1, const IceTFloat *color2)
{
    IceTBoolean result;

    result  = (color1[0] == color2[0]);
    result &= (color1[1] == color2[1]);
    result &= (color1[2] == color2[2]);
    result &= (color1[3] == color2[3]);

    return result;
}

static void BackgroundCorrectDraw(const IceTDouble *projection_matrix,
                                  const IceTDouble *modelview_matrix,
                                  const IceTFloat *background_color,
                                  const IceTInt *readback_viewport,
                                  IceTImage result)
{
    IceTDouble full_transform[16];
    IceTSizeType width;
    IceTSizeType height;
    IceTFloat *colors;
    IceTFloat blended_color[4];

    /* This is mostly done for completeness.  Because we are blending and
       correcting the color, we totally expect background_color to be all zeros,
       and therefore blended_color should be equal to g_foreground_color.  The
       real blending will happen within IceT under the covers. */
    ICET_BLEND_FLOAT(g_foreground_color, background_color, blended_color);

    width = icetImageGetWidth(result);
    height = icetImageGetHeight(result);

    colors = icetImageGetColorf(result);

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
        for (pixel = 0; pixel < width*height; pixel++) {
            colors[pixel] = -1.0;
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
                    colors[4*pixel_idx + 0] = blended_color[0];
                    colors[4*pixel_idx + 1] = blended_color[1];
                    colors[4*pixel_idx + 2] = blended_color[2];
                    colors[4*pixel_idx + 3] = blended_color[3];
                }
            }
        }
    }
}

static void BackgroundCorrectSetupRender()
{
    IceTInt num_proc;

    icetGetIntegerv(ICET_NUM_PROCESSES, &num_proc);

    icetCompositeMode(ICET_COMPOSITE_MODE_BLEND);
    icetSetColorFormat(ICET_IMAGE_COLOR_RGBA_FLOAT);
    icetSetDepthFormat(ICET_IMAGE_DEPTH_NONE);
    icetDisable(ICET_ORDERED_COMPOSITE);
    icetEnable(ICET_CORRECT_COLORED_BACKGROUND);

    icetDrawCallback(BackgroundCorrectDraw);

    icetResetTiles();
    icetAddTile(0, 0, PROC_REGION_WIDTH, PROC_REGION_HEIGHT*(num_proc+1), 0);
}

static void BackgroundCorrectGetMatrices(IceTDouble *projection_matrix,
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
                    2.0/(PROC_REGION_HEIGHT*(num_proc+1)),
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

static IceTBoolean BackgroundCorrectCheckImageRegion(
                                                const IceTFloat **pixel_p,
                                                const IceTFloat *expected_color)
{
    IceTInt x, y;
    const IceTFloat *pixel = *pixel_p;
    for (y = 0; y < PROC_REGION_HEIGHT; y++) {
        for (x = 0; x < PROC_REGION_WIDTH; x++) {
            if (!ColorsEqual(pixel, expected_color)) {
                *pixel_p = pixel;
                printrank("**** Found bad pixel!!!! ****\n");
                printrank("Region location x = %d, y = %d\n", x, y);
                printrank("Got color %f %f %f %f\n",
                          pixel[0], pixel[1], pixel[2], pixel[3]);
                printrank("Expected %f %f %f %f\n",
                          expected_color[0],
                          expected_color[1],
                          expected_color[2],
                          expected_color[3]);
                return ICET_FALSE;
            }
            pixel += 4;
        }
    }

    *pixel_p = pixel;
    return ICET_TRUE;
}

static int BackgroundCorrectCheckImage(const IceTImage image)
{
    IceTInt rank;

    icetGetIntegerv(ICET_RANK, &rank);
    if (rank == 0) {
        IceTInt num_proc;
        IceTInt proc;
        const IceTFloat *pixel;

        icetGetIntegerv(ICET_NUM_PROCESSES, &num_proc);

        pixel = icetImageGetColorcf(image);

        for (proc = 0; proc < num_proc; proc++) {
            if (!BackgroundCorrectCheckImageRegion(&pixel, g_blended_color)) {
                printrank("For process %d\n", proc);
                if (ColorsEqual(pixel, g_foreground_color)) {
                    printrank("Pixel never blended with background.\n");
                }
                return TEST_FAILED;
            }
        }

        /* Also check a final patch on top that should be background. */
        if (!BackgroundCorrectCheckImageRegion(&pixel, g_background_color)) {
            printrank("For background area\n");
            return TEST_FAILED;
        }
    }

    return TEST_PASSED;
}

static int BackgroundCorrectTryRender()
{
    IceTDouble projection_matrix[16];
    IceTDouble modelview_matrix[16];
    IceTImage image;

    BackgroundCorrectSetupRender();
    BackgroundCorrectGetMatrices(projection_matrix, modelview_matrix);

    image = icetDrawFrame(projection_matrix,
                          modelview_matrix,
                          g_background_color);

    return BackgroundCorrectCheckImage(image);
}

static int BackgroundCorrectTryStrategy()
{
    int result = TEST_PASSED;
    int strategy_idx;

    for (strategy_idx = 0; strategy_idx < STRATEGY_LIST_SIZE; strategy_idx++) {
        icetStrategy(strategy_list[strategy_idx]);
        printstat("Trying strategy %s\n", icetGetStrategyName());
        result += BackgroundCorrectTryRender();
    }

    return result;
}

static int BackgroundCorrectRun(void)
{
    return BackgroundCorrectTryStrategy();
}

int BackgroundCorrect(int argc, char *argv[])
{
    /* To remove warning. */
    (void)argc;
    (void)argv;

    return run_test(BackgroundCorrectRun);
}
