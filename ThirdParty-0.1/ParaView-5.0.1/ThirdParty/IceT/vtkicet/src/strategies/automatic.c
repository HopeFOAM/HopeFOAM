/* -*- c -*- *******************************************************/
/*
 * Copyright (C) 2010 Sandia Corporation
 * Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 * the U.S. Government retains certain rights in this software.
 *
 * This source code is released under the New BSD License.
 */

#include <IceT.h>

#include <IceTDevDiagnostics.h>
#include <IceTDevImage.h>
#include <IceTDevStrategySelect.h>

void icetAutomaticCompose(const IceTInt *compose_group,
                          IceTInt group_size,
                          IceTInt image_dest,
                          IceTSparseImage input_image,
                          IceTSparseImage *result_image,
                          IceTSizeType *piece_offset)
{
    if (group_size > 1) {
        icetRaiseDebug("Doing radix-kr compose");
        icetInvokeSingleImageStrategy(ICET_SINGLE_IMAGE_STRATEGY_RADIXKR,
                                      compose_group,
                                      group_size,
                                      image_dest,
                                      input_image,
                                      result_image,
                                      piece_offset);
    } else if (group_size == 1) {
        icetRaiseDebug("Shallow copy input.");
        *result_image = input_image;
        *piece_offset = 0;
    } else {
        icetRaiseDebug("Clearing pixels");
        icetClearSparseImage(input_image);
        *result_image = input_image;
        *piece_offset = 0;
    }
}
