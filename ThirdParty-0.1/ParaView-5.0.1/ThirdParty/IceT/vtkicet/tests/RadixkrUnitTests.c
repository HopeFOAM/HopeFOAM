/* -*- c -*- *****************************************************************
** Copyright (C) 2011 Sandia Corporation
** Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
** the U.S. Government retains certain rights in this software.
**
** This source code is released under the New BSD License.
**
** This exercises the radix-k unit tests of internal functions.
*****************************************************************************/

#include <IceT.h>
#include "test_codes.h"
#include "test_util.h"

#include <stdlib.h>

extern ICET_EXPORT IceTBoolean icetRadixkrPartitionLookupUnitTest(void);

static int RadixkUnitTestsRun(void)
{
    IceTInt rank;

    icetGetIntegerv(ICET_RANK, &rank);
    if (rank != 0) {
        return TEST_PASSED;
    }

    if (!icetRadixkrPartitionLookupUnitTest()) {
        return TEST_FAILED;
    }

    return TEST_PASSED;
}

int RadixkrUnitTests(int argc, char *argv[])
{
    /* To remove warning. */
    (void)argc;
    (void)argv;

    return run_test(RadixkUnitTestsRun);
}
