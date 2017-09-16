/* -*- c -*- *****************************************************************
** Copyright (C) 2005 Sandia Corporation
** Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
** the U.S. Government retains certain rights in this software.
**
** This source code is released under the New BSD License.
**
** This tests how compositing behaves when given various image sizes that
** have been known to cause problems in the past.
*****************************************************************************/

#include <IceT.h>
#include "test_codes.h"
#include "test_util.h"

#include <stdlib.h>
#include <stdio.h>

#define NUM_IMAGE_SIZES 3
static int c_image_sizes[NUM_IMAGE_SIZES][2] = {
    { 1, 1 },           /* Splitting algorithms must truncate. */
    { 509, 503 },       /* Each dimension is prime. */
    { 997, 1 }          /* Number of pixels is prime. */
};

static void draw(const IceTDouble *projection_matrix,
                 const IceTDouble *modelview_matrix,
                 const IceTFloat *background_color,
                 const IceTInt *readback_viewport,
                 IceTImage result)
{
    IceTUByte *color_buffer;
    IceTFloat *depth_buffer;
    IceTSizeType num_pixels;
    IceTSizeType i;

    /* Suppress compiler warnings. */
    (void)projection_matrix;
    (void)modelview_matrix;
    (void)background_color;
    (void)readback_viewport;

    num_pixels = icetImageGetNumPixels(result);

    color_buffer = icetImageGetColorub(result);
    for (i = 0; i < num_pixels*4; i++) {
        color_buffer[i] = 255;
    }

    depth_buffer = icetImageGetDepthf(result);
    for (i = 0; i < num_pixels; i++) {
        depth_buffer[i] = 0.5f;
    }
}

static void setup_tiles(int tile_dimensions,
                        IceTSizeType width,
                        IceTSizeType height)
{
    int display_node;
    int tile_y;

    icetResetTiles();
    display_node = 0;
    for (tile_y = 0; tile_y < tile_dimensions; tile_y++) {
        int tile_x;
        for (tile_x = 0; tile_x < tile_dimensions; tile_x++) {
            icetAddTile((IceTInt)(tile_x*width),
                        (IceTInt)(tile_y*height),
                        width,
                        height,
                        display_node);
            display_node++;
        }
    }
}

static int OddImageSizesRun(void)
{
    IceTDouble identity[16];
    IceTFloat black[4];
    int image_size_index;
    IceTInt num_proc;

    icetGetIntegerv(ICET_NUM_PROCESSES, &num_proc);

    icetDrawCallback(draw);

    identity[ 0] = 1.0;
    identity[ 1] = 0.0;
    identity[ 2] = 0.0;
    identity[ 3] = 0.0;

    identity[ 4] = 0.0;
    identity[ 5] = 1.0;
    identity[ 6] = 0.0;
    identity[ 7] = 0.0;

    identity[ 8] = 0.0;
    identity[ 9] = 0.0;
    identity[10] = 1.0;
    identity[11] = 0.0;

    identity[12] = 0.0;
    identity[13] = 0.0;
    identity[14] = 0.0;
    identity[15] = 1.0;

    black[0] = black[1] = black[2] = 0.0f;
    black[3] = 1.0f;

    for (image_size_index = 0;
         image_size_index < NUM_IMAGE_SIZES;
         image_size_index++) {
        IceTSizeType width, height;
        int tile_dimensions;

        width = c_image_sizes[image_size_index][0];
        height = c_image_sizes[image_size_index][1];

        printstat("\n\nUsing image size %dx%d\n", (int)width, (int)height);

        for (tile_dimensions = 1;
             tile_dimensions*tile_dimensions <= num_proc;
             tile_dimensions++) {
            int strategy_index;

            printstat("  Using tile dimensions %d\n", tile_dimensions);

            setup_tiles(tile_dimensions, width, height);

            for (strategy_index = 0;
                 strategy_index < STRATEGY_LIST_SIZE;
                 strategy_index++) {
                IceTEnum strategy = strategy_list[strategy_index];
                int single_image_strategy_index;
                int num_single_image_strategy;

                icetStrategy(strategy);
                printstat("    Using %s strategy\n", icetGetStrategyName());

                if (strategy_uses_single_image_strategy(strategy)) {
                    num_single_image_strategy = SINGLE_IMAGE_STRATEGY_LIST_SIZE;
                } else {
                    /* Set to 1 since single image strategy does not matter. */
                    num_single_image_strategy = 1;
                }

                for (single_image_strategy_index = 0;
                     single_image_strategy_index < num_single_image_strategy;
                     single_image_strategy_index++) {
                    IceTEnum single_image_strategy =
                        single_image_strategy_list[single_image_strategy_index];

                    icetSingleImageStrategy(single_image_strategy);
                    printstat("      Using %s single image sub-strategy.\n",
                              icetGetSingleImageStrategyName());

                    /* Just invoke a frame.  I'm more worried about crashing
                       than other incorrect behavior. */
                    icetDrawFrame(identity, identity, black);
                }
            }
        }
    }

    return TEST_PASSED;
}

int OddImageSizes(int argc, char *argv[])
{
    /* To remove warning. */
    (void)argc;
    (void)argv;

    return run_test(OddImageSizesRun);
}
