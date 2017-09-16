/* -*- c -*- *******************************************************/
/*
 * Copyright (C) 2003 Sandia Corporation
 * Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 * the U.S. Government retains certain rights in this software.
 *
 * This source code is released under the New BSD License.
 */

#ifndef _ICET_CONTEXT_H_
#define _ICET_CONTEXT_H_

#include <IceT.h>
#include <IceTDevState.h>

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

ICET_EXPORT IceTState icetGetState();
ICET_EXPORT IceTCommunicator icetGetCommunicator();

#ifdef __cplusplus
}
#endif

#endif /* _ICET_CONTEXT_H_ */
