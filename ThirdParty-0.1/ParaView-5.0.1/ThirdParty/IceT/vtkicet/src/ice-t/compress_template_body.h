/* -*- c -*- *******************************************************/
/*
 * Copyright (C) 2003 Sandia Corporation
 * Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 * the U.S. Government retains certain rights in this software.
 *
 * This source code is released under the New BSD License.
 */

/* This is not a traditional header file, but rather a "macro" file that defines
 * a template for a compression function.  (If this were C++, we would actually
 * use templates.)  In general, there are many flavors of the compression
 * functionality which differ only slightly.  Rather than maintain lots of
 * different code bases or try to debug big macros, we just include this file
 * with various parameters.
 *
 * In general, this file should only be included by compress_func_body.h
 *
 * The following macros must be defined:
 *      CT_COMPRESSED_IMAGE - the object that will hold the compressed image.
 *      CT_COLOR_FORMAT - color format IceTEnum for input and output
 *      CT_DEPTH_FORMAT - depth format IceTEnum for input and output
 *      CT_PIXEL_COUNT - the number of pixels in the original image (or a
 *              variable holding it.
 *      CT_ACTIVE() - provides a true value if the current pixel is active.
 *      CT_WRITE_PIXEL(pointer) - writes the current pixel to the pointer and
 *              increments the pointer.
 *      CT_INCREMENT_PIXEL() - Increments to the next input pixel.
 *
 * The following macros are optional:
 *      CT_PADDING - If defined, enables inactive pixels to be placed
 *              around the file.  If defined, then CT_SPACE_BOTTOM,
 *              CT_SPACE_TOP, CT_SPACE_LEFT, CT_SPACE_RIGHT, CT_FULL_WIDTH,
 *              and CT_FULL_HEIGHT must all also be defined.
 *
 * All of the above macros are undefined at the end of this file.
 */

#ifndef CT_COMPRESSED_IMAGE
#error Need CT_COMPRESSED_IMAGE macro.  Is this included in image.c?
#endif
#ifndef CT_COLOR_FORMAT
#error Need CT_COLOR_FORMAT macro.  Is this included in image.c?
#endif
#ifndef CT_DEPTH_FORMAT
#error Need CT_DEPTH_FORMAT macro.  Is this included in image.c ?
#endif
#ifndef CT_PIXEL_COUNT
#error Need CT_PIXEL_COUNT macro.  Is this included in image.c?
#endif
#ifndef ICET_IMAGE_DATA
#error Need ICET_IMAGE_DATA macro.  Is this included in image.c?
#endif
#ifndef INACTIVE_RUN_LENGTH
#error Need INACTIVE_RUN_LENGTH macro.  Is this included in image.c?
#endif
#ifndef ACTIVE_RUN_LENGTH
#error Need ACTIVE_RUN_LENGTH macro.  Is this included in image.c?
#endif
#ifndef RUN_LENGTH_SIZE
#error Need RUN_LENGTH_SIZE macro.  Is this included in image.c?
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4127)
#endif

