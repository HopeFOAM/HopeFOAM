/* -*- c -*- *******************************************************/
/*
 * Copyright (C) 2010 Sandia Corporation
 * Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 * the U.S. Government retains certain rights in this software.
 *
 * This source code is released under the New BSD License.
 */

/* This is not a traditional header file, but rather a "macro" file that defines
 * a function body for a compression function.  (If this were C++, we would
 * actually use templates to automatically generate all these cases.)  In
 * general, there are many flavors of the compression functionality which differ
 * only slightly.  Rather than maintain lots of different code bases or try to
 * debug big macros, we just include this file with various parameters.
 *
 * The following macros must be defined:
 *      INPUT_IMAGE - an IceTImage object containing the data to be compressed.
 *      OUTPUT_SPARSE_IMAGE - the buffer that will hold the compressed image
 *              (i.e. an allocated IceTSparseImage pointer).
 *
 * The following macros are optional:
 *      OFFSET - If defined to a number (or variable holding a number), skips
 *              that many pixels at the beginning of the image.
 *      PIXEL_COUNT - If defined to a number (or a variable holding a number),
 *              uses this as the size of the image rather than the actual size
 *              defined in the image.  This should be defined if OFFSET is
 *              defined.
 *      PADDING - If defined, enables inactive pixels to be placed
 *              around the file.  If defined, then SPACE_BOTTOM, SPACE_TOP,
 *              SPACE_LEFT, SPACE_RIGHT, FULL_WIDTH, and FULL_HEIGHT must
 *              all also be defined.
 *      REGION - If defined, pixels are taken from a particular region of the
 *              image.  Sort of like OFFSET/PIXEL_COUNT except that the pixels
 *              are contiguous in a 2D region rather than the 1D layout of
 *              pixels in memory.  If defined, then REGION_OFFSET_X,
 *              REGION_OFFSET_Y, REGION_WIDTH, and REGION_HEIGHT must also be
 *              defined.
 *
 * All of the above macros are undefined at the end of this file.
 */

#ifndef INPUT_IMAGE
#error Need INPUT_IMAGE macro.  Is this included in image.c?
#endif
#ifndef OUTPUT_SPARSE_IMAGE
#error Need OUTPUT_SPARSE_IMAGE macro.  Is this included in image.c?
#endif
#ifndef INACTIVE_RUN_LENGTH
#error Need INACTIVE_RUN_LENGTH macro.  Is this included in image.c?
#endif
#ifndef ACTIVE_RUN_LENGTH
#error Need ACTIVE_RUN_LENGTH macro.  Is this included in image.c?
#endif

#ifdef REGION
#ifdef OFFSET
#error REGION and OFFSET are incompatible
#else
#define OFFSET (REGION_OFFSET_X + _input_width*(REGION_OFFSET_Y))
#endif
#endif

