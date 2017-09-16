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
#include <string.h>
#include "common.h"

#define DIRECT_IMAGE_BUFFER             ICET_STRATEGY_BUFFER_0
#define DIRECT_IN_SPARSE_IMAGE_BUFFER   ICET_STRATEGY_BUFFER_1
#define DIRECT_OUT_SPARSE_IMAGE_BUFFER  ICET_STRATEGY_BUFFER_2
#define DIRECT_TILE_IMAGE_DEST_BUFFER   ICET_STRATEGY_BUFFER_3

IceTImage icetDirectCompose(void)
{
    IceTImage image;
    IceTVoid *inSparseImageBuffer;
    IceTSparseImage outSparseImage;
    IceTSizeType sparseImageSize;
    const IceTInt *contrib_counts;
    const IceTInt *display_nodes;
    IceTInt max_width, max_height;
    IceTInt num_tiles;
    IceTInt num_contributors;
    IceTInt display_tile;
    IceTInt tile;
    IceTInt *tile_image_dest;
    icetRaiseDebug("In Direct Compose");

    icetGetIntegerv(ICET_TILE_MAX_WIDTH, &max_width);
    icetGetIntegerv(ICET_TILE_MAX_HEIGHT, &max_height);
    icetGetIntegerv(ICET_NUM_TILES, &num_tiles);

    sparseImageSize = icetSparseImageBufferSize(max_width, max_height);

    image               = icetGetStateBufferImage(DIRECT_IMAGE_BUFFER,
                                                  max_width, max_height);
    inSparseImageBuffer = icetGetStateBuffer(DIRECT_IN_SPARSE_IMAGE_BUFFER,
                                             sparseImageSize);
    outSparseImage      = icetGetStateBufferSparseImage(
                                                 DIRECT_OUT_SPARSE_IMAGE_BUFFER,
                                                 max_width, max_height);
    tile_image_dest     = icetGetStateBuffer(DIRECT_TILE_IMAGE_DEST_BUFFER,
                                             num_tiles*sizeof(IceTInt));

    icetGetIntegerv(ICET_TILE_DISPLAYED, &display_tile);
    if (display_tile >= 0) {
        contrib_counts = icetUnsafeStateGetInteger(ICET_TILE_CONTRIB_COUNTS);
        num_contributors = contrib_counts[display_tile];
    } else {
        num_contributors = 0;
    }

    display_nodes = icetUnsafeStateGetInteger(ICET_DISPLAY_NODES);
    for (tile = 0; tile < num_tiles; tile++) {
        tile_image_dest[tile] = display_nodes[tile];
    }

    icetRaiseDebug("Rendering and transferring images.");
    icetRenderTransferFullImages(image,
                                 inSparseImageBuffer,
                                 outSparseImage,
                                 tile_image_dest);

    if (display_tile >= 0) {
        if (num_contributors > 0) {
            icetImageCorrectBackground(image);
        } else {
            /* Must be displaying a blank tile. */
            const IceTInt *tile_viewports
                = icetUnsafeStateGetInteger(ICET_TILE_VIEWPORTS);
            const IceTInt *display_tile_viewport
                = tile_viewports + 4*display_tile;
            IceTInt display_tile_width = display_tile_viewport[2];
            IceTInt display_tile_height = display_tile_viewport[3];

            icetRaiseDebug("Returning blank tile.");
            icetImageSetDimensions(image,
                                   display_tile_width,
                                   display_tile_height);
            icetClearImageTrueBackground(image);
        }
    }

    return image;
}
