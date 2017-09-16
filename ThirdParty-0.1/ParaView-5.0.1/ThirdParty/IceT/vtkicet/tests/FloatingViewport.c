/* -*- c -*- *****************************************************************
** Copyright (C) 2014 Sandia Corporation
** Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
** the U.S. Government retains certain rights in this software.
**
** This source code is released under the New BSD License.
**
** This test makes sure that the floating viewport feature is working as
** expected. It will make sure that the render is called only once for the
** floating viewport and that the image gets renderd correctly.
*****************************************************************************/

#include <IceT.h>
#include "test_codes.h"
#include "test_util.h"

#include <IceTDevCommunication.h>
#include <IceTDevMatrix.h>

#include <stdlib.h>
#include <stdio.h>

static const IceTFloat g_background_color[4] = { 0.5, 0.5, 0.5, 1.0 };
static const IceTFloat g_foreground_color[4] = { 0.0, 0.25, 0.5, 1.0 };

static IceTInt g_num_render_callbacks;

static IceTBoolean ColorsEqual(const IceTFloat *color1, const IceTFloat *color2)
{
    IceTBoolean result;

    result  = (color1[0] == color2[0]);
    result &= (color1[1] == color2[1]);
    result &= (color1[2] == color2[2]);
    result &= (color1[3] == color2[3]);

    return result;
}

static IceTBoolean CheckPixel(const IceTImage image,
                              IceTSizeType pixel,
                              const IceTFloat *expected_color)
{
    const IceTFloat *colors = icetImageGetColorcf(image);
    if (!ColorsEqual(colors + 4*pixel, expected_color)) {
        IceTUByte *buffer;
        IceTInt rank;
        char filename[255];
        printrank("BAD PIXEL %d\n", pixel);
        printrank("    Expected %f %f %f %f\n",
                  expected_color[0],
                  expected_color[1],
                  expected_color[2],
                  expected_color[3]);
        printrank("    Got %f %f %f %f\n",
                  colors[4*pixel + 0],
                  colors[4*pixel + 1],
                  colors[4*pixel + 2],
                  colors[4*pixel + 3]);
        buffer = malloc(4*icetImageGetWidth(image)*icetImageGetHeight(image));
        icetImageCopyColorub(image, buffer, ICET_IMAGE_COLOR_RGBA_UBYTE);
        icetGetIntegerv(ICET_RANK, &rank);
        sprintf(filename, "FloatingViewport_%d.ppm", rank);
        write_ppm(filename,
                  buffer,
                  icetImageGetWidth(image),
                  icetImageGetHeight(image));
        free(buffer);
        return ICET_FALSE;
    } else {
        return ICET_TRUE;
    }
}

static void RenderFloatingViewport(const IceTDouble *projection_matrix,
                                   const IceTDouble *modelview_matrix,
                                   const IceTFloat *background_color,
                                   const IceTInt *readback_viewport,
                                   IceTImage result)
{
    IceTSizeType width;
    IceTSizeType height;
    IceTFloat *colors;

    /* To remove warning. */
    (void)projection_matrix;
    (void)modelview_matrix;
    (void)background_color;

    printrank("Rendering viewport %d %d %d %d\n",
              readback_viewport[0],
              readback_viewport[1],
              readback_viewport[2],
              readback_viewport[3]);

    g_num_render_callbacks++;

    width = icetImageGetWidth(result);
    height = icetImageGetHeight(result);
    colors = icetImageGetColorf(result);

    /* Set all pixels in the readback_viewport. IceT will ignore the rest. */
    {
        IceTSizeType line_start = width*readback_viewport[1];
        IceTSizeType line;
        for (line = 0; line < readback_viewport[3]; line++) {
            IceTSizeType pixel = line_start + readback_viewport[0];
            IceTSizeType column;
            for (column = 0; column < readback_viewport[2]; column++) {
                colors[4*pixel + 0] = g_foreground_color[0];
                colors[4*pixel + 1] = g_foreground_color[1];
                colors[4*pixel + 2] = g_foreground_color[2];
                colors[4*pixel + 3] = g_foreground_color[3];
                pixel++;
            }
            line_start += width;
        }
    }
}

