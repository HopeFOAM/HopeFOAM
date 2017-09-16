/* -*- c -*- *******************************************************/
/*
 * Copyright (C) 2003 Sandia Corporation
 * Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 * the U.S. Government retains certain rights in this software.
 *
 * This source code is released under the New BSD License.
 */

#include <IceTDevDiagnostics.h>

#include <IceT.h>

#include <IceTDevCommunication.h>
#include <IceTDevContext.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <signal.h>

#ifndef WIN32
#include <unistd.h>
#endif

static IceTEnum currentError = ICET_NO_ERROR;
static IceTEnum currentLevel;

void icetRaiseDiagnostic(const char *msg, IceTEnum type,
                         IceTBitField level, const char *file, int line)
{
    static int raisingDiagnostic = 0;
    IceTBitField diagLevel;
    IceTInt tmpInt;
    static char full_message[1024];
    char *m;
    int rank;

#define FINISH        raisingDiagnostic = 0; return

    if (raisingDiagnostic) {
        printf("PANIC: diagnostic raised while rasing diagnostic!\n");
        icetStateDump();
#ifdef DEBUG
        icetDebugBreak();
#endif
        return;
    }
    if (icetGetState() == NULL) {
        printf("PANIC: diagnostic raised when no context was current!\n");
#ifdef DEBUG
        icetDebugBreak();
#endif
        return;
    }
    raisingDiagnostic = 1;
    full_message[0] = '\0';
    m = full_message;
    if ((currentError == ICET_NO_ERROR) || (level < currentLevel)) {
        currentError = type;
        currentLevel = level;
    }
    icetGetIntegerv(ICET_DIAGNOSTIC_LEVEL, &tmpInt);
    diagLevel = tmpInt;
    if ((diagLevel & level) != level) {
      /* Don't do anything if we are not reporting the raised diagnostic. */
        FINISH;
    }

    rank = icetCommRank();
    if ((diagLevel & ICET_DIAG_ALL_NODES) != 0) {
      /* Reporting on all nodes. */
        sprintf(m, "ICET,%d:", rank);
    } else if (rank == 0) {
      /* Rank 0 is lone reporter. */
        strcpy(m, "ICET:");
    } else {
      /* Not reporting because not rank 0. */
        FINISH;
    }
    m += strlen(m);
  /* Add description of diagnostic type. */
    switch (level & 0xFF) {
      case ICET_DIAG_ERRORS:
          strcpy(m, "ERROR:");
          break;
      case ICET_DIAG_WARNINGS:
          strcpy(m, "WARNING:");
          break;
      case ICET_DIAG_DEBUG:
          strcpy(m, "DEBUG:");
          break;
    }
    m += strlen(m);
#ifdef DEBUG
    sprintf(m, "%s:%d:", file, line);
    m += strlen(m);
#else
    /* shut up warnings */
    (void)file;
    (void)line;
#endif

    sprintf(m, " %s\n", msg);
    printf("%s", full_message);
    fflush(stdout);

#ifdef DEBUG
    if ((level & 0xFF) == ICET_DIAG_ERRORS) {
        icetDebugBreak();
    }
#endif

    FINISH;
}

IceTEnum icetGetError(void)
{
    IceTEnum error = currentError;
    currentError = ICET_NO_ERROR;
    return error;
}

void icetDiagnostics(IceTBitField mask)
{
    icetStateSetInteger(ICET_DIAGNOSTIC_LEVEL, mask);
}

void icetDebugBreak(void)
{
#if 0
    printf("Waiting for debugger in process %d\n", getpid());
    sleep(100);
#endif
    raise(SIGSEGV);
}
