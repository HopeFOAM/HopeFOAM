/* -*- c -*- *******************************************************/
/*
 * Copyright (C) 2010 Sandia Corporation
 * Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 * the U.S. Government retains certain rights in this software.
 *
 * This source code is released under the New BSD License.
 */

/* The Radix-k algorithm was designed by Tom Peterka at Argonne National
   Laboratory.

   Copyright (c) University of Chicago
   Permission is hereby granted to use, reproduce, prepare derivative works, and
   to redistribute to others.

   The Radix-k algorithm was ported to IceT by Wesley Kendall from University
   of Tennessee at Knoxville.

   The derived Radix-kr algorithm was designed by Kenneth Moreland at Sandia
   National Laboratories.
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <string.h>

#include <IceT.h>

#include <IceTDevCommunication.h>
#include <IceTDevDiagnostics.h>
#include <IceTDevImage.h>

#define RADIXKR_SWAP_IMAGE_TAG_START     2200

#define RADIXKR_RECEIVE_BUFFER                   ICET_SI_STRATEGY_BUFFER_0
#define RADIXKR_SEND_BUFFER                      ICET_SI_STRATEGY_BUFFER_1
#define RADIXKR_SPARE_BUFFER                     ICET_SI_STRATEGY_BUFFER_2
#define RADIXKR_INTERLACED_IMAGE_BUFFER          ICET_SI_STRATEGY_BUFFER_3
#define RADIXKR_PARTITION_INFO_BUFFER            ICET_SI_STRATEGY_BUFFER_4
#define RADIXKR_RECEIVE_REQUEST_BUFFER           ICET_SI_STRATEGY_BUFFER_5
#define RADIXKR_SEND_REQUEST_BUFFER              ICET_SI_STRATEGY_BUFFER_6
#define RADIXKR_FACTORS_ARRAY_BUFFER             ICET_SI_STRATEGY_BUFFER_7
#define RADIXKR_SPLIT_OFFSET_ARRAY_BUFFER        ICET_SI_STRATEGY_BUFFER_8
#define RADIXKR_SPLIT_IMAGE_ARRAY_BUFFER         ICET_SI_STRATEGY_BUFFER_9

typedef struct radixkrRoundInfoStruct {
    IceTInt k; /* k value for this round. */
    IceTInt r; /* remainder for this round (number of processes dropped). */
    IceTInt step; /* Ranks jump by this much in this round. */
    IceTInt split_factor; /* Number of new image partitions made from each partition. */
    IceTBoolean has_image; /* True if local process collects image data this round. */
    IceTBoolean last_partition; /* True if local process is part of the last partition. */
    IceTInt first_rank; /* The lowest rank of those participating with this process this round. */
    IceTInt partition_index; /* Index of partition at this round (if has_image true). */
} radixkrRoundInfo;

typedef struct radixkrInfoStruct {
    radixkrRoundInfo *rounds; /* Array of per round info. */
    IceTInt num_rounds;
} radixkrInfo;

typedef struct radixkrPartnerInfoStruct {
    IceTInt rank; /* Rank of partner. */
    IceTSizeType offset; /* Offset of partner's partition in image. */
    IceTVoid *receiveBuffer; /* A buffer for receiving data from partner. */
    IceTSparseImage sendImage; /* A buffer to hold data being sent to partner */
    IceTSparseImage receiveImage; /* Hold for received non-composited image. */
    IceTInt compositeLevel; /* Level in compositing tree for round. */
} radixkrPartnerInfo;

typedef struct radixkrPartnerGroupInfoStruct {
    radixkrPartnerInfo *partners; /* Array of partners in this group. */
    IceTInt num_partners; /* Number of partners in this group. */
} radixkrPartnerGroupInfo;

/* BEGIN_PIVOT_FOR(loop_var, low, pivot, high)...END_PIVOT_FOR() provides a
   special looping mechanism that iterates over the numbers pivot, pivot-1,
   pivot+1, pivot-2, pivot-3,... until all numbers between low (inclusive) and
   high (exclusive) are visited.  Any numbers outside [low,high) are skipped. */
#define BEGIN_PIVOT_FOR(loop_var, low, pivot, high) \
    { \
        IceTInt loop_var##_true_iter; \
        IceTInt loop_var##_max = 2*(  ((pivot) < ((high)+(low))/2) \
                                    ? ((high)-(pivot)) : ((pivot)-(low)+1) ); \
        for (loop_var##_true_iter = 1; \
             loop_var##_true_iter < loop_var##_max; \
             loop_var##_true_iter ++) { \
            if ((loop_var##_true_iter % 2) == 0) { \
                loop_var = (pivot) - loop_var##_true_iter/2; \
                if (loop_var < (low)) continue; \
            } else { \
                loop_var = (pivot) + loop_var##_true_iter/2; \
                if ((high) <= loop_var) continue; \
            }

#define END_PIVOT_FOR() \
        } \
    }

