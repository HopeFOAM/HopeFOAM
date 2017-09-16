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
#include <IceTDevState.h>
#include <IceTDevDiagnostics.h>
#include "common.h"

#include <stdlib.h>
#include <string.h>

#ifdef _MSC_VER
#pragma warning(disable:4127)
#endif

#define REDUCE_OUT_IMAGE_BUFFER                 ICET_STRATEGY_BUFFER_0
#define REDUCE_IN_IMAGE_BUFFER                  ICET_STRATEGY_BUFFER_1
#define REDUCE_COMPOSITE_IMAGE_BUFFER_1         ICET_STRATEGY_BUFFER_2
#define REDUCE_COMPOSITE_IMAGE_BUFFER_2         ICET_STRATEGY_BUFFER_3

#define REDUCE_NUM_PROC_FOR_TILE_BUFFER         ICET_STRATEGY_BUFFER_5
#define REDUCE_NODE_ASSIGNMENT_BUFFER           ICET_STRATEGY_BUFFER_6
#define REDUCE_TILE_PROC_GROUPS_BUFFER          ICET_STRATEGY_BUFFER_7
#define REDUCE_GROUP_SIZES_BUFFER               ICET_STRATEGY_BUFFER_8
#define REDUCE_TILE_IMAGE_DEST_BUFFER           ICET_STRATEGY_BUFFER_9
#define REDUCE_CONTRIBUTORS_BUFFER              ICET_STRATEGY_BUFFER_10

static IceTInt reduceDelegate(IceTInt **tile_image_destp,
                              IceTInt **compose_groupp, IceTInt *group_sizep,
                              IceTInt *group_image_destp);

static IceTImage reduceCollect(const IceTSparseImage composited_image,
                               IceTInt compose_tile,
                               IceTInt piece_offset);


IceTImage icetReduceCompose(void)
{
    IceTSparseImage rendered_image;
    IceTSparseImage composited_image;
    IceTImage result_image;

    IceTInt *tile_image_dest;
    IceTInt *compose_group, group_size, group_image_dest;
    IceTInt compose_tile;
    IceTSizeType piece_offset;

    icetRaiseDebug("In reduceCompose");

    /* Figure out who is compositing what tiles. */
    compose_tile = reduceDelegate(&tile_image_dest,
                                  &compose_group, &group_size,
                                  &group_image_dest);

    /* Render images and transfer to appropriate process. */
    {
        IceTInt max_width;
        IceTInt max_height;

        IceTSizeType sparse_image_size;
        IceTVoid *inImageBuffer;
        IceTSparseImage outSparseImage;
        IceTSparseImage composite_image1;
        IceTSparseImage composite_image2;

        icetGetIntegerv(ICET_TILE_MAX_WIDTH, &max_width);
        icetGetIntegerv(ICET_TILE_MAX_HEIGHT, &max_height);

        sparse_image_size = icetSparseImageBufferSize(max_width, max_height);
        inImageBuffer  = icetGetStateBuffer(REDUCE_IN_IMAGE_BUFFER,
                                            sparse_image_size);
        outSparseImage = icetGetStateBufferSparseImage(REDUCE_OUT_IMAGE_BUFFER,
                                                       max_width, max_height);
        composite_image1 = icetGetStateBufferSparseImage(
                                                REDUCE_COMPOSITE_IMAGE_BUFFER_1,
                                                max_width, max_height);
        composite_image2 = icetGetStateBufferSparseImage(
                                                REDUCE_COMPOSITE_IMAGE_BUFFER_2,
                                                max_width, max_height);

        icetRenderTransferSparseImages(composite_image1,
                                       composite_image2,
                                       inImageBuffer,
                                       outSparseImage,
                                       tile_image_dest,
                                       &rendered_image);
    }

    if (compose_tile >= 0) {
        icetSingleImageCompose(compose_group,
                               group_size,
                               group_image_dest,
                               rendered_image,
                               &composited_image,
                               &piece_offset);
    } else {
        /* Not assigned to compose any tile.  Do nothing.  These variables set
           for safety. */
        composited_image = icetSparseImageNull();
        piece_offset = 0;
    }

    if (icetIsEnabled(ICET_COLLECT_IMAGES)) {
        result_image = reduceCollect(composited_image,
                                     compose_tile,
                                     piece_offset);
    } else {
        IceTSizeType piece_size = icetSparseImageGetNumPixels(composited_image);
        const IceTInt *tile_viewports
            = icetUnsafeStateGetInteger(ICET_TILE_VIEWPORTS);
        IceTSizeType tile_width = tile_viewports[compose_tile + 2];
        IceTSizeType tile_height = tile_viewports[compose_tile + 3];
        if (piece_size > 0) {
            result_image = icetGetStateBufferImage(REDUCE_OUT_IMAGE_BUFFER,
                                                   tile_width, tile_height);
            icetDecompressSubImage(composited_image,
                                   piece_offset,
                                   result_image);
            icetStateSetInteger(ICET_VALID_PIXELS_TILE, compose_tile);
            icetStateSetInteger(ICET_VALID_PIXELS_OFFSET, piece_offset);
            icetStateSetInteger(ICET_VALID_PIXELS_NUM, piece_size);
        } else {
            result_image = icetImageNull();
            icetStateSetInteger(ICET_VALID_PIXELS_TILE, -1);
            icetStateSetInteger(ICET_VALID_PIXELS_OFFSET, 0);
            icetStateSetInteger(ICET_VALID_PIXELS_NUM, 0);
        }
    }

    return result_image;
}

