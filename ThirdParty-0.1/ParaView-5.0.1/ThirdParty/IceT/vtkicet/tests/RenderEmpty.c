/* -*- c -*- *****************************************************************
** Copyright (C) 2013 Sandia Corporation
** Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
** the U.S. Government retains certain rights in this software.
**
** This source code is released under the New BSD License.
**
** This tests the ICET_RENDER_EMPTY_IMAGES option. It makes sure that the
** render callback is invoked even if the option is on and not if the option
** is off.
*****************************************************************************/

#include <IceT.h>
#include "test_codes.h"
#include "test_util.h"

#include <IceTDevMatrix.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static const IceTFloat g_background_color[4] = { 0.5, 0.5, 0.5, 1.0 };
static const IceTFloat g_foreground_color[4] = { 0.0, 0.25, 0.5, 1.0 };

IceTBoolean g_render_callback_invoked;
IceTBoolean g_render_callback_parameters_correct;

static IceTBoolean ColorsEqual(const IceTFloat *color1, const IceTFloat *color2)
{
    IceTBoolean result;

    result  = (color1[0] == color2[0]);
    result &= (color1[1] == color2[1]);
    result &= (color1[2] == color2[2]);
    result &= (color1[3] == color2[3]);

    return result;
}

static void RenderEmptyDraw(const IceTDouble *projection_matrix,
                            const IceTDouble *modelview_matrix,
                            const IceTFloat *background_color,
                            const IceTInt *readback_viewport,
                            IceTImage result)
{
    IceTSizeType width;
    IceTSizeType height;
    IceTFloat *colors;

    /* Report that the callback was invoked. */
    g_render_callback_invoked = ICET_TRUE;

    width = icetImageGetWidth(result);
    height = icetImageGetHeight(result);
    colors = icetImageGetColorf(result);

    /* Set all pixels to something invalid.  IceT should be ignoring them. */
    {
        IceTSizeType pixel;
        for (pixel = 0; pixel < width*height; pixel++) {
            colors[4*pixel + 0] = g_foreground_color[0];
            colors[4*pixel + 1] = g_foreground_color[1];
            colors[4*pixel + 2] = g_foreground_color[2];
            colors[4*pixel + 3] = g_foreground_color[3];
        }
    }

    g_render_callback_parameters_correct = ICET_TRUE;

    /* Check to make sure that readback_viewport is actually empty. If it is
     * not, we can expect an error later when we check the image, but put a
     * warning here to help diagnose where the problem is. */

    if ((readback_viewport[2] != 0) || (readback_viewport[3] != 0)) {
        printrank("Got a readback_viewport with a positive dimension: %dx%d\n",
                  readback_viewport[2], readback_viewport[3]);
        g_render_callback_parameters_correct = ICET_FALSE;
    }

    /* Even though IceT should ignore anything in the render, it should still
     * pass valid rendering parameters. Check them now. */
    if ( (background_color[0] != 0.0f)
         || (background_color[1] != 0.0f)
         || (background_color[2] != 0.0f)
         || (background_color[3] != 0.0f)) {
        printrank("Got a bad background color %f %f %f %f.\n",
                  background_color[0],
                  background_color[1],
                  background_color[2],
                  background_color[3]);
        g_render_callback_parameters_correct = ICET_FALSE;
    }
    {
        int column_index;
        for (column_index = 0; column_index < 4; column_index++) {
            int row_index;
            for (row_index = 0; row_index < 4; row_index++) {
                IceTDouble projection_value =
                        ICET_MATRIX(projection_matrix, row_index, column_index);
                IceTDouble modelview_value =
                        ICET_MATRIX(modelview_matrix, row_index, column_index);
                IceTDouble expected_value =
                        (row_index == column_index) ? 1.0 : 0.0;
                if (projection_value != expected_value) {
                    printrank("Got bad projection matrix, "
                              "row=%d, column=%d, value=%lf\n",
                              row_index, column_index, projection_value);
                }
                if (modelview_value != expected_value) {
                    printrank("Got bad modelview matrix, "
                              "row=%d, column=%d, value=%lf\n",
                              row_index, column_index, modelview_value);
                }
            }
        }
    }
}

