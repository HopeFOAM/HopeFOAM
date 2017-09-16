/* -*- c -*- *******************************************************/
/*
 * Copyright (C) 2003 Sandia Corporation
 * Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 * the U.S. Government retains certain rights in this software.
 *
 * This source code is released under the New BSD License.
 */

#include <IceT.h>
#include <IceTDevCommunication.h>
#include <IceTDevDiagnostics.h>
#include <IceTDevImage.h>
#include <IceTDevState.h>
#include <IceTDevTiming.h>
#include "common.h"

#include <stdlib.h>
#include <string.h>

#define SPLIT_INCOMING_BUFFERS          ICET_STRATEGY_BUFFER_0
#define SPLIT_INCOMING_ARRAY_BUFFER     ICET_STRATEGY_BUFFER_1
#define SPLIT_OUTGOING_BUFFER           ICET_STRATEGY_BUFFER_2
#define SPLIT_IMAGE_FRAGMENT_BUFFER     ICET_STRATEGY_BUFFER_3
#define SPLIT_FULL_IMAGE_BUFFER         ICET_STRATEGY_BUFFER_4
#define SPLIT_REQUEST_BUFFER            ICET_STRATEGY_BUFFER_5
#define SPLIT_TILE_GROUPS_BUFFER        ICET_STRATEGY_BUFFER_6

#define IMAGE_DATA        50
#define COLOR_DATA        51
#define DEPTH_DATA        52

#define MIN(x,y) ((x) <= (y) ? (x) : (y))
#define FRAG_SIZE(total_pixels, num_pieces) \
    (((total_pixels)+(num_pieces)-1)/(num_pieces))

static void icetCollectImage(IceTImage imageFragment,
                             IceTInt my_tile,
                             const IceTInt *tile_groups,
                             IceTImage fullImage)
{
    IceTInt tile_displayed;
    const IceTInt *tile_viewports;
    const IceTInt *display_nodes;
    IceTSizeType my_fragment_size;
    IceTEnum color_format;
    IceTEnum depth_format;
    IceTSizeType pixel_size;
    IceTCommRequest requests[2];

    icetGetIntegerv(ICET_TILE_DISPLAYED, &tile_displayed);
    tile_viewports = icetUnsafeStateGetInteger(ICET_TILE_VIEWPORTS);
    display_nodes = icetUnsafeStateGetInteger(ICET_DISPLAY_NODES);

  /* Fragment might be truncated.  Adjust my_fragment_size appropriately. */
    my_fragment_size = icetImageGetNumPixels(imageFragment);

  /* Send composited fragment to display process. */
    icetImageAdjustForOutput(imageFragment);
    color_format = icetImageGetColorFormat(imageFragment);
    depth_format = icetImageGetDepthFormat(imageFragment);

    icetImageCorrectBackground(imageFragment);

    icetTimingCollectBegin();

    if (color_format != ICET_IMAGE_COLOR_NONE) {
        IceTVoid *outgoing_data = icetImageGetColorVoid(imageFragment,
                                                        &pixel_size);
        requests[0] = icetCommIsend(outgoing_data,
                                    pixel_size*my_fragment_size,
                                    ICET_BYTE,
                                    display_nodes[my_tile],
                                    COLOR_DATA);
    }
    if (depth_format != ICET_IMAGE_DEPTH_NONE) {
        IceTVoid *outgoing_data = icetImageGetDepthVoid(imageFragment,
                                                        &pixel_size);
        requests[1] = icetCommIsend(outgoing_data,
                                    pixel_size*my_fragment_size,
                                    ICET_BYTE,
                                    display_nodes[my_tile],
                                    DEPTH_DATA);
    }

  /* If I am displaying a tile, receive image data. */
    if (tile_displayed >= 0) {
        int displayed_width = tile_viewports[4*tile_displayed + 2];
        int displayed_height = tile_viewports[4*tile_displayed + 3];

        icetImageAdjustForOutput(fullImage);
        icetImageSetDimensions(fullImage, displayed_width, displayed_height);

      /* Check to make sure tile is not blank. */
        if (tile_groups[tile_displayed+1] > tile_groups[tile_displayed]) {
            int displayed_fragment_size
                = FRAG_SIZE(displayed_width*displayed_height,
                            (  tile_groups[tile_displayed+1]
                             - tile_groups[tile_displayed] ));
            if (color_format != ICET_IMAGE_COLOR_NONE) {
              /* Use IceTByte for byte-based pointer arithmetic. */
                IceTByte *cb = icetImageGetColorVoid(fullImage, &pixel_size);
                IceTInt node;
                for (node = tile_groups[tile_displayed];
                     node < tile_groups[tile_displayed+1]; node++) {
                    icetRaiseDebug1("Getting final color fragment from %d",
                                    node);
                    icetCommRecv(cb, pixel_size*displayed_fragment_size,
                                 ICET_BYTE, node, COLOR_DATA);
                    cb += pixel_size*displayed_fragment_size;
                }
            }
            if (depth_format != ICET_IMAGE_DEPTH_NONE) {
              /* Use IceTByte for byte-based pointer arithmetic. */
                IceTByte *db = icetImageGetDepthVoid(fullImage, &pixel_size);
                IceTInt node;
                for (node = tile_groups[tile_displayed];
                     node < tile_groups[tile_displayed+1]; node++) {
                    icetRaiseDebug1("Getting final depth fragment from %d",
                                    node);
                    icetCommRecv(db, pixel_size*displayed_fragment_size,
                                 ICET_INT, node, DEPTH_DATA);
                    db += pixel_size*displayed_fragment_size;
                }
            }
        } else {
            icetClearImageTrueBackground(fullImage);
        }
    }

    if (color_format != ICET_IMAGE_COLOR_NONE) {
        icetCommWait(&requests[0]);
    }
    if (depth_format != ICET_IMAGE_DEPTH_NONE) {
        icetCommWait(&requests[1]);
    }

    icetTimingCollectEnd();
}

