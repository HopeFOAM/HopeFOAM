/* -*- c -*- *******************************************************/
/*
 * Copyright (C) 2003 Sandia Corporation
 * Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 * the U.S. Government retains certain rights in this software.
 *
 * This source code is released under the New BSD License.
 */

#include <IceT.h>

#include <IceTDevImage.h>
#include <IceTDevCommunication.h>
#include <IceTDevState.h>
#include <IceTDevDiagnostics.h>
#include "common.h"

#include <string.h>

#define VTREE_IMAGE_BUFFER              ICET_STRATEGY_BUFFER_0
#define VTREE_IN_SPARSE_IMAGE_BUFFER    ICET_STRATEGY_BUFFER_1
#define VTREE_OUT_SPARSE_IMAGE_BUFFER   ICET_STRATEGY_BUFFER_2
#define VTREE_INFO_BUFFER               ICET_STRATEGY_BUFFER_3
#define VTREE_ALL_CONTAINED_TMASKS_BUFFER ICET_STRATEGY_BUFFER_4

#define VTREE_IMAGE_DATA 40

struct node_info {
    int rank;
    int num_contained;
    int tile_held;
    int tile_sending;
    int tile_receiving;
    int send_dest;
    int recv_src;
};

static void sort_by_contained(struct node_info *info, int size);
static int find_sender(struct node_info *info, int num_proc,
                       int recv_node, int tile,
                       int display_node,
                       int num_tiles, IceTBoolean *all_contained_tmasks);
static int find_receiver(struct node_info *info, int num_proc,
                         int send_node, int tile,
                         int display_node,
                         int num_tiles, IceTBoolean *all_contained_tmasks);
static void do_send_receive(const struct node_info *my_info, int tile_held,
                            IceTInt num_tiles,
                            IceTBoolean *all_contained_tmasks,
                            IceTImage image,
                            IceTVoid *inSparseImageBuffer,
                            IceTSizeType inSparseImageBufferSize,
                            IceTSparseImage outSparseImage);