static IceTInt radixkrFindFloorLog2(IceTInt x)
{
    IceTInt lg;
    for (lg = 0; (IceTUInt)(1 << lg) <= (IceTUInt)x; lg++);
    lg--;
    return lg;
}

static void radixkrSwapImages(IceTSparseImage *image1, IceTSparseImage *image2)
{
    IceTSparseImage old_image1 = *image1;
    *image1 = *image2;
    *image2 = old_image1;
}

/* radixkrGetPartitionIndices

   my position in each round forms an num_rounds-dimensional vector
   [round 0 pos, round 1 pos, ... round num_rounds-1 pos]
   where pos is my position in the group of partners within that round

   inputs:
     info: holds the number of rounds and k values for each round
     group_rank: my rank in composite order (compose_group in icetRadixkrCompose)

   outputs:
     fills info with step, split_factor, has_image, and partition_index for
     each round.
*/
static void radixkrGetPartitionIndices(radixkrInfo info,
                                       IceTInt group_size,
                                       IceTInt group_rank)
{

    IceTInt step; /* step size in rank for a lattice direction */
    IceTInt total_partitions;
    IceTInt current_group_size;
    IceTInt current_round;
    IceTInt max_image_split;

    icetGetIntegerv(ICET_MAX_IMAGE_SPLIT, &max_image_split);

    total_partitions = 1;
    /* Procs with the same image partition are step ranks from each other. */
    step = 1;
    current_group_size = group_size;
    current_round = 0;
    while (current_round < info.num_rounds) {
        radixkrRoundInfo *round_info = &info.rounds[current_round];
        IceTInt split;
        IceTInt next_step = step*round_info->k;
        IceTInt next_group_size = current_group_size / round_info->k;
        IceTInt end_of_groups = next_step*next_group_size;
        IceTInt first_rank;

        first_rank = group_rank % step + (group_rank/next_step)*next_step;
        if (first_rank >= end_of_groups) {
            first_rank -= next_step;
        }

        if ((max_image_split < 1)
            || (total_partitions * round_info->k <= max_image_split)) {
            split = round_info->k;
        } else {
            split = max_image_split/total_partitions;
        }

        total_partitions *= split;
        round_info->split_factor = split;
        round_info->first_rank = first_rank;
        round_info->partition_index = (group_rank - first_rank)/step;
        round_info->last_partition = ((first_rank+next_step) >= end_of_groups);
        round_info->step = step;
        current_group_size = next_group_size;
        step = next_step;
        /* The has_image test must follow the changes to current_group_size and
           step so that the group sizes gets rounded and the local rank will
           pop out of the last partition. */
        round_info->has_image = (group_rank < (step * current_group_size));
        round_info->has_image &= (round_info->partition_index < split);

        current_round++;
    }

}

