/* -*- c -*- *******************************************************/
/*
 * Copyright (C) 2003 Sandia Corporation
 * Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 * the U.S. Government retains certain rights in this software.
 *
 * This source code is released under the New BSD License.
 */

#include <IceTDevState.h>

#include <IceT.h>
#include <IceTDevCommunication.h>
#include <IceTDevContext.h>
#include <IceTDevDiagnostics.h>
#include <IceTDevPorting.h>
#include <IceTDevStrategySelect.h>
#include <IceTDevTiming.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef DEBUG
#define ICET_STATE_CHECK_MEM
#endif

struct IceTStateValue {
    IceTEnum type;
    IceTSizeType num_entries;
    IceTSizeType buffer_size;
    void *data;
    IceTTimeStamp mod_time;
};

#ifdef ICET_STATE_CHECK_MEM
static void stateCheck(IceTEnum pname, const IceTState state);
#else
#define stateCheck(pname, state)
#endif

static IceTVoid *stateAllocate(IceTEnum pname,
                               IceTSizeType num_entries,
                               IceTEnum type,
                               IceTState state);

static void stateFree(IceTEnum pname, IceTState state);

static void stateSet(IceTEnum pname,
                     IceTSizeType num_entries,
                     IceTEnum type,
                     const IceTVoid *data);

IceTState icetStateCreate(void)
{
    IceTState state;

    state = (IceTState)malloc(sizeof(struct IceTStateValue) * ICET_STATE_SIZE);
    if (state == NULL) {
        icetRaiseError("Could not allocate memory for state.",
                       ICET_OUT_OF_MEMORY);
        return NULL;
    }

    memset(state, 0, sizeof(struct IceTStateValue) * ICET_STATE_SIZE);

    return state;
}

void icetStateDestroy(IceTState state)
{
    IceTEnum pname;

    for (pname = ICET_STATE_ENGINE_START;
         pname < ICET_STATE_ENGINE_END;
         pname++) {
        stateFree(pname, state);
    }
    free(state);
}

void icetStateCopy(IceTState dest, const IceTState src)
{
    IceTEnum pname;
    IceTSizeType type_width;
    IceTTimeStamp mod_time;

    mod_time = icetGetTimeStamp();

    for (pname = ICET_STATE_ENGINE_START;
         pname < ICET_STATE_ENGINE_END;
         pname++) {
        if (   (pname == ICET_RANK)
            || (pname == ICET_NUM_PROCESSES)
            || (pname == ICET_DATA_REPLICATION_GROUP)
            || (pname == ICET_DATA_REPLICATION_GROUP_SIZE)
            || (pname == ICET_COMPOSITE_ORDER)
            || (pname == ICET_PROCESS_ORDERS) )
        {
            continue;
        }

        type_width = icetTypeWidth(src[pname].type);

        if (type_width > 0) {
            IceTVoid *data = stateAllocate(pname,
                                           src[pname].num_entries,
                                           src[pname].type,
                                           dest);
            memcpy(data, src[pname].data, src[pname].num_entries * type_width);
        } else {
            stateFree(pname, dest);
        }
        dest[pname].mod_time = mod_time;
    }
}

static IceTFloat black[] = {0.0f, 0.0f, 0.0f, 0.0f};

