/* -*- c -*- *******************************************************/
/*
 * Copyright (C) 2010 Sandia Corporation
 * Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 * the U.S. Government retains certain rights in this software.
 *
 * This source code is released under the New BSD License.
 */

#include <IceT.h>

#include <IceTDevCommunication.h>
#include <IceTDevDiagnostics.h>
#include <IceTDevImage.h>

#define TREE_IN_SPARSE_IMAGE_BUFFER     ICET_SI_STRATEGY_BUFFER_0
#define TREE_SPARSE_IMAGE_BUFFER        ICET_SI_STRATEGY_BUFFER_1

#define TREE_IMAGE_DATA 23

static void RecursiveTreeCompose(const IceTInt *compose_group,
                                 IceTInt group_size,
                                 IceTInt group_rank,
                                 IceTInt image_dest,
                                 IceTSparseImage *imageData,
                                 IceTVoid *inSparseImageBuffer,
                                 IceTSparseImage *imageBuffer)
{
    IceTInt middle;
    enum { NO_IMAGE, SEND_IMAGE, RECV_IMAGE } current_image;
    IceTInt pair_proc;

    if (group_size <= 1) return;

  /* Build composite tree by splitting down middle. */
  /* If middle is in a group, then the image is sent there, otherwise the
   * image is sent to the processor of group_rank 0 (for that subgroup). */
    middle = group_size/2;
    if (group_rank < middle) {
        RecursiveTreeCompose(compose_group, middle, group_rank, image_dest,
                             imageData, inSparseImageBuffer, imageBuffer);
        if (group_rank == image_dest) {
          /* I'm the destination.  GIMME! */
            current_image = RECV_IMAGE;
            pair_proc = middle;
        } else if (   (group_rank == 0)
                   && ((image_dest < 0) || (image_dest >= middle)) ) {
          /* I have an image by default. */
            if ((image_dest >= middle) && (image_dest < group_size)) {
              /* Opposite subtree has destination.  Hand it over. */
                current_image = SEND_IMAGE;
                pair_proc = image_dest;
            } else {
              /* Opposite subtree does not have destination either.  Give
                 to me by default. */
                current_image = RECV_IMAGE;
                pair_proc = middle;
            }
        } else {
          /* I don't even have an image anymore. */
            current_image = NO_IMAGE;
            pair_proc = -1;
        }
    } else {
        RecursiveTreeCompose(compose_group + middle, group_size - middle,
                             group_rank - middle, image_dest - middle,
                             imageData, inSparseImageBuffer, imageBuffer);
        if (group_rank == image_dest) {
          /* I'm the destination.  GIMME! */
            current_image = RECV_IMAGE;
            pair_proc = 0;
        } else if (   (group_rank == middle)
                   && ((image_dest < middle) || (image_dest >= group_size)) ) {
          /* I have an image by default. */
            current_image = SEND_IMAGE;
            if ((image_dest >= 0) && (image_dest < middle)) {
                pair_proc = image_dest;
            } else {
                pair_proc = 0;
            }
        } else {
          /* I don't even have an image anymore. */
            current_image = NO_IMAGE;
            pair_proc = -1;
        }
    }

    if (current_image == SEND_IMAGE) {
      /* Hasta la vista, baby. */
        IceTVoid *package_buffer;
        IceTSizeType package_size;
        icetRaiseDebug1("Sending image to %d", (int)compose_group[pair_proc]);
        icetSparseImagePackageForSend(*imageData,
                                      &package_buffer,
                                      &package_size);
        icetCommSend(package_buffer, package_size, ICET_BYTE,
                     compose_group[pair_proc], TREE_IMAGE_DATA);
    } else if (current_image == RECV_IMAGE) {
      /* Get my image. */
        IceTSparseImage inSparseImage;
        IceTSizeType incoming_size;
        icetRaiseDebug1("Getting image from %d", (int)compose_group[pair_proc]);
        incoming_size = icetSparseImageBufferSizeType(
                                      icetSparseImageGetColorFormat(*imageData),
                                      icetSparseImageGetDepthFormat(*imageData),
                                      icetSparseImageGetWidth(*imageData),
                                      icetSparseImageGetHeight(*imageData));
        icetCommRecv(inSparseImageBuffer, incoming_size, ICET_BYTE,
                     compose_group[pair_proc], TREE_IMAGE_DATA);
        inSparseImage
            = icetSparseImageUnpackageFromReceive(inSparseImageBuffer);
        if (group_rank < pair_proc) {
            icetCompressedCompressedComposite(*imageData,
                                              inSparseImage,
                                              *imageBuffer);
        } else {
            icetCompressedCompressedComposite(inSparseImage,
                                              *imageData,
                                              *imageBuffer);
        }
        /* The actual image data is now in imageBuffer, so switch imageBuffer
           and imageData. */
        {
            IceTSparseImage oldImage = *imageData;
            *imageData = *imageBuffer;
            *imageBuffer = oldImage;
        }
    }
}

void icetTreeCompose(const IceTInt *compose_group,
                     IceTInt group_size,
                     IceTInt image_dest,
                     IceTSparseImage input_image,
                     IceTSparseImage *result_image,
                     IceTSizeType *piece_offset)
{
    IceTInt group_rank;
    IceTVoid *inSparseImageBuffer;
    IceTSparseImage imageData;
    IceTSparseImage imageBuffer;
    IceTSizeType width, height;
    IceTSizeType sparseBufferSize;

    width = icetSparseImageGetWidth(input_image);
    height = icetSparseImageGetHeight(input_image);

    sparseBufferSize = icetSparseImageBufferSize(width, height);

    inSparseImageBuffer = icetGetStateBuffer(TREE_IN_SPARSE_IMAGE_BUFFER,
                                             sparseBufferSize);
    imageData = input_image;
    imageBuffer = icetGetStateBufferSparseImage(TREE_SPARSE_IMAGE_BUFFER,
                                                width, height);

    group_rank = icetFindMyRankInGroup(compose_group, group_size);
    if (group_rank < 0) {
        icetRaiseError("Local process not in compose_group?",
                       ICET_SANITY_CHECK_FAIL);
        return;
    }

    RecursiveTreeCompose(compose_group, group_size, group_rank, image_dest,
                         &imageData, inSparseImageBuffer, &imageBuffer);

    *result_image = imageData;
    *piece_offset = 0;
    if (image_dest != group_rank) {
        icetSparseImageSetDimensions(*result_image, 0, 0);
    }
}
