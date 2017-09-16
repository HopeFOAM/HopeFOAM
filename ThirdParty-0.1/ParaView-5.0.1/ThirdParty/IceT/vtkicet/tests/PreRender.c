/* -*- c -*- *****************************************************************
** Copyright (C) 2014 Sandia Corporation
** Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
** the U.S. Government retains certain rights in this software.
**
** This source code is released under the New BSD License.
**
** Tests the icetCompositeImage method to composite an image that has been
** pre-rendered (as opposed to rendered on demand through callbacks).
*****************************************************************************/

#include <IceT.h>
#include <IceTDevCommunication.h>
#include <IceTDevState.h>
#include "test_codes.h"
#include "test_util.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

static IceTInt g_local_valid_pixel_viewport[4];
static IceTInt *g_all_valid_pixel_viewports;

static void SetUpTiles(IceTInt tile_dimension)
{
    IceTInt tile_index = 0;
    IceTInt tile_x;
    IceTInt tile_y;

    icetResetTiles();
    for (tile_y = 0; tile_y < tile_dimension; tile_y++) {
        for (tile_x = 0; tile_x < tile_dimension; tile_x++) {
            icetAddTile(tile_x*SCREEN_WIDTH,
                        tile_y*SCREEN_HEIGHT,
                        SCREEN_WIDTH,
                        SCREEN_HEIGHT,
                        tile_index);
            tile_index++;
        }
    }
}

static void MakeValidPixelViewports()
{
    IceTInt global_viewport[4];
    IceTInt global_width;
    IceTInt global_height;
    IceTInt value1, value2;

    icetGetIntegerv(ICET_GLOBAL_VIEWPORT, global_viewport);
    global_width = global_viewport[2];
    global_height = global_viewport[3];

    value1 = rand()%global_width; value2 = rand()%global_height;
    if (value1 < value2) {
        g_local_valid_pixel_viewport[0] = value1;
        g_local_valid_pixel_viewport[2] = value2 - value1;
    } else {
        g_local_valid_pixel_viewport[0] = value2;
        g_local_valid_pixel_viewport[2] = value1 - value2;
    }

    value1 = rand()%global_width; value2 = rand()%global_height;
    if (value1 < value2) {
        g_local_valid_pixel_viewport[1] = value1;
        g_local_valid_pixel_viewport[3] = value2 - value1;
    } else {
        g_local_valid_pixel_viewport[1] = value2;
        g_local_valid_pixel_viewport[3] = value1 - value2;
    }

    icetCommAllgather(g_local_valid_pixel_viewport,
                      4,
                      ICET_INT,
                      g_all_valid_pixel_viewports);
}

static void MakeImageBuffers(IceTUInt **color_buffer_p,
                             IceTFloat **depth_buffer_p)
{
    IceTUInt *color_buffer;
    IceTFloat *depth_buffer;
    IceTInt global_viewport[4];
    IceTInt rank;
    IceTInt num_proc;
    IceTInt width;
    IceTInt height;
    IceTInt x;
    IceTInt y;
    IceTInt pixel;
    IceTInt left;
    IceTInt right;
    IceTInt bottom;
    IceTInt top;

    icetGetIntegerv(ICET_GLOBAL_VIEWPORT, global_viewport);
    width = global_viewport[2];
    height = global_viewport[3];

    icetGetIntegerv(ICET_RANK, &rank);
    icetGetIntegerv(ICET_NUM_PROCESSES, &num_proc);

    left = g_local_valid_pixel_viewport[0];
    right = left + g_local_valid_pixel_viewport[2];
    bottom = g_local_valid_pixel_viewport[1];
    top = bottom + g_local_valid_pixel_viewport[3];

    color_buffer = malloc(width*height*sizeof(IceTUInt));
    depth_buffer = malloc(width*height*sizeof(IceTFloat));
    pixel = 0;
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            if ((x >= left) && (x < right) && (y >= bottom) && (y < top)) {
                color_buffer[pixel] = rank;
                depth_buffer[pixel] = ((IceTFloat)rank)/num_proc;
            } else {
                color_buffer[pixel] = 0xDEADDEAD; /* garbage value */
            }
            pixel++;
        }
    }

    *color_buffer_p = color_buffer;
    *depth_buffer_p = depth_buffer;
}

