/* -*- c -*- *******************************************************/
/*
 * Copyright (C) 2003 Sandia Corporation
 * Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 * the U.S. Government retains certain rights in this software.
 *
 * This source code is released under the New BSD License.
 */

/* This is not a traditional header file, but rather a "macro" file that defines
 * a template for a decompression function.  (If this were C++, we would
 * actually use tempaltes.)  In general, there are many flavors of the
 * decompression functionality which differ only slightly.  Rather than maintain
 * lots of different code bases or try to debug big macros, we just include this
 * file with various parameters.
 *
 * The following macros must be defined:
 *	DT_COMPRESSED_IMAGE - the buffer that holds the compressed image.
 *	DT_READ_PIXEL(pointer) - reads the current pixel from the pointer and
 *		increments the pointer.
 *	DT_INCREMENT_INACTIVE_PIXELS(count) - Increments over count pixels,
 *		setting them all to appropriate inactive values.
 *
 * All of the above macros are undefined at the end of this file.
 */

#ifndef ICET_IMAGE_DATA
#error Need ICET_IMAGE_DATA macro.  Is this included in image.c?
#endif
#ifndef INACTIVE_RUN_LENGTH
#error Need INACTIVE_RUN_LENGTH macro.  Is this included in image.c?
#endif
#ifndef ACTIVE_RUN_LENGTH
#error Need ACTIVE_RUN_LENGTH macro.  Is this included in image.c?
#endif

{
    const IceTByte *_src;  /* Use IceTByte for byte-based pointer arithmetic. */
    IceTSizeType _pixels;
    IceTSizeType _p;
    IceTSizeType _i;

    _pixels = icetSparseImageGetNumPixels(DT_COMPRESSED_IMAGE);
    _src = ICET_IMAGE_DATA(DT_COMPRESSED_IMAGE);

    _p = 0;
    while (_p < _pixels) {
	const IceTVoid *_runlengths;
	IceTSizeType _rl;

        _runlengths = _src;
        _src += RUN_LENGTH_SIZE;

      /* Set background pixels. */
	_rl = INACTIVE_RUN_LENGTH(_runlengths);
	_p += _rl;
	if (_p > _pixels) {
	    icetRaiseError("Corrupt compressed image.", ICET_INVALID_VALUE);
	    break;
	}
	DT_INCREMENT_INACTIVE_PIXELS(_rl);

      /* Set active pixels. */
	_rl = ACTIVE_RUN_LENGTH(_runlengths);
	_p += _rl;
	if (_p > _pixels) {
	    icetRaiseError("Corrupt compressed image.", ICET_INVALID_VALUE);
	    break;
	}
	for (_i = 0; _i < _rl; _i++) {
	    DT_READ_PIXEL(_src);
	}
    }
}

#undef DT_COMPRESSED_IMAGE
#undef DT_READ_PIXEL
#undef DT_INCREMENT_INACTIVE_PIXELS
