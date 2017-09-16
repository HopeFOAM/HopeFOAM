/* -*- c -*- *******************************************************/
/*
 * Copyright (C) 2003 Sandia Corporation
 * Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 * the U.S. Government retains certain rights in this software.
 *
 * This source code is released under the New BSD License.
 */

#include <IceT.h>

#include <IceTDevState.h>
#include <IceTDevDiagnostics.h>
#include <IceTDevPorting.h>

#include <stdlib.h>

void icetResetTiles(void)
{
    IceTInt iarray[4];

    icetStateSetInteger(ICET_NUM_TILES, 0);
    icetStateSetIntegerv(ICET_TILE_VIEWPORTS, 0, NULL);
    icetStateSetInteger(ICET_TILE_DISPLAYED, -1);
    icetStateSetIntegerv(ICET_DISPLAY_NODES, 0, NULL);

    iarray[0] = 0;  iarray[1] = 0;  iarray[2] = 0;  iarray[3] = 0;
    icetStateSetIntegerv(ICET_GLOBAL_VIEWPORT, 4, iarray);

    icetStateSetInteger(ICET_TILE_MAX_WIDTH, 0);
    icetStateSetInteger(ICET_TILE_MAX_HEIGHT, 0);

    icetPhysicalRenderSize(0, 0);
}

#ifndef MIN
#define MIN(x, y) ((x) <= (y) ? (x) : (y))
#endif
#ifndef MAX
#define MAX(x, y) ((x) >= (y) ? (x) : (y))
#endif
int  icetAddTile(IceTInt x, IceTInt y, IceTSizeType width, IceTSizeType height,
                 int display_rank)
{
    IceTInt num_tiles;
    IceTInt *viewports;
    IceTInt gvp[4];
    IceTInt max_width, max_height;
    IceTInt *display_nodes;
    IceTInt rank;
    IceTInt num_processors;
    char msg[256];
    int i;

    if ((width < 1) || (height < 1)) {
        icetRaiseError("Attempted to create a tile with no pixels.",
                       ICET_INVALID_VALUE);
        return -1;
    }

  /* Get current number of tiles. */
    icetGetIntegerv(ICET_NUM_TILES, &num_tiles);

  /* Get display node information. */
    icetGetIntegerv(ICET_RANK, &rank);
    icetGetIntegerv(ICET_NUM_PROCESSES, &num_processors);
    display_nodes = malloc((num_tiles+1)*4*sizeof(IceTInt));
    icetGetIntegerv(ICET_DISPLAY_NODES, display_nodes);

  /* Check and update display ranks. */
    if (display_rank >= num_processors) {
        sprintf(msg, "icetDisplayNodes: Invalid rank for tile %d.",
                (int)num_tiles);
        icetRaiseError(msg, ICET_INVALID_VALUE);
        free(display_nodes);
        return -1;
    }
    for (i = 0; i < num_tiles; i++) {
        if (display_nodes[i] == display_rank) {
            sprintf(msg, "icetDisplayNodes: Rank %d used for tiles %d and %d.",
                    display_rank, i, (int)num_tiles);
            icetRaiseError(msg, ICET_INVALID_VALUE);
            free(display_nodes);
            return -1;
        }
    }
    display_nodes[num_tiles] = display_rank;
    icetStateSetIntegerv(ICET_DISPLAY_NODES, num_tiles+1, display_nodes);
    free(display_nodes);
    if (display_rank == rank) {
        icetStateSetInteger(ICET_TILE_DISPLAYED, num_tiles);
    }

  /* Get viewports. */
    viewports = malloc((num_tiles+1)*4*sizeof(IceTInt));
    icetGetIntegerv(ICET_TILE_VIEWPORTS, viewports);

  /* Figure out current global viewport. */
    gvp[0] = x;  gvp[1] = y;
    gvp[2] = x + (IceTInt)width;  gvp[3] = y + (IceTInt)height;
    for (i = 0; i < num_tiles; i++) {
        gvp[0] = MIN(gvp[0], viewports[i*4+0]);
        gvp[1] = MIN(gvp[1], viewports[i*4+1]);
        gvp[2] = MAX(gvp[2], viewports[i*4+0] + viewports[i*4+2]);
        gvp[3] = MAX(gvp[3], viewports[i*4+1] + viewports[i*4+3]);
    }
    gvp[2] -= gvp[0];
    gvp[3] -= gvp[1];

  /* Add new viewport to current viewports. */
    viewports[4*num_tiles+0] = x;
    viewports[4*num_tiles+1] = y;
    viewports[4*num_tiles+2] = (IceTInt)width;
    viewports[4*num_tiles+3] = (IceTInt)height;

  /* Set new state. */
    icetStateSetInteger(ICET_NUM_TILES, num_tiles+1);
    icetStateSetIntegerv(ICET_TILE_VIEWPORTS, (num_tiles+1)*4, viewports);
    icetStateSetIntegerv(ICET_GLOBAL_VIEWPORT, 4, gvp);

    free(viewports);

    icetGetIntegerv(ICET_TILE_MAX_WIDTH, &max_width);
    max_width = MAX(max_width, (IceTInt)width);
    icetStateSetInteger(ICET_TILE_MAX_WIDTH, max_width);
    icetGetIntegerv(ICET_TILE_MAX_HEIGHT, &max_height);
    max_height = MAX(max_height, (IceTInt)height);
    icetStateSetInteger(ICET_TILE_MAX_HEIGHT, max_height);

    icetPhysicalRenderSize(max_width, max_height);

  /* Return index to tile. */
    return num_tiles;
}