static radixkrInfo radixkrGetK(IceTInt compose_group_size,
                               IceTInt group_rank)
{
    /* Divide the world size into groups that are closest to the magic k
       value. */
    radixkrInfo info;
    IceTInt magic_k;
    IceTInt max_num_k;
    IceTInt next_divide;

    /* Special case of when compose_group_size == 1. */
    if (compose_group_size < 2) {
        info.rounds = icetGetStateBuffer(RADIXKR_FACTORS_ARRAY_BUFFER,
                                         sizeof(radixkrRoundInfo) * 1);
        info.rounds[0].k = 1;
        info.rounds[0].r = 0;
        info.rounds[0].step = 1;
        info.rounds[0].split_factor = 1;
        info.rounds[0].has_image = ICET_TRUE;
        info.rounds[0].partition_index = 0;
        info.num_rounds = 1;
        return info;
    }

    info.num_rounds = 0;

    icetGetIntegerv(ICET_MAGIC_K, &magic_k);

    /* The maximum number of factors possible is the floor of log base 2. */
    max_num_k = radixkrFindFloorLog2(compose_group_size);
    info.rounds = icetGetStateBuffer(RADIXKR_FACTORS_ARRAY_BUFFER,
                                     sizeof(radixkrRoundInfo) * max_num_k);

    next_divide = compose_group_size;
    while (next_divide > 1) {
        IceTInt next_k;
        IceTInt next_r;

        if (next_divide > magic_k) {
            /* First guess is the magic k. */
            next_k = magic_k;
            next_r = next_divide % magic_k;
        } else {
            /* Can't do better than doing direct send. */
            next_k = next_divide;
            next_r = 0;
        }

        /* If the k value we picked is not a perfect factor, try to find
         * another value that is a perfect factor or has a smaller remainder.
         */
        if (next_r > 0) {
            IceTInt try_k;
            for (try_k = magic_k-1; try_k >= 2; try_k--) {
                IceTInt try_r = next_divide % try_k;
                if (try_r < next_r) {
                    next_k = try_k;
                    next_r = try_r;
                    if (next_r == 0) { break; }
                }
            }
        }

        /* Set the k value in the array. */
        info.rounds[info.num_rounds].k = next_k;
        info.rounds[info.num_rounds].r = next_r;
        next_divide /= next_k;
        info.num_rounds++;

        if (info.num_rounds > max_num_k) {
            icetRaiseError("Somehow we got more factors than possible.",
                           ICET_SANITY_CHECK_FAIL);
        }
    }

    /* Sanity check to make sure that the k's actually multiply to the number
     * of processes. */
    {
        IceTInt product = 1;
        IceTInt round;
        for (round = info.num_rounds-1; round >= 0; --round) {
            product = product*info.rounds[round].k + info.rounds[round].r;
        }
        if (product != compose_group_size) {
            icetRaiseError("Product of k's not equal to number of processes.",
                           ICET_SANITY_CHECK_FAIL);
        }
    }

    radixkrGetPartitionIndices(info, compose_group_size, group_rank);

    return info;
}

/* radixkrGetFinalPartitionIndex

   After radix-k completes on a group of size p, the image is partitioned into
   p pieces (with a caveat for the maximum number of partitions and remainder).
   This function finds the index for the final partition (with respect to all
   partitions, not just one within a round) for a given rank.

   inputs:
     info: information about rounds

   returns:
     index of final global partition.  -1 if this process does not end up with
     a piece.
*/
static IceTInt radixkrGetFinalPartitionIndex(const radixkrInfo *info)
{
    IceTInt current_round;
    IceTInt partition_index;

    partition_index = 0;
    for (current_round = 0; current_round < info->num_rounds; current_round++) {
        const radixkrRoundInfo *r = &info->rounds[current_round];
        if (r->has_image) {
            partition_index *= r->split_factor;
            partition_index += r->partition_index;
        } else /* !r->has_image */ {
            /* local process has no partition */
            return -1;
        }
    }

    return partition_index;
}

/* radixkrGetTotalNumPartitions

   After radix-kr completes on a group of size p, the image is partitioned into
   p pieces with some set maximum. This function finds the index for the final
   partition (with respect to all partitions, not just one within a round) for
   a given rank.

   inputs:
     info: information about rounds

   returns:
     total number of partitions created
*/
static IceTInt radixkrGetTotalNumPartitions(const radixkrInfo *info)
{
    IceTInt current_round;
    IceTInt num_partitions;

    num_partitions = 1;
    for (current_round = 0; current_round < info->num_rounds; current_round++) {
        num_partitions *= info->rounds[current_round].split_factor;
    }

    return num_partitions;
}

/* radixkrGetGroupRankForFinalPartitionIndex

   After radix-kr completes on a group of size p, the image is partitioned into
   p pieces. This function finds the group rank for the given index of the
   final partition. This function is the inverse if
   radixkrGetFinalPartitionIndex.

   inputs:
     info: information about rounds
     partition_index: index of final global partition

   returns:
     group rank holding partition_index
*/
static IceTInt radixkrGetGroupRankForFinalPartitionIndex(
        const radixkrInfo *info, IceTInt partition_index)
{
    IceTInt current_round;
    IceTInt partition_up_to_round;
    IceTInt group_rank;

    partition_up_to_round = partition_index;
    group_rank = 0;
    for (current_round = info->num_rounds - 1;
         current_round >= 0;
         current_round--) {
        const IceTInt step = info->rounds[current_round].step;
        const IceTInt split = info->rounds[current_round].split_factor;
        group_rank += step * (partition_up_to_round % split);
        partition_up_to_round /= split;
    }

    return group_rank;
}

