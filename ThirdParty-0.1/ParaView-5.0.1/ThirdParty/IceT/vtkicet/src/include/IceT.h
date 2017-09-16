/* -*- c -*- *******************************************************/
/*
 * Copyright (C) 2003 Sandia Corporation
 * Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 * the U.S. Government retains certain rights in this software.
 *
 * This source code is released under the New BSD License.
 */

#ifndef __IceT_h
#define __IceT_h

#include <IceTConfig.h>

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

typedef IceTUnsignedInt32       IceTEnum;
typedef IceTUnsignedInt32       IceTBitField;
typedef IceTFloat64             IceTDouble;
typedef IceTFloat32             IceTFloat;
typedef IceTInt32               IceTInt;
typedef IceTUnsignedInt32       IceTUInt;
typedef IceTInt16               IceTShort;
typedef IceTUnsignedInt16       IceTUShort;
typedef IceTInt8                IceTByte;
typedef IceTUnsignedInt8        IceTUByte;
typedef IceTUnsignedInt8        IceTBoolean;
typedef void                    IceTVoid;
typedef IceTInt32               IceTSizeType;

struct IceTContextStruct;
typedef struct IceTContextStruct *IceTContext;

struct IceTCommunicatorStruct;

typedef struct IceTCommRequestStruct {
    IceTEnum magic_number;
    IceTVoid *internals;
} *IceTCommRequest;
#define ICET_COMM_REQUEST_NULL ((IceTCommRequest)NULL)

struct IceTCommunicatorStruct {
    struct IceTCommunicatorStruct *
         (*Duplicate)(struct IceTCommunicatorStruct *self);
    void (*Destroy)(struct IceTCommunicatorStruct *self);
    struct IceTCommunicatorStruct *
         (*Subset)(struct IceTCommunicatorStruct *self,
                   int count,
                   const IceTInt32 *ranks);
    void (*Barrier)(struct IceTCommunicatorStruct *self);
    void (*Send)(struct IceTCommunicatorStruct *self,
                 const void *buf,
                 int count,
                 IceTEnum datatype,
                 int dest,
                 int tag);
    void (*Recv)(struct IceTCommunicatorStruct *self,
                 void *buf,
                 int count,
                 IceTEnum datatype,
                 int src,
                 int tag);

    void (*Sendrecv)(struct IceTCommunicatorStruct *self,
                     const void *sendbuf,
                     int sendcount,
                     IceTEnum sendtype,
                     int dest,
                     int sendtag,
                     void *recvbuf,
                     int recvcount,
                     IceTEnum recvtype,
                     int src,
                     int recvtag);
    void (*Gather)(struct IceTCommunicatorStruct *self,
                   const void *sendbuf,
                   int sendcount,
                   IceTEnum datatype,
                   void *recvbuf,
                   int root);
    void (*Gatherv)(struct IceTCommunicatorStruct *self,
                    const void *sendbuf,
                    int sendcount,
                    IceTEnum datatype,
                    void *recvbuf,
                    const int *recvcounts,
                    const int *recvoffsets,
                    int root);
    void (*Allgather)(struct IceTCommunicatorStruct *self,
                      const void *sendbuf,
                      int sendcount,
                      IceTEnum datatype,
                      void *recvbuf);
    void (*Alltoall)(struct IceTCommunicatorStruct *self,
                     const void *sendbuf,
                     int sendcount,
                     IceTEnum datatype,
                     void *recvbuf);

    IceTCommRequest (*Isend)(struct IceTCommunicatorStruct *self,
                             const void *buf,
                             int count,
                             IceTEnum datatype,
                             int dest,
                             int tag);
    IceTCommRequest (*Irecv)(struct IceTCommunicatorStruct *self,
                             void *buf,
                             int count,
                             IceTEnum datatype,
                             int src,
                             int tag);

    void (*Wait)(struct IceTCommunicatorStruct *self, IceTCommRequest *request);
    int  (*Waitany)(struct IceTCommunicatorStruct *self,
                    int count, IceTCommRequest *array_of_requests);