IceTImage icetVtreeCompose(void)
{
    IceTInt rank, num_proc;
    IceTInt num_tiles;
    IceTInt max_width, max_height;
    const IceTInt *display_nodes;
    IceTInt tile_displayed;
    IceTBoolean *all_contained_tmasks;
    const IceTInt *tile_viewports;
    IceTImage image;
    IceTVoid *inSparseImageBuffer;
    IceTSparseImage outSparseImage;
    IceTSizeType sparseImageSize;
    struct node_info *info;
    struct node_info *my_info;
    int tile, node;
    int tiles_transfered;
    int tile_held = -1;

    icetRaiseDebug("In vtreeCompose");

  /* Get state. */
    icetGetIntegerv(ICET_RANK, &rank);
    icetGetIntegerv(ICET_NUM_PROCESSES, &num_proc);
    icetGetIntegerv(ICET_NUM_TILES, &num_tiles);
    icetGetIntegerv(ICET_TILE_MAX_WIDTH, &max_width);
    icetGetIntegerv(ICET_TILE_MAX_HEIGHT, &max_height);
    display_nodes = icetUnsafeStateGetInteger(ICET_DISPLAY_NODES);
    tile_viewports = icetUnsafeStateGetInteger(ICET_TILE_VIEWPORTS);
    icetGetIntegerv(ICET_TILE_DISPLAYED, &tile_displayed);

  /* Allocate buffers. */
    sparseImageSize = icetSparseImageBufferSize(max_width, max_height);

    image                = icetGetStateBufferImage(VTREE_IMAGE_BUFFER,
                                                   max_width, max_height);
    inSparseImageBuffer  = icetGetStateBuffer(VTREE_IN_SPARSE_IMAGE_BUFFER,
                                              sparseImageSize);
    outSparseImage       = icetGetStateBufferSparseImage(
                                                  VTREE_OUT_SPARSE_IMAGE_BUFFER,
                                                  max_width, max_height);
    info                 = icetGetStateBuffer(VTREE_INFO_BUFFER,
                                             sizeof(struct node_info)*num_proc);
    all_contained_tmasks = icetGetStateBuffer(VTREE_ALL_CONTAINED_TMASKS_BUFFER,
                                        sizeof(IceTBoolean)*num_proc*num_tiles);

    icetGetBooleanv(ICET_ALL_CONTAINED_TILES_MASKS, all_contained_tmasks);
    
  /* Initialize info array. */
    for (node = 0; node < num_proc; node++) {
        info[node].rank = node;
        info[node].tile_held = -1;        /* Id of tile image held in memory. */
        info[node].num_contained = 0;        /* # of images to be rendered. */
        for (tile = 0; tile < num_tiles; tile++) {
            if (all_contained_tmasks[node*num_tiles + tile]) {
                info[node].num_contained++;
            }
        }
    }

#define CONTAINS_TILE(nodei, tile)                                \
    (all_contained_tmasks[info[nodei].rank*num_tiles+(tile)])

    tile_held = -1;
    do {
        int recv_node;

        tiles_transfered = 0;
        sort_by_contained(info, num_proc);
        for (node = 0; node < num_proc; node++) {
            info[node].tile_sending = -1;
            info[node].tile_receiving = -1;
        }

        for (recv_node = 0; recv_node < num_proc; recv_node++) {
            struct node_info *recv_info = info + recv_node;

            if (recv_info->tile_receiving >= 0) continue;

            if (recv_info->tile_held >= 0) {
              /* This node is holding a tile.  It must either send or
                 receive this tile. */
                if (find_sender(info, num_proc, recv_node, recv_info->tile_held,
                                display_nodes[recv_info->tile_held],
                                num_tiles, all_contained_tmasks)) {
                    tiles_transfered = 1;
                    continue;
                }

              /* Could not find a match for a sender, how about someone who
                 can receive it? */
                if (   (recv_info->tile_sending < 0)
                    && (recv_info->rank != display_nodes[recv_info->tile_held])
                    && find_receiver(info, num_proc, recv_node,
                                     recv_info->tile_held,
                                     display_nodes[recv_info->tile_held],
                                     num_tiles, all_contained_tmasks) ) {
                    tiles_transfered = 1;
                } else {
                  /* Could not send or receive.  Give up. */
                    continue;
                }
            }

          /* OK.  Let's try to receive any tile that we still have. */
            for (tile = 0; tile < num_tiles; tile++) {
                if (   (   !CONTAINS_TILE(recv_node, tile)
                        && (display_nodes[tile] != recv_info->rank) )
                    || (recv_info->tile_sending == tile) ) continue;
                if (find_sender(info, num_proc, recv_node, tile,
                                display_nodes[tile], num_tiles,
                                all_contained_tmasks)) {
                    tiles_transfered = 1;
                    break;
                }
            }
        }

      /* Now that we figured out who is sending to who, do the actual
         send and receive. */
        my_info = NULL;
        for (node = 0; node < num_proc; node++) {
            if (info[node].rank == rank) {
                my_info = info + node;
                break;
            }
        }

        do_send_receive(my_info, tile_held, num_tiles,
                        all_contained_tmasks,
                        image, inSparseImageBuffer, sparseImageSize,
                        outSparseImage);

        tile_held = my_info->tile_held;

    } while (tiles_transfered);

  /* It's possible that a composited image ended up on a processor that        */
  /* is not the display node for that image.  Do one last round of        */
  /* transfers to make sure all the tiles ended up in the right place.        */
    for (node = 0; node < num_proc; node++) {
        if (info[node].rank == rank) {
            my_info = info + node;
            break;
        }
    }
    my_info->tile_receiving = -1;
    my_info->tile_sending = -1;
    if ((my_info->tile_held >= 0) && (my_info->tile_held != tile_displayed)) {
      /* I'm holding an image that does not belong to me.  Ship it off. */
        my_info->tile_sending = my_info->tile_held;
        my_info->send_dest = display_nodes[my_info->tile_held];
        my_info->tile_held = -1;
    }
    if ((my_info->tile_held != tile_displayed) && (tile_displayed >= 0)) {
      /* Someone may be holding an image that belongs to me.  Check. */
        for (node = 0; node < num_proc; node++) {
            if (info[node].tile_held == tile_displayed) {
                my_info->tile_receiving = tile_displayed;
                my_info->recv_src = info[node].rank;
                my_info->tile_held = tile_displayed;
                break;
            }
        }
    }
    do_send_receive(my_info, tile_held,
                    num_tiles, all_contained_tmasks,
                    image, inSparseImageBuffer, sparseImageSize,
                    outSparseImage);
    tile_held = my_info->tile_held;

  /* Hacks for when "this" tile was not rendered. */
    if ((tile_displayed >= 0) && (tile_displayed != tile_held)) {
        if (all_contained_tmasks[rank*num_tiles + tile_displayed]) {
          /* Only "this" node draws "this" tile.  Because the image never needed
              to be transferred, it was never rendered above.  Just render it
              now.  We might save some time by rendering with the true
              background color rather than correcting it later. */
            IceTFloat true_background[4];
            IceTInt true_background_word;
            IceTFloat original_background[4];
            IceTInt original_background_word;

            icetGetFloatv(ICET_TRUE_BACKGROUND_COLOR, true_background);
            icetGetIntegerv(ICET_TRUE_BACKGROUND_COLOR_WORD,
                            &true_background_word);

            icetGetFloatv(ICET_BACKGROUND_COLOR, original_background);
            icetGetIntegerv(ICET_BACKGROUND_COLOR_WORD,
                            &original_background_word);

            icetStateSetFloatv(ICET_BACKGROUND_COLOR, 4, true_background);
            icetStateSetInteger(ICET_BACKGROUND_COLOR_WORD,
                                true_background_word);

            icetRaiseDebug("Rendering tile to display.");
          /* This may uncessarily read a buffer if not outputing an input
             buffer */
            icetGetTileImage(tile_displayed, image);

            icetStateSetFloatv(ICET_BACKGROUND_COLOR, 4, original_background);
            icetStateSetInteger(ICET_BACKGROUND_COLOR_WORD,
                                original_background_word);
        } else {
          /* "This" tile is blank. */
            const IceTInt *display_tile_viewport
                = tile_viewports + 4*tile_displayed;
            IceTInt display_tile_width = display_tile_viewport[2];
            IceTInt display_tile_height = display_tile_viewport[3];

            icetRaiseDebug("Returning blank image.");
            icetImageSetDimensions(
                                image, display_tile_width, display_tile_height);
            icetClearImageTrueBackground(image);
        }
    } else if (tile_displayed >= 0) {
        icetImageCorrectBackground(image);
    }

    return image;
}