/* radixkrGetPartners
   gets the ranks of my trading partners

   inputs:
    round_info: structure with information on the current round
    remaining_partitions: Number of pieces the image will be split into by
        the end of the algorithm.
    compose_group: array of world ranks representing the group of processes
        participating in compositing (passed into icetRadixkrCompose)
    group_rank: Index in compose_group that represents me
    start_size: Size of image partition that is being divided in current_round

   output:
    partner_group: Structure of information about the group of processes that
        are partnering in this round.
*/
static radixkrPartnerGroupInfo radixkrGetPartners(
        const radixkrRoundInfo *round_info,
        IceTInt remaining_partitions,
        const IceTInt *compose_group,
        IceTSizeType start_size)
{
    const IceTInt current_k = round_info->k;
    const IceTInt current_r = round_info->r;
    const IceTInt split_factor = round_info->split_factor;
    const IceTInt step = round_info->step;
    radixkrPartnerGroupInfo p_group;
    IceTInt num_partners;
    IceTBoolean receiving_data;
    IceTBoolean sending_data;
    IceTVoid *recv_buf_pool;
    IceTVoid *send_buf_pool;
    IceTSizeType partition_num_pixels;
    IceTSizeType sparse_image_size;
    IceTInt i;

    num_partners = current_k;
    if (round_info->last_partition) {
        num_partners += current_r;
    }

    p_group.partners = icetGetStateBuffer(
                RADIXKR_PARTITION_INFO_BUFFER,
                sizeof(radixkrPartnerInfo) * num_partners);
    p_group.num_partners = num_partners;

    /* Allocate arrays that can be used as send/receive buffers. */
    receiving_data = round_info->has_image;
    if (split_factor > 1) {
        partition_num_pixels
            = icetSparseImageSplitPartitionNumPixels(start_size,
                                                     split_factor,
                                                     remaining_partitions);
        sending_data = ICET_TRUE;
    } else {
        /* Not really splitting image, and the receiver does not need to send
         * at all. */
        partition_num_pixels = start_size;
        sending_data = !receiving_data;
    }
    sparse_image_size = icetSparseImageBufferSize(partition_num_pixels, 1);
    if (receiving_data) {
        recv_buf_pool = icetGetStateBuffer(RADIXKR_RECEIVE_BUFFER,
                                           sparse_image_size * num_partners);
    } else {
        recv_buf_pool = NULL;
    }
    if (sending_data) {
        /* Only need send buff when splitting, always need when splitting. */
        send_buf_pool = icetGetStateBuffer(RADIXKR_SEND_BUFFER,
                                           sparse_image_size * split_factor);
    } else {
        send_buf_pool = NULL;
    }

    for (i = 0; i < num_partners; i++) {
        radixkrPartnerInfo *p = &p_group.partners[i];
        IceTInt partner_group_rank = round_info->first_rank + i*step;

        p->rank = compose_group[partner_group_rank];

        /* To be filled later. */
        p->offset = -1;

        if (receiving_data) {
            p->receiveBuffer = ((IceTByte*)recv_buf_pool + i*sparse_image_size);
        } else {
            p->receiveBuffer = NULL;
        }

        if (sending_data && (i < split_factor)) {
            IceTVoid *send_buffer
                    = ((IceTByte*)send_buf_pool + i*sparse_image_size);
            p->sendImage = icetSparseImageAssignBuffer(send_buffer,
                                                       partition_num_pixels, 1);
        } else {
            p->sendImage = icetSparseImageNull();
        }

        p->receiveImage = icetSparseImageNull();

        p->compositeLevel = -1;
    }

    return p_group;
}

/* As applicable, posts an asynchronous receive for each process from which
   we are receiving an image piece. */
static IceTCommRequest *radixkrPostReceives(radixkrPartnerGroupInfo p_group,
                                            const radixkrRoundInfo *round_info,
                                            IceTInt current_round,
                                            IceTInt remaining_partitions,
                                            IceTSizeType start_size)
{
    IceTCommRequest *receive_requests;
    IceTSizeType partition_num_pixels;
    IceTSizeType sparse_image_size;
    IceTInt tag;
    IceTInt i;

    /* If not collecting any image partition, post no receives. */
    if (!round_info->has_image) { return NULL; }

    receive_requests =icetGetStateBuffer(
                RADIXKR_RECEIVE_REQUEST_BUFFER,
                p_group.num_partners * sizeof(IceTCommRequest));

    if (round_info->split_factor) {
        partition_num_pixels
            = icetSparseImageSplitPartitionNumPixels(start_size,
                                                     round_info->split_factor,
                                                     remaining_partitions);
    } else {
        partition_num_pixels = start_size;
    }
    sparse_image_size = icetSparseImageBufferSize(partition_num_pixels, 1);

    tag = RADIXKR_SWAP_IMAGE_TAG_START + current_round;

    for (i = 0; i < p_group.num_partners; i++) {
        radixkrPartnerInfo *p = &p_group.partners[i];
        if (i != round_info->partition_index) {
            receive_requests[i] = icetCommIrecv(p->receiveBuffer,
                                                sparse_image_size,
                                                ICET_BYTE,
                                                p->rank,
                                                tag);
            p->compositeLevel = -1;
        } else {
            /* No need to send to myself. */
            receive_requests[i] = ICET_COMM_REQUEST_NULL;
        }
    }

    return receive_requests;
}