void icetStateSetDefaults(void)
{
    IceTInt *int_array;
    int i;
    int comm_size, comm_rank;

    icetDiagnostics(ICET_DIAG_ALL_NODES | ICET_DIAG_WARNINGS);

    comm_size = icetCommSize();
    comm_rank = icetCommRank();
    icetStateSetInteger(ICET_RANK, comm_rank);
    icetStateSetInteger(ICET_NUM_PROCESSES, comm_size);
    /* icetStateSetInteger(ICET_ABSOLUTE_FAR_DEPTH, 1); */
  /*icetStateSetInteger(ICET_ABSOLUTE_FAR_DEPTH, 0xFFFFFFFF);*/
    icetStateSetFloatv(ICET_BACKGROUND_COLOR, 4, black);
    icetStateSetInteger(ICET_BACKGROUND_COLOR_WORD, 0);
    icetStateSetInteger(ICET_COLOR_FORMAT, ICET_IMAGE_COLOR_RGBA_UBYTE);
    icetStateSetInteger(ICET_DEPTH_FORMAT, ICET_IMAGE_DEPTH_FLOAT);

    icetResetTiles();
    icetStateSetIntegerv(ICET_DISPLAY_NODES, 0, NULL);

    icetStateSetDoublev(ICET_GEOMETRY_BOUNDS, 0, NULL);
    icetStateSetInteger(ICET_NUM_BOUNDING_VERTS, 0);
    icetStateSetInteger(ICET_STRATEGY, ICET_STRATEGY_UNDEFINED);
    icetSingleImageStrategy(ICET_SINGLE_IMAGE_STRATEGY_AUTOMATIC);
    icetCompositeMode(ICET_COMPOSITE_MODE_Z_BUFFER);
    int_array = icetStateAllocateInteger(ICET_COMPOSITE_ORDER, comm_size);
    for (i = 0; i < comm_size; i++) {
        int_array[i] = i;
    }
    int_array = icetStateAllocateInteger(ICET_PROCESS_ORDERS, comm_size);
    for (i = 0; i < comm_size; i++) {
        int_array[i] = i;
    }

    icetStateSetInteger(ICET_DATA_REPLICATION_GROUP, comm_rank);
    icetStateSetInteger(ICET_DATA_REPLICATION_GROUP_SIZE, 1);
    icetStateSetInteger(ICET_FRAME_COUNT, 0);

    if (getenv("ICET_MAGIC_K") != NULL) {
        IceTInt magic_k = atoi(getenv("ICET_MAGIC_K"));
        if (magic_k > 1) {
            icetStateSetInteger(ICET_MAGIC_K, magic_k);
        } else {
            icetRaiseError("Environment varible ICET_MAGIC_K must be set"
                           " to an integer greater than 1.",
                           ICET_INVALID_VALUE);
            icetStateSetInteger(ICET_MAGIC_K, ICET_MAGIC_K_DEFAULT);
        }
    } else {
        icetStateSetInteger(ICET_MAGIC_K, ICET_MAGIC_K_DEFAULT);
    }

    if (getenv("ICET_MAX_IMAGE_SPLIT") != NULL) {
        IceTInt max_image_split = atoi(getenv("ICET_MAX_IMAGE_SPLIT"));
        if (max_image_split > 0) {
            icetStateSetInteger(ICET_MAX_IMAGE_SPLIT, max_image_split);
        } else {
            icetRaiseError("Environment variable ICET_MAX_IMAGE_SPLIT must be"
                           " set to an integer greater than 0.",
                           ICET_INVALID_VALUE);
            icetStateSetInteger(ICET_MAX_IMAGE_SPLIT,
                                ICET_MAX_IMAGE_SPLIT_DEFAULT);
        }
    } else {
        icetStateSetInteger(ICET_MAX_IMAGE_SPLIT, ICET_MAX_IMAGE_SPLIT_DEFAULT);
    }

    icetStateSetPointer(ICET_DRAW_FUNCTION, NULL);
    icetStateSetPointer(ICET_RENDER_LAYER_DESTRUCTOR, NULL);

    icetEnable(ICET_FLOATING_VIEWPORT);
    icetDisable(ICET_ORDERED_COMPOSITE);
    icetDisable(ICET_CORRECT_COLORED_BACKGROUND);
    icetEnable(ICET_COMPOSITE_ONE_BUFFER);
    icetEnable(ICET_INTERLACE_IMAGES);
    icetEnable(ICET_COLLECT_IMAGES);
    icetDisable(ICET_RENDER_EMPTY_IMAGES);

    icetStateSetBoolean(ICET_IS_DRAWING_FRAME, 0);

    icetStateSetInteger(ICET_VALID_PIXELS_TILE, -1);
    icetStateSetInteger(ICET_VALID_PIXELS_OFFSET, 0);
    icetStateSetInteger(ICET_VALID_PIXELS_NUM, 0);

    icetStateResetTiming();
}

