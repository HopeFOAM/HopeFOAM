/* -*- c -*- *******************************************************/
/*
 * Copyright (C) 2011 Sandia Corporation
 * Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 * the U.S. Government retains certain rights in this software.
 *
 * This source code is released under the New BSD License.
 */

#include <IceTDevTiming.h>

#include <IceTDevDiagnostics.h>
#include <IceTDevState.h>

#include <stdlib.h>
#include <stdio.h>

#ifdef ICET_USE_MPE
#include <mpe_log.h>

static void icetEventBegin(IceTEnum pname);
static void icetEventEnd(IceTEnum pname);

#else

#define icetEventBegin(pname)
#define icetEventEnd(pname)

#endif

void icetStateResetTiming(void)
{
    icetStateSetDouble(ICET_RENDER_TIME, 0.0);
    icetStateSetDouble(ICET_BUFFER_READ_TIME, 0.0);
    icetStateSetDouble(ICET_BUFFER_WRITE_TIME, 0.0);
    icetStateSetDouble(ICET_COMPRESS_TIME, 0.0);
    icetStateSetDouble(ICET_INTERLACE_TIME, 0.0);
    icetStateSetDouble(ICET_BLEND_TIME, 0.0);
    icetStateSetDouble(ICET_COMPOSITE_TIME, 0.0);
    icetStateSetDouble(ICET_COLLECT_TIME, 0.0);
    icetStateSetDouble(ICET_TOTAL_DRAW_TIME, 0.0);

    icetStateSetInteger(ICET_DRAW_TIME_ID, 0);
    icetStateSetInteger(ICET_SUBFUNC_TIME_ID, 0);

    icetStateSetInteger(ICET_BYTES_SENT, 0);
}

static void icetTimingBegin(IceTEnum start_pname,
                            IceTEnum id_pname,
                            IceTEnum result_pname,
                            const char *name)
{
    icetRaiseDebug1("Beginning %s", name);
    (void)name;

    icetEventBegin(result_pname);

    {
        IceTInt current_id;
        icetGetIntegerv(id_pname, &current_id);
        if (current_id != 0) {
            char message[256];
            sprintf(message,
                    "Called start for timer 0x%x,"
                    " but end never called for timer 0x%x",
                    result_pname,
                    current_id);
            icetRaiseError(message, ICET_SANITY_CHECK_FAIL);
        }
    }

    icetStateSetInteger(id_pname, result_pname);
    icetStateSetDouble(start_pname, icetWallTime());
}

static void icetTimingEnd(IceTEnum start_pname,
                          IceTEnum id_pname,
                          IceTEnum result_pname,
                          const char *name)
{
    icetRaiseDebug1("Ending %s", name);
    (void)name;

    icetEventEnd(result_pname);

    {
        IceTInt current_id;
        icetGetIntegerv(id_pname, &current_id);
        if ((IceTEnum)current_id != result_pname) {
            char message[256];
            sprintf(message,
                    "Started timer 0x%x, but ended timer 0x%x",
                    result_pname,
                    current_id);
            icetRaiseError(message, ICET_SANITY_CHECK_FAIL);
        }
    }

    icetStateSetInteger(id_pname, 0);

    {
        IceTDouble start_time;
        IceTDouble old_time;
        icetGetDoublev(start_pname, &start_time);
        icetGetDoublev(result_pname, &old_time);
        icetStateSetDouble(result_pname,
                           old_time + (icetWallTime() - start_time));
    }
}

void icetTimingRenderBegin(void)
{
    icetTimingBegin(ICET_SUBFUNC_START_TIME,
                    ICET_SUBFUNC_TIME_ID,
                    ICET_RENDER_TIME,
                    "render");
}
void icetTimingRenderEnd(void)
{
    icetTimingEnd(ICET_SUBFUNC_START_TIME,
                  ICET_SUBFUNC_TIME_ID,
                  ICET_RENDER_TIME,
                  "render");
}

void icetTimingBufferReadBegin(void)
{
    icetTimingBegin(ICET_SUBFUNC_START_TIME,
                    ICET_SUBFUNC_TIME_ID,
                    ICET_BUFFER_READ_TIME,
                    "buffer read");
}
void icetTimingBufferReadEnd(void)
{
    icetTimingEnd(ICET_SUBFUNC_START_TIME,
                  ICET_SUBFUNC_TIME_ID,
                  ICET_BUFFER_READ_TIME,
                  "buffer read");
}

void icetTimingBufferWriteBegin(void)
{
    icetTimingBegin(ICET_SUBFUNC_START_TIME,
                    ICET_SUBFUNC_TIME_ID,
                    ICET_BUFFER_WRITE_TIME,
                    "buffer write");
}
void icetTimingBufferWriteEnd(void)
{
    icetTimingEnd(ICET_SUBFUNC_START_TIME,
                  ICET_SUBFUNC_TIME_ID,
                  ICET_BUFFER_WRITE_TIME,
                  "buffer write");
}

void icetTimingCompressBegin(void)
{
    icetTimingBegin(ICET_SUBFUNC_START_TIME,
                    ICET_SUBFUNC_TIME_ID,
                    ICET_COMPRESS_TIME,
                    "compress");
}
void icetTimingCompressEnd(void)
{
    icetTimingEnd(ICET_SUBFUNC_START_TIME,
                  ICET_SUBFUNC_TIME_ID,
                  ICET_COMPRESS_TIME,
                  "compress");
}

void icetTimingInterlaceBegin(void)
{
    icetTimingBegin(ICET_SUBFUNC_START_TIME,
                    ICET_SUBFUNC_TIME_ID,
                    ICET_INTERLACE_TIME,
                    "interlace");
}
void icetTimingInterlaceEnd(void)
{
    icetTimingEnd(ICET_SUBFUNC_START_TIME,
                  ICET_SUBFUNC_TIME_ID,
                  ICET_INTERLACE_TIME,
                  "interlace");
}

