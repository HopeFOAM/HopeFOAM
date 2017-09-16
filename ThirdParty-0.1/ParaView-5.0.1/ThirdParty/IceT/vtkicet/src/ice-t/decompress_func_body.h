/* -*- c -*- *******************************************************/
/*
 * Copyright (C) 2010 Sandia Corporation
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
 *      INPUT_SPARSE_IMAGE - an IceTSparseImage object containing the data to
 *              be decompressed.
 *      OUTPUT_IMAGE - an allocated IceTImage object to place the decompressed
 *              image.
 *
 * The following macros are optional:
 *      TIME_DECOMPRESSION - if defined, the time to perform the decompression
 *              is added to the total compress time.
 *      COMPOSITE - if defined, OUTPUT_IMAGE is expected to already have data
 *              onto which to composite the data from the INPUT_SPARSE_IMAGE.
 *              (It is more efficient to do both operations simultaneously.)
 *              If defined, the following also need to be defined:
 *              BLEND_RGBA_UBYTE(src, dest) - blend the incoming color from
 *                      the compressed image (src) to the data value in the
 *                      output image (dest).  Store the result in dest.  Both
 *                      src and dest are IceTUByte arrays representing the RGBA
 *                      values.
 *              BLEND_RGBA_FLOAT(src, dest) - same as above except src and dest
 *                      are IceTFloat arrays.
 *	CORRECT_BACKGROUND - if defined, the output color will be blended
 *		with the true background color.  This should only be set
 *		if ICET_NEED_BACKGROUND_CORRECTION is true.
 *      OFFSET - If defined to a number (or variable holding a number), skips
 *              that many pixels at the beginning of the image.
 *      PIXEL_COUNT - If defined to a number (or a variable holding a number),
 *              uses this as the size of the image rather than the actual size
 *              defined in the image.  This should be defined if OFFSET is
 *              defined.
 *
 * All of the above macros are undefined at the end of this file.
 */

#ifndef INPUT_SPARSE_IMAGE
#error Need INPUT_SPARSE_IMAGE macro.  Is this included in image.c?
#endif
#ifndef OUTPUT_IMAGE
#error Need OUTPUT_IMAGE macro.  Is this included in image.c?
#endif
#ifndef INACTIVE_RUN_LENGTH
#error Need INACTIVE_RUN_LENGTH macro.  Is this included in image.c?
#endif
#ifndef ACTIVE_RUN_LENGTH
#error Need ACTIVE_RUN_LENGTH macro.  Is this included in image.c?
#endif