void icetStateCheckMemory(void)
{
#ifdef ICET_STATE_CHECK_MEM
    IceTState state = icetGetState();
    IceTEnum pname;

    for (pname = ICET_STATE_ENGINE_START;
         pname < ICET_STATE_ENGINE_END;
         pname++) {
        stateCheck(pname, state);
    }
#endif
}

void icetStateSetDoublev(IceTEnum pname,
                         IceTSizeType num_entries,
                         const IceTDouble *data)
{
    stateSet(pname, num_entries, ICET_DOUBLE, data);
}
void icetStateSetFloatv(IceTEnum pname,
                        IceTSizeType num_entries,
                        const IceTFloat *data)
{
    stateSet(pname, num_entries, ICET_FLOAT, data);
}
void icetStateSetIntegerv(IceTEnum pname,
                          IceTSizeType num_entries,
                          const IceTInt *data)
{
    stateSet(pname, num_entries, ICET_INT, data);
}
void icetStateSetBooleanv(IceTEnum pname,
                          IceTSizeType num_entries,
                          const IceTBoolean *data)
{
    stateSet(pname, num_entries, ICET_BOOLEAN, data);
}
void icetStateSetPointerv(IceTEnum pname,
                          IceTSizeType num_entries,
                          const IceTVoid **data)
{
    stateSet(pname, num_entries, ICET_POINTER, data);
}

void icetStateSetDouble(IceTEnum pname, IceTDouble value)
{
    stateSet(pname, 1, ICET_DOUBLE, &value);
}
void icetStateSetFloat(IceTEnum pname, IceTFloat value)
{
    stateSet(pname, 1, ICET_FLOAT, &value);
}
void icetStateSetInteger(IceTEnum pname, IceTInt value)
{
    stateSet(pname, 1, ICET_INT, &value);
}
void icetStateSetBoolean(IceTEnum pname, IceTBoolean value)
{
    stateSet(pname, 1, ICET_BOOLEAN, &value);
}
void icetStateSetPointer(IceTEnum pname, const IceTVoid *value)
{
    stateSet(pname, 1, ICET_POINTER, &value);
}

IceTEnum icetStateGetType(IceTEnum pname)
{
    stateCheck(pname, icetGetState());
    return icetGetState()[pname].type;
}
IceTSizeType icetStateGetNumEntries(IceTEnum pname)
{
    stateCheck(pname, icetGetState());
    return icetGetState()[pname].num_entries;
}
IceTTimeStamp icetStateGetTime(IceTEnum pname)
{
    stateCheck(pname, icetGetState());
    return icetGetState()[pname].mod_time;
}

#define copyArrayGivenCType(type_dest, array_dest, type_src, array_src, size) \
    for (i = 0; i < (size); i++) {                                      \
        ((type_dest *)(array_dest))[i]                                  \
            = (type_dest)(((type_src *)(array_src))[i]);                \
    }
#define copyArray(type_dest, array_dest, type_src, array_src, size)            \
    switch (type_src) {                                                        \
      case ICET_DOUBLE:                                                        \
          copyArrayGivenCType(type_dest,array_dest, IceTDouble,array_src, size); \
          break;                                                               \
      case ICET_FLOAT:                                                         \
          copyArrayGivenCType(type_dest,array_dest, IceTFloat,array_src, size);  \
          break;                                                               \
      case ICET_BOOLEAN:                                                       \
          copyArrayGivenCType(type_dest,array_dest, IceTBoolean,array_src, size);\
          break;                                                               \
      case ICET_INT:                                                           \
          copyArrayGivenCType(type_dest,array_dest, IceTInt,array_src, size);    \
          break;                                                               \
      case ICET_NULL:                                                          \
          {                                                                    \
              char msg[256];                                                   \
              sprintf(msg, "No such parameter, 0x%x.", (int)pname);            \
              icetRaiseError(msg, ICET_INVALID_ENUM);                          \
          }                                                                    \
          break;                                                               \
      default:                                                                 \
          {                                                                    \
              char msg[256];                                                   \
              sprintf(msg, "Could not cast value for 0x%x.", (int)pname);      \
              icetRaiseError(msg, ICET_BAD_CAST);                              \
          }                                                                    \
    }

