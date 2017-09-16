/* -*- c -*- *******************************************************/
/*
 * Copyright (C) 2011 Sandia Corporation
 * Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 * the U.S. Government retains certain rights in this software.
 *
 * This source code is released under the New BSD License.
 */

#ifndef __IceTDevTiming_h
#define __IceTDevTiming_h

#include <IceT.h>
#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

ICET_EXPORT void icetStateResetTiming(void);

ICET_EXPORT void icetTimingRenderBegin(void);
ICET_EXPORT void icetTimingRenderEnd(void);

ICET_EXPORT void icetTimingBufferReadBegin(void);
ICET_EXPORT void icetTimingBufferReadEnd(void);

ICET_EXPORT void icetTimingBufferWriteBegin(void);
ICET_EXPORT void icetTimingBufferWriteEnd(void);

ICET_EXPORT void icetTimingCompressBegin(void);
ICET_EXPORT void icetTimingCompressEnd(void);

ICET_EXPORT void icetTimingInterlaceBegin(void);
ICET_EXPORT void icetTimingInterlaceEnd(void);

ICET_EXPORT void icetTimingBlendBegin(void);
ICET_EXPORT void icetTimingBlendEnd(void);

ICET_EXPORT void icetTimingCollectBegin(void);
ICET_EXPORT void icetTimingCollectEnd(void);

ICET_EXPORT void icetTimingDrawFrameBegin(void);
ICET_EXPORT void icetTimingDrawFrameEnd(void);

#ifdef __cplusplus
}
#endif


#endif /* __IceTDevTiming_h */
