/* -*- c -*- *******************************************************/
/*
 * Copyright (C) 2003 Sandia Corporation
 * Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 * the U.S. Government retains certain rights in this software.
 *
 * This source code is released under the New BSD License.
 */

#ifndef __IceTDevPorting_h
#define __IceTDevPorting_h

#include <IceT.h>

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

/* Returns the size of the type given by the identifier (ICET_INT, ICET_FLOAT,
   etc.)  in bytes. */
ICET_EXPORT IceTInt icetTypeWidth(IceTEnum type);

#ifdef __cplusplus
}
#endif

#endif /*__IceTDevPorting_h*/