void icetGetDoublev(IceTEnum pname, IceTDouble *params)
{
    struct IceTStateValue *value = icetGetState() + pname;
    int i;
    stateCheck(pname, icetGetState());
    copyArray(IceTDouble, params, value->type, value->data, value->num_entries);
}
void icetGetFloatv(IceTEnum pname, IceTFloat *params)
{
    struct IceTStateValue *value = icetGetState() + pname;
    int i;
    stateCheck(pname, icetGetState());
    copyArray(IceTFloat, params, value->type, value->data, value->num_entries);
}
void icetGetBooleanv(IceTEnum pname, IceTBoolean *params)
{
    struct IceTStateValue *value = icetGetState() + pname;
    int i;
    stateCheck(pname, icetGetState());
    copyArray(IceTBoolean, params, value->type, value->data,value->num_entries);
}
void icetGetIntegerv(IceTEnum pname, IceTInt *params)
{
    struct IceTStateValue *value = icetGetState() + pname;
    int i;
    stateCheck(pname, icetGetState());
    copyArray(IceTInt, params, value->type, value->data, value->num_entries);
}

void icetGetEnumv(IceTEnum pname, IceTEnum *params)
{
    struct IceTStateValue *value = icetGetState() + pname;
    int i;
    stateCheck(pname, icetGetState());
    if ((value->type == ICET_FLOAT) || (value->type == ICET_DOUBLE)) {
        icetRaiseError("Floating point values cannot be enumerations.",
                       ICET_BAD_CAST);
    }
    copyArray(IceTEnum, params, value->type, value->data, value->num_entries);
}
void icetGetBitFieldv(IceTEnum pname, IceTBitField *params)
{
    struct IceTStateValue *value = icetGetState() + pname;
    int i;
    stateCheck(pname, icetGetState());
    if ((value->type == ICET_FLOAT) || (value->type == ICET_DOUBLE)) {
        icetRaiseError("Floating point values cannot be enumerations.",
                       ICET_BAD_CAST);
    }
    copyArray(IceTBitField, params, value->type,
              value->data, value->num_entries);
}

void icetGetPointerv(IceTEnum pname, IceTVoid **params)
{
    struct IceTStateValue *value = icetGetState() + pname;
    int i;
    stateCheck(pname, icetGetState());
    if (value->type == ICET_NULL) {
        char msg[256];
        sprintf(msg, "No such parameter, 0x%x.", (int)pname);
        icetRaiseError(msg, ICET_INVALID_ENUM);
    }
    if (value->type != ICET_POINTER) {
        char msg[256];
        sprintf(msg, "Could not cast value for 0x%x.", (int)pname);
        icetRaiseError(msg, ICET_BAD_CAST);
    }
    copyArrayGivenCType(IceTVoid *, params, IceTVoid *,
                        value->data, value->num_entries);
}

void icetEnable(IceTEnum pname)
{
    if ((pname < ICET_STATE_ENABLE_START) || (pname >= ICET_STATE_ENABLE_END)) {
        icetRaiseError("Bad value to icetEnable", ICET_INVALID_VALUE);
        return;
    }
    icetStateSetBoolean(pname, ICET_TRUE);
}

void icetDisable(IceTEnum pname)
{
    if ((pname < ICET_STATE_ENABLE_START) || (pname >= ICET_STATE_ENABLE_END)) {
        icetRaiseError("Bad value to icetDisable", ICET_INVALID_VALUE);
        return;
    }
    icetStateSetBoolean(pname, ICET_FALSE);
}