/* As applicable, posts an asynchronous send for each process to which we are
   sending an image piece. */
static IceTCommRequest *radixkrPostSends(radixkrPartnerGroupInfo p_group,
                                         const radixkrRoundInfo *round_info,
                                         IceTInt current_round,
                                         IceTInt remaining_partitions,
                                         IceTSizeType start_offset,
                                         const IceTSparseImage image)
{
    IceTCommRequest *send_requests;
    IceTInt *piece_offsets;
    IceTSparseImage *image_pieces;
    IceTInt tag;
    IceTInt i;

    tag = RADIXKR_SWAP_IMAGE_TAG_START + current_round;

    if (round_info->split_factor > 1) {
        send_requests=icetGetStateBuffer(
                    RADIXKR_SEND_REQUEST_BUFFER,
                    round_info->split_factor * sizeof(IceTCommRequest));

        piece_offsets = icetGetStateBuffer(
                    RADIXKR_SPLIT_OFFSET_ARRAY_BUFFER,
                    round_info->split_factor * sizeof(IceTInt));
        image_pieces = icetGetStateBuffer(
                    RADIXKR_SPLIT_IMAGE_ARRAY_BUFFER,
                    round_info->split_factor * sizeof(IceTSparseImage));
        for (i = 0; i < round_info->split_factor; i++) {
            image_pieces[i] = p_group.partners[i].sendImage;
        }
        icetSparseImageSplit(image,
                             start_offset,
                             round_info->split_factor,
                             remaining_partitions,
                             image_pieces,
                             piece_offsets);

        /* The pivot for loop arranges the sends to happen in an order such that
           those to be composited first in their destinations will be sent
           first.  This serves little purpose other than to try to stagger the
           order of sending images so that not everyone sends to the same
           process first. */
        BEGIN_PIVOT_FOR(i,
                        0,
                        round_info->partition_index % round_info->split_factor,
                        round_info->split_factor) {
            radixkrPartnerInfo *p = &p_group.partners[i];
            p->offset = piece_offsets[i];
            if (i != round_info->partition_index) {
                IceTVoid *package_buffer;
                IceTSizeType package_size;

                icetSparseImagePackageForSend(image_pieces[i],
                                              &package_buffer, &package_size);

                send_requests[i] = icetCommIsend(package_buffer,
                                                 package_size,
                                                 ICET_BYTE,
                                                 p->rank,
                                                 tag);
            } else {
                /* Implicitly send to myself. */
                send_requests[i] = ICET_COMM_REQUEST_NULL;
                p->receiveImage = p->sendImage;
                p->compositeLevel = 0;
            }
        } END_PIVOT_FOR();
    } else { /* round_info->split_factor == 1 */
        radixkrPartnerInfo *p = &p_group.partners[round_info->partition_index];
        send_requests = icetGetStateBuffer(RADIXKR_SEND_REQUEST_BUFFER,
                                           sizeof(IceTCommRequest));
        if (round_info->has_image) {
            send_requests[0] = ICET_COMM_REQUEST_NULL;
            p->receiveImage = p->sendImage = image;
            p->offset = start_offset;
            p->compositeLevel = 0;
        } else {
            IceTVoid *package_buffer;
            IceTSizeType package_size;
            IceTInt recv_rank = p_group.partners[0].rank;

            icetSparseImagePackageForSend(image,&package_buffer,&package_size);

            send_requests[0] = icetCommIsend(package_buffer,
                                             package_size,
                                             ICET_BYTE,
                                             recv_rank,
                                             tag);

            p->offset = 0;
        }
    }

    return send_requests;
}

/* When compositing incoming images, we pair up the images and composite in
   a tree.  This minimizes the amount of times non-overlapping pixels need
   to be copied.  Returns true when all images are composited */
