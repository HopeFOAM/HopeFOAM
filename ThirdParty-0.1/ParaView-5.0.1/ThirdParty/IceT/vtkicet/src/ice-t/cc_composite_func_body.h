/* -*- c -*- *******************************************************/
/*
 * Copyright (C) 2011 Sandia Corporation
 * Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 * the U.S. Government retains certain rights in this software.
 *
 * This source code is released under the New BSD License.
 */

/* This is not a traditional header file, but rather a "macro" file that defines
 * a function body for a decompression function.  (If this were C++, we would
 * actually use templates to automatically generate all these cases.)  In
 * general, there are many flavors of the decompression functionality which
 * differ only slightly.  Rather than maintain lots of different code bases or
 * try to debug big macros, we just include this file with various parameters.
 *
 * The following macros must be defined:
 *      FRONT_SPARSE_IMAGE - an IceTSparseImage object containing the data to
 *              be composited in front.
 *      BACK_SPARSE_IMAGE - an IceTSparseImage object containing the data to
 *              be composited in back.  (Obviously if the current compositing
 *              operation is order independent, the FRONT and BACK may be
 *              switched without effect.)
 *      DEST_SPARSE_IMAGE - an IceTSparseImage object to place the result.
 *
 * All of the above macros are undefined at the end of this file.
 */

#ifndef FRONT_SPARSE_IMAGE
#error Need FRONT_SPARSE_IMAGE macro.  Is this included in image.c?
#endif
#ifndef BACK_SPARSE_IMAGE
#error Need BACK_SPARSE_IMAGE macro.  Is this included in image.c?
#endif
#ifndef DEST_SPARSE_IMAGE
#error Need DEST_SPARSE_IMAGE macro.  Is this included in image.c?
#endif
#ifndef INACTIVE_RUN_LENGTH
#error Need INACTIVE_RUN_LENGTH macro.  Is this included in image.c?
#endif
#ifndef ACTIVE_RUN_LENGTH
#error Need ACTIVE_RUN_LENGTH macro.  Is this included in image.c?
#endif

