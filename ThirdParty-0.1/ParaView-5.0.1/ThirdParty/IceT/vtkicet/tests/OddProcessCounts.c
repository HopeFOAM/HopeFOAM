/* -*- c -*- *****************************************************************
** Copyright (C) 2011 Sandia Corporation
** Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
** the U.S. Government retains certain rights in this software.
**
** This source code is released under the New BSD License.
**
** This tests how compositing behaves when given process counts that have
** been known to cause problems in the past.
*****************************************************************************/

#include <IceT.h>
#include "test_codes.h"
#include "test_util.h"

#include <IceTDevCommunication.h>
#include <IceTDevState.h>

#include <stdlib.h>
#include <stdio.h>

static void draw(const IceTDouble *projection_matrix,
                 const IceTDouble *modelview_matrix,
                 const IceTFloat *background_color,
                 const IceTInt *readback_viewport,
                 IceTImage result)
{
    IceTUByte *color_buffer;
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
}

static int OddProcessCountsTryFrame(void)
{
    IceTDouble identity[16];
    IceTFloat black[4];

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

    /* For now, just invoke a frame and see if it crashes.  May want to
       check for image correctness later. */
    icetDrawFrame(identity, identity, black);

    icetCommBarrier();

    return TEST_PASSED;
}

static int OddProcessCountsTryCollectOptions(void)
{
    IceTInt si_strategy;

    icetGetIntegerv(ICET_SINGLE_IMAGE_STRATEGY, &si_strategy);

    if ((si_strategy == ICET_SINGLE_IMAGE_STRATEGY_RADIXK)
        || (si_strategy == ICET_SINGLE_IMAGE_STRATEGY_RADIXKR)) {
        IceTInt rank;
        IceTInt num_proc;
        IceTInt magic_k;

        icetGetIntegerv(ICET_RANK, &rank);
        icetGetIntegerv(ICET_NUM_PROCESSES, &num_proc);

        for (magic_k = 2; magic_k <= num_proc; magic_k *= 2) {
            IceTInt max_image_split;

            printstat("      Using magic k value of %d\n", magic_k);
            icetStateSetInteger(ICET_MAGIC_K, magic_k);

            for (max_image_split = 0;
                 max_image_split <= num_proc;
                 max_image_split += magic_k) {
                IceTInt result;

                printstat("        Using image split of %d\n",
                          max_image_split);
                icetStateSetInteger(ICET_MAX_IMAGE_SPLIT, max_image_split);

                result = OddProcessCountsTryFrame();
                if (result != TEST_PASSED) { return result; }
            }
        }

        return TEST_PASSED;
    } else {
        return OddProcessCountsTryFrame();
    }
}

static int OddProcessCountsTryStrategy(void)
{
    IceTInt rank;
    int single_image_strategy_index;

    icetGetIntegerv(ICET_RANK, &rank);

    /* Only iterating over single image strategies.  We are specifically using
       the reduce strategy to simulate smaller process groups.  Besides, the
       single image strategies generally have more complicated indexing for odd
       process counts. */
    for (single_image_strategy_index = 0;
         single_image_strategy_index < SINGLE_IMAGE_STRATEGY_LIST_SIZE;
         single_image_strategy_index++) {
        int result;

        IceTEnum single_image_strategy =
            single_image_strategy_list[single_image_strategy_index];

        icetSingleImageStrategy(single_image_strategy);
        printstat("    Using %s single image sub-strategy.\n",
                  icetGetSingleImageStrategyName());

        result = OddProcessCountsTryCollectOptions();

        if (result != TEST_PASSED) { return result; }
    }

    return TEST_PASSED;
}

static int OddProcessCountsTryCount(void)
{
    IceTInt rank;
    IceTInt max_proc;
    IceTInt mid_proc;
    IceTInt add_proc;
    IceTInt last_add_proc;

    /* Use the reduce strategy to simulate using a certain process count. */
    icetStrategy(ICET_STRATEGY_REDUCE);

    icetGetIntegerv(ICET_RANK, &rank);
    icetGetIntegerv(ICET_NUM_PROCESSES, &max_proc);

    mid_proc = 1;
    while (mid_proc < max_proc) { mid_proc *= 2; }
    mid_proc /= 2;

    last_add_proc = add_proc = 1;
    while (mid_proc + add_proc <= max_proc) {
        IceTInt num_proc = mid_proc + add_proc;
        IceTInt result;

        printstat("  Using %d processes\n", num_proc);

        if ((max_proc - rank) <= num_proc) {
            /* In visible range. */
            icetBoundingBoxd(-1.0, 1.0,
                             -1.0, 1.0,
                             -1.0, 1.0);
        } else {
            /* Outside of visible range. */
            icetBoundingBoxd(100000.0, 100001.0,
                             100000.0, 100001.0,
                             100000.0, 100001.0);
        }

        result = OddProcessCountsTryStrategy();
        if (result != TEST_PASSED) { return result; }

        {
            IceTInt next_add_proc = last_add_proc + add_proc;
            last_add_proc = add_proc;
            add_proc = next_add_proc;
        }
    }

    return TEST_PASSED;
}

static int OddProcessCountsRun(void)
{
    IceTInt num_proc;
    IceTInt *process_ranks;
    IceTInt proc;

    icetGetIntegerv(ICET_NUM_PROCESSES, &num_proc);

    icetCompositeMode(ICET_COMPOSITE_MODE_BLEND);
    icetSetColorFormat(ICET_IMAGE_COLOR_RGBA_UBYTE);
    icetSetDepthFormat(ICET_IMAGE_DEPTH_NONE);
    icetDisable(ICET_CORRECT_COLORED_BACKGROUND);

    icetDrawCallback(draw);

    icetResetTiles();
    icetAddTile(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, num_proc-1);

    process_ranks = malloc(num_proc * sizeof(IceTInt));
    for (proc = 0; proc < num_proc; proc++) {
        process_ranks[proc] = proc;
    }
    icetEnable(ICET_ORDERED_COMPOSITE);
    icetCompositeOrder(process_ranks);
    free(process_ranks);

    return OddProcessCountsTryCount();
}

int OddProcessCounts(int argc, char *argv[])
{
    /* To remove warning. */
    (void)argc;
    (void)argv;

    return run_test(OddProcessCountsRun);
}