    int  (*Comm_size)(struct IceTCommunicatorStruct *self);
    int  (*Comm_rank)(struct IceTCommunicatorStruct *self);
    void *data;
};

typedef struct IceTCommunicatorStruct *IceTCommunicator;
#define ICET_COMM_NULL ((IceTCommunicator)NULL)

ICET_EXPORT IceTDouble  icetWallTime(void);

ICET_EXPORT IceTContext icetCreateContext(IceTCommunicator comm);
ICET_EXPORT void        icetDestroyContext(IceTContext context);
ICET_EXPORT IceTContext icetGetContext(void);
ICET_EXPORT void        icetSetContext(IceTContext context);
ICET_EXPORT void        icetCopyState(IceTContext dest, const IceTContext src);

#define ICET_BOOLEAN    (IceTEnum)0x8000
#define ICET_BYTE       (IceTEnum)0x8001
#define ICET_SHORT      (IceTEnum)0x8002
#define ICET_INT        (IceTEnum)0x8003
#define ICET_FLOAT      (IceTEnum)0x8004
#define ICET_DOUBLE     (IceTEnum)0x8005
#define ICET_SIZE_TYPE  ICET_INT
#define ICET_POINTER    (IceTEnum)0x8008
#define ICET_VOID       (IceTEnum)0x800F
#define ICET_NULL       (IceTEnum)0x0000

#define ICET_FALSE      0
#define ICET_TRUE       1

ICET_EXPORT void icetBoundingVertices(IceTInt size, IceTEnum type,
                                      IceTSizeType stride, IceTSizeType count,
                                      const IceTVoid *pointer);
ICET_EXPORT void icetBoundingBoxd(IceTDouble x_min, IceTDouble x_max,
                                  IceTDouble y_min, IceTDouble y_max,
                                  IceTDouble z_min, IceTDouble z_max);
ICET_EXPORT void icetBoundingBoxf(IceTFloat x_min, IceTFloat x_max,
                                  IceTFloat y_min, IceTFloat y_max,
                                  IceTFloat z_min, IceTFloat z_max);

ICET_EXPORT void icetResetTiles(void);
ICET_EXPORT int  icetAddTile(IceTInt x, IceTInt y,
                             IceTSizeType width, IceTSizeType height,
                             int display_rank);

ICET_EXPORT void icetPhysicalRenderSize(IceTInt width, IceTInt height);

typedef struct { IceTVoid *opaque_internals; } IceTImage;

#define ICET_IMAGE_COLOR_RGBA_UBYTE     (IceTEnum)0xC001
#define ICET_IMAGE_COLOR_RGBA_FLOAT     (IceTEnum)0xC002
#define ICET_IMAGE_COLOR_NONE           (IceTEnum)0xC000

#define ICET_IMAGE_DEPTH_FLOAT          (IceTEnum)0xD001
#define ICET_IMAGE_DEPTH_NONE           (IceTEnum)0xD000

ICET_EXPORT void icetSetColorFormat(IceTEnum color_format);
ICET_EXPORT void icetSetDepthFormat(IceTEnum depth_format);

ICET_EXPORT IceTImage icetImageNull(void);
ICET_EXPORT IceTBoolean icetImageIsNull(const IceTImage image);
ICET_EXPORT IceTEnum icetImageGetColorFormat(const IceTImage image);
ICET_EXPORT IceTEnum icetImageGetDepthFormat(const IceTImage image);
ICET_EXPORT IceTSizeType icetImageGetWidth(const IceTImage image);
ICET_EXPORT IceTSizeType icetImageGetHeight(const IceTImage image);
ICET_EXPORT IceTSizeType icetImageGetNumPixels(const IceTImage image);
ICET_EXPORT IceTUByte *icetImageGetColorub(IceTImage image);
ICET_EXPORT IceTUInt *icetImageGetColorui(IceTImage image);
ICET_EXPORT IceTFloat *icetImageGetColorf(IceTImage image);
ICET_EXPORT IceTFloat *icetImageGetDepthf(IceTImage image);
ICET_EXPORT const IceTUByte *icetImageGetColorcub(const IceTImage image);
ICET_EXPORT const IceTUInt *icetImageGetColorcui(const IceTImage image);
ICET_EXPORT const IceTFloat *icetImageGetColorcf(const IceTImage image);
ICET_EXPORT const IceTFloat *icetImageGetDepthcf(const IceTImage image);
ICET_EXPORT void icetImageCopyColorub(const IceTImage image,
                                      IceTUByte *color_buffer,
                                      IceTEnum color_format);