void icetPhysicalRenderSize(IceTInt width, IceTInt height)
{
    IceTInt max_width, max_height;

  /* Perhaps this test should be moved to right before the render callback
     is invoked. */
    icetGetIntegerv(ICET_TILE_MAX_WIDTH, &max_width);
    icetGetIntegerv(ICET_TILE_MAX_HEIGHT, &max_height);
    if ((width < max_width) || (height < max_height)) {
        icetRaiseWarning("Physical render dimensions not large enough"
                         " to render all tiles.", ICET_INVALID_VALUE);
    }

    icetStateSetInteger(ICET_PHYSICAL_RENDER_WIDTH, width);
    icetStateSetInteger(ICET_PHYSICAL_RENDER_HEIGHT, height);
}

void icetBoundingBoxd(IceTDouble x_min, IceTDouble x_max,
                      IceTDouble y_min, IceTDouble y_max,
                      IceTDouble z_min, IceTDouble z_max)
{
    IceTDouble vertices[8*3];

    vertices[3*0+0] = x_min;  vertices[3*0+1] = y_min;  vertices[3*0+2] = z_min;
    vertices[3*1+0] = x_min;  vertices[3*1+1] = y_min;  vertices[3*1+2] = z_max;
    vertices[3*2+0] = x_min;  vertices[3*2+1] = y_max;  vertices[3*2+2] = z_min;
    vertices[3*3+0] = x_min;  vertices[3*3+1] = y_max;  vertices[3*3+2] = z_max;
    vertices[3*4+0] = x_max;  vertices[3*4+1] = y_min;  vertices[3*4+2] = z_min;
    vertices[3*5+0] = x_max;  vertices[3*5+1] = y_min;  vertices[3*5+2] = z_max;
    vertices[3*6+0] = x_max;  vertices[3*6+1] = y_max;  vertices[3*6+2] = z_min;
    vertices[3*7+0] = x_max;  vertices[3*7+1] = y_max;  vertices[3*7+2] = z_max;

    icetStateSetDoublev(ICET_GEOMETRY_BOUNDS, 8*3, vertices);
    icetStateSetInteger(ICET_NUM_BOUNDING_VERTS, 8);
}

void icetBoundingBoxf(IceTFloat x_min, IceTFloat x_max,
                      IceTFloat y_min, IceTFloat y_max,
                      IceTFloat z_min, IceTFloat z_max)
{
    icetBoundingBoxd(x_min, x_max, y_min, y_max, z_min, z_max);
}

void icetBoundingVertices(IceTInt size, IceTEnum type, IceTSizeType stride,
                          IceTSizeType count, const IceTVoid *pointer)
{
    IceTDouble *verts;
    int i, j;

    if (count < 1) {
        /* No vertices. (Must be clearing them out.) */
        icetStateSetDoublev(ICET_GEOMETRY_BOUNDS, 0, NULL);
        icetStateSetInteger(ICET_NUM_BOUNDING_VERTS, 0);
        return;
    }

    if (stride < 1) {
        stride = size*icetTypeWidth(type);
    }

    verts = malloc(count*3*sizeof(IceTDouble));
    for (i = 0; i < count; i++) {
        for (j = 0; j < 3; j++) {
            switch (type) {
#define castcopy(ptype)							\
  if (j < size) {							\
      verts[i*3+j] = ((ptype *)pointer)[i*stride/sizeof(type)+j];	\
  } else {								\
      verts[i*3+j] = 0.0;						\
  }									\
  if (size >= 4) {							\
      verts[i*3+j] /= ((ptype *)pointer)[i*stride/sizeof(type)+4];	\
  }									\
  break;
              case ICET_SHORT:
                  castcopy(IceTShort);
              case ICET_INT:
                  castcopy(IceTInt);
              case ICET_FLOAT:
                  castcopy(IceTFloat);
              case ICET_DOUBLE:
                  castcopy(IceTDouble);
              default:
                  icetRaiseError("Bad type to icetBoundingVertices.",
                                 ICET_INVALID_ENUM);
                  free(verts);
                  return;
            }
        }
    }

    icetStateSetDoublev(ICET_GEOMETRY_BOUNDS, count*3, verts);
    free(verts);
    icetStateSetInteger(ICET_NUM_BOUNDING_VERTS, (IceTInt)count);
}
