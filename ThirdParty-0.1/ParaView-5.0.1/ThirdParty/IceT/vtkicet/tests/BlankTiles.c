/* -*- c -*- *****************************************************************
** Copyright (C) 2003 Sandia Corporation
** Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
** the U.S. Government retains certain rights in this software.
**
** This source code is released under the New BSD License.
**
** Tests to make sure blank tiles are correctly handled.
*****************************************************************************/

#include <IceTGL.h>
#include "test_util.h"
#include "test_codes.h"

#include <stdlib.h>
#include <stdio.h>

static void draw(void)
{
    /* printrank("In draw\n"); */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glBegin(GL_QUADS);
      glVertex3d(-0.5, -0.5, 0.0);
      glVertex3d(0.5, -0.5, 0.0);
      glVertex3d(0.5, 0.5, 0.0);
      glVertex3d(-0.5, 0.5, 0.0);
    glEnd();
    /* printrank("Leaving draw\n"); */
}

static int BlankTilesDoTest(void)
{
    int result = TEST_PASSED;
    int tile_dim;
    IceTInt rank, num_proc;

    icetGetIntegerv(ICET_RANK, &rank);
    icetGetIntegerv(ICET_NUM_PROCESSES, &num_proc);

    for (tile_dim = 1; tile_dim*tile_dim <= num_proc; tile_dim++) {
        int x, y;
        IceTSizeType my_width = -1;
        IceTSizeType my_height = -1;
        IceTImage image;
        printstat("\nRunning on a %d x %d display.\n", tile_dim, tile_dim);
        icetResetTiles();
        for (y = 0; y < tile_dim; y++) {
            for (x = 0; x < tile_dim; x++) {
                int tile_rank = y*tile_dim + x;
              /* Modify the width and height a bit to detect bad image sizes. */
                IceTSizeType tile_width = SCREEN_WIDTH - x;
                IceTSizeType tile_height = SCREEN_HEIGHT - y;
                icetAddTile((IceTInt)(x*SCREEN_WIDTH),
                            (IceTInt)(y*SCREEN_HEIGHT),
                            tile_width,
                            tile_height,
                            tile_rank);
                if (tile_rank == rank) {
                    my_width = tile_width;
                    my_height = tile_height;
                }
            }
        }

        printstat("Rendering frame.\n");
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-1, tile_dim*2-1, -1, tile_dim*2-1, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        image = icetGLDrawFrame();
        swap_buffers();

        if (rank == 0) {
            /* printrank("Rank == 0, tile should have stuff in it.\n"); */
        } else if (rank < tile_dim*tile_dim) {
            IceTFloat *cb;
            int color_component;

            if (   (my_width != icetImageGetWidth(image))
                || (my_height != icetImageGetHeight(image)) ) {
                printrank("Image size is wrong!!!!!!!!!\n");
                result = TEST_FAILED;
            }

            /* printrank("Checking returned image data.\n"); */
            cb = icetImageGetColorf(image);
            for (color_component = 0;
                 color_component < my_width*my_height*4;
                 color_component++) {
                if (cb[color_component] != 0.25f) {
                    printrank("Found bad pixel!!!!!!!!\n");
                    result = TEST_FAILED;
                    break;
                }
            }
        } else {
            /* printrank("Not a display node.  Not testing image.\n"); */
        }
    }

    return result;
}

static int BlankTilesRun()
{
    int result = TEST_PASSED;
    int strategy_index;

    glClearColor(0.25f, 0.25f, 0.25f, 0.25f);

    icetGLDrawCallback(draw);
    icetBoundingBoxd(-0.5, 0.5, -0.5, 0.5, -0.5, 0.5);

    /* Turn on blending and colored background collection.  Even though in
       general this would not work unless you also ordered the compositing, we
       just want to make sure we get the right color in empty tiles for this
       test. */
    icetSetColorFormat(ICET_IMAGE_COLOR_RGBA_FLOAT);
    icetSetDepthFormat(ICET_IMAGE_DEPTH_NONE);
    icetCompositeMode(ICET_COMPOSITE_MODE_BLEND);
    icetEnable(ICET_CORRECT_COLORED_BACKGROUND);

    for (strategy_index = 0;
         strategy_index < STRATEGY_LIST_SIZE; strategy_index++) {
        IceTEnum strategy = strategy_list[strategy_index];
        int single_image_strategy_index;
        int num_single_image_strategy;

        icetStrategy(strategy);
        printstat("\n\nUsing %s strategy.\n", icetGetStrategyName());

        if (strategy_uses_single_image_strategy(strategy)) {
            num_single_image_strategy = SINGLE_IMAGE_STRATEGY_LIST_SIZE;
        } else {
          /* Set to one since single image strategy does not matter. */
            num_single_image_strategy = 1;
        }

        for (single_image_strategy_index = 0;
             single_image_strategy_index < num_single_image_strategy;
             single_image_strategy_index++) {
            IceTEnum single_image_strategy
                = single_image_strategy_list[single_image_strategy_index];
            int test_result;

            icetSingleImageStrategy(single_image_strategy);
            printstat("Using %s single image sub-strategy.\n",
                      icetGetSingleImageStrategyName());

            test_result = BlankTilesDoTest();
            if (test_result != TEST_PASSED) {
                result = test_result;
            }
        }
    }

    return result;
}

int BlankTiles(int argc, char *argv[])
{
  /* To remove warning */
  (void)argc;
  (void)argv;

  return run_test(BlankTilesRun);
}