ICET_EXPORT void icetImageCopyColorf(const IceTImage image,
                                     IceTFloat *color_buffer,
                                     IceTEnum color_format);
ICET_EXPORT void icetImageCopyDepthf(const IceTImage image,
                                     IceTFloat *depth_buffer,
                                     IceTEnum depth_format);

#define ICET_STRATEGY_DIRECT            (IceTEnum)0x6001
#define ICET_STRATEGY_SEQUENTIAL        (IceTEnum)0x6002
#define ICET_STRATEGY_SPLIT             (IceTEnum)0x6003
#define ICET_STRATEGY_REDUCE            (IceTEnum)0x6004
#define ICET_STRATEGY_VTREE             (IceTEnum)0x6005

ICET_EXPORT void icetStrategy(IceTEnum strategy);

ICET_EXPORT const char *icetGetStrategyName(void);

#define ICET_SINGLE_IMAGE_STRATEGY_AUTOMATIC    (IceTEnum)0x7001
#define ICET_SINGLE_IMAGE_STRATEGY_BSWAP        (IceTEnum)0x7002
#define ICET_SINGLE_IMAGE_STRATEGY_TREE         (IceTEnum)0x7003
#define ICET_SINGLE_IMAGE_STRATEGY_RADIXK       (IceTEnum)0x7004
#define ICET_SINGLE_IMAGE_STRATEGY_RADIXKR      (IceTEnum)0x7005
#define ICET_SINGLE_IMAGE_STRATEGY_BSWAP_FOLDING (IceTEnum)0x7006

ICET_EXPORT void icetSingleImageStrategy(IceTEnum strategy);

ICET_EXPORT const char *icetGetSingleImageStrategyName(void);

#define ICET_COMPOSITE_MODE_Z_BUFFER    (IceTEnum)0x0301
#define ICET_COMPOSITE_MODE_BLEND       (IceTEnum)0x0302
ICET_EXPORT void icetCompositeMode(IceTEnum mode);

ICET_EXPORT void icetCompositeOrder(const IceTInt *process_ranks);

ICET_EXPORT void icetDataReplicationGroup(IceTInt size,
                                          const IceTInt *processes);
ICET_EXPORT void icetDataReplicationGroupColor(IceTInt color);

typedef void (*IceTDrawCallbackType)(const IceTDouble *projection_matrix,
                                     const IceTDouble *modelview_matrix,
                                     const IceTFloat *background_color,
                                     const IceTInt *readback_viewport,
                                     IceTImage result);

ICET_EXPORT void icetDrawCallback(IceTDrawCallbackType callback);

ICET_EXPORT IceTImage icetDrawFrame(const IceTDouble *projection_matrix,
                                    const IceTDouble *modelview_matrix,
                                    const IceTFloat *background_color);

ICET_EXPORT IceTImage icetCompositeImage(const IceTVoid *color_buffer,
                                         const IceTVoid *depth_buffer,
                                         const IceTInt *valid_pixels_viewport,
                                         const IceTDouble *projection_matrix,
                                         const IceTDouble *modelview_matrix,
                                         const IceTFloat *background_color);

#define ICET_DIAG_OFF           (IceTEnum)0x0000
#define ICET_DIAG_ERRORS        (IceTEnum)0x0001
#define ICET_DIAG_WARNINGS      (IceTEnum)0x0003
#define ICET_DIAG_DEBUG         (IceTEnum)0x0007
#define ICET_DIAG_ROOT_NODE     (IceTEnum)0x0000
#define ICET_DIAG_ALL_NODES     (IceTEnum)0x0100
#define ICET_DIAG_FULL          (IceTEnum)0xFFFF