static int find_sender(struct node_info *info, int num_proc,
                       int recv_node, int tile,
                       int display_node, int num_tiles,
                       IceTBoolean *all_contained_tmasks)
{
    int send_node;
    int sender = -1;

    for (send_node = num_proc-1; send_node >= 0; send_node--) {
        if (   (info[send_node].tile_sending >= 0)
            || !CONTAINS_TILE(send_node, tile)
            || (info[send_node].tile_receiving == tile)
            || (info[send_node].rank == display_node)
            || (send_node == recv_node) ) {
            continue;
        }
        if (info[send_node].tile_held == tile) {
          /* Favor sending held images. */
            sender = send_node;
            break;
        } else if (sender < 0) {
            sender = send_node;
        }
    }
    
    if (sender >= 0) {
        info[recv_node].tile_held = tile;
        info[recv_node].tile_receiving = tile;
        info[recv_node].recv_src = info[sender].rank;
        info[sender].tile_sending = tile;
        info[sender].send_dest = info[recv_node].rank;
        if (info[sender].tile_held == tile) info[sender].tile_held = -1;
        info[sender].num_contained--;
        all_contained_tmasks[info[sender].rank*num_tiles + tile] = 0;
        return 1;
    } else {
        return 0;
    }
}
 
static int find_receiver(struct node_info *info, int num_proc,
                         int send_node, int tile,
                         int display_node, int num_tiles,
                         IceTBoolean *all_contained_tmasks)
{
    int recv_node;

    for (recv_node = send_node+1; recv_node < num_proc; recv_node++) {
        if (   (info[recv_node].tile_receiving < 0)
            && (   (info[recv_node].tile_held < 0)
                || (info[recv_node].tile_held == tile) )
            && (   CONTAINS_TILE(recv_node, tile)
                || (info[recv_node].rank == display_node) ) ) {
            info[recv_node].tile_held = tile;
            info[recv_node].tile_receiving = tile;
            info[recv_node].recv_src = info[send_node].rank;
            info[send_node].tile_sending = tile;
            info[send_node].send_dest = info[recv_node].rank;
            if (info[send_node].tile_held == tile) {
                info[send_node].tile_held = -1;
            }
            info[send_node].num_contained--;
            all_contained_tmasks[info[send_node].rank*num_tiles + tile] = 0;
            return 1;
        }
    }

    return 0;
}

