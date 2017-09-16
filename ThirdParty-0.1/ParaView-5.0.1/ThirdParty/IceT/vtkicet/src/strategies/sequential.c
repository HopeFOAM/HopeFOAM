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

#define SEQUENTIAL_IMAGE_BUFFER                 ICET_STRATEGY_BUFFER_0
#define SEQUENTIAL_FINAL_IMAGE_BUFFER           ICET_STRATEGY_BUFFER_1
#define SEQUENTIAL_INTERMEDIATE_IMAGE_BUFFER    ICET_STRATEGY_BUFFER_2
#define SEQUENTIAL_COMPOSE_GROUP_BUFFER         ICET_STRATEGY_BUFFER_3

IceTImage icetSequentialCompose(void)
{
    IceTInt num_tiles;
    IceTInt rank;
    IceTInt num_proc;
    const IceTInt *display_nodes;
    const IceTInt *tile_viewports;
    IceTBoolean ordered_composite;
    IceTBoolean image_collect;
    IceTImage my_image;
    IceTInt *compose_group;
    int i;

    icetGetIntegerv(ICET_NUM_TILES, &num_tiles);
    icetGetIntegerv(ICET_RANK, &rank);
    icetGetIntegerv(ICET_NUM_PROCESSES, &num_proc);
    display_nodes = icetUnsafeStateGetInteger(ICET_DISPLAY_NODES);
    tile_viewports = icetUnsafeStateGetInteger(ICET_TILE_VIEWPORTS);
    ordered_composite = icetIsEnabled(ICET_ORDERED_COMPOSITE);
    image_collect = icetIsEnabled(ICET_COLLECT_IMAGES);

    if (!image_collect && (num_tiles > 1)) {
        icetRaiseWarning("Sequential strategy must collect images with more"
                         " than one tile.",
                         ICET_INVALID_OPERATION);
        image_collect = ICET_TRUE;
    }

    compose_group = icetGetStateBuffer(SEQUENTIAL_COMPOSE_GROUP_BUFFER,
                                       sizeof(IceTInt)*num_proc);

    my_image = icetImageNull();

    if (ordered_composite) {
	icetGetIntegerv(ICET_COMPOSITE_ORDER, compose_group);
    } else {
	for (i = 0; i < num_proc; i++) {
	    compose_group[i] = i;
	}
    }

  /* Render and compose every tile. */
    for (i = 0; i < num_tiles; i++) {
	int d_node = display_nodes[i];
	int image_dest;
        IceTSparseImage rendered_image;
        IceTSparseImage composited_image;
        IceTSizeType piece_offset;
        IceTSizeType tile_width;
        IceTSizeType tile_height;

        tile_width = tile_viewports[4*i + 2];
        tile_height = tile_viewports[4*i + 3];

      /* Make the image go to the display node. */
	if (ordered_composite) {
	    for (image_dest = 0; compose_group[image_dest] != d_node;
		 image_dest++);
	} else {
	  /* Technically, the above computation will work, but this is
	     faster. */
	    image_dest = d_node;
	}


        rendered_image = icetGetStateBufferSparseImage(SEQUENTIAL_IMAGE_BUFFER,
                                                       tile_width, tile_height);

        icetGetCompressedTileImage(i, rendered_image);
	icetSingleImageCompose(compose_group,
                               num_proc,
                               image_dest,
                               rendered_image,
                               &composited_image,
                               &piece_offset);

        if (image_collect) {
            IceTImage tile_image;

            /* If this processor is display node, make sure image goes to
               myColorBuffer. */
            if (d_node == rank) {
                tile_image = icetGetStateBufferImage(
                                                  SEQUENTIAL_FINAL_IMAGE_BUFFER,
                                                  tile_width, tile_height);
            } else {
                tile_image = icetGetStateBufferImage(
                                           SEQUENTIAL_INTERMEDIATE_IMAGE_BUFFER,
                                           tile_width, tile_height);
            }

            icetSingleImageCollect(composited_image,
                                   d_node,
                                   piece_offset,
                                   tile_image);

            if (d_node == rank) {
                my_image = tile_image;
            }
        } else { /* !image_collect */
            IceTSizeType piece_size
                = icetSparseImageGetNumPixels(composited_image);
            if (piece_size > 0) {
                my_image = icetGetStateBufferImage(
                                                  SEQUENTIAL_FINAL_IMAGE_BUFFER,
                                                  tile_width, tile_height);
                icetDecompressSubImageCorrectBackground(composited_image,
                                                        piece_offset,
                                                        my_image);
                icetStateSetInteger(ICET_VALID_PIXELS_TILE, i);
                icetStateSetInteger(ICET_VALID_PIXELS_OFFSET, piece_offset);
                icetStateSetInteger(ICET_VALID_PIXELS_NUM, piece_size);
            } else {
                my_image = icetImageNull();
                icetStateSetInteger(ICET_VALID_PIXELS_TILE, -1);
                icetStateSetInteger(ICET_VALID_PIXELS_OFFSET, 0);
                icetStateSetInteger(ICET_VALID_PIXELS_NUM, 0);
            }
        }
    }

    return my_image;
}