ICET_EXPORT void icetDiagnostics(IceTBitField mask);


#define ICET_STATE_ENGINE_START (IceTEnum)0x00000000

#define ICET_DIAGNOSTIC_LEVEL   (ICET_STATE_ENGINE_START | (IceTEnum)0x0001)
#define ICET_RANK               (ICET_STATE_ENGINE_START | (IceTEnum)0x0002)
#define ICET_NUM_PROCESSES      (ICET_STATE_ENGINE_START | (IceTEnum)0x0003)
/* #define ICET_ABSOLUTE_FAR_DEPTH (ICET_STATE_ENGINE_START | (IceTEnum)0x0004) */
#define ICET_BACKGROUND_COLOR   (ICET_STATE_ENGINE_START | (IceTEnum)0x0005)
#define ICET_BACKGROUND_COLOR_WORD (ICET_STATE_ENGINE_START | (IceTEnum)0x0006)
#define ICET_PHYSICAL_RENDER_WIDTH (ICET_STATE_ENGINE_START | (IceTEnum)0x0007)
#define ICET_PHYSICAL_RENDER_HEIGHT (ICET_STATE_ENGINE_START| (IceTEnum)0x0008)
#define ICET_COLOR_FORMAT       (ICET_STATE_ENGINE_START | (IceTEnum)0x0009)
#define ICET_DEPTH_FORMAT       (ICET_STATE_ENGINE_START | (IceTEnum)0x000A)

#define ICET_NUM_TILES          (ICET_STATE_ENGINE_START | (IceTEnum)0x0010)
#define ICET_TILE_VIEWPORTS     (ICET_STATE_ENGINE_START | (IceTEnum)0x0011)
#define ICET_GLOBAL_VIEWPORT    (ICET_STATE_ENGINE_START | (IceTEnum)0x0012)
#define ICET_TILE_MAX_WIDTH     (ICET_STATE_ENGINE_START | (IceTEnum)0x0013)
#define ICET_TILE_MAX_HEIGHT    (ICET_STATE_ENGINE_START | (IceTEnum)0x0014)
#define ICET_DISPLAY_NODES      (ICET_STATE_ENGINE_START | (IceTEnum)0x001A)
#define ICET_TILE_DISPLAYED     (ICET_STATE_ENGINE_START | (IceTEnum)0x001B)

#define ICET_GEOMETRY_BOUNDS    (ICET_STATE_ENGINE_START | (IceTEnum)0x0022)
#define ICET_NUM_BOUNDING_VERTS (ICET_STATE_ENGINE_START | (IceTEnum)0x0023)
#define ICET_STRATEGY           (ICET_STATE_ENGINE_START | (IceTEnum)0x0024)
#define ICET_SINGLE_IMAGE_STRATEGY (ICET_STATE_ENGINE_START | (IceTEnum)0x0025)
#define ICET_COMPOSITE_MODE     (ICET_STATE_ENGINE_START | (IceTEnum)0x0028)
#define ICET_COMPOSITE_ORDER    (ICET_STATE_ENGINE_START | (IceTEnum)0x0029)
#define ICET_PROCESS_ORDERS     (ICET_STATE_ENGINE_START | (IceTEnum)0x002A)
#define ICET_STRATEGY_SUPPORTS_ORDERING (ICET_STATE_ENGINE_START | (IceTEnum)0x002B)
#define ICET_DATA_REPLICATION_GROUP (ICET_STATE_ENGINE_START | (IceTEnum)0x002C)
#define ICET_DATA_REPLICATION_GROUP_SIZE (ICET_STATE_ENGINE_START | (IceTEnum)0x002D)
#define ICET_FRAME_COUNT        (ICET_STATE_ENGINE_START | (IceTEnum)0x002E)

#define ICET_MAGIC_K            (ICET_STATE_ENGINE_START | (IceTEnum)0x0040)
#define ICET_MAX_IMAGE_SPLIT    (ICET_STATE_ENGINE_START | (IceTEnum)0x0041)