static IceTBoolean radixkrTryCompositeIncoming(
        radixkrPartnerGroupInfo p_group,
        IceTInt incoming_index,
        IceTSparseImage *spare_image_p,
        IceTSparseImage final_image)
{
    const IceTInt num_partners = p_group.num_partners;
    radixkrPartnerInfo *partners = p_group.partners;
    IceTSparseImage spare_image = *spare_image_p;
    IceTInt to_composite_index = incoming_index;

    while (ICET_TRUE) {
        IceTInt level = partners[to_composite_index].compositeLevel;
        IceTInt dist_to_sibling = (1 << level);
        IceTInt subtree_size = (dist_to_sibling << 1);
        IceTInt front_index;
        IceTInt back_index;

        if (to_composite_index%subtree_size == 0) {
            front_index = to_composite_index;
            back_index = to_composite_index + dist_to_sibling;

            if (back_index >= num_partners) {
                /* This image has no partner at this level.  Just promote
                   the level and continue. */
                if (front_index == 0) {
                    /* Special case.  When index 0 has no partner, we must
                       be at the top of the tree and we are done. */
                    break;
                }
                partners[to_composite_index].compositeLevel++;
                continue;
            }
        } else {
            back_index = to_composite_index;
            front_index = to_composite_index - dist_to_sibling;
        }

        if (   partners[front_index].compositeLevel
            != partners[back_index].compositeLevel ) {
            /* Paired images are not on the same level.  Cannot composite
               until more images come in.  We are done for now. */
            break;
        }

        if ((front_index == 0) && (subtree_size >= num_partners)) {
            /* This will be the last image composited.  Composite to final
               location. */
            spare_image = final_image;
        }
        icetCompressedCompressedComposite(partners[front_index].receiveImage,
                                          partners[back_index].receiveImage,
                                          spare_image);
        radixkrSwapImages(&partners[front_index].receiveImage, &spare_image);
        if (icetSparseImageEqual(spare_image, final_image)) {
            /* Special case, front image was sharing buffer with final.
               Use back image for next spare. */
            spare_image = partners[back_index].receiveImage;
            partners[back_index].receiveImage = icetSparseImageNull();
        }
        partners[front_index].compositeLevel++;
        to_composite_index = front_index;
    }

    *spare_image_p = spare_image;
    return ((1 << partners[0].compositeLevel) >= num_partners);
}

static void radixkrCompositeIncomingImages(radixkrPartnerGroupInfo p_group,
                                          IceTCommRequest *receive_requests,
                                          const radixkrRoundInfo *round_info,
                                          IceTSparseImage image)
{
    radixkrPartnerInfo *partners = p_group.partners;
    IceTInt num_partners = p_group.num_partners;
    radixkrPartnerInfo *me = &partners[round_info->partition_index];

    IceTSparseImage spare_image;
    IceTInt total_composites;

    IceTSizeType width;
    IceTSizeType height;

    IceTBoolean composites_done;

    /* If not receiving an image, return right away. */
    if (!round_info->has_image) {
        return;
    }

    /* Regardless of order, there are num_partners-1 composite operations to
       perform. */
    total_composites = num_partners - 1;

    /* We will be reusing buffers like crazy, but we'll need at least one more
       for the first composite, assuming we have at least two composites. */
    width = icetSparseImageGetWidth(me->receiveImage);
    height = icetSparseImageGetHeight(me->receiveImage);
    if (total_composites >= 2) {
        spare_image = icetGetStateBufferSparseImage(RADIXKR_SPARE_BUFFER,
                                                    width,
                                                    height);
    } else {
        spare_image = icetSparseImageNull();
    }

    /* Grumble.  Stupid special case where there is only one composite and we
       want the result to go in the same image as my receive image (which can
       happen when not splitting). */
    if (icetSparseImageEqual(me->receiveImage,image) && (total_composites < 2)){
        spare_image = icetGetStateBufferSparseImage(RADIXKR_SPARE_BUFFER,
                                                    width,
                                                    height);
        icetSparseImageCopyPixels(me->receiveImage,
                                  0,
                                  width*height,
                                  spare_image);
        me->receiveImage = spare_image;
    }

    /* Start by trying to composite the implicit receive from myself.  It won't
       actually composite anything, but it may change the composite level.  It
       will also defensively set composites_done correctly. */
    composites_done = radixkrTryCompositeIncoming(p_group,
                                                  round_info->partition_index,
                                                  &spare_image,
                                                  image);

    while (!composites_done) {
        IceTInt receive_idx;
        radixkrPartnerInfo *receiver;

        /* Wait for an image to come in. */
        receive_idx = icetCommWaitany(num_partners, receive_requests);
        receiver = &partners[receive_idx];
        receiver->compositeLevel = 0;
        receiver->receiveImage
            = icetSparseImageUnpackageFromReceive(receiver->receiveBuffer);
        if (   (icetSparseImageGetWidth(receiver->receiveImage) != width)
            || (icetSparseImageGetHeight(receiver->receiveImage) != height) ) {
            icetRaiseError("Radix-kr received image with wrong size.",
                           ICET_SANITY_CHECK_FAIL);
        }

        /* Try to composite that image. */
        composites_done = radixkrTryCompositeIncoming(p_group,
                                                      receive_idx,
                                                      &spare_image,
                                                      image);
    }
}