void icetTimingBlendBegin(void)
{
    icetTimingBegin(ICET_SUBFUNC_START_TIME,
                    ICET_SUBFUNC_TIME_ID,
                    ICET_BLEND_TIME,
                    "blend");
}
void icetTimingBlendEnd(void)
{
    icetTimingEnd(ICET_SUBFUNC_START_TIME,
                  ICET_SUBFUNC_TIME_ID,
                  ICET_BLEND_TIME,
                  "blend");
}

void icetTimingCollectBegin(void)
{
    icetTimingBegin(ICET_SUBFUNC_START_TIME,
                    ICET_SUBFUNC_TIME_ID,
                    ICET_COLLECT_TIME,
                    "collect");
}

void icetTimingCollectEnd(void)
{
    icetTimingEnd(ICET_SUBFUNC_START_TIME,
                  ICET_SUBFUNC_TIME_ID,
                  ICET_COLLECT_TIME,
                  "collect");
}

void icetTimingDrawFrameBegin(void)
{
    icetTimingBegin(ICET_DRAW_START_TIME,
                    ICET_DRAW_TIME_ID,
                    ICET_TOTAL_DRAW_TIME,
                    "draw frame");
}
void icetTimingDrawFrameEnd(void)
{
    icetTimingEnd(ICET_DRAW_START_TIME,
                  ICET_DRAW_TIME_ID,
                  ICET_TOTAL_DRAW_TIME,
                  "draw frame");
}

#ifdef ICET_USE_MPE

typedef struct IceTEventInfoStruct {
    IceTEnum pname;
    int mpe_event_begin;
    int mpe_event_end;
    struct IceTEventInfoStruct *next;
} IceTEventInfo;

static const IceTEventInfo *icetEventFind(IceTEnum pname)
{
    static IceTEventInfo *events = NULL;

    if (events == NULL) {
        IceTEventInfo *new_event;

        new_event = malloc(sizeof(IceTEventInfo));
        new_event->pname = ICET_RENDER_TIME;
        MPE_Log_get_state_eventIDs(&new_event->mpe_event_begin,
                                   &new_event->mpe_event_end);
        MPE_Describe_state(new_event->mpe_event_begin,
                           new_event->mpe_event_end,
                           "render",
                           "lawn green");
        new_event->next = events;
        events = new_event;

        new_event = malloc(sizeof(IceTEventInfo));
        new_event->pname = ICET_BUFFER_READ_TIME;
        MPE_Log_get_state_eventIDs(&new_event->mpe_event_begin,
                                   &new_event->mpe_event_end);
        MPE_Describe_state(new_event->mpe_event_begin,
                           new_event->mpe_event_end,
                           "buffer read",
                           "dark green");
        new_event->next = events;
        events = new_event;

        new_event = malloc(sizeof(IceTEventInfo));
        new_event->pname = ICET_BUFFER_WRITE_TIME;
        MPE_Log_get_state_eventIDs(&new_event->mpe_event_begin,
                                   &new_event->mpe_event_end);
        MPE_Describe_state(new_event->mpe_event_begin,
                           new_event->mpe_event_end,
                           "buffer write",
                           "dark olive green");
        new_event->next = events;
        events = new_event;

        new_event = malloc(sizeof(IceTEventInfo));
        new_event->pname = ICET_COMPRESS_TIME;
        MPE_Log_get_state_eventIDs(&new_event->mpe_event_begin,
                                   &new_event->mpe_event_end);
        MPE_Describe_state(new_event->mpe_event_begin,
                           new_event->mpe_event_end,
                           "compress/decompress",
                           "cadet blue");
        new_event->next = events;
        events = new_event;

        new_event = malloc(sizeof(IceTEventInfo));
        new_event->pname = ICET_BLEND_TIME;
        MPE_Log_get_state_eventIDs(&new_event->mpe_event_begin,
                                   &new_event->mpe_event_end);
        MPE_Describe_state(new_event->mpe_event_begin,
                           new_event->mpe_event_end,
                           "blend",
                           "cyan");
        new_event->next = events;
        events = new_event;

        new_event = malloc(sizeof(IceTEventInfo));
        new_event->pname = ICET_COLLECT_TIME;
        MPE_Log_get_state_eventIDs(&new_event->mpe_event_begin,
                                   &new_event->mpe_event_end);
        MPE_Describe_state(new_event->mpe_event_begin,
                           new_event->mpe_event_end,
                           "collect",
                           "royal blue");
        new_event->next = events;
        events = new_event;

        new_event = malloc(sizeof(IceTEventInfo));
        new_event->pname = ICET_TOTAL_DRAW_TIME;
        MPE_Log_get_state_eventIDs(&new_event->mpe_event_begin,
                                   &new_event->mpe_event_end);
        MPE_Describe_state(new_event->mpe_event_begin,
                           new_event->mpe_event_end,
                           "draw frame",
                           "blue");
        new_event->next = events;
        events = new_event;
    }

    {
        IceTEventInfo *e = events;
        while ((e != NULL) && (e->pname != pname)) {
            e = e->next;
        }
        return e;
    }
}

static void icetEventBegin(IceTEnum pname)
{
    const IceTEventInfo *event = icetEventFind(pname);

    if (event) {
        MPE_Log_event(event->mpe_event_begin, 0, NULL);
    }
}

static void icetEventEnd(IceTEnum pname)
{
    const IceTEventInfo *event = icetEventFind(pname);

    if (event) {
        MPE_Log_event(event->mpe_event_end, 0, NULL);
    }
}

#endif /*ICET_USE_MPE*/