#define ICET_DRAW_FUNCTION      (ICET_STATE_ENGINE_START | (IceTEnum)0x0060)
#define ICET_RENDER_LAYER_DESTRUCTOR (ICET_STATE_ENGINE_START|(IceTEnum)0x0061)

#define ICET_STATE_FRAME_START  (IceTEnum)0x00000080

#define ICET_IS_DRAWING_FRAME   (ICET_STATE_FRAME_START | (IceTEnum)0x0000)
#define ICET_PROJECTION_MATRIX  (ICET_STATE_FRAME_START | (IceTEnum)0x0001)
#define ICET_MODELVIEW_MATRIX   (ICET_STATE_FRAME_START | (IceTEnum)0x0002)
#define ICET_CONTAINED_VIEWPORT (ICET_STATE_FRAME_START | (IceTEnum)0x0003)
#define ICET_NEAR_DEPTH         (ICET_STATE_FRAME_START | (IceTEnum)0x0004)
#define ICET_FAR_DEPTH          (ICET_STATE_FRAME_START | (IceTEnum)0x0005)
#define ICET_NUM_CONTAINED_TILES (ICET_STATE_FRAME_START| (IceTEnum)0x0006)
#define ICET_CONTAINED_TILES_LIST (ICET_STATE_FRAME_START|(IceTEnum)0x0007)
#define ICET_CONTAINED_TILES_MASK (ICET_STATE_FRAME_START|(IceTEnum)0x0008)
#define ICET_ALL_CONTAINED_TILES_MASKS (ICET_STATE_FRAME_START|(IceTEnum)0x0009)
#define ICET_TILE_CONTRIB_COUNTS (ICET_STATE_FRAME_START| (IceTEnum)0x000A)
#define ICET_TOTAL_IMAGE_COUNT  (ICET_STATE_FRAME_START | (IceTEnum)0x000B)
#define ICET_NEED_BACKGROUND_CORRECTION (ICET_STATE_FRAME_START | (IceTEnum)0x000C)
#define ICET_TRUE_BACKGROUND_COLOR (ICET_STATE_FRAME_START | (IceTEnum)0x000D)
#define ICET_TRUE_BACKGROUND_COLOR_WORD (ICET_STATE_FRAME_START | (IceTEnum)0x000E)

#define ICET_VALID_PIXELS_TILE  (ICET_STATE_FRAME_START | (IceTEnum)0x0018)
#define ICET_VALID_PIXELS_OFFSET (ICET_STATE_FRAME_START | (IceTEnum)0x0019)
#define ICET_VALID_PIXELS_NUM   (ICET_STATE_FRAME_START | (IceTEnum)0x001A)

#define ICET_RENDERED_VIEWPORT  (ICET_STATE_FRAME_START | (IceTEnum)0x0020)
#define ICET_RENDER_BUFFER      (ICET_STATE_FRAME_START | (IceTEnum)0x0021)
#define ICET_PRE_RENDERED       (ICET_STATE_FRAME_START | (IceTEnum)0x0022)
#define ICET_TILE_PROJECTIONS   (ICET_STATE_FRAME_START | (IceTEnum)0x0023)

#define ICET_STATE_TIMING_START (IceTEnum)0x000000C0

#define ICET_RENDER_TIME        (ICET_STATE_TIMING_START | (IceTEnum)0x0001)
#define ICET_BUFFER_READ_TIME   (ICET_STATE_TIMING_START | (IceTEnum)0x0002)
#define ICET_BUFFER_WRITE_TIME  (ICET_STATE_TIMING_START | (IceTEnum)0x0003)
#define ICET_COMPRESS_TIME      (ICET_STATE_TIMING_START | (IceTEnum)0x0004)
#define ICET_INTERLACE_TIME     (ICET_STATE_TIMING_START | (IceTEnum)0x0005)
#define ICET_COMPARE_TIME       (ICET_STATE_TIMING_START | (IceTEnum)0x0006)
#define ICET_BLEND_TIME         ICET_COMPARE_TIME
#define ICET_COMPOSITE_TIME     (ICET_STATE_TIMING_START | (IceTEnum)0x0007)
#define ICET_COLLECT_TIME       (ICET_STATE_TIMING_START | (IceTEnum)0x0008)
#define ICET_TOTAL_DRAW_TIME    (ICET_STATE_TIMING_START | (IceTEnum)0x0009)
#define ICET_BYTES_SENT         (ICET_STATE_TIMING_START | (IceTEnum)0x000A)