{
    IceTEnum _color_format, _depth_format;
    IceTSizeType _pixel_count;
    IceTEnum _composite_mode;
#ifdef REGION
    IceTSizeType _input_width = icetImageGetWidth(INPUT_IMAGE);
    IceTSizeType _region_width = REGION_WIDTH;
    IceTSizeType _region_x_skip = _input_width - (REGION_WIDTH);
#endif

    icetGetEnumv(ICET_COMPOSITE_MODE, &_composite_mode);

    _color_format = icetImageGetColorFormat(INPUT_IMAGE);
    _depth_format = icetImageGetDepthFormat(INPUT_IMAGE);

#ifdef PIXEL_COUNT
    _pixel_count = PIXEL_COUNT;
#elif defined(REGION)
    _pixel_count = (REGION_WIDTH)*(REGION_HEIGHT);
#else
    _pixel_count = icetImageGetNumPixels(INPUT_IMAGE);
#endif

#ifdef DEBUG
    if (   (icetSparseImageGetColorFormat(OUTPUT_SPARSE_IMAGE) != _color_format)
        || (icetSparseImageGetDepthFormat(OUTPUT_SPARSE_IMAGE) != _depth_format)
           ) {
        icetRaiseError("Format of input and output to compress do not match.",
                       ICET_SANITY_CHECK_FAIL);
    }
#ifdef PADDING
    if (   icetSparseImageGetNumPixels(OUTPUT_SPARSE_IMAGE)
        != (  _pixel_count + (FULL_WIDTH)*(SPACE_TOP+SPACE_BOTTOM)
            + ((FULL_HEIGHT)-(SPACE_TOP+SPACE_BOTTOM))*(SPACE_LEFT+SPACE_RIGHT))
           ) {
        icetRaiseError("Size of input and output to compress do not match.",
                       ICET_SANITY_CHECK_FAIL);
    }
#else /*PADDING*/
    if (icetSparseImageGetNumPixels(OUTPUT_SPARSE_IMAGE) != _pixel_count) {
        icetRaiseError("Size of input and output to compress do not match.",
                       ICET_SANITY_CHECK_FAIL);
    }
#endif /*PADDING*/
#ifdef REGION
    if (    (REGION_OFFSET_X < 0) || (REGION_OFFSET_Y < 0)
         || (REGION_OFFSET_X+REGION_WIDTH > icetImageGetWidth(INPUT_IMAGE))
         || (REGION_OFFSET_Y+REGION_HEIGHT > icetImageGetHeight(INPUT_IMAGE)) ){
        icetRaiseError("Size of input incompatible with region.",
                       ICET_SANITY_CHECK_FAIL);
    }
#endif /*REGION*/
#endif /*DEBUG*/

    if (_composite_mode == ICET_COMPOSITE_MODE_Z_BUFFER) {
        if (_depth_format == ICET_IMAGE_DEPTH_FLOAT) {
          /* Use Z buffer for active pixel testing. */
            const IceTFloat *_depth = icetImageGetDepthcf(INPUT_IMAGE);
#ifdef OFFSET
            _depth += OFFSET;
#endif
            if (_color_format == ICET_IMAGE_COLOR_RGBA_UBYTE) {
                const IceTUInt *_color;
                IceTUInt *_c_out;
                IceTFloat *_d_out;
#ifdef REGION
                IceTSizeType _region_count = 0;
#endif
                _color = icetImageGetColorcui(INPUT_IMAGE);
#ifdef OFFSET
                _color += OFFSET;
#endif
#define CT_COMPRESSED_IMAGE     OUTPUT_SPARSE_IMAGE
#define CT_COLOR_FORMAT         _color_format
#define CT_DEPTH_FORMAT         _depth_format
#define CT_PIXEL_COUNT          _pixel_count
#define CT_ACTIVE()             (_depth[0] < 1.0)
#define CT_WRITE_PIXEL(dest)    _c_out = (IceTUInt *)dest;      \
                                _c_out[0] = _color[0];          \
                                dest += sizeof(IceTUInt);       \
                                _d_out = (IceTFloat *)dest;     \
                                _d_out[0] = _depth[0];          \
                                dest += sizeof(IceTFloat);
#ifdef REGION
#define CT_INCREMENT_PIXEL()    _color++;  _depth++;                    \
                                _region_count++;                        \
                                if (_region_count >= _region_width) {   \
                                    _color += _region_x_skip;           \
                                    _depth += _region_x_skip;           \
                                    _region_count = 0;                  \
                                }
#else
#define CT_INCREMENT_PIXEL()    _color++;  _depth++;
#endif
#ifdef PADDING
#define CT_PADDING
#define CT_SPACE_BOTTOM         SPACE_BOTTOM
#define CT_SPACE_TOP            SPACE_TOP
#define CT_SPACE_LEFT           SPACE_LEFT
#define CT_SPACE_RIGHT          SPACE_RIGHT
#define CT_FULL_WIDTH           FULL_WIDTH
#define CT_FULL_HEIGHT          FULL_HEIGHT
#endif
#include "compress_template_body.h"
            } else if (_color_format == ICET_IMAGE_COLOR_RGBA_FLOAT) {
                const IceTFloat *_color;
                IceTFloat *_out;
#ifdef REGION
                IceTSizeType _region_count = 0;
#endif
                _color = icetImageGetColorcf(INPUT_IMAGE);
#ifdef OFFSET
                _color += 4*(OFFSET);
#endif
#define CT_COMPRESSED_IMAGE     OUTPUT_SPARSE_IMAGE
#define CT_COLOR_FORMAT         _color_format
#define CT_DEPTH_FORMAT         _depth_format
#define CT_PIXEL_COUNT          _pixel_count
#define CT_ACTIVE()             (_depth[0] < 1.0)
#define CT_WRITE_PIXEL(dest)    _out = (IceTFloat *)dest;       \
                                _out[0] = _color[0];            \
                                _out[1] = _color[1];            \
                                _out[2] = _color[2];            \
                                _out[3] = _color[3];            \
                                _out[4] = _depth[0];            \
                                dest += 5*sizeof(IceTFloat);
#ifdef REGION
#define CT_INCREMENT_PIXEL()    _color += 4;  _depth++;                 \
                                _region_count++;                        \
                                if (_region_count >= _region_width) {   \
                                    _color += 4*_region_x_skip;         \
                                    _depth += _region_x_skip;           \
                                    _region_count = 0;                  \
                                }
#else
#define CT_INCREMENT_PIXEL()    _color += 4;  _depth++;
#endif
#ifdef PADDING
#define CT_PADDING
#define CT_SPACE_BOTTOM         SPACE_BOTTOM
#define CT_SPACE_TOP            SPACE_TOP
#define CT_SPACE_LEFT           SPACE_LEFT
#define CT_SPACE_RIGHT          SPACE_RIGHT
#define CT_FULL_WIDTH           FULL_WIDTH
#define CT_FULL_HEIGHT          FULL_HEIGHT
#endif
#include "compress_template_body.h"
            } else if (_color_format == ICET_IMAGE_COLOR_NONE) {
                IceTFloat *_out;
#ifdef REGION
                IceTSizeType _region_count = 0;
#endif
#define CT_COMPRESSED_IMAGE     OUTPUT_SPARSE_IMAGE
#define CT_COLOR_FORMAT         _color_format
#define CT_DEPTH_FORMAT         _depth_format
#define CT_PIXEL_COUNT          _pixel_count
#define CT_ACTIVE()             (_depth[0] < 1.0)
#define CT_WRITE_PIXEL(dest)    _out = (IceTFloat *)dest;       \
                                _out[0] = _depth[0];            \
                                dest += 1*sizeof(IceTFloat);
#ifdef REGION
#define CT_INCREMENT_PIXEL()    _depth++;                               \
                                _region_count++;                        \
                                if (_region_count >= _region_width) {   \
                                    _depth += _region_x_skip;           \
                                    _region_count = 0;                  \
                                }
#else
#define CT_INCREMENT_PIXEL()    _depth++;
#endif
#ifdef PADDING
#define CT_PADDING
#define CT_SPACE_BOTTOM         SPACE_BOTTOM
#define CT_SPACE_TOP            SPACE_TOP
#define CT_SPACE_LEFT           SPACE_LEFT
#define CT_SPACE_RIGHT          SPACE_RIGHT
#define CT_FULL_WIDTH           FULL_WIDTH
#define CT_FULL_HEIGHT          FULL_HEIGHT
#endif
#include "compress_template_body.h"
            } else {
                icetRaiseError("Encountered invalid color format.",
                               ICET_SANITY_CHECK_FAIL);
            }
        } else if (_depth_format == ICET_IMAGE_DEPTH_NONE) {
            icetRaiseError("Cannot use Z buffer compression with no"
                           " Z buffer.", ICET_INVALID_OPERATION);
        } else {
            icetRaiseError("Encountered invalid depth format.",
                           ICET_SANITY_CHECK_FAIL);
        }
    } else if (_composite_mode == ICET_COMPOSITE_MODE_BLEND) {
      /* Use alpha for active pixel testing. */
        if (_depth_format != ICET_IMAGE_DEPTH_NONE) {
            icetRaiseWarning("Z buffer ignored during blend compress"
                             " operation.  Output z buffer meaningless.",
                             ICET_INVALID_VALUE);
        }
        if (_color_format == ICET_IMAGE_COLOR_RGBA_UBYTE) {
            const IceTUInt *_color;
            IceTUInt *_out;
#ifdef REGION
            IceTSizeType _region_count = 0;
#endif
            _color = icetImageGetColorcui(INPUT_IMAGE);
#ifdef OFFSET
            _color += OFFSET;
#endif
#define CT_COMPRESSED_IMAGE     OUTPUT_SPARSE_IMAGE
#define CT_COLOR_FORMAT         _color_format
#define CT_DEPTH_FORMAT         _depth_format
#define CT_PIXEL_COUNT          _pixel_count
#define CT_ACTIVE()             (((IceTUByte*)_color)[3] != 0x00)
#define CT_WRITE_PIXEL(dest)    _out = (IceTUInt *)dest;        \
                                _out[0] = _color[0];            \
                                dest += sizeof(IceTUInt);
#ifdef REGION
#define CT_INCREMENT_PIXEL()    _color++;                               \
                                _region_count++;                        \
                                if (_region_count >= _region_width) {   \
                                    _color += _region_x_skip;           \
                                    _region_count = 0;                  \
                                }
#else
#define CT_INCREMENT_PIXEL()    _color++;
#endif
#ifdef PADDING
#define CT_PADDING
#define CT_SPACE_BOTTOM         SPACE_BOTTOM
#define CT_SPACE_TOP            SPACE_TOP
#define CT_SPACE_LEFT           SPACE_LEFT
#define CT_SPACE_RIGHT          SPACE_RIGHT
#define CT_FULL_WIDTH           FULL_WIDTH
#define CT_FULL_HEIGHT          FULL_HEIGHT
#endif
#include "compress_template_body.h"
        } else if (_color_format == ICET_IMAGE_COLOR_RGBA_FLOAT) {
            const IceTFloat *_color;
            IceTFloat *_out;
#ifdef REGION
            IceTSizeType _region_count = 0;
#endif
            _color = icetImageGetColorcf(INPUT_IMAGE);
#ifdef OFFSET
            _color += 4*(OFFSET);
#endif
#define CT_COMPRESSED_IMAGE     OUTPUT_SPARSE_IMAGE
#define CT_COLOR_FORMAT         _color_format
#define CT_DEPTH_FORMAT         _depth_format
#define CT_PIXEL_COUNT          _pixel_count
#define CT_ACTIVE()             (_color[3] != 0.0)
#define CT_WRITE_PIXEL(dest)    _out = (IceTFloat *)dest;       \
                                _out[0] = _color[0];            \
                                _out[1] = _color[1];            \
                                _out[2] = _color[2];            \
                                _out[3] = _color[3];            \
                                dest += 4*sizeof(IceTUInt);
#ifdef REGION
#define CT_INCREMENT_PIXEL()    _color += 4;                            \
                                _region_count++;                        \
                                if (_region_count >= _region_width) {   \
                                    _color += 4*_region_x_skip;         \
                                    _region_count = 0;                  \
                                }
#else
#define CT_INCREMENT_PIXEL()    _color += 4;
#endif
#ifdef PADDING
#define CT_PADDING
#define CT_SPACE_BOTTOM         SPACE_BOTTOM
#define CT_SPACE_TOP            SPACE_TOP
#define CT_SPACE_LEFT           SPACE_LEFT
#define CT_SPACE_RIGHT          SPACE_RIGHT
#define CT_FULL_WIDTH           FULL_WIDTH
#define CT_FULL_HEIGHT          FULL_HEIGHT
#endif
#include "compress_template_body.h"
        } else if (_color_format == ICET_IMAGE_COLOR_NONE) {
            IceTUInt *_out;
            icetRaiseWarning("Compressing image with no data.",
                             ICET_INVALID_OPERATION);
            _out = ICET_IMAGE_DATA(OUTPUT_SPARSE_IMAGE);
            INACTIVE_RUN_LENGTH(_out) = _pixel_count;
            ACTIVE_RUN_LENGTH(_out) = 0;
            _out++;
            icetSparseImageSetActualSize(OUTPUT_SPARSE_IMAGE, _out);
        } else {
            icetRaiseError("Encountered invalid color format.",
                           ICET_SANITY_CHECK_FAIL);
        }
    } else {
        icetRaiseError("Encountered invalid composite mode.",
                       ICET_SANITY_CHECK_FAIL);
    }

    icetRaiseDebug1("Compression: %f%%\n",
        100.0f - (  100.0f*icetSparseImageGetCompressedBufferSize(OUTPUT_SPARSE_IMAGE)
                  / icetImageBufferSizeType(_color_format, _depth_format,
                                            icetSparseImageGetWidth(OUTPUT_SPARSE_IMAGE),
                                            icetSparseImageGetHeight(OUTPUT_SPARSE_IMAGE)) ));
}

#undef INPUT_IMAGE
#undef OUTPUT_SPARSE_IMAGE

#ifdef PADDING
#undef PADDING
#undef SPACE_BOTTOM
#undef SPACE_TOP
#undef SPACE_LEFT
#undef SPACE_RIGHT
#undef FULL_WIDTH
#undef FULL_HEIGHT
#endif

#ifdef REGION
#undef REGION
#undef REGION_OFFSET_X
#undef REGION_OFFSET_Y
#undef REGION_WIDTH
#undef REGION_HEIGHT
#endif

#ifdef OFFSET
#undef OFFSET
#endif

#ifdef PIXEL_COUNT
#undef PIXEL_COUNT
#endif