{
    IceTEnum _color_format;
    IceTEnum _depth_format;
    IceTEnum _composite_mode;

    icetGetEnumv(ICET_COMPOSITE_MODE, &_composite_mode);

    _color_format = icetSparseImageGetColorFormat(FRONT_SPARSE_IMAGE);
    _depth_format = icetSparseImageGetDepthFormat(FRONT_SPARSE_IMAGE);

    if (   (_color_format != icetSparseImageGetColorFormat(BACK_SPARSE_IMAGE))
        || (_color_format != icetSparseImageGetColorFormat(DEST_SPARSE_IMAGE))
        || (_depth_format != icetSparseImageGetDepthFormat(BACK_SPARSE_IMAGE))
        || (_depth_format != icetSparseImageGetDepthFormat(DEST_SPARSE_IMAGE))
           ) {
        icetRaiseError("Input buffers do not agree for compressed-compressed"
                       " composite.",
                       ICET_SANITY_CHECK_FAIL);
    }

    if (_composite_mode == ICET_COMPOSITE_MODE_Z_BUFFER) {
        if (_depth_format == ICET_IMAGE_DEPTH_FLOAT) {
          /* Use Z buffer for active pixel testing and compositing. */
            if (_color_format == ICET_IMAGE_COLOR_RGBA_UBYTE) {
#define UNPACK_PIXEL(pointer, color, depth)     \
    color = (IceTUInt *)pointer;                \
    pointer += sizeof(IceTUInt);                \
    depth = (IceTFloat *)pointer;               \
    pointer += sizeof(IceTFloat);
#define CCC_FRONT_COMPRESSED_IMAGE FRONT_SPARSE_IMAGE
#define CCC_BACK_COMPRESSED_IMAGE BACK_SPARSE_IMAGE
#define CCC_DEST_COMPRESSED_IMAGE DEST_SPARSE_IMAGE
#define CCC_COMPOSITE(src1_pointer, src2_pointer, dest_pointer)         \
    {                                                                   \
        const IceTUInt *src1_color;                                     \
        const IceTFloat *src1_depth;                                    \
        const IceTUInt *src2_color;                                     \
        const IceTFloat *src2_depth;                                    \
        IceTUInt *dest_color;                                           \
        IceTFloat *dest_depth;                                          \
        UNPACK_PIXEL(src1_pointer, src1_color, src1_depth);             \
        UNPACK_PIXEL(src2_pointer, src2_color, src2_depth);             \
        UNPACK_PIXEL(dest_pointer, dest_color, dest_depth);             \
        if (src1_depth[0] < src2_depth[0]) {                            \
            dest_color[0] = src1_color[0];                              \
            dest_depth[0] = src1_depth[0];                              \
        } else {                                                        \
            dest_color[0] = src2_color[0];                              \
            dest_depth[0] = src2_depth[0];                              \
        }                                                               \
    }
#define CCC_PIXEL_SIZE (sizeof(IceTUInt) + sizeof(IceTFloat))
#include "cc_composite_template_body.h"
#undef UNPACK_PIXEL
            } else if (_color_format == ICET_IMAGE_COLOR_RGBA_FLOAT) {
#define UNPACK_PIXEL(pointer, color, depth)     \
    color = (IceTFloat *)pointer;               \
    pointer += 4*sizeof(IceTUInt);              \
    depth = (IceTFloat *)pointer;               \
    pointer += sizeof(IceTFloat);
#define CCC_FRONT_COMPRESSED_IMAGE FRONT_SPARSE_IMAGE
#define CCC_BACK_COMPRESSED_IMAGE BACK_SPARSE_IMAGE
#define CCC_DEST_COMPRESSED_IMAGE DEST_SPARSE_IMAGE
#define CCC_COMPOSITE(src1_pointer, src2_pointer, dest_pointer)         \
    {                                                                   \
        const IceTFloat *src1_color;                                    \
        const IceTFloat *src1_depth;                                    \
        const IceTFloat *src2_color;                                    \
        const IceTFloat *src2_depth;                                    \
        IceTFloat *dest_color;                                          \
        IceTFloat *dest_depth;                                          \
        UNPACK_PIXEL(src1_pointer, src1_color, src1_depth);             \
        UNPACK_PIXEL(src2_pointer, src2_color, src2_depth);             \
        UNPACK_PIXEL(dest_pointer, dest_color, dest_depth);             \
        if (src1_depth[0] < src2_depth[0]) {                            \
            dest_color[0] = src1_color[0];                              \
            dest_color[1] = src1_color[1];                              \
            dest_color[2] = src1_color[2];                              \
            dest_color[3] = src1_color[3];                              \
            dest_depth[0] = src1_depth[0];                              \
        } else {                                                        \
            dest_color[0] = src2_color[0];                              \
            dest_color[1] = src2_color[1];                              \
            dest_color[2] = src2_color[2];                              \
            dest_color[3] = src2_color[3];                              \
            dest_depth[0] = src2_depth[0];                              \
        }                                                               \
    }
#define CCC_PIXEL_SIZE (5*sizeof(IceTFloat))
#include "cc_composite_template_body.h"
#undef UNPACK_PIXEL
            } else if (_color_format == ICET_IMAGE_COLOR_NONE) {
#define UNPACK_PIXEL(pointer, depth)            \
    depth = (IceTFloat *)pointer;               \
    pointer += sizeof(IceTFloat);
#define CCC_FRONT_COMPRESSED_IMAGE FRONT_SPARSE_IMAGE
#define CCC_BACK_COMPRESSED_IMAGE BACK_SPARSE_IMAGE
#define CCC_DEST_COMPRESSED_IMAGE DEST_SPARSE_IMAGE
#define CCC_COMPOSITE(src1_pointer, src2_pointer, dest_pointer)         \
    {                                                                   \
        const IceTFloat *src1_depth;                                    \
        const IceTFloat *src2_depth;                                    \
        IceTFloat *dest_depth;                                          \
        UNPACK_PIXEL(src1_pointer, src1_depth);                         \
        UNPACK_PIXEL(src2_pointer, src2_depth);                         \
        UNPACK_PIXEL(dest_pointer, dest_depth);                         \
        if (src1_depth[0] < src2_depth[0]) {                            \
            dest_depth[0] = src1_depth[0];                              \
        } else {                                                        \
            dest_depth[0] = src2_depth[0];                              \
        }                                                               \
    }
#define CCC_PIXEL_SIZE (sizeof(IceTFloat))
#include "cc_composite_template_body.h"
#undef UNPACK_PIXEL
            } else {
                icetRaiseError("Encountered invalid color format.",
                               ICET_SANITY_CHECK_FAIL);
            }
        } else if (_depth_format == ICET_IMAGE_DEPTH_NONE) {
            icetRaiseError("Cannot use Z buffer compositing operation with no"
                           " Z buffer.", ICET_INVALID_OPERATION);
        } else {
            icetRaiseError("Encountered invalid depth format.",
                           ICET_SANITY_CHECK_FAIL);
        }
    } else if (_composite_mode == ICET_COMPOSITE_MODE_BLEND) {
      /* Use alpha for active pixel and compositing. */
        if (_depth_format == ICET_IMAGE_DEPTH_NONE) {
            if (_color_format == ICET_IMAGE_COLOR_RGBA_UBYTE) {
#define UNPACK_PIXEL(pointer, color)            \
    color = (IceTUInt *)pointer;                \
    pointer += sizeof(IceTUInt);
#define CCC_FRONT_COMPRESSED_IMAGE FRONT_SPARSE_IMAGE
#define CCC_BACK_COMPRESSED_IMAGE BACK_SPARSE_IMAGE
#define CCC_DEST_COMPRESSED_IMAGE DEST_SPARSE_IMAGE
#define CCC_COMPOSITE(front_pointer, back_pointer, dest_pointer)        \
    {                                                                   \
        const IceTUInt *front_color;                                    \
        const IceTUInt *back_color;                                     \
        IceTUInt *dest_color;                                           \
        UNPACK_PIXEL(front_pointer, front_color);                       \
        UNPACK_PIXEL(back_pointer, back_color);                         \
        UNPACK_PIXEL(dest_pointer, dest_color);                         \
        ICET_BLEND_UBYTE((const IceTUByte *)front_color,                \
                         (const IceTUByte *)back_color,                 \
                         (IceTUByte *)dest_color);                      \
    }
#define CCC_PIXEL_SIZE (sizeof(IceTUInt))
#include "cc_composite_template_body.h"
#undef UNPACK_PIXEL
            } else if (_color_format == ICET_IMAGE_COLOR_RGBA_FLOAT) {
#define UNPACK_PIXEL(pointer, color)            \
    color = (IceTFloat *)pointer;               \
    pointer += 4*sizeof(IceTUInt);
#define CCC_FRONT_COMPRESSED_IMAGE FRONT_SPARSE_IMAGE
#define CCC_BACK_COMPRESSED_IMAGE BACK_SPARSE_IMAGE
#define CCC_DEST_COMPRESSED_IMAGE DEST_SPARSE_IMAGE
#define CCC_COMPOSITE(front_pointer, back_pointer, dest_pointer)        \
    {                                                                   \
        const IceTFloat *front_color;                                   \
        const IceTFloat *back_color;                                    \
        IceTFloat *dest_color;                                          \
        UNPACK_PIXEL(front_pointer, front_color);                       \
        UNPACK_PIXEL(back_pointer, back_color);                         \
        UNPACK_PIXEL(dest_pointer, dest_color);                         \
        ICET_BLEND_FLOAT(front_color, back_color, dest_color);          \
    }
#define CCC_PIXEL_SIZE (4*sizeof(IceTFloat))
#include "cc_composite_template_body.h"
#undef UNPACK_PIXEL
            } else if (_color_format == ICET_IMAGE_COLOR_NONE) {
                icetRaiseWarning("Compositing image with no data.",
                                 ICET_INVALID_OPERATION);
                icetClearSparseImage(DEST_SPARSE_IMAGE);
            } else {
                icetRaiseError("Encountered invalid color format.",
                               ICET_SANITY_CHECK_FAIL);
            }
        } else {
            icetRaiseError("Cannot use blend composite with a depth buffer.",
                           ICET_INVALID_VALUE);
        }
    } else {
        icetRaiseError("Encountered invalid composite mode.",
                       ICET_SANITY_CHECK_FAIL);
    }
}

#undef FRONT_SPARSE_IMAGE
#undef BACK_SPARSE_IMAGE
#undef DEST_SPARSE_IMAGE