#define ICET_DRAW_START_TIME    (ICET_STATE_TIMING_START | (IceTEnum)0x0010)
#define ICET_DRAW_TIME_ID       (ICET_STATE_TIMING_START | (IceTEnum)0x0011)
#define ICET_SUBFUNC_START_TIME (ICET_STATE_TIMING_START | (IceTEnum)0x0012)
#define ICET_SUBFUNC_TIME_ID    (ICET_STATE_TIMING_START | (IceTEnum)0x0013)

/* This set of state variables are reserved for the rendering layer. */
#define ICET_RENDER_LAYER_STATE_START (IceTEnum)0x00000100
#define ICET_RENDER_LAYER_STATE_END   (IceTEnum)0x00000140

#define ICET_STATE_ENABLE_START (IceTEnum)0x00000140
#define ICET_STATE_ENABLE_END   (IceTEnum)0x00000180

#define ICET_FLOATING_VIEWPORT  (ICET_STATE_ENABLE_START | (IceTEnum)0x0001)
#define ICET_ORDERED_COMPOSITE  (ICET_STATE_ENABLE_START | (IceTEnum)0x0002)
#define ICET_CORRECT_COLORED_BACKGROUND (ICET_STATE_ENABLE_START | (IceTEnum)0x0003)
#define ICET_COMPOSITE_ONE_BUFFER (ICET_STATE_ENABLE_START | (IceTEnum)0x0004)
#define ICET_INTERLACE_IMAGES   (ICET_STATE_ENABLE_START | (IceTEnum)0x0005)
#define ICET_COLLECT_IMAGES     (ICET_STATE_ENABLE_START | (IceTEnum)0x0006)
#define ICET_RENDER_EMPTY_IMAGES (ICET_STATE_ENABLE_START | (IceTEnum)0x0007)

/* This set of enable state variables are reserved for the rendering layer. */
#define ICET_RENDER_LAYER_ENABLE_START (ICET_STATE_ENABLE_START | (IceTEnum)0x0030)
#define ICET_RENDER_LAYER_ENABLE_END   (ICET_STATE_ENABLE_END)

/* These variables are used to store buffers. */
#define ICET_STATE_BUFFER_START (IceTEnum)0x00000180
#define ICET_STATE_BUFFER_END   (IceTEnum)0x00000200

#define ICET_CORE_BUFFER_START  (ICET_STATE_BUFFER_START | (IceTEnum)0x0000)
#define ICET_CORE_BUFFER_END    (ICET_STATE_BUFFER_START | (IceTEnum)0x0010)

#define ICET_TRANSFORMED_BOUNDS (ICET_CORE_BUFFER_START | (IceTEnum)0x0000)
#define ICET_CONTAINED_LIST_BUF (ICET_CORE_BUFFER_START | (IceTEnum)0x0001)
#define ICET_CONTAINED_MASK_BUF (ICET_CORE_BUFFER_START | (IceTEnum)0x0002)
#define ICET_DATA_REP_GROUP_BUF (ICET_CORE_BUFFER_START | (IceTEnum)0x0003)
#define ICET_COMM_COUNT_BUF     (ICET_CORE_BUFFER_START | (IceTEnum)0x0004)
#define ICET_COMM_OFFSET_BUF    (ICET_CORE_BUFFER_START | (IceTEnum)0x0005)
#define ICET_STRATEGY_COMMON_BUF_0 (ICET_CORE_BUFFER_START | (IceTEnum)0x0006)
#define ICET_STRATEGY_COMMON_BUF_1 (ICET_CORE_BUFFER_START | (IceTEnum)0x0007)
#define ICET_STRATEGY_COMMON_BUF_2 (ICET_CORE_BUFFER_START | (IceTEnum)0x0008)

