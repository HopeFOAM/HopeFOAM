/* -*- c -*- *******************************************************/
/*
 * Copyright (C) 2003 Sandia Corporation
 * Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 * the U.S. Government retains certain rights in this software.
 *
 * This source code is released under the New BSD License.
 */

#ifndef __IceTDevProjections_h
#define __IceTDevProjections_h

#include <IceT.h>

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

/* Given the index to a tile, returns an adjusted ICET_PROJECTION_MATRIX
   for the tile. If the physical render size is larger than the tile,
   then the tile projection occurs in the lower left corner. */
ICET_EXPORT void icetProjectTile(IceTInt tile, IceTDouble *mat_out);

/* Given a viewport (defined by x, y, width, and height), return a matrix
   transforms a projection from the global display to the display provided. */
ICET_EXPORT void icetGetViewportProject(IceTInt x, IceTInt y,
                                        IceTSizeType width, IceTSizeType height,
                                        IceTDouble *mat_out);

/* Given two viewports, returns the region that is their intersection. It
   is OK if either of the two src_viewport pointer point to the same memory
   as dest_viewport. If the intersection is empty, the width and height
   of dest_viewport will be 0 and the x and y will be set to -1000000 (to
   place them outside of reasonable viewports). */
ICET_EXPORT void icetIntersectViewports(const IceTInt *src_viewport1,
                                        const IceTInt *src_viewport2,
                                        IceTInt *dest_viewport);

#ifdef __cplusplus
}
#endif

#endif /*__IceTDevProjections_h*/
