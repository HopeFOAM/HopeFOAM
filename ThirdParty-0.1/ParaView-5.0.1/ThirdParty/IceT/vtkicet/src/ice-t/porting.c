/* -*- c -*- *******************************************************/
/*
 * Copyright (C) 2003 Sandia Corporation
 * Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 * the U.S. Government retains certain rights in this software.
 *
 * This source code is released under the New BSD License.
 */

#include <IceTDevPorting.h>

#include <IceT.h>

#include <IceTDevDiagnostics.h>

#ifndef WIN32
#include <sys/time.h>
#else
#include <windows.h>
#include <winbase.h>
#endif

#ifndef WIN32
double icetWallTime(void)
{
    static struct timeval start = { 0, 0 };
    struct timeval now;
    struct timeval *tp;

  /* Make the first call to icetWallTime happen at second 0.  This should
     allow for more significant bits in the microseconds. */
    if (start.tv_sec == 0) {
        tp = &start;
    } else {
        tp = &now;
    }

    gettimeofday(tp, NULL);

    return (tp->tv_sec - start.tv_sec) + 0.000001*(double)tp->tv_usec;
}
#else /*WIN32*/
double icetWallTime(void)
{
    static DWORD start = 0;

    if (start == 0) {
        start = GetTickCount();
        return 0.0;
    } else {
        DWORD now = GetTickCount();
        return 0.001*(now-start);
    }
}
#endif /*WIN32*/

IceTInt icetTypeWidth(IceTEnum type)
{
    switch (type) {
      case ICET_BOOLEAN:
          return sizeof(IceTBoolean);
      case ICET_BYTE:
          return sizeof(IceTByte);
      case ICET_SHORT:
          return sizeof(IceTShort);
      case ICET_INT:
          return sizeof(IceTInt);
      case ICET_FLOAT:
          return sizeof(IceTFloat);
      case ICET_DOUBLE:
          return sizeof(IceTDouble);
      case ICET_POINTER:
          return sizeof(IceTVoid *);
      case ICET_VOID:
          return 1;
      case ICET_NULL:
          return 0;
      default:
          icetRaiseError("Bad type identifier.", ICET_INVALID_VALUE);
    }

    return 0;
}