IceTBoolean icetIsEnabled(IceTEnum pname)
{
    IceTBoolean isEnabled;

    if ((pname < ICET_STATE_ENABLE_START) || (pname >= ICET_STATE_ENABLE_END)) {
        icetRaiseError("Bad value to icetIsEnabled", ICET_INVALID_VALUE);
        return ICET_FALSE;
    }
    icetGetBooleanv(pname, &isEnabled);
    return isEnabled;
}

#ifdef ICET_STATE_CHECK_MEM
#define STATE_PADDING_SIZE (16)
#define STATE_DATA_WIDTH(type, num_entries) \
    (icetTypeWidth(type)*num_entries)
#define STATE_DATA_ALLOCATE(type, num_entries) \
    (STATE_DATA_WIDTH(type, num_entries) + 2*STATE_PADDING_SIZE)
#define STATE_DATA_PRE_PADDING(pname, state) \
    (((IceTByte *)state[pname].data) - STATE_PADDING_SIZE)
#define STATE_DATA_POST_PADDING(pname, state) \
    (  ((IceTByte *)state[pname].data) \
     + STATE_DATA_WIDTH(state[pname].type, state[pname].num_entries))
static const IceTByte g_pre_padding[STATE_PADDING_SIZE] = {
    0x9A, 0xBC, 0xDE, 0xF0,
    0x12, 0x34, 0x56, 0x78,
    0x29, 0x38, 0x47, 0x56,
    0xDE, 0xAD, 0xBE, 0xEF
};
static const IceTByte g_post_padding[STATE_PADDING_SIZE] = {
    0xDE, 0xAD, 0xBE, 0xEF,
    0x12, 0x34, 0x56, 0x78,
    0x9A, 0xBC, 0xDE, 0xF0,
    0x29, 0x38, 0x47, 0x56
};

static void stateCheck(IceTEnum pname, const IceTState state)
{
    if (state[pname].type != ICET_NULL) {
        if (state[pname].buffer_size > 0) {
            IceTSizeType i;
            IceTByte *padding;
            padding = STATE_DATA_PRE_PADDING(pname, state);
            for (i = 0; i < STATE_PADDING_SIZE; i++) {
                if (padding[i] != g_pre_padding[i]) {
                    char message[256];
                    sprintf(message,
                            "Lower buffer overrun detected in "
                            " state variable 0x%X",
                            pname);
                    icetRaiseError(message, ICET_SANITY_CHECK_FAIL);
                }
            }
            padding = STATE_DATA_POST_PADDING(pname, state);
            for (i = 0; i < STATE_PADDING_SIZE; i++) {
                if (padding[i] != g_post_padding[i]) {
                    char message[256];
                    sprintf(message,
                            "Upper buffer overrun detected in "
                            " state variable 0x%X",
                            pname);
                    icetRaiseError(message, ICET_SANITY_CHECK_FAIL);
                }
            }
        } else {
            if (state[pname].data != NULL) {
                char message[256];
                sprintf(message,
                        "State variable 0x%X has zero sized buffer but"
                        " non-null pointer.",
                        pname);
                icetRaiseError(message, ICET_SANITY_CHECK_FAIL);
            }
        }
    } else { /* state[pname].type == ICET_NULL */
        if (state[pname].data != NULL) {
            char message[256];
            sprintf(message,
                    "State variable 0x%X has ICET_NULL type but"
                    " non-null pointer.",
                    pname);
            icetRaiseError(message, ICET_SANITY_CHECK_FAIL);
        }
        if (state[pname].num_entries != 0) {
            char message[256];
            sprintf(message,
                    "State variable 0x%X has ICET_NULL type but"
                    " also has %d entries (!= 0).",
                    pname,
                    (int)state[pname].num_entries);
            icetRaiseError(message, ICET_SANITY_CHECK_FAIL);
        }
        if (state[pname].buffer_size != 0) {
            char message[256];
            sprintf(message,
                    "State variable 0x%X has ICET_NULL type but"
                    " also has a buffer of size %d (!= 0).",
                    pname,
                    (int)state[pname].buffer_size);
            icetRaiseError(message, ICET_SANITY_CHECK_FAIL);
        }
    }
}
#else /* ICET_STATE_CHECK_MEM */
#define STATE_DATA_WIDTH(type, num_entries) \
    (icetTypeWidth(type)*num_entries)