static void RenderEmptySetupRender()
{
    icetCompositeMode(ICET_COMPOSITE_MODE_BLEND);
    icetSetColorFormat(ICET_IMAGE_COLOR_RGBA_FLOAT);
    icetSetDepthFormat(ICET_IMAGE_DEPTH_NONE);
    icetDisable(ICET_ORDERED_COMPOSITE);
    icetEnable(ICET_CORRECT_COLORED_BACKGROUND);

    /* Sequential rendering means that the compositor will not try to skip
     * empty tiles. */
    icetStrategy(ICET_STRATEGY_SEQUENTIAL);

    icetDrawCallback(RenderEmptyDraw);

    icetResetTiles();
    icetAddTile(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
}

static void RenderEmptyGetMatrices(IceTDouble *projection_matrix,
                                   IceTDouble *modelview_matrix)
{
    /* Don't really care about the matricies.  Just use identities. */
    icetMatrixIdentity(projection_matrix);
    icetMatrixIdentity(modelview_matrix);

    /* Declare the geometry well outside the view. */
    icetBoundingBoxd(1000.0, 2000.0, 1000.0, 2000.0, 1000.0, 2000.0);
}

static int RenderEmptyCheckImage(const IceTImage image)
{
    IceTInt rank;

    icetGetIntegerv(ICET_RANK, &rank);
    if (rank == 0) {
        const IceTFloat *buffer;
        IceTSizeType x, y;

        buffer = icetImageGetColorcf(image);

        for (y = 0; y < SCREEN_HEIGHT; y++) {
            for (x = 0; x < SCREEN_WIDTH; x++) {
                const IceTFloat *pixel = buffer + 4*(y*SCREEN_WIDTH + x);
                if (!ColorsEqual(pixel, g_background_color)) {
                    printrank("**** Found bad pixel!!!! ****\n");
                    printrank("Region location x = %d, y = %d\n", x, y);
                    printrank("Got color %f %f %f %f\n",
                              pixel[0], pixel[1], pixel[2], pixel[3]);
                    printrank("Expected %f %f %f %f\n",
                              g_background_color[0],
                              g_background_color[1],
                              g_background_color[2],
                              g_background_color[3]);
                    return TEST_FAILED;
                }
            }
        }
    }

    return TEST_PASSED;
}

static int RenderEmptyTryRender()
{
    IceTDouble projection_matrix[16];
    IceTDouble modelview_matrix[16];
    IceTImage image;

    RenderEmptySetupRender();
    RenderEmptyGetMatrices(projection_matrix, modelview_matrix);

    image = icetDrawFrame(projection_matrix,
                          modelview_matrix,
                          g_background_color);

    return RenderEmptyCheckImage(image);
}

static int RenderEmptyTryOptions()
{
    int result = TEST_PASSED;

    printstat("ICET_RENDER_EMPTY_IMAGES disabled. "
              "Should NOT be in draw callback.\n");
    icetDisable(ICET_RENDER_EMPTY_IMAGES);
    g_render_callback_invoked = ICET_FALSE;
    g_render_callback_parameters_correct = ICET_FALSE;
    result += RenderEmptyTryRender();
    if (g_render_callback_invoked) {
        printrank("**** The draw callback was called ****\n");
        result = TEST_FAILED;
    }

    /* Second call, empty renders should be called. */
    printstat("ICET_RENDER_EMPTY_IMAGES enabled. "
              "Draw callback should be invoked everywhere.\n");
    icetEnable(ICET_RENDER_EMPTY_IMAGES);
    g_render_callback_invoked = ICET_FALSE;
    g_render_callback_parameters_correct = ICET_FALSE;
    result += RenderEmptyTryRender();
    if (!g_render_callback_invoked) {
        printrank("**** The draw callback was not called ****\n");
        result = TEST_FAILED;
    }
    if (!g_render_callback_parameters_correct) {
        printrank("**** Matricies were wrong in callback ****\n");
        result = TEST_FAILED;
    }

    return result;
}

static int RenderEmptyRun(void)
{
    return RenderEmptyTryOptions();
}

int RenderEmpty(int argc, char *argv[])
{
    /* To remove warning. */
    (void)argc;
    (void)argv;

    return run_test(RenderEmptyRun);
}