static IceTInt reduceDelegate(IceTInt **tile_image_destp,
                              IceTInt **compose_groupp,
                              IceTInt *group_sizep,
                              IceTInt *group_image_destp)
{
    const IceTBoolean *all_contained_tiles_masks;
    const IceTInt *contrib_counts;
    IceTInt total_image_count;

    IceTInt num_tiles;
    IceTInt num_processes;
    IceTInt rank;
    const IceTInt *tile_display_nodes;
    const IceTInt *composite_order;

    IceTInt *num_proc_for_tile;
    IceTInt *node_assignment;
    IceTInt *tile_proc_groups;
    IceTInt *group_sizes;
    IceTInt *tile_image_dest;
    IceTInt group_image_dest = 0;
    IceTInt *contributors;

    IceTInt pcount;

    IceTInt tile, node;
    IceTInt snode, rnode, dest;
    IceTInt piece;
    IceTInt first_loop;

    all_contained_tiles_masks
        = icetUnsafeStateGetBoolean(ICET_ALL_CONTAINED_TILES_MASKS);
    contrib_counts = icetUnsafeStateGetInteger(ICET_TILE_CONTRIB_COUNTS);
    icetGetIntegerv(ICET_TOTAL_IMAGE_COUNT, &total_image_count);
    tile_display_nodes = icetUnsafeStateGetInteger(ICET_DISPLAY_NODES);
    composite_order = icetUnsafeStateGetInteger(ICET_COMPOSITE_ORDER);

    icetGetIntegerv(ICET_NUM_TILES, &num_tiles);
    icetGetIntegerv(ICET_NUM_PROCESSES, &num_processes);
    icetGetIntegerv(ICET_RANK, &rank);

    if (total_image_count < 1) {
        icetRaiseDebug("No nodes are drawing.");
        *tile_image_destp = NULL;
        *compose_groupp = NULL;
        *group_sizep = 0;
        *group_image_destp = 0;
        return -1;
    }

    num_proc_for_tile = icetGetStateBuffer(REDUCE_NUM_PROC_FOR_TILE_BUFFER,
                                           num_tiles * sizeof(IceTInt));
    node_assignment   = icetGetStateBuffer(REDUCE_NODE_ASSIGNMENT_BUFFER,
                                           num_processes * sizeof(IceTInt));
    tile_proc_groups  = icetGetStateBuffer(REDUCE_TILE_PROC_GROUPS_BUFFER,
                                             num_tiles * num_processes
                                           * sizeof(IceTInt));
    group_sizes       = icetGetStateBuffer(REDUCE_GROUP_SIZES_BUFFER,
                                           num_tiles * sizeof(IceTInt));
    tile_image_dest   = icetGetStateBuffer(REDUCE_TILE_IMAGE_DEST_BUFFER,
                                           num_tiles * sizeof(IceTInt));
    contributors      = icetGetStateBuffer(REDUCE_CONTRIBUTORS_BUFFER,
                                           num_processes * sizeof(IceTInt));

  /* Decide the minimum amount of processes that should be added to each
     tile. */
    pcount = 0;
    for (tile = 0; tile < num_tiles; tile++) {
        IceTInt allocate = (contrib_counts[tile]*num_processes)/total_image_count;
      /* Make sure at least one process is assigned to tiles that have at
         least one image. */
        if ((allocate < 1) && (contrib_counts[tile] > 0)) allocate = 1;
      /* Don't allocate more processes to a tile than images for that
         tile. */
        if (allocate > contrib_counts[tile]) allocate = contrib_counts[tile];

        num_proc_for_tile[tile] = allocate;

      /* Keep track of how many processes have been allocated. */
        pcount += allocate;
    }

  /* Handle when we have not allocated all the processes. */
    while (num_processes > pcount) {
      /* Find the tile with the largest image to process ratio that
         can still have a process added to it. */
        IceTInt max = 0;
        for (tile = 1; tile < num_tiles; tile++) {
            if (   (num_proc_for_tile[tile] < contrib_counts[tile])
                && (   (num_proc_for_tile[max] == contrib_counts[max])
                    || (  (float)contrib_counts[max]/num_proc_for_tile[max]
                        < (float)contrib_counts[tile]/num_proc_for_tile[tile])))
            {
                max = tile;
            }
        }
        if (num_proc_for_tile[max] < contrib_counts[max]) {
            num_proc_for_tile[max]++;
            pcount++;
        } else {
          /* Cannot assign any more processes. */
            break;
        }
    }

  /* Handle when we have allocated too many processes. */
    while (num_processes < pcount) {
      /* Find tile with the smallest image to process ratio that can still
         have a process removed. */
        IceTInt min = 0;
        for (tile = 1; tile < num_tiles; tile++) {
            if (   (num_proc_for_tile[tile] > 1)
                && (   (num_proc_for_tile[min] < 2)
                    || (  (float)contrib_counts[min]/num_proc_for_tile[min]
                        > (float)contrib_counts[tile]/num_proc_for_tile[tile])))
            {
                min = tile;
            }
        }
        num_proc_for_tile[min]--;
        pcount--;
    }

  /* Clear out arrays. */
    memset(group_sizes, 0, num_tiles*sizeof(IceTInt));
    for (node = 0; node < num_processes; node++) {
        node_assignment[node] = -1;
    }

#define ASSIGN_NODE2TILE(node, tile)                                    \
node_assignment[(node)] = (tile);                                       \
tile_proc_groups[(tile)*num_processes + group_sizes[(tile)]] = (node);  \
group_sizes[(tile)]++;

  /* Assign each display node to the group processing that tile if there
     are any images in that tile. */
    for (tile = 0; tile < num_tiles; tile++) {
        if (contrib_counts[tile] > 0) {
            ASSIGN_NODE2TILE(tile_display_nodes[tile], tile);
        }
    }

  /* Assign each node to a tile it is rendering, if possible. */
    for (node = 0; node < num_processes; node++) {
        if (node_assignment[node] < 0) {
            const IceTBoolean *tile_mask
                = all_contained_tiles_masks + node*num_tiles;
            for (tile = 0; tile < num_tiles; tile++) {
                if (   (tile_mask[tile])
                    && (group_sizes[tile] < num_proc_for_tile[tile])) {
                    ASSIGN_NODE2TILE(node, tile);
                    break;
                }
            }
        }
    }

  /* Assign rest of the nodes. */
    node = 0;
    for (tile = 0; tile < num_tiles; tile++) {
        while (group_sizes[tile] < num_proc_for_tile[tile]) {
            while (node_assignment[node] >= 0) node++;
            ASSIGN_NODE2TILE(node, tile);
        }
    }

  /* Now figure out who I am sending to and how many I am receiving. */
    for (tile = 0; tile < num_tiles; tile++) {
        IceTInt *proc_group = tile_proc_groups + tile*num_processes;

        if (   (node_assignment[rank] != tile)
            && !all_contained_tiles_masks[rank*num_tiles + tile]) {
          /* Not involved with this tile.  Skip it. */
            continue;
        }

        if (!icetIsEnabled(ICET_ORDERED_COMPOSITE)) {
          /* If we are not doing an ordered composite, then we are free
           * to assign processes to images in any way we please.  Here we
           * will do everything we can to minimize communication. */

          /* First, have processes send images to themselves when possible. */
            if (   (node_assignment[rank] == tile)
                && all_contained_tiles_masks[rank*num_tiles + tile]) {
                tile_image_dest[tile] = rank;
            }

            snode = -1;
            rnode = -1;
            first_loop = 1;
            while (1) {
              /* Find next process that still needs a place to send images. */
                do {
                    snode++;
                } while (   (snode < num_processes)
                         && !all_contained_tiles_masks[snode*num_tiles + tile]);
                if (snode >= num_processes) {
                  /* We must be finished. */
                    break;
                }
                if (node_assignment[snode] == tile) {
                  /* This node keeps image in itself. */
                    continue;
                }

              /* Find the next process that can accept the image. */
                do {
                    rnode++;
                    if (rnode >= group_sizes[tile]) {
                        rnode = 0;
                        first_loop = 0;
                    }
                    dest = proc_group[rnode];
                } while (   first_loop
                         && all_contained_tiles_masks[dest*num_tiles + tile]
                         && (node_assignment[dest] == tile));

              /* Check to see if this node is sending the image data. */
                if (snode == rank) {
                    tile_image_dest[tile] = dest;
                }
            }
        } else {
          /* We are doing an ordered composite.  It is vital that each process
           * gets images that are consecutive in the ordering.  Communication
           * costs come second. */

            IceTInt num_contributors = 0;
            int i;
          /* First, we make a list of all processes contributing to this
           * tile in the order in which the images need to be composed.
           * We will then split this list into contiguous chunks and assign
           * each chunk to a process. */
            for (i = 0; i < num_processes; i++) {
                snode = composite_order[i];
                if (all_contained_tiles_masks[snode*num_tiles + tile]) {
                  /* The process snode contains an image for this tile.
                   * Add it to compositors. */
                    contributors[num_contributors] = snode;
                    num_contributors++;
                }
            }
#ifdef DEBUG
            if (num_contributors != contrib_counts[tile]) {
                icetRaiseError("Miscounted number of tile contributions",
                               ICET_SANITY_CHECK_FAIL);
            }
#endif

          /* contributors is split up as evenly as possible.  We will
           * assign nodes in the order they appear in proc_group.  We now
           * re-order proc_group to minimize the communications. */
            for (i = 0; i < num_contributors; i++) {
                piece = i*group_sizes[tile]/num_contributors;
                snode = contributors[i];
                if (node_assignment[snode] == tile) {
                  /* snode is part of this group.  Assign it to piece it is
                   * part of. */
                    int j;
                    for (j = group_sizes[tile]-1; j >=0; j--) {
                        if (proc_group[j] == snode) {
                          /* Found snode in proc_group.  Place it at
                           * "piece" in proc_group. */
                            proc_group[j] = proc_group[piece];
                            proc_group[piece] = snode;
                            break;
                        }
                    }
#ifdef DEBUG
                    if (j < 0) {
                        icetRaiseError("node_assignment/proc_group mismatch",
                                       ICET_SANITY_CHECK_FAIL);
                    }
#endif
                }
            }

          /* We have just shuffled proc_group, so the tile display node is
           * no longer necessarily at 0.  Find out where it should be. */
            if (node_assignment[rank] == tile) {
                for (i = 0; i < group_sizes[tile]; i++) {
                    if (proc_group[i] == tile_display_nodes[tile]) {
                        group_image_dest = i;
                        break;
                    }
                }
#ifdef DEBUG
                if (i == group_sizes[tile]) {
                    icetRaiseError("Display process not participating in tile?",
                                   ICET_SANITY_CHECK_FAIL);
                }
#endif
            }

          /* Assign nodes in the order they appear in proc_group. */
            for (i = 0; i < num_contributors; i++) {
                piece = i*group_sizes[tile]/num_contributors;
                snode = contributors[i];
                rnode = proc_group[piece];
                
              /* Check to see if this node is sending the image data. */
                if (snode == rank) {
                    tile_image_dest[tile] = rnode;
                }
            }
        }
    }

    *tile_image_destp = tile_image_dest;
    if (node_assignment[rank] < 0) {
        *compose_groupp = NULL;
        *group_sizep = 0;
        *group_image_destp = 0;
    } else {
        *compose_groupp = tile_proc_groups+node_assignment[rank]*num_processes;
        *group_sizep = group_sizes[node_assignment[rank]];
        *group_image_destp = group_image_dest;
    }
    return node_assignment[rank];
}