{
  IceTByte *_dest;  /* Use IceTByte for byte-based pointer arithmetic. */
    IceTSizeType _pixels = CT_PIXEL_COUNT;
    IceTSizeType _p;
    IceTSizeType _count;
#ifdef DEBUG
    IceTSizeType _totalcount = 0;
#endif
    IceTSizeType _compressed_size;

    icetTimingCompressBegin();

    _dest = ICET_IMAGE_DATA(CT_COMPRESSED_IMAGE);

#ifndef CT_PADDING
    _count = 0;
#else /* CT_PADDING */
    _count = CT_SPACE_BOTTOM*CT_FULL_WIDTH;

    if ((CT_SPACE_LEFT != 0) || (CT_SPACE_RIGHT != 0)) {
        IceTSizeType _line, _lastline;
        for (_line = CT_SPACE_BOTTOM, _lastline = CT_FULL_HEIGHT-CT_SPACE_TOP;
             _line < _lastline; _line++) {
            IceTSizeType _x = CT_SPACE_LEFT;
            IceTSizeType _lastx = CT_FULL_WIDTH-CT_SPACE_RIGHT;
            _count += CT_SPACE_LEFT;
            while (ICET_TRUE) {
                IceTVoid *_runlengths;
                while ((_x < _lastx) && (!CT_ACTIVE())) {
                    _x++;
                    _count++;
                    CT_INCREMENT_PIXEL();
                }
                if (_x >= _lastx) break;
                _runlengths = _dest;
                _dest += RUN_LENGTH_SIZE;
                INACTIVE_RUN_LENGTH(_runlengths) = _count;
#ifdef DEBUG
                _totalcount += _count;
#endif
                _count = 0;
                while ((_x < _lastx) && CT_ACTIVE()) {
                    CT_WRITE_PIXEL(_dest);
                    CT_INCREMENT_PIXEL();
                    _count++;
                    _x++;
                }
                ACTIVE_RUN_LENGTH(_runlengths) = _count;
#ifdef DEBUG
                _totalcount += _count;
#endif
                _count = 0;
                if (_x >= _lastx) break;
            }
            _count += CT_SPACE_RIGHT;
        }
    } else { /* CT_SPACE_LEFT == CT_SPACE_RIGHT == 0 */
        _pixels = (CT_FULL_HEIGHT-CT_SPACE_BOTTOM-CT_SPACE_TOP)*CT_FULL_WIDTH;
#endif /* CT_PADDING */

        _p = 0;
        while (_p < _pixels) {
            IceTVoid *_runlengths = _dest;
            _dest += RUN_LENGTH_SIZE;
          /* Count background pixels. */
            while ((_p < _pixels) && (!CT_ACTIVE())) {
                _p++;
                _count++;
                CT_INCREMENT_PIXEL();
            }
            INACTIVE_RUN_LENGTH(_runlengths) = _count;
#ifdef DEBUG
            _totalcount += _count;
#endif

          /* Count and store active pixels. */
            _count = 0;
            while ((_p < _pixels) && CT_ACTIVE()) {
                CT_WRITE_PIXEL(_dest);
                CT_INCREMENT_PIXEL();
                _count++;
                _p++;
            }
            ACTIVE_RUN_LENGTH(_runlengths) = _count;
#ifdef DEBUG
            _totalcount += _count;
#endif

            _count = 0;
        }
#ifdef CT_PADDING
    }

    _count += CT_SPACE_TOP*CT_FULL_WIDTH;
    if (_count > 0) {
        INACTIVE_RUN_LENGTH(_dest) = _count;
        ACTIVE_RUN_LENGTH(_dest) = 0;
        _dest += RUN_LENGTH_SIZE;
#ifdef DEBUG
        _totalcount += _count;
#endif /*DEBUG*/
    }
#endif /*CT_PADDING*/

#ifdef DEBUG
#ifdef CT_PADDING
    _totalcount -= (CT_FULL_WIDTH)*(CT_SPACE_TOP+CT_SPACE_BOTTOM);
    _totalcount -= (  (CT_FULL_HEIGHT-(CT_SPACE_TOP+CT_SPACE_BOTTOM))
                    * (CT_SPACE_LEFT+CT_SPACE_RIGHT) );
#endif /*CT_PADDING*/
    if (_totalcount != (IceTSizeType)CT_PIXEL_COUNT) {
        char msg[256];
        sprintf(msg, "Total run lengths don't equal pixel count: %d != %d",
                (int)_totalcount, (int)(CT_PIXEL_COUNT));
        icetRaiseError(msg, ICET_SANITY_CHECK_FAIL);
    }
#endif /*DEBUG*/

    icetTimingCompressEnd();

    _compressed_size
        = (IceTSizeType)
            (  (IceTPointerArithmetic)_dest
             - (IceTPointerArithmetic)ICET_IMAGE_HEADER(CT_COMPRESSED_IMAGE));
    ICET_IMAGE_HEADER(CT_COMPRESSED_IMAGE)[ICET_IMAGE_ACTUAL_BUFFER_SIZE_INDEX]
      = (IceTInt)_compressed_size;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#undef CT_COMPRESSED_IMAGE
#undef CT_COLOR_FORMAT
#undef CT_DEPTH_FORMAT
#undef CT_PIXEL_COUNT
#undef CT_ACTIVE
#undef CT_WRITE_PIXEL
#undef CT_INCREMENT_PIXEL
#undef COMPRESSED_SIZE

#ifdef CT_PADDING
#undef CT_PADDING
#undef CT_SPACE_BOTTOM
#undef CT_SPACE_TOP
#undef CT_SPACE_LEFT
#undef CT_SPACE_RIGHT
#undef CT_FULL_WIDTH
#undef CT_FULL_HEIGHT
#endif