#define STATE_DATA_ALLOCATE(type, num_entries) \
    (STATE_DATA_WIDTH(type, num_entries))
#endif /* ICET_STATE_CHECK_MEM */

static IceTVoid *stateAllocate(IceTEnum pname,
                               IceTSizeType num_entries,
                               IceTEnum type,
                               IceTState state)
{
    stateCheck(pname, state);

    if (num_entries < 0) {
        icetRaiseError("Asked to allocate buffer of negative size",
                       ICET_SANITY_CHECK_FAIL);
    }

    if (   (num_entries == state[pname].num_entries)
        && (type == state[pname].type) ) {
        /* Current buffer already configured correctly. */
        state[pname].mod_time = icetGetTimeStamp();
    } else if ((num_entries > 0) || (state[pname].buffer_size > 0)) {
        IceTSizeType buffer_size = STATE_DATA_ALLOCATE(type, num_entries);
        if (buffer_size < state[pname].buffer_size) {
            /* Reuse larger buffer. */
            stateCheck(pname, state);
        } else {
            /* Create a new buffer. */
            IceTVoid *buffer;

            stateFree(pname, state);
            buffer = malloc(STATE_DATA_ALLOCATE(type, num_entries));
            if (buffer == NULL) {
                icetRaiseError("Could not allocate memory for state variable.",
                               ICET_OUT_OF_MEMORY);
                return NULL;
            }
#ifdef ICET_STATE_CHECK_MEM
            /* Skip past padding. */
            buffer = (IceTByte *)buffer + STATE_PADDING_SIZE;
#endif
            state[pname].buffer_size = buffer_size;
            state[pname].data = buffer;
        }

        state[pname].type = type;
        state[pname].num_entries = num_entries;
        state[pname].mod_time = icetGetTimeStamp();

#ifdef ICET_STATE_CHECK_MEM
        /* Set padding data. */
        {
            IceTSizeType i;
            IceTByte *padding;
            padding = STATE_DATA_PRE_PADDING(pname, state);
            for (i = 0; i < STATE_PADDING_SIZE; i++) {
                padding[i] = g_pre_padding[i];
            }
            padding = STATE_DATA_POST_PADDING(pname, state);
            for (i = 0; i < STATE_PADDING_SIZE; i++) {
                padding[i] = g_post_padding[i];
            }
        }
#endif
    } else { /* num_entries <= 0 */
        state[pname].type = type;
        state[pname].num_entries = 0;
        state[pname].buffer_size = 0;
        state[pname].data = NULL;
        state[pname].mod_time = icetGetTimeStamp();
    }

#ifdef ICET_STATE_CHECK_MEM
    memset(state[pname].data, 0xDC, STATE_DATA_WIDTH(type, num_entries));
#endif
    return state[pname].data;
}

static void stateFree(IceTEnum pname, IceTState state)
{
    stateCheck(pname, state);

    if ((state[pname].type != ICET_NULL) && (state[pname].buffer_size > 0)) {
#ifdef ICET_STATE_CHECK_MEM
        free(STATE_DATA_PRE_PADDING(pname, state));
#else
        free(state[pname].data);
#endif
        state[pname].type = ICET_NULL;
        state[pname].num_entries = 0;
        state[pname].buffer_size = 0;
        state[pname].data = NULL;
        state[pname].mod_time = 0;
    }
}

static void stateSet(IceTEnum pname,
                     IceTSizeType num_entries,
                     IceTEnum type,
                     const IceTVoid *data)
{
    IceTSizeType type_width = icetTypeWidth(type);
    void *datacopy = stateAllocate(pname, num_entries, type, icetGetState());

    stateCheck(pname, icetGetState());

    memcpy(datacopy, data, num_entries * type_width);

    stateCheck(pname, icetGetState());
}