IceTImage icetSplitCompose(void)
{
    IceTInt *tile_groups;
    IceTInt my_tile;
    IceTInt group_size;
    IceTSizeType my_fragment_size;

    IceTInt rank;
    IceTInt num_proc;
    IceTInt num_tiles;
    IceTInt max_width, max_height;
    const IceTInt *tile_contribs;
    IceTInt total_image_count;
    IceTInt tile_displayed;
    const IceTInt *tile_viewports;
    IceTInt my_width, my_height;

    IceTInt num_contained_tiles;
    const IceTInt *contained_tiles_list;
    const IceTBoolean *all_contained_tiles_masks;

    int tile, image, node;
    int num_allocated;

    IceTSparseImage incoming;
    IceTVoid **incomingBuffers;
    IceTByte *nextInBuf;  /* Use IceTByte for byte-based pointer arithmetic. */
    IceTSparseImage outgoing;
    IceTImage imageFragment;
    IceTImage fullImage;

    IceTSizeType fragmentSparseImageSize;

    int num_requests;
    IceTCommRequest *requests;

    int first_incoming;

    icetRaiseDebug("In splitStrategy");

    icetGetIntegerv(ICET_RANK, &rank);
    icetGetIntegerv(ICET_NUM_PROCESSES, &num_proc);
    icetGetIntegerv(ICET_NUM_TILES, &num_tiles);
    icetGetIntegerv(ICET_TILE_MAX_WIDTH, &max_width);
    icetGetIntegerv(ICET_TILE_MAX_HEIGHT, &max_height);
    tile_contribs = icetUnsafeStateGetInteger(ICET_TILE_CONTRIB_COUNTS);
    icetGetIntegerv(ICET_TOTAL_IMAGE_COUNT, &total_image_count);
    icetGetIntegerv(ICET_TILE_DISPLAYED, &tile_displayed);
    tile_viewports = icetUnsafeStateGetInteger(ICET_TILE_VIEWPORTS);
    icetGetIntegerv(ICET_NUM_CONTAINED_TILES, &num_contained_tiles);
    contained_tiles_list = icetUnsafeStateGetInteger(ICET_CONTAINED_TILES_LIST);
    all_contained_tiles_masks
        = icetUnsafeStateGetBoolean(ICET_ALL_CONTAINED_TILES_MASKS);

    fullImage = icetImageNull();

  /* Special case: no images rendered whatsoever. */
    if (total_image_count < 1) {
        icetRaiseDebug("Not rendering any images.  Quit early.");
        if (tile_displayed >= 0) {
            my_width = tile_viewports[4*tile_displayed + 2];
            my_height = tile_viewports[4*tile_displayed + 3];
            fullImage = icetGetStateBufferImage(SPLIT_FULL_IMAGE_BUFFER,
                                                my_width, my_height);
            icetClearImageTrueBackground(fullImage);
        }
        return fullImage;
    }

    tile_groups = icetGetStateBuffer(SPLIT_TILE_GROUPS_BUFFER,
                                     sizeof(int)*(num_tiles+1));

    num_allocated = 0;
    tile_groups[0] = 0;
  /* Set entry of tile_groups[i+1] to the number of processes to help
     compose the image in tile i. */
  /* TODO: Consider tile sizes when allocating processes. */
    for (tile = 0; tile < num_tiles; tile++) {
        int allocate = (tile_contribs[tile]*num_proc)/total_image_count;
        if ((allocate < 1) && (tile_contribs[tile] > 0)) {
            allocate = 1;
        }
        tile_groups[tile+1] = allocate;
        num_allocated += allocate;
    }

  /* Make the number of processes allocated equal exactly the number of
     processes available. */
    while (num_allocated < num_proc) {
      /* Add processes to the tile with the lowest process:image ratio. */
        int min_id = -1;
        float min_ratio = (float)num_proc;
        for (tile = 0; tile < num_tiles; tile++) {
            float ratio;
          /* Don't even consider tiles with no contributors. */
            if (tile_contribs[tile] == 0) continue;
            ratio = (float)tile_groups[tile+1]/tile_contribs[tile];
            if (ratio < min_ratio) {
                min_ratio = ratio;
                min_id = tile;
            }
        }
#ifdef DEBUG
        if (min_id < 0) {
            icetRaiseError("Could not find candidate to add tile.",
                           ICET_SANITY_CHECK_FAIL);
        }
#endif
        tile_groups[min_id+1]++;
        num_allocated++;
    }
    while (num_allocated > num_proc) {
      /* Remove processes from the tile with the highest process:image
         ratio. */
        int max_id = -1;
        float max_ratio = 0;
        for (tile = 0; tile < num_tiles; tile++) {
            float ratio;
          /* Don't even consider tiles with a minimum allocation. */
            if (tile_groups[tile+1] <= 1) continue;
            ratio = (float)tile_groups[tile+1]/tile_contribs[tile];
            if (ratio > max_ratio) {
                max_ratio = ratio;
                max_id = tile;
            }
        }
#ifdef DEBUG
        if (max_id < 0) {
            icetRaiseError("Could not find candidate to remove tile.",
                           ICET_SANITY_CHECK_FAIL);
        }
#endif
        tile_groups[max_id+1]--;
        num_allocated--;
    }

  /* Processes are assigned sequentially from 0 to N to each tile as
     needed.  Change each tile_groups[i] entry to be the lowest rank of the
     processes assigned to tile i.  Thus the processes assigned to tile i
     are tile_groups[i] through tile_groups[i+1]-1. */
    for (tile = 1; tile < num_tiles; tile++) {
        tile_groups[tile] += tile_groups[tile-1];
    }
    tile_groups[num_tiles] = num_proc;

  /* Figure out which tile I am assigned to. */
    for (my_tile = 0; rank >= tile_groups[my_tile+1]; my_tile++) {
      /* Nothing to do.  Just drop out of loop when my_tile is correct. */
    }
    icetRaiseDebug1("My tile is %d", my_tile);

    group_size = tile_groups[my_tile+1] - tile_groups[my_tile];
    my_width = tile_viewports[4*my_tile + 2];
    my_height = tile_viewports[4*my_tile + 3];
    my_fragment_size = FRAG_SIZE(max_width*max_height, group_size);

    num_requests = tile_contribs[my_tile];

    fragmentSparseImageSize = icetSparseImageBufferSize(my_fragment_size, 1);

    incomingBuffers
        = icetGetStateBuffer(SPLIT_INCOMING_ARRAY_BUFFER,
                             sizeof(IceTVoid*)*tile_contribs[my_tile]);
    outgoing      = icetGetStateBufferSparseImage(SPLIT_OUTGOING_BUFFER,
                                                  max_width, max_height);
    imageFragment = icetGetStateBufferImage(SPLIT_IMAGE_FRAGMENT_BUFFER,
                                            my_fragment_size, 1);
    fullImage     = icetGetStateBufferImage(SPLIT_FULL_IMAGE_BUFFER,
                                            max_width, max_height);
    requests = icetGetStateBuffer(SPLIT_REQUEST_BUFFER,
                                  sizeof(IceTCommRequest)*num_requests);

  /* Set up asynchronous receives for all incoming image fragments. */
    nextInBuf = icetGetStateBuffer(SPLIT_INCOMING_BUFFERS,
                                fragmentSparseImageSize*tile_contribs[my_tile]);
    for (image = 0, node = 0; image < tile_contribs[my_tile]; node++) {
        if (all_contained_tiles_masks[node*num_tiles + my_tile]) {
            icetRaiseDebug1("Setting up receive from node %d", node);
            incomingBuffers[image] = nextInBuf;
            requests[image] =
                icetCommIrecv(incomingBuffers[image],
                              fragmentSparseImageSize,
                              ICET_BYTE, node, IMAGE_DATA);
            image++;
            nextInBuf += fragmentSparseImageSize;
        }
    }

  /* Render and send all tile images I am rendering. */
    for (image = 0; image < num_contained_tiles; image++) {
        IceTSizeType sending_frag_size;
        IceTSizeType offset;

        tile = contained_tiles_list[image];
        icetGetTileImage(tile, fullImage);
        icetRaiseDebug1("Rendered image for tile %d", tile);
        offset = 0;
        sending_frag_size = FRAG_SIZE(icetImageGetNumPixels(fullImage),
                                      tile_groups[tile+1]-tile_groups[tile]);
        for (node = tile_groups[tile]; node < tile_groups[tile+1]; node++) {
            IceTVoid *package_buffer;
            IceTSizeType package_size;
            IceTSizeType truncated_size;

            truncated_size = MIN(sending_frag_size,
                                 icetImageGetNumPixels(fullImage) - offset);

            icetRaiseDebug2("Sending tile %d to node %d", tile, node);
            icetRaiseDebug2("Pixels %d to %d",
                            (int)offset, (int)truncated_size-1);
            icetCompressSubImage(fullImage, offset,
                                 truncated_size, outgoing);
            icetSparseImagePackageForSend(outgoing,
                                          &package_buffer, &package_size);
            icetCommSend(package_buffer, package_size,
                         ICET_BYTE, node, IMAGE_DATA);
            offset += truncated_size;
        }
    }

  /* Wait for images to come in and Z compare them. */
    first_incoming = 1;
    for (image = 0; image < tile_contribs[my_tile]; image++) {
        int idx;
        idx = icetCommWaitany(tile_contribs[my_tile], requests);
        incoming = icetSparseImageUnpackageFromReceive(incomingBuffers[idx]);
        if (first_incoming) {
            icetRaiseDebug1("Got first image (%d).", idx);
            icetDecompressImage(incoming, imageFragment);
            first_incoming = 0;
        } else {
            icetRaiseDebug1("Got subsequent image (%d).", idx);
            icetCompressedComposite(imageFragment, incoming, 1);
        }
    }

    if (icetIsEnabled(ICET_COLLECT_IMAGES)) {
        icetCollectImage(imageFragment,
                         my_tile,
                         tile_groups,
                         fullImage);
    } else {
        IceTSizeType offset;

        offset = (rank - tile_groups[my_tile])*my_fragment_size;

        /* Fragment might be truncated.  Adjust my_fragment_size
           appropriately. */
        my_fragment_size = icetImageGetNumPixels(imageFragment);

        icetImageCopyPixels(imageFragment,
                            0,
                            fullImage,
                            offset,
                            my_fragment_size);

        icetStateSetInteger(ICET_VALID_PIXELS_TILE, my_tile);
        icetStateSetInteger(ICET_VALID_PIXELS_OFFSET, offset);
        icetStateSetInteger(ICET_VALID_PIXELS_NUM, my_fragment_size);
    }

    return fullImage;
}