void icetRadixkrCompose(const IceTInt *compose_group,
                       IceTInt group_size,
                       IceTInt image_dest,
                       IceTSparseImage input_image,
                       IceTSparseImage *result_image,
                       IceTSizeType *piece_offset)
{
    radixkrInfo info = { NULL, 0 };

    IceTSizeType my_offset;
    IceTInt current_round;
    IceTInt total_num_partitions;
    IceTInt remaining_partitions;
    IceTInt group_rank;
    IceTBoolean use_interlace;
    IceTSparseImage working_image = input_image;
    IceTSizeType original_image_size = icetSparseImageGetNumPixels(input_image);

    /* This hint of an argument is ignored. */
    (void)image_dest;

    icetRaiseDebug("In radix-kr compose");

    /* Find your rank in your group. */
    group_rank = icetFindMyRankInGroup(compose_group, group_size);
    if (group_rank < 0) {
        icetRaiseError("Local process not in compose_group?",
                       ICET_SANITY_CHECK_FAIL);
        *result_image = icetSparseImageNull();
        *piece_offset = 0;
        return;
    }

    if (group_size == 1) {
        /* I am the only process in the group.  No compositing to be done.
         * Just return and the image will be complete. */
        *result_image = input_image;
        *piece_offset = 0;
        return;
    }

    info = radixkrGetK(group_size, group_rank);

    /* num_rounds > 0 is assumed several places throughout this function */
    if (info.num_rounds <= 0) {
        icetRaiseError("Radix-kr has no rounds?", ICET_SANITY_CHECK_FAIL);
    }

    /* IceT does some tricks with partitioning an image so that no matter what
       order we do the divisions, we end up with the same partitions. To do
       that, we need to know how many divisions are going to be created in
       all. */
    total_num_partitions = radixkrGetTotalNumPartitions(&info);

    /* Now that we know the total number of partitions, we can interlace the
       input image to improve load balancing. */
    use_interlace = icetIsEnabled(ICET_INTERLACE_IMAGES);
    use_interlace &= (info.num_rounds > 1);

    if (use_interlace) {
        IceTSparseImage interlaced_image = icetGetStateBufferSparseImage(
                                       RADIXKR_INTERLACED_IMAGE_BUFFER,
                                       icetSparseImageGetWidth(working_image),
                                       icetSparseImageGetHeight(working_image));
        icetSparseImageInterlace(working_image,
                                 total_num_partitions,
                                 RADIXKR_SPLIT_OFFSET_ARRAY_BUFFER,
                                 interlaced_image);
        working_image = interlaced_image;
    }

    /* Any peer we communicate with in round i starts that round with a block of
       the same size as ours prior to splitting for sends/recvs.  So we can
       calculate the current round's peer sizes based on our current size and
       the split values in the info structure. */
    my_offset = 0;
    remaining_partitions = total_num_partitions;

    for (current_round = 0; current_round < info.num_rounds; current_round++) {
        IceTSizeType my_size = icetSparseImageGetNumPixels(working_image);
        const radixkrRoundInfo *round_info = &info.rounds[current_round];
        radixkrPartnerGroupInfo p_group
                = radixkrGetPartners(round_info,
                                     remaining_partitions,
                                     compose_group,
                                     my_size);
        IceTCommRequest *receive_requests;
        IceTCommRequest *send_requests;

        receive_requests = radixkrPostReceives(p_group,
                                               round_info,
                                               current_round,
                                               remaining_partitions,
                                               my_size);

        send_requests = radixkrPostSends(p_group,
                                         round_info,
                                         current_round,
                                         remaining_partitions,
                                         my_offset,
                                         working_image);

        radixkrCompositeIncomingImages(p_group,
                                       receive_requests,
                                       round_info,
                                       working_image);

        icetCommWaitall(round_info->split_factor, send_requests);

        my_offset = p_group.partners[round_info->partition_index].offset;
        if (round_info->has_image) {
            remaining_partitions /= round_info->split_factor;
        } else {
            icetSparseImageSetDimensions(working_image, 0, 0);
            break;
        }
    } /* for all rounds */

    /* If we interlaced the image and are actually returning something,
       correct the offset. */
    if (use_interlace && (icetSparseImageGetNumPixels(working_image) > 0)) {
        IceTInt partition_index = radixkrGetFinalPartitionIndex(&info);
        *piece_offset = icetGetInterlaceOffset(partition_index,
                                               total_num_partitions,
                                               original_image_size);
    } else {
        *piece_offset = my_offset;
    }

    *result_image = working_image;

    return;
}


