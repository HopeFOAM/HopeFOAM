/* -*- c -*- *******************************************************/
/*
 * Copyright (C) 2015 Sandia Corporation
 * Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 * the U.S. Government retains certain rights in this software.
 *
 * This source code is released under the New BSD License.
 */

#include "test_util.h"
#include "test_codes.h"

#include <IceTDevCommunication.h>

#ifndef __USE_POSIX
#define __USE_POSIX
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifndef WIN32
#include <unistd.h>
#else
#include <io.h>
#define dup(fildes)             _dup(fildes)
#define dup2(fildes, fildes2)   _dup2(fildes, fildes2)
#endif

IceTEnum strategy_list[5];
int STRATEGY_LIST_SIZE = 5;
/* int STRATEGY_LIST_SIZE = 1; */

IceTEnum single_image_strategy_list[6];
int SINGLE_IMAGE_STRATEGY_LIST_SIZE = 6;
/* int SINGLE_IMAGE_STRATEGY_LIST_SIZE = 1; */

IceTSizeType SCREEN_WIDTH;
IceTSizeType SCREEN_HEIGHT;

static void checkIceTError(void)
{
    IceTEnum error = icetGetError();

#define CASE_ERROR(ename)                                               \
    case ename: printrank("## Current IceT error = " #ename "\n"); break;

    switch (error) {
      case ICET_NO_ERROR: break;
      CASE_ERROR(ICET_SANITY_CHECK_FAIL);
      CASE_ERROR(ICET_INVALID_ENUM);
      CASE_ERROR(ICET_BAD_CAST);
      CASE_ERROR(ICET_OUT_OF_MEMORY);
      CASE_ERROR(ICET_INVALID_OPERATION);
      CASE_ERROR(ICET_INVALID_VALUE);
      default:
          printrank("## UNKNOWN ICET ERROR CODE!!!!!\n");
          break;
    }

#undef CASE_ERROR
}

/* Just in case I need to actually print stuff out to the screen in the
   future. */
static FILE *realstdout;
#if 0
static void realprintf(const char *fmt, ...)
{
    va_list ap;

    if (realstdout != NULL) {
        va_start(ap, fmt);
        vfprintf(realstdout, fmt, ap);
        va_end(ap);
        fflush(realstdout);
    }
}
#endif

void printstat(const char *fmt, ...)
{
    va_list ap;
    IceTInt rank;

    icetGetIntegerv(ICET_RANK, &rank);

    if ((rank == 0) || (realstdout == NULL)) {
        va_start(ap, fmt);
        vprintf(fmt, ap);
        va_end(ap);
        fflush(stdout);
    }
}

void printrank(const char *fmt, ...)
{
    va_list ap;
    IceTInt rank;

    icetGetIntegerv(ICET_RANK, &rank);

    printf("%d> ", rank);
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    fflush(stdout);
}

static IceTContext context;

static void usage(char **argv)
{
    printf("\nUSAGE: %s [options] [-R] testname [testargs]\n", argv[0]);
    printf("\nWhere options are:\n");
    printf("  -width <n>  Width of window (default n=1024)\n");
    printf("  -height <n> Height of window (default n=768).\n");
    printf("  -display <display>\n");
    printf("              X server each node contacts.  Default display=localhost:0\n");
    printf("  -logdebug   Add debugging statements.  Provides more information, but\n");
    printf("              makes identifying errors, warnings, and statuses harder\n");
    printf("  -redirect   Redirect standard output to log.????, where ???? is the rank\n");
    printf("  --          Parse no more arguments.\n");
    printf("  -h, -help   This help message.\n");
}

extern void initialize_render_window(int width, int height);
void initialize_test(int *argcp, char ***argvp, IceTCommunicator comm)
{
    int arg;
    int argc = *argcp;
    char **argv = *argvp;
    int width = 1024;
    int height = 768;
    IceTBitField diag_level = ICET_DIAG_ALL_NODES | ICET_DIAG_WARNINGS;
    int redirect = 0;
    int rank, num_proc;

    rank = (*comm->Comm_rank)(comm);
    num_proc = (*comm->Comm_size)(comm);

  /* This is convenience code to attach a debugger to a particular process at
     the start of a test. */
#if 0
    if (rank == 0) {
        int i = 0;
        printf("Waiting in process %d\n", getpid());
        while (i == 0) sleep(1);
    }
#endif

  /* Parse my arguments. */
    for (arg = 1; arg < argc; arg++) {
        if (strcmp(argv[arg], "-width") == 0) {
            width = atoi(argv[++arg]);
        } else if (strcmp(argv[arg], "-height") == 0) {
            height = atoi(argv[++arg]);
        } else if (strcmp(argv[arg], "-logdebug") == 0) {
            diag_level = ICET_DIAG_FULL;
        } else if (strcmp(argv[arg], "-redirect") == 0) {
            redirect = 1;
        } else if (   (strcmp(argv[arg], "-h") == 0)
                   || (strcmp(argv[arg], "-help") == 0) ) {
            usage(argv);
            exit(0);
        } else if (   (strcmp(argv[arg], "-R") == 0)
                   || (strncmp(argv[arg], "-", 1) != 0) ) {
            break;
        } else if (strcmp(argv[arg], "--") == 0) {
            arg++;
            break;
        } else {
            printf("Unknown options `%s'.  Try -h for help.\n", argv[arg]);
            exit(1);
        }
    }

  /* Fix arguments for next bout of parsing. */
    *argcp = 1;
    for ( ; arg < argc; arg++, (*argcp)++) {
        argv[*argcp] = argv[arg];
    }
    argc = *argcp;

  /* Make sure selected options are consistent. */
    if ((num_proc > 1) && (argc < 2)) {
        printf("You must select a test on the command line when using more than one process.\n");
        printf("Try -h for help.\n");
        exit(1);
    }
    if (redirect && (argc < 2)) {
        printf("You must select a test on the command line when redirecting the output.\n");
        printf("Try -h for help.\n");
        exit(1);
    }

    SCREEN_WIDTH = width;
    SCREEN_HEIGHT = height;

  /* Create an IceT context. */
    context = icetCreateContext(comm);
    icetDiagnostics(diag_level);

    initialize_render_window(width, height);

  /* Redirect standard output on demand. */
    if (redirect) {
        char filename[64];
        int outfd;
        if (rank == 0) {
            realstdout = fdopen(dup(1), "wt");
        } else {
            realstdout = NULL;
        }
        sprintf(filename, "log.%04d", rank);
        outfd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (outfd < 0) {
            printf("Could not open %s for writing.\n", filename);
            exit(1);
        }
        dup2(outfd, 1);
    } else {
        realstdout = stdout;
    }

    strategy_list[0] = ICET_STRATEGY_DIRECT;
    strategy_list[1] = ICET_STRATEGY_SEQUENTIAL;
    strategy_list[2] = ICET_STRATEGY_SPLIT;
    strategy_list[3] = ICET_STRATEGY_REDUCE;
    strategy_list[4] = ICET_STRATEGY_VTREE;

    single_image_strategy_list[0] = ICET_SINGLE_IMAGE_STRATEGY_AUTOMATIC;
    single_image_strategy_list[1] = ICET_SINGLE_IMAGE_STRATEGY_BSWAP;
    single_image_strategy_list[2] = ICET_SINGLE_IMAGE_STRATEGY_RADIXK;
    single_image_strategy_list[3] = ICET_SINGLE_IMAGE_STRATEGY_RADIXKR;
    single_image_strategy_list[4] = ICET_SINGLE_IMAGE_STRATEGY_TREE;
    single_image_strategy_list[5] = ICET_SINGLE_IMAGE_STRATEGY_BSWAP_FOLDING;
}

IceTBoolean strategy_uses_single_image_strategy(IceTEnum strategy)
{
    switch (strategy) {
      case ICET_STRATEGY_DIRECT:        return ICET_FALSE;
      case ICET_STRATEGY_SEQUENTIAL:    return ICET_TRUE;
      case ICET_STRATEGY_SPLIT:         return ICET_FALSE;
      case ICET_STRATEGY_REDUCE:        return ICET_TRUE;
      case ICET_STRATEGY_VTREE:         return ICET_FALSE;
      default:
          printrank("ERROR: unknown strategy type.");
          return ICET_TRUE;
    }
}

int run_test_base(int (*test_function)(void))
{
    int result;

    result = test_function();

    finalize_test(result);

    return result;
}

#define TEST_RESULT_TAG 3492
extern void finalize_rendering(void);
extern void finalize_communication(void);
void finalize_test(IceTInt result)
{
    IceTInt rank;
    IceTInt num_proc;

    finalize_rendering();

    checkIceTError();

    icetGetIntegerv(ICET_RANK, &rank);
    icetGetIntegerv(ICET_NUM_PROCESSES, &num_proc);

    if (rank == 0) {
        IceTInt p_id;
        for (p_id = 1; p_id < num_proc; p_id++) {
            IceTInt remote_result;
            icetCommRecv(&remote_result, 1, ICET_INT, p_id, TEST_RESULT_TAG);
            if (remote_result != TEST_PASSED) {
                result = remote_result;
            }
        }

        switch (result) {
          case TEST_PASSED:
              printf("***Test Passed***\n");
              break;
          case TEST_NOT_RUN:
              printf("***TEST NOT RUN***\n");
              break;
          case TEST_NOT_PASSED:
              printf("***TEST NOT PASSED***\n");
              break;
          case TEST_FAILED:
          default:
              printf("***TEST FAILED***\n");
              break;
        }
    } else {
        icetCommSend(&result, 1, ICET_INT, 0, TEST_RESULT_TAG);
    }

    icetDestroyContext(context);
    finalize_communication();
}