{
    IceTEnum _color_format, _depth_format;
    IceTSizeType _pixel_count;
    IceTEnum _composite_mode;

#ifdef TIME_DECOMPRESSION
    icetTimingCompressBegin();
#endif /* TIME_DECOMPRESSION */

    icetGetEnumv(ICET_COMPOSITE_MODE, &_composite_mode);

    _color_format = icetSparseImageGetColorFormat(INPUT_SPARSE_IMAGE);
    _depth_format = icetSparseImageGetDepthFormat(INPUT_SPARSE_IMAGE);
    _pixel_count = icetSparseImageGetNumPixels(INPUT_SPARSE_IMAGE);

    if (_color_format != icetImageGetColorFormat(OUTPUT_IMAGE)) {
        icetRaiseError("Input/output buffers have different color formats.",
                       ICET_SANITY_CHECK_FAIL);
    }
    if (_depth_format != icetImageGetDepthFormat(OUTPUT_IMAGE)) {
        icetRaiseError("Input/output buffers have different depth formats.",
                       ICET_SANITY_CHECK_FAIL);
    }
#ifdef PIXEL_COUNT
    if (_pixel_count  != PIXEL_COUNT) {
        icetRaiseError("Unexpected input pixel count.",
                       ICET_SANITY_CHECK_FAIL);
    }
#else
    if (_pixel_count  != icetImageGetNumPixels(OUTPUT_IMAGE)) {
        icetRaiseError("Unexpected input pixel count.",
                       ICET_SANITY_CHECK_FAIL);
    }
#endif
#ifdef OFFSET
    if (_pixel_count > icetImageGetNumPixels(OUTPUT_IMAGE) - OFFSET) {
        icetRaiseError("Offset pixels outside range of output image.",
                       ICET_SANITY_CHECK_FAIL);
    }
#endif

    if (_composite_mode == ICET_COMPOSITE_MODE_Z_BUFFER) {
        if (_depth_format == ICET_IMAGE_DEPTH_FLOAT) {
          /* Use Z buffer for active pixel testing and compositing. */
            IceTFloat *_depth = icetImageGetDepthf(OUTPUT_IMAGE);
#ifdef OFFSET
            _depth += OFFSET;
#endif
            if (_color_format == ICET_IMAGE_COLOR_RGBA_UBYTE) {
                IceTUInt *_color;
                const IceTUInt *_c_in;
                const IceTFloat *_d_in;
                IceTUInt _background_color;
                _color = icetImageGetColorui(OUTPUT_IMAGE);
#ifdef OFFSET
                _color += OFFSET;
#endif
                icetGetIntegerv(ICET_BACKGROUND_COLOR_WORD,
                                (IceTInt *)&_background_color);
#ifdef COMPOSITE
#define COPY_PIXEL(c_src, c_dest, d_src, d_dest)                \
                                if (d_src[0] < d_dest[0]) {     \
                                    c_dest[0] = c_src[0];       \
                                    d_dest[0] = d_src[0];       \
                                }
#else
#define COPY_PIXEL(c_src, c_dest, d_src, d_dest)                \
                                c_dest[0] = c_src[0];           \
                                d_dest[0] = d_src[0];
#endif
#define DT_COMPRESSED_IMAGE     INPUT_SPARSE_IMAGE
#define DT_READ_PIXEL(src)      _c_in = (IceTUInt *)src;        \
                                src += sizeof(IceTUInt);        \
                                _d_in = (IceTFloat *)src;       \
                                src += sizeof(IceTFloat);       \
                                COPY_PIXEL(_c_in, _color,       \
                                           _d_in, _depth);      \
                                _color++;  _depth++;
#ifdef COMPOSITE
#define DT_INCREMENT_INACTIVE_PIXELS(count) _color += count;  _depth += count;
#else
#define DT_INCREMENT_INACTIVE_PIXELS(count)                             \
                                {                                       \
                                    IceTSizeType __i;                   \
                                    for (__i = 0; __i < count; __i++) { \
                                        *(_color++) = _background_color;\
                                        *(_depth++) = 1.0f;             \
                                    }                                   \
                                }
#endif
#include "decompress_template_body.h"
#undef COPY_PIXEL
            } else if (_color_format == ICET_IMAGE_COLOR_RGBA_FLOAT) {
                IceTFloat *_color;
                const IceTFloat *_c_in;
                const IceTFloat *_d_in;
                IceTFloat _background_color[4];
                _color = icetImageGetColorf(OUTPUT_IMAGE);
#ifdef OFFSET
                _color += 4*(OFFSET);
#endif
                icetGetFloatv(ICET_BACKGROUND_COLOR, _background_color);
#ifdef COMPOSITE
#define COPY_PIXEL(c_src, c_dest, d_src, d_dest)                \
                                if (d_src[0] < d_dest[0]) {     \
                                    c_dest[0] = c_src[0];       \
                                    c_dest[1] = c_src[1];       \
                                    c_dest[2] = c_src[2];       \
                                    c_dest[3] = c_src[3];       \
                                    d_dest[0] = d_src[0];       \
                                }
#else
#define COPY_PIXEL(c_src, c_dest, d_src, d_dest)                \
                                c_dest[0] = c_src[0];           \
                                c_dest[1] = c_src[1];           \
                                c_dest[2] = c_src[2];           \
                                c_dest[3] = c_src[3];           \
                                d_dest[0] = d_src[0];
#endif
#define DT_COMPRESSED_IMAGE     INPUT_SPARSE_IMAGE
#define DT_READ_PIXEL(src)      _c_in = (IceTFloat *)src;       \
                                src += 4*sizeof(IceTFloat);     \
                                _d_in = (IceTFloat *)src;       \
                                src += sizeof(IceTFloat);       \
                                COPY_PIXEL(_c_in, _color,       \
                                           _d_in, _depth);      \
                                _color += 4;  _depth++;
#ifdef COMPOSITE
#define DT_INCREMENT_INACTIVE_PIXELS(count) _color += 4*count;  _depth += count;
#else
#define DT_INCREMENT_INACTIVE_PIXELS(count)                             \
                                {                                       \
                                    IceTSizeType __i;                   \
                                    for (__i = 0; __i < count; __i++) { \
                                        _color[0] =_background_color[0];\
                                        _color[1] =_background_color[1];\
                                        _color[2] =_background_color[2];\
                                        _color[3] =_background_color[3];\
                                        _color += 4;                    \
                                        *(_depth++) = 1.0f;             \
                                    }                                   \
                                }
#endif
#include "decompress_template_body.h"
#undef COPY_PIXEL
            } else if (_color_format == ICET_IMAGE_COLOR_NONE) {
                const IceTFloat *_d_in;
#ifdef COMPOSITE
#define COPY_PIXEL(d_src, d_dest)                               \
                                if (d_src[0] < d_dest[0]) {     \
                                    d_dest[0] = d_src[0];       \
                                }
#else
#define COPY_PIXEL(d_src, d_dest)                               \
                                d_dest[0] = d_src[0];
#endif
#define DT_COMPRESSED_IMAGE     INPUT_SPARSE_IMAGE
#define DT_READ_PIXEL(src)      _d_in = (IceTFloat *)src;       \
                                src += sizeof(IceTFloat);       \
                                COPY_PIXEL(_d_in, _depth);      \
                                _depth++;
#ifdef COMPOSITE
#define DT_INCREMENT_INACTIVE_PIXELS(count) _depth += count;
#else
#define DT_INCREMENT_INACTIVE_PIXELS(count)                             \
                                {                                       \
                                    IceTSizeType __i;                   \
                                    for (__i = 0; __i < count; __i++) { \
                                        *(_depth++) = 1.0f;             \
                                    }                                   \
                                }
#endif
#include "decompress_template_body.h"
#undef COPY_PIXEL
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
        if (_depth_format != ICET_IMAGE_DEPTH_NONE) {
            icetRaiseWarning("Z buffer ignored during blend composite"
                             " operation.  Output z buffer meaningless.",
                             ICET_INVALID_VALUE);
        }
        if (_color_format == ICET_IMAGE_COLOR_RGBA_UBYTE) {
            IceTUInt *_color;
            const IceTUInt *_c_in;
            IceTUInt _background_color;
            _color = icetImageGetColorui(OUTPUT_IMAGE);
#ifdef OFFSET
            _color += OFFSET;
#endif
#ifdef CORRECT_BACKGROUND
            icetGetIntegerv(ICET_TRUE_BACKGROUND_COLOR_WORD,
                            (IceTInt *)&_background_color);
#else
            icetGetIntegerv(ICET_BACKGROUND_COLOR_WORD,
                            (IceTInt *)&_background_color);
#endif
#ifdef COMPOSITE
#define COPY_PIXEL(c_src, c_dest) \
            BLEND_RGBA_UBYTE(((IceTUByte*)c_src), ((IceTUByte*)c_dest))
#elif defined(CORRECT_BACKGROUND)
#define COPY_PIXEL(c_src, c_dest)                               \
            ICET_BLEND_UBYTE(((IceTUByte*)c_src),               \
                             ((IceTUByte*)&_background_color),  \
                             ((IceTUByte*)c_dest))
#else
#define COPY_PIXEL(c_src, c_dest) \
            c_dest[0] = c_src[0];
#endif
#define DT_COMPRESSED_IMAGE     INPUT_SPARSE_IMAGE
#define DT_READ_PIXEL(src)      _c_in = (IceTUInt *)src;        \
                                src += sizeof(IceTUInt);        \
                                COPY_PIXEL(_c_in, _color);      \
                                _color++;
#ifdef COMPOSITE
#define DT_INCREMENT_INACTIVE_PIXELS(count) _color += count;
#else
#define DT_INCREMENT_INACTIVE_PIXELS(count)                             \
                                {                                       \
                                    IceTSizeType __i;                   \
                                    for (__i = 0; __i < count; __i++) { \
                                        *(_color++) = _background_color;\
                                    }                                   \
                                }
#endif
#include "decompress_template_body.h"
#undef COPY_PIXEL
        } else if (_color_format == ICET_IMAGE_COLOR_RGBA_FLOAT) {
            IceTFloat *_color;
            const IceTFloat *_c_in;
            IceTFloat _background_color[4];
            _color = icetImageGetColorf(OUTPUT_IMAGE);
#ifdef OFFSET
            _color += 4*(OFFSET);
#endif
#ifdef CORRECT_BACKGROUND
            icetGetFloatv(ICET_TRUE_BACKGROUND_COLOR, _background_color);
#else
            icetGetFloatv(ICET_BACKGROUND_COLOR, _background_color);
#endif
#ifdef COMPOSITE
#define COPY_PIXEL(c_src, c_dest) BLEND_RGBA_FLOAT(c_src, c_dest);
#elif defined(CORRECT_BACKGROUND)
#define COPY_PIXEL(c_src, c_dest) \
            ICET_BLEND_FLOAT(c_src, _background_color, c_dest);
#else
#define COPY_PIXEL(c_src, c_dest)                               \
                                c_dest[0] = c_src[0];           \
                                c_dest[1] = c_src[1];           \
                                c_dest[2] = c_src[2];           \
                                c_dest[3] = c_src[3];
#endif
#define DT_COMPRESSED_IMAGE     INPUT_SPARSE_IMAGE
#define DT_READ_PIXEL(src)      _c_in = (IceTFloat *)src;       \
                                src += 4*sizeof(IceTFloat);     \
                                COPY_PIXEL(_c_in, _color);      \
                                _color += 4;
#ifdef COMPOSITE
#define DT_INCREMENT_INACTIVE_PIXELS(count) _color += 4*count;
#else
#define DT_INCREMENT_INACTIVE_PIXELS(count)                             \
                                {                                       \
                                    IceTSizeType __i;                   \
                                    for (__i = 0; __i < count; __i++) { \
                                        _color[0] =_background_color[0];\
                                        _color[1] =_background_color[1];\
                                        _color[2] =_background_color[2];\
                                        _color[3] =_background_color[3];\
                                        _color += 4;                    \
                                    }                                   \
                                }
#endif
#include "decompress_template_body.h"
#undef COPY_PIXEL
        } else if (_color_format == ICET_IMAGE_COLOR_NONE) {
            icetRaiseWarning("Decompressing image with no data.",
                             ICET_INVALID_OPERATION);
        } else {
            icetRaiseError("Encountered invalid color format.",
                           ICET_SANITY_CHECK_FAIL);
        }
    } else {
        icetRaiseError("Encountered invalid composite mode.",
                       ICET_SANITY_CHECK_FAIL);
    }

#ifdef TIME_DECOMPRESSION
    icetTimingCompressEnd();
#endif
}

#undef INPUT_SPARSE_IMAGE
#undef OUTPUT_IMAGE

#ifdef TIME_DECOMPRESSION
#undef TIME_DECOMPRESSION
#endif

#ifdef COMPOSITE
#undef COMPOSITE
#undef BLEND_RGBA_UBYTE
#undef BLEND_RGBA_FLOAT
#endif

#ifdef OFFSET
#undef OFFSET
#endif

#ifdef PIXEL_COUNT
#undef PIXEL_COUNT
#endif