IceTImage reduceCollect(const IceTSparseImage composited_image,
                        IceTInt compose_tile,
                        IceTInt piece_offset)
{
    IceTInt tile_displayed;
    IceTInt num_tiles;
    const IceTInt *tile_viewports;
    const IceTInt *tile_display_nodes;
    IceTImage result_image;
    IceTInt tile_idx;

    icetGetIntegerv(ICET_TILE_DISPLAYED, &tile_displayed);
    icetGetIntegerv(ICET_NUM_TILES, &num_tiles);
    tile_viewports = icetUnsafeStateGetInteger(ICET_TILE_VIEWPORTS);
    tile_display_nodes = icetUnsafeStateGetInteger(ICET_DISPLAY_NODES);

    /* Run collect function for all tiles with data.  Unlike compose where
       we only had to call it for the tile we participated in, with collect
       all processes have to make a call for each tile. */
    result_image = icetImageNull();
    for (tile_idx = 0; tile_idx < num_tiles; tile_idx++) {
        const IceTInt *collect_tile_viewport = tile_viewports + 4*tile_idx;
        IceTSizeType collect_tile_width = collect_tile_viewport[2];
        IceTSizeType collect_tile_height = collect_tile_viewport[3];
        IceTSizeType offset;
        IceTSparseImage out_image;
        IceTImage in_image;

        if (tile_idx == compose_tile) {
            offset = piece_offset;
            out_image = composited_image;
        } else {
            offset = 0;
            out_image = icetSparseImageNull();
        }

        if (tile_idx == tile_displayed) {
            result_image = icetGetStateBufferImage(REDUCE_OUT_IMAGE_BUFFER,
                                                   collect_tile_width,
                                                   collect_tile_height);
            in_image = result_image;
        } else if (tile_idx == compose_tile) {
            in_image = icetGetStateBufferImage(REDUCE_IN_IMAGE_BUFFER,
                                               collect_tile_width,
                                               collect_tile_height);
        } else {
            in_image = icetImageNull();
        }

        icetSingleImageCollect(out_image,
                               tile_display_nodes[tile_idx],
                               offset,
                               in_image);
    }

    icetGetIntegerv(ICET_TILE_DISPLAYED, &tile_displayed);
    if ((tile_displayed >= 0) && (tile_displayed != compose_tile)) {
        /* Return empty image if nothing in this tile. */
        icetRaiseDebug("Clearing pixels");
        icetClearImageTrueBackground(result_image);
    }

    return result_image;
}