static void FloatingViewportSetupRender(void)
{
    icetCompositeMode(ICET_COMPOSITE_MODE_BLEND);
    icetSetColorFormat(ICET_IMAGE_COLOR_RGBA_FLOAT);
    icetSetDepthFormat(ICET_IMAGE_DEPTH_NONE);
    icetDisable(ICET_ORDERED_COMPOSITE);
    icetEnable(ICET_CORRECT_COLORED_BACKGROUND);

    /* The strategy should not matter for this test. */
    icetStrategy(ICET_STRATEGY_SEQUENTIAL);

    icetDrawCallback(RenderFloatingViewport);

    icetResetTiles();
    icetAddTile(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    icetAddTile(SCREEN_WIDTH, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 1);
}

static void FloatingViewportGetMatrices(IceTDouble *projection_matrix,
                                        IceTDouble *modelview_matrix)
{
    /* Identity matrices make it easy to predict where geometry is projected. */
    icetMatrixIdentity(projection_matrix);
    icetMatrixIdentity(modelview_matrix);

    /* The geometry is a square straddling the two tiles. */
    icetBoundingBoxd(-0.25, 0.25, -0.5, 0.5, -0.5, 0.5);
}

static IceTBoolean FloatingViewportCheckImage(const IceTImage image)
{
    IceTInt rank;
    IceTSizeType pixel;
    IceTSizeType count;
    IceTSizeType width;
    IceTSizeType height;

    icetGetIntegerv(ICET_RANK, &rank);

    /* Display ranks are 0 and 1. */
    if (rank > 1) { return ICET_TRUE; }

    width = icetImageGetWidth(image);
    height = icetImageGetHeight(image);

    /* Not that in these checks we avoid problems with floating point
     * inaccuracies by only checking parts well within regions. For example,
     * we expect the bottom 25% of an image to be background, but we only
     * check the bottom 20%. */

    /* Bottom 25% of both images should be background. */
    pixel = 0;
    for (count = 0; count < width*(height/5); count++) {
        if (!CheckPixel(image, pixel, g_background_color)) {return ICET_FALSE;}
        pixel++;
    }

    /* Top 25% of both images should be background. */
    pixel = width*(4*height/5);
    for (count = 0; count < width*(height/5); count++) {
        if (!CheckPixel(image, pixel, g_background_color)) {return ICET_FALSE;}
        pixel++;
    }

    /* In the middle 50%, the left 75% is background on rank 0 and the right
     * 75% is background on rank 1. */
    pixel = width*(2*height/5);
    if (rank == 1) { pixel += width - 3*width/5; }
    for (count = 0; count < height/5; count++) {
        IceTSizeType count2;
        for (count2 = 0; count2 < 3*width/5; count2++) {
            if (!CheckPixel(image, pixel+count2, g_background_color)) {
                return ICET_FALSE;
            }
        }
        pixel += width;
    }

    /* In the middle 50%, the right 25% is foreground on rank 0 and the left
     * 25% is foreground on rank 1. */
    pixel = width*(2*height/5);
    if (rank == 0) { pixel += width - width/5; }
    for (count = 0; count < height/5; count++) {
        IceTSizeType count2;
        for (count2 = 0; count2 < width/5; count2++) {
            if (!CheckPixel(image, pixel+count2, g_foreground_color)) {
                return ICET_FALSE;
            }
        }
        pixel += width;
    }

    return ICET_TRUE;
}

static IceTBoolean FloatingViewportTryRender(void)
{
    IceTDouble projection_matrix[16];
    IceTDouble modelview_matrix[16];
    IceTImage image;

    FloatingViewportSetupRender();
    FloatingViewportGetMatrices(projection_matrix, modelview_matrix);

    g_num_render_callbacks = 0;

    image = icetDrawFrame(projection_matrix,
                          modelview_matrix,
                          g_background_color);

    if (g_num_render_callbacks != 1) {
        printrank("**** Rendering callback not called expected number of times. ****\n");
        printrank("Expected 1 call, got %d\n", g_num_render_callbacks);
        return ICET_FALSE;
    }

    return FloatingViewportCheckImage(image);
}

static int FloatingViewportTryMultipleRenders(void)
{
    IceTBoolean success = ICET_TRUE;
    IceTInt trial;

    for (trial = 0; trial < 5; trial++) {
        printstat("Trial render %d\n", trial);
        success &= FloatingViewportTryRender();
    }

    if (success) {
        return TEST_PASSED;
    } else {
        return TEST_FAILED;
    }
}

static int FloatingViewportRun(void)
{
    IceTInt num_proc;

    /* Check to make sure floating viewport is on by default. */
    if (!icetIsEnabled(ICET_FLOATING_VIEWPORT)) {
        printstat("The floating viewport option is not on by default.");
        return TEST_FAILED;
    }

    icetGetIntegerv(ICET_NUM_PROCESSES, &num_proc);
    if (num_proc < 2) {
        printf("Need at least two processes to run FloatingViewport test.");
        return TEST_NOT_RUN;
    }

    return FloatingViewportTryMultipleRenders();
}

int FloatingViewport(int argc, char *argv[])
{
    /* To remove warning. */
    (void)argc;
    (void)argv;

    return run_test(FloatingViewportRun);
}