static void *icetUnsafeStateGet(IceTEnum pname, IceTEnum type)
{
    stateCheck(pname, icetGetState());

    if (icetGetState()[pname].type != type) {
        icetRaiseError("Mismatched types in unsafe state get.",
                       ICET_SANITY_CHECK_FAIL);
        return NULL;
    }
    return icetGetState()[pname].data;
}

const IceTDouble *icetUnsafeStateGetDouble(IceTEnum pname)
{
    return icetUnsafeStateGet(pname, ICET_DOUBLE);
}
const IceTFloat *icetUnsafeStateGetFloat(IceTEnum pname)
{
    return icetUnsafeStateGet(pname, ICET_FLOAT);
}
const IceTInt *icetUnsafeStateGetInteger(IceTEnum pname)
{
    return icetUnsafeStateGet(pname, ICET_INT);
}
const IceTBoolean *icetUnsafeStateGetBoolean(IceTEnum pname)
{
    return icetUnsafeStateGet(pname, ICET_BOOLEAN);
}
const IceTVoid **icetUnsafeStateGetPointer(IceTEnum pname)
{
    return icetUnsafeStateGet(pname, ICET_POINTER);
}
const IceTVoid *icetUnsafeStateGetBuffer(IceTEnum pname)
{
    return icetUnsafeStateGet(pname, ICET_VOID);
}

IceTDouble *icetStateAllocateDouble(IceTEnum pname, IceTSizeType num_entries)
{
    return stateAllocate(pname, num_entries, ICET_DOUBLE, icetGetState());
}
IceTFloat *icetStateAllocateFloat(IceTEnum pname, IceTSizeType num_entries)
{
    return stateAllocate(pname, num_entries, ICET_FLOAT, icetGetState());
}
IceTInt *icetStateAllocateInteger(IceTEnum pname, IceTSizeType num_entries)
{
    return stateAllocate(pname, num_entries, ICET_INT, icetGetState());
}
IceTBoolean *icetStateAllocateBoolean(IceTEnum pname, IceTSizeType num_entries)
{
    return stateAllocate(pname, num_entries, ICET_BOOLEAN, icetGetState());
}
IceTVoid **icetStateAllocatePointer(IceTEnum pname, IceTSizeType num_entries)
{
    return stateAllocate(pname, num_entries, ICET_POINTER, icetGetState());
}

IceTVoid *icetGetStateBuffer(IceTEnum pname, IceTSizeType num_bytes)
{
  /* Check to make sure this state variable has not been used for anything
   * besides a buffer. */
    if (   (icetStateGetType(pname) != ICET_VOID)
        && (icetStateGetType(pname) != ICET_NULL) ) {
        icetRaiseWarning("A non-buffer state variable is being reallocated as"
                         " a state variable.  This is probably indicative of"
                         " mixing up state variables.",
                         ICET_SANITY_CHECK_FAIL);
    }

    return stateAllocate(pname, num_bytes, ICET_VOID, icetGetState());
}

IceTTimeStamp icetGetTimeStamp(void)
{
    static IceTTimeStamp current_time = 0;

    return current_time++;
}

void icetStateDump(void)
{
    IceTEnum pname;
    IceTState state;

    state = icetGetState();
    printf("State dump:\n");
    for (pname = ICET_STATE_ENGINE_START;
         pname < ICET_STATE_ENGINE_END;
         pname++) {
        stateCheck(pname, state);
        if (state->type != ICET_NULL) {
            printf("param       = 0x%x\n", pname);
            printf("type        = 0x%x\n", (int)state->type);
            printf("num_entries = %d\n", (int)state->num_entries);
            printf("data        = %p\n", state->data);
            printf("mod         = %d\n", (int)state->mod_time);
        }
        state++;
    }
}