static IceTBoolean CheckCompositedImage(const IceTImage image)
{
    IceTInt tile_displayed;
    const IceTUInt *color_buffer;
    IceTSizeType width;
    IceTSizeType height;
    const IceTInt *tile_viewport;
    IceTInt local_x, local_y;
    IceTInt global_x, global_y;
    IceTInt num_proc;

    icetGetIntegerv(ICET_VALID_PIXELS_TILE, &tile_displayed);
    if (tile_displayed < 0) {
        /* No local tile. Nothing to compare. Just return success. */
        return ICET_TRUE;
    }

    icetGetIntegerv(ICET_NUM_PROCESSES, &num_proc);

    color_buffer = icetImageGetColorcui(image);
    width = icetImageGetWidth(image);
    height = icetImageGetHeight(image);

    tile_viewport =
            icetUnsafeStateGetInteger(ICET_TILE_VIEWPORTS) + 4*tile_displayed;
    if ((tile_viewport[2] != width) || (tile_viewport[3] != height)) {
        printrank("***** Tile width or height does not match image! *****\n");
        printrank("Tile:  %d x %d\n", tile_viewport[2], tile_viewport[3]);
        printrank("Image: %d x %d\n", width, height);
        return ICET_FALSE;
    }

    for (local_y = 0, global_y = tile_viewport[1];
         local_y < height;
         local_y++, global_y++) {
        for (local_x = 0, global_x = tile_viewport[0];
             local_x < width;
             local_x++, global_x++) {
            IceTUInt image_value;
            IceTUInt expected_value;
            IceTInt proc_index;

            image_value = color_buffer[local_x + local_y*width];

            expected_value = 0xFFFFFFFF;
            for (proc_index = 0; proc_index < num_proc; proc_index++) {
                IceTInt *proc_viewport =
                        g_all_valid_pixel_viewports + 4*proc_index;
                if (   (global_x >= proc_viewport[0])
                    && (global_x < proc_viewport[0]+proc_viewport[2])
                    && (global_y >= proc_viewport[1])
                    && (global_y < proc_viewport[1]+proc_viewport[3]) ) {
                    expected_value = proc_index;
                    break;
                }
            }

            if (image_value != expected_value) {
                printrank("***** Got an unexpected value in the image *****\n");
                printrank("Located at pixel %d,%d (globally %d,%d)\n",
                          local_x, local_y, global_x, global_y);
                printrank("Expected 0x%X. Got 0x%X\n",
                          expected_value, image_value);
                return ICET_FALSE;
            }
        }
    }

    return ICET_TRUE;
}

static IceTBoolean PreRenderTryComposite(IceTUInt *color_buffer,
                                         IceTFloat *depth_buffer)
{
    IceTFloat background_color[4] = { 1.0, 1.0, 1.0, 1.0 };
    IceTImage image;

    icetSetColorFormat(ICET_IMAGE_COLOR_RGBA_UBYTE);
    icetSetDepthFormat(ICET_IMAGE_DEPTH_FLOAT);
    icetCompositeMode(ICET_COMPOSITE_MODE_Z_BUFFER);
    icetDisable(ICET_ORDERED_COMPOSITE);

    image = icetCompositeImage(color_buffer,
                               depth_buffer,
                               g_local_valid_pixel_viewport,
                               NULL,
                               NULL,
                               background_color);

    return CheckCompositedImage(image);
}

static IceTBoolean PreRenderTryStrategy()
{
    IceTBoolean success = ICET_TRUE;

    IceTUInt *color_buffer;
    IceTFloat *depth_buffer;
    IceTInt strategy_index;

    MakeImageBuffers(&color_buffer, &depth_buffer);

    for (strategy_index = 0;
         strategy_index < STRATEGY_LIST_SIZE;
         strategy_index++) {
        icetStrategy(strategy_list[strategy_index]);
        printstat("  Using %s strategy.\n", icetGetStrategyName());

        success &= PreRenderTryComposite(color_buffer, depth_buffer);
    }

    free(color_buffer);
    free(depth_buffer);

    return success;
}

static IceTBoolean PreRenderTryTiles(void)
{
    IceTBoolean success = ICET_TRUE;
    IceTInt num_proc;
    IceTInt tile_dimension;

    icetGetIntegerv(ICET_NUM_PROCESSES, &num_proc);

    for (tile_dimension = 1;
         (tile_dimension <= 4) && (tile_dimension*tile_dimension <= num_proc);
         tile_dimension++) {
        printstat("\nUsing %dx%d tiles\n", tile_dimension, tile_dimension);

        SetUpTiles(tile_dimension);
        MakeValidPixelViewports();

        success &= PreRenderTryStrategy();
    }

    return success;
}

static int PreRenderRun(void)
{
    IceTInt rank;
    IceTInt num_proc;
    unsigned int seed;

    icetGetIntegerv(ICET_RANK, &rank);
    icetGetIntegerv(ICET_NUM_PROCESSES, &num_proc);

    g_all_valid_pixel_viewports = malloc(4*num_proc*sizeof(IceTInt));

    /* Establish a random seed. */
    if (rank == 0) {
        IceTInt remote_process;

        seed = (int)time(NULL);
        printstat("Base seed = %u\n", seed);
        srand(seed);

        for (remote_process = 1; remote_process < num_proc; remote_process++) {
            icetCommSend(&seed, 1, ICET_INT, remote_process, 29);
        }
    } else {
        icetCommRecv(&seed, 1, ICET_INT, 0, 29);
        srand(seed + rank);
    }

    {
        IceTBoolean success = PreRenderTryTiles();

        free(g_all_valid_pixel_viewports);

        return (success ? TEST_PASSED : TEST_FAILED);
    }
}

int PreRender(int argc, char *argv[])
{
    /* Suppress warning. */
    (void)argc;
    (void)argv;

    return run_test(PreRenderRun);
}
