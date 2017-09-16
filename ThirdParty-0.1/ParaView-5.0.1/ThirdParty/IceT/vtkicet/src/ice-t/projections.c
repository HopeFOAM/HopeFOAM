/* -*- c -*- *******************************************************/
/*
 * Copyright (C) 2003 Sandia Corporation
 * Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 * the U.S. Government retains certain rights in this software.
 *
 * This source code is released under the New BSD License.
 */

#include <IceTDevProjections.h>

#include <IceT.h>

#include <IceTDevState.h>
#include <IceTDevDiagnostics.h>
#include <IceTDevMatrix.h>

#include <stdlib.h>
#include <string.h>

#ifndef MIN
#define MIN(x, y)       ((x) < (y) ? (x) : (y))
#endif
#ifndef MAX
#define MAX(x, y)       ((x) < (y) ? (y) : (x))
#endif


static void update_tile_projections(void);

void icetProjectTile(IceTInt tile, IceTDouble *mat_out)
{
    IceTInt num_tiles;
    const IceTInt *viewports;
    const IceTDouble *tile_projections;
    IceTInt tile_width, tile_height;
    IceTInt renderable_width, renderable_height;
    const IceTDouble *tile_proj;
    IceTDouble tile_viewport_proj[16];
    const IceTDouble *global_proj;

  /* Update tile projections. */
    update_tile_projections();

    icetGetIntegerv(ICET_NUM_TILES, &num_tiles);
    if ((tile < 0) || (tile >= num_tiles)) {
        icetRaiseError("Bad tile passed to icetProjectTile.",
                       ICET_INVALID_VALUE);
        return;
    }

    viewports = icetUnsafeStateGetInteger(ICET_TILE_VIEWPORTS);
    tile_width = viewports[tile*4+2];
    tile_height = viewports[tile*4+3];
    icetGetIntegerv(ICET_PHYSICAL_RENDER_WIDTH, &renderable_width);
    icetGetIntegerv(ICET_PHYSICAL_RENDER_HEIGHT, &renderable_height);
    tile_projections = icetUnsafeStateGetDouble(ICET_TILE_PROJECTIONS);

    tile_proj = tile_projections + 16*tile;

    if ((renderable_width != tile_width) || (renderable_height != tile_height)){
      /* Compensate for fact that tile is smaller than actual window.
         Use an orthographic projection to place the tile in the lower left
         corner of the tile. */
        IceTDouble viewport_proj[16];
        icetMatrixOrtho(-1.0, 2.0*renderable_width/tile_width - 1.0,
                        -1.0, 2.0*renderable_height/tile_height - 1.0,
                        1.0, -1.0, viewport_proj);

        icetMatrixMultiply(tile_viewport_proj,
                           (const IceTDouble *)viewport_proj,
                           (const IceTDouble *)tile_proj);
    } else {
        memcpy(tile_viewport_proj, (const IceTDouble*)tile_proj,
               16*sizeof(IceTDouble));
    }

  /* Project the user requested view to the tile projection. */
    global_proj = icetUnsafeStateGetDouble(ICET_PROJECTION_MATRIX);
    icetMatrixMultiply(mat_out,
                       (const IceTDouble *)tile_viewport_proj,
                       (const IceTDouble *)global_proj);
}

