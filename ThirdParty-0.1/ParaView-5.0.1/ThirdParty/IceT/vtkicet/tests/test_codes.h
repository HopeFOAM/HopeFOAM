/* -*- c -*- */
/*
 * Copyright (C) 2003 Sandia Corporation
 * Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 * the U.S. Government retains certain rights in this software.
 *
 * This source code is released under the New BSD License.
 */

#ifndef _TEST_CODES_H_
#define _TEST_CODES_H_

/* Return this if the test passed. */
#define TEST_PASSED	0

/* Return this if the test could not be run because of some failed
 * pre-condition.  Example: test is run on one node and needs to be run on
 * at least 2. */
#define TEST_NOT_RUN	-1

/* Return this if the test failed for a reason not related to IceT.
 * Example: could not read or write to a file.  Note that out of memory
 * errors may be caused by IceT memory leaks. */
#define TEST_NOT_PASSED	-2

/* Return this if the test failed outright. */
#define TEST_FAILED	-3

#endif /*_TEST_CODES_H_*/