/* Yes, bubblesort is a slow way to sort, but I don't expect the keys to
   change much between iterations and the list is not very long, so we
   should be OK. */
static void sort_by_contained(struct node_info *info, int size)
{
    int swap_happened;
    int node;

    do {
        swap_happened = 0;
        for (node = 0; node < size-1; node++) {
            if (info[node].num_contained > info[node+1].num_contained) {
                struct node_info tmp = info[node];
                info[node] = info[node+1];
                info[node+1] = tmp;
                swap_happened = 1;
            }
        }
    } while (swap_happened);
}

static void do_send_receive(const struct node_info *my_info, int tile_held,
                            IceTInt num_tiles,
                            IceTBoolean *all_contained_tmasks,
                            IceTImage image,
                            IceTVoid *inSparseImageBuffer,
                            IceTSizeType inSparseImageBufferSize,
                            IceTSparseImage outSparseImage)
{
    IceTSparseImage inSparseImage;
    IceTVoid *package_buffer;
    IceTSizeType package_size;

    if (my_info->tile_sending != -1) {
        icetRaiseDebug2("Sending tile %d to node %d.", my_info->tile_sending,
                        my_info->send_dest);
        if (tile_held == my_info->tile_sending) {
            icetCompressImage(image, outSparseImage);
            tile_held = -1;
        } else {
            icetGetCompressedTileImage(my_info->tile_sending,
                                       outSparseImage);
        }
        icetSparseImagePackageForSend(outSparseImage,
                                      &package_buffer, &package_size);
    }

    if (my_info->tile_receiving != -1) {
        icetRaiseDebug2("Receiving tile %d from node %d.",
                        my_info->tile_receiving, my_info->recv_src);
        if (   (tile_held != my_info->tile_receiving)
            && all_contained_tmasks[my_info->rank*num_tiles
                                   +my_info->tile_receiving])
        {
            icetGetTileImage(my_info->tile_receiving, image);
            tile_held = my_info->tile_receiving;
        }

        if (my_info->tile_sending != -1) {
            icetCommSendrecv(package_buffer, package_size, ICET_BYTE,
                             my_info->send_dest, VTREE_IMAGE_DATA,
                             inSparseImageBuffer, inSparseImageBufferSize,
                             ICET_BYTE, my_info->recv_src, VTREE_IMAGE_DATA);
        } else {
            icetCommRecv(inSparseImageBuffer, inSparseImageBufferSize,
                         ICET_BYTE, my_info->recv_src, VTREE_IMAGE_DATA);
        }
        inSparseImage =icetSparseImageUnpackageFromReceive(inSparseImageBuffer);

        if (tile_held == my_info->tile_receiving) {
            icetCompressedComposite(image, inSparseImage, 1);
        } else {
            icetDecompressImage(inSparseImage, image);
        }

    } else if (my_info->tile_sending != -1) {
        icetCommSend(package_buffer, package_size, ICET_BYTE,
                     my_info->send_dest, VTREE_IMAGE_DATA);
    }
}