void icetGetViewportProject(IceTInt x, IceTInt y, IceTSizeType width,
                            IceTSizeType height, IceTDouble *mat_out)
{
    IceTInt global_viewport[4];
/*     IceTDouble viewport_transform[16]; */
/*     IceTDouble tile_transform[16]; */

    icetGetIntegerv(ICET_GLOBAL_VIEWPORT, global_viewport);

/*     viewport_transform[ 0] = 0.5*global_viewport[2]; */
/*     viewport_transform[ 1] = 0.0; */
/*     viewport_transform[ 2] = 0.0; */
/*     viewport_transform[ 3] = 0.0; */

/*     viewport_transform[ 4] = 0.0; */
/*     viewport_transform[ 5] = 0.5*global_viewport[3]; */
/*     viewport_transform[ 6] = 0.0; */
/*     viewport_transform[ 7] = 0.0; */

/*     viewport_transform[ 8] = 0.0; */
/*     viewport_transform[ 9] = 0.0; */
/*     viewport_transform[10] = 1.0; */
/*     viewport_transform[11] = 0.0; */

/*     viewport_transform[12] = 0.5*global_viewport[2] + global_viewport[0]*; */
/*     viewport_transform[13] = 0.5*global_viewport[3] + global_viewport[1]*; */
/*     viewport_transform[14] = 0.0; */
/*     viewport_transform[15] = 1.0; */

/*     tile_transform[ 0] = 2.0/width; */
/*     tile_transform[ 1] = 0.0; */
/*     tile_transform[ 2] = 0.0; */
/*     tile_transform[ 3] = 0.0; */

/*     tile_transform[ 4] = 0.0; */
/*     tile_transform[ 5] = 2.0/height; */
/*     tile_transform[ 6] = 0.0; */
/*     tile_transform[ 7] = 0.0; */

/*     tile_transform[ 8] = 0.0; */
/*     tile_transform[ 9] = 0.0; */
/*     tile_transform[10] = 1.0; */
/*     tile_transform[11] = 0.0; */

/*     tile_transform[12] = -(2.0*x)/width - 1.0; */
/*     tile_transform[13] = -(2.0*y)/height - 1.0; */
/*     tile_transform[14] = 0.0; */
/*     tile_transform[15] = 1.0; */

/*     multMatrix(mat_out, tile_transform, viewport_transform); */

    mat_out[ 0] = (IceTDouble)global_viewport[2]/width;
    mat_out[ 1] = 0.0;
    mat_out[ 2] = 0.0;
    mat_out[ 3] = 0.0;

    mat_out[ 4] = 0.0;
    mat_out[ 5] = (IceTDouble)global_viewport[3]/height;
    mat_out[ 6] = 0.0;
    mat_out[ 7] = 0.0;

    mat_out[ 8] = 0.0;
    mat_out[ 9] = 0.0;
    mat_out[10] = 1.0;
    mat_out[11] = 0.0;

    mat_out[12] = (IceTDouble)(  global_viewport[2] + 2*global_viewport[0]
                             - 2*x - width)/width;
    mat_out[13] = (IceTDouble)(  global_viewport[3] + 2*global_viewport[1]
                             - 2*y - height)/height;
    mat_out[14] = 0.0;
    mat_out[15] = 1.0;
}

static void update_tile_projections(void)
{
    IceTInt num_tiles;
    const IceTInt *viewports;
    IceTDouble *tile_projections;
    IceTInt tile_idx;

    if (  icetStateGetTime(ICET_TILE_VIEWPORTS)
        < icetStateGetTime(ICET_TILE_PROJECTIONS) ) {
        /* Projections already up to date. */
        return;
    }

    icetGetIntegerv(ICET_NUM_TILES, &num_tiles);
    tile_projections = icetStateAllocateDouble(ICET_TILE_PROJECTIONS,
                                               num_tiles*16);
    viewports = icetUnsafeStateGetInteger(ICET_TILE_VIEWPORTS);

    for (tile_idx = 0; tile_idx < num_tiles; tile_idx++) {
        icetGetViewportProject(viewports[tile_idx*4+0], viewports[tile_idx*4+1],
                               viewports[tile_idx*4+2], viewports[tile_idx*4+3],
                               tile_projections + 16*tile_idx);
    }
}

void icetIntersectViewports(const IceTInt *src_viewport1,
                            const IceTInt *src_viewport2,
                            IceTInt *dest_viewport)
{
    const IceTInt min_x1 = src_viewport1[0];
    const IceTInt max_x1 = min_x1 + src_viewport1[2];
    const IceTInt min_y1 = src_viewport1[1];
    const IceTInt max_y1 = min_y1 + src_viewport1[3];

    const IceTInt min_x2 = src_viewport2[0];
    const IceTInt max_x2 = min_x2 + src_viewport2[2];
    const IceTInt min_y2 = src_viewport2[1];
    const IceTInt max_y2 = min_y2 + src_viewport2[3];

    const IceTInt min_x = MAX(min_x1, min_x2);
    const IceTInt min_y = MAX(min_y1, min_y2);
    const IceTInt max_x = MIN(max_x1, max_x2);
    const IceTInt max_y = MIN(max_y1, max_y2);

    const IceTInt width = max_x - min_x;
    const IceTInt height = max_y - min_y;

    if ((width > 0) && (height > 0)) {
        dest_viewport[0] = min_x;
        dest_viewport[1] = min_y;
        dest_viewport[2] = width;
        dest_viewport[3] = height;
    } else {
        dest_viewport[0] = dest_viewport[1] = -1000000;
        dest_viewport[2] = dest_viewport[3] = 0;
    }
}
