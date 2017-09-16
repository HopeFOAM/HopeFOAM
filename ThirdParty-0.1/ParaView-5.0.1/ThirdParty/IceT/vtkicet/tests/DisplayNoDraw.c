/* -*- c -*- *****************************************************************
** Copyright (C) 2003 Sandia Corporation
** Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
** the U.S. Government retains certain rights in this software.
**
** This source code is released under the New BSD License.
**
** This test has the display node not draw anything while everyone else
** does.  This should test some boundry conditions where the display node
** will receive images even when it doesn't generate any and for other
** nodes to send images without receiving any.
**
** In addition, this test will also place a blank tile in one of the other
** processors.  This should flag some problems with assumed far depths.
*****************************************************************************/

#include <IceTGL.h>
#include "test_codes.h"
#include "test_util.h"

#include <stdlib.h>
#include <stdio.h>

static int global_iteration;
static int global_rank;
static int global_num_proc;
static int global_result;

static void draw(void)
{
    /* printrank("In draw\n"); */
    if (global_rank == 0) {
        printrank("ERROR: Draw called on rank 0!\n");
        global_result = TEST_FAILED;
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (global_rank != global_iteration) {
        glBegin(GL_QUADS);
          glVertex3d(-1.0, -1.0, 0.0);
          glVertex3d(1.0, -1.0, 0.0);
          glVertex3d(1.0, 1.0, 0.0);
          glVertex3d(-1.0, 1.0, 0.0);
        glEnd();
    }
    /* printrank("Leaving draw\n"); */
}

static void DisplayNoDrawInit(void)
{
    printstat("Setting tile.");
    icetResetTiles();
    icetAddTile(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);

    icetGLDrawCallback(draw);

    icetSetColorFormat(ICET_IMAGE_COLOR_RGBA_UBYTE);
    icetSetDepthFormat(ICET_IMAGE_DEPTH_FLOAT);

    if (global_rank == 0) {
        icetBoundingBoxd(100.0, 101.0, 100.0, 101.0, 100.0, 101.0);
    } else {
        icetBoundingBoxd(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
    }

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-0.5, 0.5, -0.5, 0.5, -0.5, 0.5);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glColor4d(1.0, 1.0, 1.0, 1.0);
}

static void DisplayNoDrawDoTest(void)
{

    for (global_iteration = 0; global_iteration < global_num_proc;
         global_iteration++) {
        IceTImage image;
        IceTUByte *color_buffer;

        printstat("Blank image is rank %d\n", global_iteration);

        image = icetGLDrawFrame();
        swap_buffers();

        if (   (global_rank == 0)
            && (global_num_proc > 1)
          /* This last case covers when there is only 2 processes,
           * the root, as always, is not drawing anything and the
           * other process is drawing the clear screen. */
            && ((global_num_proc > 2) || (global_iteration != 1)) ) {
            int p;
            int bad_count = 0;
            printstat("Checking pixels.\n");
            color_buffer = icetImageGetColorub(image);
            for (p = 0;
                 (p < SCREEN_WIDTH*SCREEN_HEIGHT*4) && (bad_count < 10); p++) {
                if (color_buffer[p] != 255) {
                    printrank("BAD PIXEL %d.%d\n", p/4, p%4);
                    printrank("    Expected 255, got %d\n", color_buffer[p]);
                    bad_count++;
                }
            }
            if (bad_count >= 10) {
                char filename[256];
                global_result = TEST_FAILED;
                sprintf(filename, "DisplayNoDraw_%s_%s_%d.ppm",
                        icetGetStrategyName(), icetGetSingleImageStrategyName(),
                        global_iteration);
                write_ppm(filename, color_buffer,
                          (int)SCREEN_WIDTH, (int)SCREEN_HEIGHT);
            }
        }
    }
}

static int DisplayNoDrawRun(void)
{
    int strategy_index;

    icetGetIntegerv(ICET_RANK, &global_rank);
    icetGetIntegerv(ICET_NUM_PROCESSES, &global_num_proc);

    printstat("Starting DisplayNoDraw.\n");

    global_result = TEST_PASSED;

    DisplayNoDrawInit();

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

            icetSingleImageStrategy(single_image_strategy);
            printstat("Using %s single image sub-strategy.\n",
                      icetGetSingleImageStrategyName());

            DisplayNoDrawDoTest();
        }
    }

    return global_result;
}

int DisplayNoDraw(int argc, char *argv[])
{
    /* To remove warning */
    (void)argc;
    (void)argv;

    return run_test(DisplayNoDrawRun);
}