#define ICET_RENDER_LAYER_BUFFER_START (ICET_STATE_BUFFER_START | (IceTEnum)0x0010)
#define ICET_RENDER_LAYER_BUFFER_END   (ICET_STATE_BUFFER_START | (IceTEnum)0x0020)

#define ICET_STRATEGY_BUFFER_START (ICET_STATE_BUFFER_START | (IceTEnum)0x0020)
#define ICET_STRATEGY_BUFFER_END   (ICET_STATE_BUFFER_START | (IceTEnum)0x0030)
#define ICET_NUM_STRATEGY_BUFFERS  (ICET_STRATEGY_BUFFER_END - ICET_STRATEGY_BUFFER_START)
#define ICET_STRATEGY_BUFFER_0  (ICET_STRATEGY_BUFFER_START | (IceTEnum)0x0000)
#define ICET_STRATEGY_BUFFER_1  (ICET_STRATEGY_BUFFER_START | (IceTEnum)0x0001)
#define ICET_STRATEGY_BUFFER_2  (ICET_STRATEGY_BUFFER_START | (IceTEnum)0x0002)
#define ICET_STRATEGY_BUFFER_3  (ICET_STRATEGY_BUFFER_START | (IceTEnum)0x0003)
#define ICET_STRATEGY_BUFFER_4  (ICET_STRATEGY_BUFFER_START | (IceTEnum)0x0004)
#define ICET_STRATEGY_BUFFER_5  (ICET_STRATEGY_BUFFER_START | (IceTEnum)0x0005)
#define ICET_STRATEGY_BUFFER_6  (ICET_STRATEGY_BUFFER_START | (IceTEnum)0x0006)
#define ICET_STRATEGY_BUFFER_7  (ICET_STRATEGY_BUFFER_START | (IceTEnum)0x0007)
#define ICET_STRATEGY_BUFFER_8  (ICET_STRATEGY_BUFFER_START | (IceTEnum)0x0008)
#define ICET_STRATEGY_BUFFER_9  (ICET_STRATEGY_BUFFER_START | (IceTEnum)0x0009)
#define ICET_STRATEGY_BUFFER_10 (ICET_STRATEGY_BUFFER_START | (IceTEnum)0x000A)
#define ICET_STRATEGY_BUFFER_11 (ICET_STRATEGY_BUFFER_START | (IceTEnum)0x000B)
#define ICET_STRATEGY_BUFFER_12 (ICET_STRATEGY_BUFFER_START | (IceTEnum)0x000C)
#define ICET_STRATEGY_BUFFER_13 (ICET_STRATEGY_BUFFER_START | (IceTEnum)0x000D)
#define ICET_STRATEGY_BUFFER_14 (ICET_STRATEGY_BUFFER_START | (IceTEnum)0x000E)
#define ICET_STRATEGY_BUFFER_15 (ICET_STRATEGY_BUFFER_START | (IceTEnum)0x000F)