static IceTBoolean radixkrTryPartitionLookup(IceTInt group_size)
{
    IceTInt *partition_assignments;
    IceTInt group_rank;
    IceTInt partition_index;
    IceTInt num_partitions;

    partition_assignments = malloc(group_size * sizeof(IceTInt));
    for (partition_index = 0;
         partition_index < group_size;
         partition_index++) {
        partition_assignments[partition_index] = -1;
    }

    num_partitions = 0;
    for (group_rank = 0; group_rank < group_size; group_rank++) {
        radixkrInfo info;
        IceTInt rank_assignment;

        info = radixkrGetK(group_size, group_rank);
        partition_index = radixkrGetFinalPartitionIndex(&info);
        /* Check if this rank has no partition. */
        if (partition_index < 0) { continue; }
        num_partitions++;
        if (group_size <= partition_index) {
            printf("Invalid partition for rank %d.  Got partition %d.\n",
                   group_rank, partition_index);
            return ICET_FALSE;
        }
        if (partition_assignments[partition_index] != -1) {
            printf("Both ranks %d and %d report assigned partition %d.\n",
                   group_rank,
                   partition_assignments[partition_index],
                   partition_index);
            return ICET_FALSE;
        }
        partition_assignments[partition_index] = group_rank;

        rank_assignment
            = radixkrGetGroupRankForFinalPartitionIndex(&info, partition_index);
        if (rank_assignment != group_rank) {
            printf("Rank %d reports partition %d, but partition reports rank %d.\n",
                   group_rank, partition_index, rank_assignment);
            return ICET_FALSE;
        }
    }

    {
        radixkrInfo info;
        info = radixkrGetK(group_size, 0);
        if (num_partitions != radixkrGetTotalNumPartitions(&info)) {
            printf("Expected %d partitions, found %d\n",
                   radixkrGetTotalNumPartitions(&info),
                   num_partitions);
            return ICET_FALSE;
        }
    }

    {
        IceTInt max_image_split;

        icetGetIntegerv(ICET_MAX_IMAGE_SPLIT, &max_image_split);
        if (num_partitions > max_image_split) {
            printf("Got %d partitions.  Expected no more than %d\n",
                   num_partitions, max_image_split);
            return ICET_FALSE;
        }
    }

    free(partition_assignments);

    return ICET_TRUE;
}

ICET_EXPORT IceTBoolean icetRadixkrPartitionLookupUnitTest(void)
{
    const IceTInt group_sizes_to_try[] = {
        2,                              /* Base case. */
        ICET_MAGIC_K_DEFAULT,           /* Largest direct send. */
        ICET_MAGIC_K_DEFAULT*2,         /* Changing group sizes. */
        ICET_MAGIC_K_DEFAULT*ICET_MAGIC_K_DEFAULT*37,
                                        /* Odd factor past some rounds. */
        1024,                           /* Large(ish) power of two. */
        576,                            /* Factors into 2 and 3. */
        509                             /* Prime. */
    };
    const IceTInt num_group_sizes_to_try
        = sizeof(group_sizes_to_try)/sizeof(IceTInt);
    IceTInt group_size_index;

    printf("\nTesting rank/partition mapping.\n");

    for (group_size_index = 0;
         group_size_index < num_group_sizes_to_try;
         group_size_index++) {
        IceTInt group_size = group_sizes_to_try[group_size_index];
        IceTInt max_image_split;

        printf("Trying size %d\n", group_size);

        for (max_image_split = 1;
             max_image_split/2 < group_size;
             max_image_split *= 2) {
            icetStateSetInteger(ICET_MAX_IMAGE_SPLIT, max_image_split);

            printf("  Maximum num splits set to %d\n", max_image_split);

            if (!radixkrTryPartitionLookup(group_size)) {
                return ICET_FALSE;
            }
        }
    }

    return ICET_TRUE;
}
