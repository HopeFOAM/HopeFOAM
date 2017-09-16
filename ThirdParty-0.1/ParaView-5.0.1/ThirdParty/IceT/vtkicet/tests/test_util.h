/* -*- c -*- *******************************************************/
/*
 * Copyright (C) 2003 Sandia Corporation
 * Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 * the U.S. Government retains certain rights in this software.
 *
 * This source code is released under the New BSD License.
 */

#ifndef _TEST_UTIL_H_
#define _TEST_UTIL_H_

#include "test_config.h"

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

#include <IceT.h>

extern IceTEnum strategy_list[];
extern int STRATEGY_LIST_SIZE;

extern IceTEnum single_image_strategy_list[];
extern int SINGLE_IMAGE_STRATEGY_LIST_SIZE;

extern IceTSizeType SCREEN_WIDTH;
extern IceTSizeType SCREEN_HEIGHT;

void initialize_test(int *argcp, char ***argvp, IceTCommunicator comm);

int run_test(int (*test_function)(void));

/* Used like printf but prints status only on process 0 or to independent
   logs. */
void printstat(const char *fmt, ...);

/* Like printf but adds the rank of the local process. */
void printrank(const char *fmt, ...);

void swap_buffers(void);

void finalize_test(IceTInt result);

void write_ppm(const char *filename,
               const IceTUByte *image,
               int width, int height);

IceTBoolean strategy_uses_single_image_strategy(IceTEnum strategy);

#ifdef __cplusplus
}
#endif

#endif /*_TEST_UTIL_H_*/