#define ICET_SI_STRATEGY_BUFFER_START   (ICET_STATE_BUFFER_START | (IceTEnum)0x0030)
#define ICET_SI_STRATEGY_BUFFER_END     (ICET_STATE_BUFFER_START | (IceTEnum)0x0040)
#define ICET_NUM_SI_STRATEGY_BUFFERS    (SI_STRATEGY_BUFFER_END - SI_STRATEGY_BUFFER_START)
#define ICET_SI_STRATEGY_BUFFER_0  (ICET_SI_STRATEGY_BUFFER_START | (IceTEnum)0x0000)
#define ICET_SI_STRATEGY_BUFFER_1  (ICET_SI_STRATEGY_BUFFER_START | (IceTEnum)0x0001)
#define ICET_SI_STRATEGY_BUFFER_2  (ICET_SI_STRATEGY_BUFFER_START | (IceTEnum)0x0002)
#define ICET_SI_STRATEGY_BUFFER_3  (ICET_SI_STRATEGY_BUFFER_START | (IceTEnum)0x0003)
#define ICET_SI_STRATEGY_BUFFER_4  (ICET_SI_STRATEGY_BUFFER_START | (IceTEnum)0x0004)
#define ICET_SI_STRATEGY_BUFFER_5  (ICET_SI_STRATEGY_BUFFER_START | (IceTEnum)0x0005)
#define ICET_SI_STRATEGY_BUFFER_6  (ICET_SI_STRATEGY_BUFFER_START | (IceTEnum)0x0006)
#define ICET_SI_STRATEGY_BUFFER_7  (ICET_SI_STRATEGY_BUFFER_START | (IceTEnum)0x0007)
#define ICET_SI_STRATEGY_BUFFER_8  (ICET_SI_STRATEGY_BUFFER_START | (IceTEnum)0x0008)
#define ICET_SI_STRATEGY_BUFFER_9  (ICET_SI_STRATEGY_BUFFER_START | (IceTEnum)0x0009)
#define ICET_SI_STRATEGY_BUFFER_10 (ICET_SI_STRATEGY_BUFFER_START | (IceTEnum)0x000A)
#define ICET_SI_STRATEGY_BUFFER_11 (ICET_SI_STRATEGY_BUFFER_START | (IceTEnum)0x000B)
#define ICET_SI_STRATEGY_BUFFER_12 (ICET_SI_STRATEGY_BUFFER_START | (IceTEnum)0x000C)
#define ICET_SI_STRATEGY_BUFFER_13 (ICET_SI_STRATEGY_BUFFER_START | (IceTEnum)0x000D)
#define ICET_SI_STRATEGY_BUFFER_14 (ICET_SI_STRATEGY_BUFFER_START | (IceTEnum)0x000E)
#define ICET_SI_STRATEGY_BUFFER_15 (ICET_SI_STRATEGY_BUFFER_START | (IceTEnum)0x000F)

#define ICET_COMMUNICATION_LAYER_START (ICET_STATE_BUFFER_START | (IceTEnum)0x0040)
#define ICET_COMMUNICATION_LAYER_END  (ICET_STATE_BUFFER_START | (IceTEnum)0x0050)

#define ICET_STATE_SIZE         (IceTEnum)0x00000200
#define ICET_STATE_ENGINE_END   (ICET_STATE_ENGINE_START + ICET_STATE_SIZE)

ICET_EXPORT void icetGetDoublev(IceTEnum pname, IceTDouble *params);
ICET_EXPORT void icetGetFloatv(IceTEnum pname, IceTFloat *params);
ICET_EXPORT void icetGetIntegerv(IceTEnum pname, IceTInt *params);
ICET_EXPORT void icetGetBooleanv(IceTEnum pname, IceTBoolean *params);
ICET_EXPORT void icetGetEnumv(IceTEnum pname, IceTEnum *params);
ICET_EXPORT void icetGetBitFieldv(IceTEnum pname, IceTBitField *bitfield);
ICET_EXPORT void icetGetPointerv(IceTEnum pname, IceTVoid **params);

ICET_EXPORT void icetEnable(IceTEnum pname);
ICET_EXPORT void icetDisable(IceTEnum pname);
ICET_EXPORT IceTBoolean icetIsEnabled(IceTEnum pname);

#define ICET_NO_ERROR           (IceTEnum)0x00000000
#define ICET_SANITY_CHECK_FAIL  (IceTEnum)0xFFFFFFFF
#define ICET_INVALID_ENUM       (IceTEnum)0xFFFFFFFE
#define ICET_BAD_CAST           (IceTEnum)0xFFFFFFFD
#define ICET_OUT_OF_MEMORY      (IceTEnum)0xFFFFFFFC
#define ICET_INVALID_OPERATION  (IceTEnum)0xFFFFFFFB
#define ICET_INVALID_VALUE      (IceTEnum)0xFFFFFFFA

ICET_EXPORT IceTEnum icetGetError(void);

#ifdef __cplusplus
}
#endif

#endif /* __IceT_h */
