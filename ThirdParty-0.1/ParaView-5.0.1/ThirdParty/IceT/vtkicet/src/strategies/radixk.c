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

/* #define RADIXK_USE_TELESCOPE */

#define RADIXK_SWAP_IMAGE_TAG_START     2200
#define RADIXK_TELESCOPE_IMAGE_TAG      2300

#define RADIXK_RECEIVE_BUFFER                   ICET_SI_STRATEGY_BUFFER_0
#define RADIXK_SEND_BUFFER                      ICET_SI_STRATEGY_BUFFER_1
#define RADIXK_SPARE_BUFFER                     ICET_SI_STRATEGY_BUFFER_2
#define RADIXK_INTERLACED_IMAGE_BUFFER          ICET_SI_STRATEGY_BUFFER_3
#define RADIXK_PARTITION_INFO_BUFFER            ICET_SI_STRATEGY_BUFFER_4
#define RADIXK_RECEIVE_REQUEST_BUFFER           ICET_SI_STRATEGY_BUFFER_5
#define RADIXK_SEND_REQUEST_BUFFER              ICET_SI_STRATEGY_BUFFER_6
#define RADIXK_FACTORS_ARRAY_BUFFER             ICET_SI_STRATEGY_BUFFER_7
#define RADIXK_SPLIT_OFFSET_ARRAY_BUFFER        ICET_SI_STRATEGY_BUFFER_8
#define RADIXK_SPLIT_IMAGE_ARRAY_BUFFER         ICET_SI_STRATEGY_BUFFER_9
#define RADIXK_RANK_LIST_BUFFER                 ICET_SI_STRATEGY_BUFFER_10

typedef struct radixkRoundInfoStruct {
    IceTInt k; /* k value for this round. */
    IceTInt step; /* Ranks jump by this much in this round. */
    IceTBoolean split; /* True if image should be split and divided. */
    IceTBoolean has_image; /* True if local process collects image data this round. */
    IceTInt partition_index; /* Index of partition at this round (if has_image true). */
} radixkRoundInfo;

typedef struct radixkInfoStruct {
    radixkRoundInfo *rounds; /* Array of per round info. */
    IceTInt num_rounds;
} radixkInfo;

typedef struct radixkPartnerInfoStruct {
    IceTInt rank; /* Rank of partner. */
    IceTSizeType offset; /* Offset of partner's partition in image. */
    IceTVoid *receiveBuffer; /* A buffer for receiving data from partner. */
    IceTSparseImage sendImage; /* A buffer to hold data being sent to partner */
    IceTSparseImage receiveImage; /* Hold for received non-composited image. */
    IceTInt compositeLevel; /* Level in compositing tree for round. */
} radixkPartnerInfo;

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

#ifdef RADIXK_USE_TELESCOPE
/* Finds the largest power of 2 equal to or smaller than x. */
static IceTInt radixkFindPower2(IceTInt x)
{
    IceTInt pow2;
    for (pow2 = 1; pow2 <= x; pow2 = pow2 << 1);
    pow2 = pow2 >> 1;
    return pow2;
}
#endif

static IceTInt radixkFindFloorPow2(IceTInt x)
{
    IceTInt lg;
    for (lg = 0; (IceTUInt)(1 << lg) <= (IceTUInt)x; lg++);
    lg--;
    return lg;
}

static void radixkSwapImages(IceTSparseImage *image1, IceTSparseImage *image2)
{
    IceTSparseImage old_image1 = *image1;
    *image1 = *image2;
    *image2 = old_image1;
}

/* radixkGetPartitionIndices

   my position in each round forms an num_rounds-dimensional vector
   [round 0 pos, round 1 pos, ... round num_rounds-1 pos]
   where pos is my position in the group of partners within that round

   inputs:
     info: holds the number of rounds and k values for each round
     group_rank: my rank in composite order (compose_group in icetRadixkCompose)

   outputs:
     fills info with split, has_image, and partition_index for each round.
*/
static void radixkGetPartitionIndices(radixkInfo info,
                                      IceTInt group_rank)
{

    IceTInt step; /* step size in rank for a lattice direction */
    IceTInt total_partitions;
    IceTInt current_round;
    IceTInt max_image_split;

    icetGetIntegerv(ICET_MAX_IMAGE_SPLIT, &max_image_split);

    total_partitions = 1;
    step = 1;
    current_round = 0;
    while (current_round < info.num_rounds) {
        radixkRoundInfo *round_info = &info.rounds[current_round];
        IceTInt next_total_partitions = total_partitions * round_info->k;

        if (max_image_split < next_total_partitions) { break; }

        total_partitions = next_total_partitions;
        round_info->split = ICET_TRUE;
        round_info->has_image = ICET_TRUE;
        round_info->partition_index = (group_rank / step) % round_info->k;
        round_info->step = step;
        step *= round_info->k;

        current_round++;
    }
    while (current_round < info.num_rounds) {
        radixkRoundInfo *round_info = &info.rounds[current_round];
        IceTInt next_step = step * round_info->k;

        round_info->split = ICET_FALSE;
        round_info->partition_index = (group_rank / step) % round_info->k;
        round_info->has_image = (round_info->partition_index == 0);
        round_info->step = step;
        step = next_step;

        current_round++;
    }

}

static radixkInfo radixkGetK(IceTInt compose_group_size,
                             IceTInt group_rank)
{
    /* Divide the world size into groups that are closest to the magic k
       value. */
    radixkInfo info;
    IceTInt magic_k;
    IceTInt max_num_k;
    IceTInt next_divide;

    /* Special case of when compose_group_size == 1. */
    if (compose_group_size < 2) {
        info.rounds = icetGetStateBuffer(RADIXK_FACTORS_ARRAY_BUFFER,
                                         sizeof(radixkRoundInfo) * 1);
        info.rounds[0].k = 1;
        info.rounds[0].split = ICET_TRUE;
        info.rounds[0].has_image = ICET_TRUE;
        info.rounds[0].partition_index = 0;
        info.num_rounds = 1;
        return info;
    }

    info.num_rounds = 0;

    icetGetIntegerv(ICET_MAGIC_K, &magic_k);

    /* The maximum number of factors possible is the floor of log base 2. */
    max_num_k = radixkFindFloorPow2(compose_group_size);
    info.rounds = icetGetStateBuffer(RADIXK_FACTORS_ARRAY_BUFFER,
                                     sizeof(radixkRoundInfo) * max_num_k);

    next_divide = compose_group_size;
    while (next_divide > 1) {
        IceTInt next_k = -1;

        /* If the magic k value is perfectly divisible by the next_divide
           size, we are good to go */
        if ((next_divide % magic_k) == 0) {
            next_k = magic_k;
        }

        /* If that does not work, look for a factor near the magic_k. */
        if (next_k == -1) {
            /* All these unsigned ints just get around a compiler warning
               about a compiler optimization that assumes no signed overflow.
               Yes, I am aware that all numbers will be positive. */
            IceTUInt try_k;
            BEGIN_PIVOT_FOR(try_k, 2, (IceTUInt)magic_k, 2*(IceTUInt)magic_k) {
                if ((next_divide % try_k) == 0) {
                    next_k = try_k;
                    break;
                }
            } END_PIVOT_FOR()
        }

        /* If you STILL don't have a good factor, progress upwards to find the
           best match. */
        if (next_k == -1) {
            IceTInt try_k;
            IceTInt max_k;

            /* The largest possible smallest factor (other than next_divide
               itself) is the square root of next_divide.  We don't have to
               check the values between the square root and next_divide. */
            max_k = (IceTInt)floor(sqrt(next_divide));

            /* It would be better to just visit prime numbers, but other than
               having a huge table, how would you do that?  Hopefully this is an
               uncommon use case. */
            for (try_k = 2*magic_k; try_k < max_k; try_k++) {
                if ((next_divide % try_k) == 0) {
                    next_k = try_k;
                    break;
                }
            }
        }

        /* If we STILL don't have a factor, then next_division must be a large
           prime.  Basically give up by using next_divide as the next k. */
        if (next_k == -1) {
            next_k = next_divide;
        }

        /* Set the k value in the array. */
        info.rounds[info.num_rounds].k = next_k;
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
        IceTInt product = info.rounds[0].k;
        IceTInt i;
        for (i = 1; i < info.num_rounds; ++i) {
            product *= info.rounds[i].k;
        }
        if (product != compose_group_size) {
            icetRaiseError("Product of k's not equal to number of processes.",
                           ICET_SANITY_CHECK_FAIL);
        }
    }

    radixkGetPartitionIndices(info, group_rank);

    return info;
}

/* radixkGetFinalPartitionIndex

   After radix-k completes on a group of size p, the image is partitioned into p
   pieces (with a caveat for the maximum number of partitions).  This function
   finds the index for the final partition (with respect to all partitions, not
   just one within a round) for a given rank.

   inputs:
     info: information about rounds

   returns:
     index of final global partition.  -1 if this process does not end up with
     a piece.
*/
static IceTInt radixkGetFinalPartitionIndex(const radixkInfo *info)
{
    IceTInt current_round;
    IceTInt partition_index;

    partition_index = 0;
    for (current_round = 0; current_round < info->num_rounds; current_round++) {
        const radixkRoundInfo *r = &info->rounds[current_round];
        if (r->split) {
            partition_index *= r->k;
            partition_index += r->partition_index;
        } else if (r->has_image) {
            /* partition_index does not change */
        } else {
            /* local process has no partition */
            return -1;
        }
    }

    return partition_index;
}

/* radixkGetTotalNumPartitions

   After radix-k completes on a group of size p, the image is partitioned into p
   pieces with some set maximum.  This function finds the index for the final
   partition (with respect to all partitions, not just one within a round) for a
   given rank.

   inputs:
     info: information about rounds

   returns:
     total number of partitions created
*/
static IceTInt radixkGetTotalNumPartitions(const radixkInfo *info)
{
    IceTInt current_round;
    IceTInt num_partitions;

    num_partitions = 1;
    for (current_round = 0; current_round < info->num_rounds; current_round++) {
        if (info->rounds[current_round].split) {
            num_partitions *= info->rounds[current_round].k;
        }
    }

    return num_partitions;
}

/* radixkGetGroupRankForFinalPartitionIndex

   After radix-k completes on a group of size p, the image is partitioned into p
   pieces.  This function finds the group rank for the given index of the final
   partition.  This function is the inverse if radixkGetFinalPartitionIndex.

   inputs:
     info: information about rounds
     partition_index: index of final global partition

   returns:
     group rank holding partition_index
*/
static IceTInt radixkGetGroupRankForFinalPartitionIndex(const radixkInfo *info,
                                                        IceTInt partition_index)
{
    IceTInt current_round;
    IceTInt partition_up_to_round;
    IceTInt group_rank;

    partition_up_to_round = partition_index;
    group_rank = 0;
    for (current_round = info->num_rounds - 1;
         current_round >= 0;
         current_round--) {
        if (info->rounds[current_round].split) {
            const IceTInt step = info->rounds[current_round].step;
            const IceTInt k_value = info->rounds[current_round].k;
            group_rank += step * (partition_up_to_round % k_value);
            partition_up_to_round /= k_value;
        } else {
            /* Partition rank does not change when not splitting. */
        }
    }

    return group_rank;
}

/* radixkGetPartners
   gets the ranks of my trading partners

   inputs:
    k_array: vector of k values
    current_round: current round number (0 to num_rounds - 1)
    partition_index: image partition to collect (0 to k[current_round] - 1)
    remaining_partitions: Number of pieces the image will be split into by
        the end of the algorithm.
    compose_group: array of world ranks representing the group of processes
        participating in compositing (passed into icetRadixkCompose)
    group_rank: Index in compose_group that represents me
    start_offset: Start of partition that is being divided in current_round
    start_size: Size of partition that is being divided in current_round

   output:
    partners: Array of radixkPartnerInfo describing all the processes
        participating in this round.
*/
static radixkPartnerInfo *radixkGetPartners(const radixkRoundInfo *round_info,
                                            IceTInt remaining_partitions,
                                            const IceTInt *compose_group,
                                            IceTInt group_rank,
                                            IceTSizeType start_size)
{
    const IceTInt current_k = round_info->k;
    const IceTInt step = round_info->step;
    radixkPartnerInfo *partners;
    IceTBoolean receiving_data;
    IceTBoolean sending_data;
    IceTVoid *recv_buf_pool;
    IceTVoid *send_buf_pool;
    IceTSizeType partition_num_pixels;
    IceTSizeType sparse_image_size;
    IceTInt first_partner_group_rank;
    IceTInt i;

    partners = icetGetStateBuffer(RADIXK_PARTITION_INFO_BUFFER,
                                  sizeof(radixkPartnerInfo) * current_k);

    /* Allocate arrays that can be used as send/receive buffers. */
    receiving_data = round_info->has_image;
    if (round_info->split) {
        partition_num_pixels
            = icetSparseImageSplitPartitionNumPixels(start_size,
                                                     current_k,
                                                     remaining_partitions);
        sending_data = ICET_TRUE;
    } else {
        partition_num_pixels = start_size;
        sending_data = !receiving_data;
    }
    sparse_image_size = icetSparseImageBufferSize(partition_num_pixels, 1);
    if (receiving_data) {
        recv_buf_pool = icetGetStateBuffer(RADIXK_RECEIVE_BUFFER,
                                           sparse_image_size * current_k);
    } else {
        recv_buf_pool = NULL;
    }
    if (round_info->split) {
        /* Only need send buff when splitting, always need when splitting. */
        send_buf_pool = icetGetStateBuffer(RADIXK_SEND_BUFFER,
                                           sparse_image_size * current_k);
    } else {
        send_buf_pool = NULL;
    }

    first_partner_group_rank
        = group_rank % step + (group_rank/(step*current_k))*(step*current_k);
    for (i = 0; i < current_k; i++) {
        radixkPartnerInfo *p = &partners[i];
        IceTInt partner_group_rank = first_partner_group_rank + i*step;
        IceTVoid *send_buffer;

        p->rank = compose_group[partner_group_rank];

        /* To be filled later. */
        p->offset = -1;

        if (receiving_data) {
            p->receiveBuffer = ((IceTByte*)recv_buf_pool + i*sparse_image_size);
        } else {
            p->receiveBuffer = NULL;
        }

        if (round_info->split) {
            /* Only need send buff when splitting, always need when splitting.*/
            send_buffer = ((IceTByte*)send_buf_pool + i*sparse_image_size);
            p->sendImage = icetSparseImageAssignBuffer(send_buffer,
                                                       partition_num_pixels, 1);
        } else {
            send_buffer = NULL;
            p->sendImage = icetSparseImageNull();
        }

        p->receiveImage = icetSparseImageNull();

        p->compositeLevel = -1;
    }

    return partners;
}

/* As applicable, posts an asynchronous receive for each process from which
   we are receiving an image piece. */
static IceTCommRequest *radixkPostReceives(radixkPartnerInfo *partners,
                                           const radixkRoundInfo *round_info,
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

    receive_requests =icetGetStateBuffer(RADIXK_RECEIVE_REQUEST_BUFFER,
                                         round_info->k*sizeof(IceTCommRequest));

    if (round_info->split) {
        partition_num_pixels
            = icetSparseImageSplitPartitionNumPixels(start_size,
                                                     round_info->k,
                                                     remaining_partitions);
    } else {
        partition_num_pixels = start_size;
    }
    sparse_image_size = icetSparseImageBufferSize(partition_num_pixels, 1);

    tag = RADIXK_SWAP_IMAGE_TAG_START + current_round;

    for (i = 0; i < round_info->k; i++) {
        radixkPartnerInfo *p = &partners[i];
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
static IceTCommRequest *radixkPostSends(radixkPartnerInfo *partners,
                                        const radixkRoundInfo *round_info,
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

    tag = RADIXK_SWAP_IMAGE_TAG_START + current_round;

    if (round_info->split) {
        send_requests=icetGetStateBuffer(RADIXK_SEND_REQUEST_BUFFER,
                                         round_info->k*sizeof(IceTCommRequest));

        piece_offsets = icetGetStateBuffer(RADIXK_SPLIT_OFFSET_ARRAY_BUFFER,
                                           round_info->k * sizeof(IceTInt));
        image_pieces =icetGetStateBuffer(RADIXK_SPLIT_IMAGE_ARRAY_BUFFER,
                                         round_info->k*sizeof(IceTSparseImage));
        for (i = 0; i < round_info->k; i++) {
            image_pieces[i] = partners[i].sendImage;
        }
        icetSparseImageSplit(image,
                             start_offset,
                             round_info->k,
                             remaining_partitions,
                             image_pieces,
                             piece_offsets);

        /* The pivot for loop arranges the sends to happen in an order such that
           those to be composited first in their destinations will be sent
           first.  This serves little purpose other than to try to stagger the
           order of sending images so that no everyone sends to the same process
           first. */
        BEGIN_PIVOT_FOR(i, 0, round_info->partition_index, round_info->k) {
            radixkPartnerInfo *p = &partners[i];
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
    } else { /* !round_info->split */
        radixkPartnerInfo *p = &partners[round_info->partition_index];
        send_requests = icetGetStateBuffer(RADIXK_SEND_REQUEST_BUFFER,
                                           sizeof(IceTCommRequest));
        if (round_info->has_image) {
            send_requests[0] = ICET_COMM_REQUEST_NULL;
            p->receiveImage = p->sendImage = image;
            p->offset = start_offset;
            p->compositeLevel = 0;
        } else {
            IceTVoid *package_buffer;
            IceTSizeType package_size;
            IceTInt recv_rank = partners[0].rank;

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
static IceTBoolean radixkTryCompositeIncoming(radixkPartnerInfo *partners,
                                              const radixkRoundInfo *round_info,
                                              IceTInt incoming_index,
                                              IceTSparseImage *spare_image_p,
                                              IceTSparseImage final_image)
{
    const IceTInt current_k = round_info->k;
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

            if (back_index >= current_k) {
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

        if ((front_index == 0) && (subtree_size >= current_k)) {
            /* This will be the last image composited.  Composite to final
               location. */
            spare_image = final_image;
        }
        icetCompressedCompressedComposite(partners[front_index].receiveImage,
                                          partners[back_index].receiveImage,
                                          spare_image);
        radixkSwapImages(&partners[front_index].receiveImage, &spare_image);
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
    return ((1 << partners[0].compositeLevel) >= current_k);
}

static void radixkCompositeIncomingImages(radixkPartnerInfo *partners,
                                          IceTCommRequest *receive_requests,
                                          const radixkRoundInfo *round_info,
                                          IceTSparseImage image)
{
    radixkPartnerInfo *me = &partners[round_info->partition_index];

    IceTSparseImage spare_image;
    IceTInt total_composites;

    IceTSizeType width;
    IceTSizeType height;

    IceTBoolean composites_done;

    /* If not receiving an image, return right away. */
    if ((!round_info->split) && (!round_info->has_image)) {
        return;
    }

    /* Regardless of order, there are k-1 composite operations to perform. */
    total_composites = round_info->k - 1;

    /* We will be reusing buffers like crazy, but we'll need at least one more
       for the first composite, assuming we have at least two composites. */
    width = icetSparseImageGetWidth(me->receiveImage);
    height = icetSparseImageGetHeight(me->receiveImage);
    if (total_composites >= 2) {
        spare_image = icetGetStateBufferSparseImage(RADIXK_SPARE_BUFFER,
                                                    width,
                                                    height);
    } else {
        spare_image = icetSparseImageNull();
    }

    /* Grumble.  Stupid special case where there is only one composite and we
       want the result to go in the same image as my receive image (which can
       happen when not splitting). */
    if (icetSparseImageEqual(me->receiveImage,image) && (total_composites < 2)){
        spare_image = icetGetStateBufferSparseImage(RADIXK_SPARE_BUFFER,
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
    composites_done = radixkTryCompositeIncoming(partners,
                                                 round_info,
                                                 round_info->partition_index,
                                                 &spare_image,
                                                 image);

    while (!composites_done) {
        IceTInt receive_idx;
        radixkPartnerInfo *receiver;

        /* Wait for an image to come in. */
        receive_idx = icetCommWaitany(round_info->k, receive_requests);
        receiver = &partners[receive_idx];
        receiver->compositeLevel = 0;
        receiver->receiveImage
            = icetSparseImageUnpackageFromReceive(receiver->receiveBuffer);
        if (   (icetSparseImageGetWidth(receiver->receiveImage) != width)
            || (icetSparseImageGetHeight(receiver->receiveImage) != height) ) {
            icetRaiseError("Radix-k received image with wrong size.",
                           ICET_SANITY_CHECK_FAIL);
        }

        /* Try to composite that image. */
        composites_done = radixkTryCompositeIncoming(partners,
                                                     round_info,
                                                     receive_idx,
                                                     &spare_image,
                                                     image);
    }
}

static void icetRadixkBasicCompose(const radixkInfo *info,
                                   const IceTInt *compose_group,
                                   IceTInt group_size,
                                   IceTInt total_num_partitions,
                                   IceTSparseImage working_image,
                                   IceTSizeType *piece_offset)
{
    IceTSizeType my_offset;
    IceTInt current_round;
    IceTInt remaining_partitions;

    /* Find your rank in your group. */
    IceTInt group_rank = icetFindMyRankInGroup(compose_group, group_size);
    if (group_rank < 0) {
        icetRaiseError("Local process not in compose_group?",
                       ICET_SANITY_CHECK_FAIL);
        *piece_offset = 0;
        return;
    }

    if (group_size == 1) {
        /* I am the only process in the group.  No compositing to be done.
         * Just return and the image will be complete. */
        *piece_offset = 0;
        return;
    }

    /* num_rounds > 0 is assumed several places throughout this function */
    if (info->num_rounds <= 0) {
        icetRaiseError("Radix-k has no rounds?", ICET_SANITY_CHECK_FAIL);
    }

    /* Any peer we communicate with in round i starts that round with a block of
       the same size as ours prior to splitting for sends/recvs.  So we can
       calculate the current round's peer sizes based on our current size and
       the k_array[i] info. */
    my_offset = 0;
    remaining_partitions = total_num_partitions;

    for (current_round = 0; current_round < info->num_rounds; current_round++) {
        IceTSizeType my_size = icetSparseImageGetNumPixels(working_image);
        const radixkRoundInfo *round_info = &info->rounds[current_round];
        radixkPartnerInfo *partners = radixkGetPartners(round_info,
                                                        remaining_partitions,
                                                        compose_group,
                                                        group_rank,
                                                        my_size);
        IceTCommRequest *receive_requests;
        IceTCommRequest *send_requests;

        receive_requests = radixkPostReceives(partners,
                                              round_info,
                                              current_round,
                                              remaining_partitions,
                                              my_size);

        send_requests = radixkPostSends(partners,
                                        round_info,
                                        current_round,
                                        remaining_partitions,
                                        my_offset,
                                        working_image);

        radixkCompositeIncomingImages(partners,
                                      receive_requests,
                                      round_info,
                                      working_image);

        if (round_info->split) {
            icetCommWaitall(round_info->k, send_requests);
        } else {
            icetCommWait(&send_requests[0]);
        }

        my_offset = partners[round_info->partition_index].offset;
        if (round_info->split) {
            remaining_partitions /= round_info->k;
        } else if (!round_info->has_image) {
            icetSparseImageSetDimensions(working_image, 0, 0);
            break;
        }
    } /* for all rounds */

    *piece_offset = my_offset;

    return;
}

#ifdef RADIXK_USE_TELESCOPE

static IceTInt icetRadixkTelescopeFindUpperGroupSender(
                                                     const IceTInt *my_group,
                                                     IceTInt my_group_size,
                                                     const IceTInt *upper_group,
                                                     IceTInt upper_group_size)
{
    radixkInfo info;
    IceTInt my_group_rank;
    IceTInt my_partition_index;
    IceTInt my_num_partitions;
    IceTInt upper_num_partitions;
    IceTInt group_difference_factor;
    IceTInt upper_partition_index;
    IceTInt sender_group_rank;

    my_group_rank = icetFindMyRankInGroup(my_group, my_group_size);
    info = radixkGetK(my_group_size, my_group_rank);

    my_partition_index = radixkGetFinalPartitionIndex(&info);
    if (my_partition_index < 0) {
        /* This process does not have a piece, so there is no upper sender. */
        return -1;
    }

    my_num_partitions = radixkGetTotalNumPartitions(&info);

    info = radixkGetK(radixkFindPower2(upper_group_size), 0);
    upper_num_partitions = radixkGetTotalNumPartitions(&info);
    group_difference_factor = my_num_partitions/upper_num_partitions;

    upper_partition_index = my_partition_index / group_difference_factor;

    sender_group_rank
        = radixkGetGroupRankForFinalPartitionIndex(&info,
                                                   upper_partition_index);
    return upper_group[sender_group_rank];
}

static void icetRadixkTelescopeFindLowerGroupReceivers(
                                                     const IceTInt *lower_group,
                                                     IceTInt lower_group_size,
                                                     const IceTInt *my_group,
                                                     IceTInt my_group_size,
                                                     IceTInt **receiver_ranks_p,
                                                     IceTInt *num_receivers_p)
{
    IceTInt my_group_rank = icetFindMyRankInGroup(my_group, my_group_size);
    radixkInfo info;
    IceTInt my_num_partitions;
    IceTInt my_partition_index;
    IceTInt lower_num_partitions;
    IceTInt lower_partition_index;
    IceTInt num_receivers;
    IceTInt *receiver_ranks;
    IceTInt receiver_idx;

    info = radixkGetK(my_group_size, my_group_rank);
    my_num_partitions = radixkGetTotalNumPartitions(&info);
    my_partition_index = radixkGetFinalPartitionIndex(&info);
    if (my_partition_index < 0) {
        *receiver_ranks_p = NULL;
        *num_receivers_p = 0;
        return;
    }

    info = radixkGetK(lower_group_size, 0);
    lower_num_partitions = radixkGetTotalNumPartitions(&info);
    num_receivers = lower_num_partitions/my_num_partitions;
    receiver_ranks = icetGetStateBuffer(RADIXK_RANK_LIST_BUFFER,
                                        num_receivers*sizeof(IceTInt));

    lower_partition_index = my_partition_index*num_receivers;
    for (receiver_idx = 0; receiver_idx < num_receivers; receiver_idx++) {
        IceTInt receiver_group_rank
            = radixkGetGroupRankForFinalPartitionIndex(&info,
                                                       lower_partition_index);
        receiver_ranks[receiver_idx] = lower_group[receiver_group_rank];
        lower_partition_index++;
    }

    *receiver_ranks_p = receiver_ranks;
    *num_receivers_p = num_receivers;
}

static void icetRadixkTelescopeComposeReceive(const IceTInt *my_group,
                                              IceTInt my_group_size,
                                              const IceTInt *upper_group,
                                              IceTInt upper_group_size,
                                              IceTInt total_num_partitions,
                                              IceTBoolean local_in_front,
                                              IceTSparseImage input_image,
                                              IceTSparseImage *result_image,
                                              IceTSizeType *piece_offset)
{
    IceTSparseImage working_image = input_image;
    IceTInt upper_sender;
    radixkInfo info;
    IceTInt my_group_rank;

    my_group_rank = icetFindMyRankInGroup(my_group, my_group_size);
    info = radixkGetK(my_group_size, my_group_rank);

    /* Start with the basic compose of my group. */
    icetRadixkBasicCompose(&info,
                           my_group,
                           my_group_size,
                           total_num_partitions,
                           working_image,
                           piece_offset);

    if (0 < upper_group_size) {
        upper_sender
            = icetRadixkTelescopeFindUpperGroupSender(my_group,
                                                      my_group_size,
                                                      upper_group,
                                                      upper_group_size);
    } else {
        upper_sender = -1;
    }

    /* Collect image from upper group. */
    if (0 <= upper_sender) {
        IceTVoid *incoming_image_buffer;
        IceTSparseImage incoming_image;
        IceTSparseImage composited_image;
        IceTSizeType sparse_image_size;

        sparse_image_size = icetSparseImageBufferSize(
                                 icetSparseImageGetNumPixels(working_image), 1);
        incoming_image_buffer = icetGetStateBuffer(RADIXK_RECEIVE_BUFFER,
                                                   sparse_image_size);

        /* Reuse the spare buffer for the final image.  At this point we are
           finishing compositing in this process and are either returning the
           image or sending it elsewhere, so using this buffer should be safe.
           I know.  Yuck. */
        composited_image = icetGetStateBufferSparseImage(
                                       RADIXK_SPARE_BUFFER,
                                       icetSparseImageGetWidth(working_image),
                                       icetSparseImageGetHeight(working_image));

        icetCommRecv(incoming_image_buffer,
                     sparse_image_size,
                     ICET_BYTE,
                     upper_sender,
                     RADIXK_TELESCOPE_IMAGE_TAG);
        incoming_image
            = icetSparseImageUnpackageFromReceive(incoming_image_buffer);

        if (local_in_front) {
            icetCompressedCompressedComposite(working_image,
                                              incoming_image,
                                              composited_image);
        } else {
            icetCompressedCompressedComposite(incoming_image,
                                              working_image,
                                              composited_image);
        }

        *result_image = composited_image;
    } else {
        *result_image = working_image;
    }

    return;
}

static void icetRadixkTelescopeComposeSend(const IceTInt *lower_group,
                                           IceTInt lower_group_size,
                                           const IceTInt *my_group,
                                           IceTInt my_group_size,
                                           IceTInt total_num_partitions,
                                           IceTSparseImage input_image)
{
    const IceTInt *main_group;
    IceTInt main_group_size;
    const IceTInt *sub_group;
    IceTInt sub_group_size;
    IceTBoolean main_in_front;
    IceTInt main_group_rank;

    /* Check for any further telescoping. */
    main_group_size = radixkFindPower2(my_group_size);
    sub_group_size = my_group_size - main_group_size;

    main_group = my_group;
    sub_group = my_group + main_group_size;
    main_in_front = ICET_TRUE;
    main_group_rank = icetFindMyRankInGroup(main_group, main_group_size);

    if (main_group_rank >= 0) {
        /* In the main group. */
        IceTSparseImage working_image;
        IceTSizeType piece_offset;
        IceTInt *receiver_ranks;
        IceTInt num_receivers;
        IceTSizeType partition_num_pixels;
        IceTSizeType sparse_image_size;
        IceTVoid *send_buf_pool;
        IceTInt *piece_offsets;
        IceTSparseImage *image_pieces;
        IceTInt receiver_idx;
        IceTInt num_local_partitions;
        IceTCommRequest *send_requests;

        icetRadixkTelescopeComposeReceive(main_group,
                                          main_group_size,
                                          sub_group,
                                          sub_group_size,
                                          total_num_partitions,
                                          main_in_front,
                                          input_image,
                                          &working_image,
                                          &piece_offset);

        {
            radixkInfo info = radixkGetK(main_group_size, 0);
            num_local_partitions = radixkGetTotalNumPartitions(&info);
        }

        icetRadixkTelescopeFindLowerGroupReceivers(lower_group,
                                                   lower_group_size,
                                                   main_group,
                                                   main_group_size,
                                                   &receiver_ranks,
                                                   &num_receivers);

        if (num_receivers > 1) {
            partition_num_pixels = icetSparseImageSplitPartitionNumPixels(
                                     icetSparseImageGetNumPixels(working_image),
                                     num_receivers,
                                     total_num_partitions/num_local_partitions);
        } else if (num_receivers == 1) {
            partition_num_pixels = icetSparseImageGetNumPixels(working_image);
        } else { /* num_receivers == 0 */
            /* Nothing to send.  Quit. */
            return;
        }

        sparse_image_size = icetSparseImageBufferSize(partition_num_pixels, 1);
        send_buf_pool = icetGetStateBuffer(RADIXK_SEND_BUFFER,
                                           sparse_image_size * num_receivers);

        piece_offsets = icetGetStateBuffer(RADIXK_SPLIT_OFFSET_ARRAY_BUFFER,
                                           num_receivers * sizeof(IceTInt));
        image_pieces = icetGetStateBuffer(
                                       RADIXK_SPLIT_IMAGE_ARRAY_BUFFER,
                                       num_receivers * sizeof(IceTSparseImage));
        for (receiver_idx = 0; receiver_idx < num_receivers; receiver_idx++) {
            IceTVoid *send_buffer
                = ((IceTByte*)send_buf_pool + receiver_idx*sparse_image_size);
            image_pieces[receiver_idx]
                = icetSparseImageAssignBuffer(send_buffer,
                                              partition_num_pixels, 1);
        }

        if (num_receivers > 1) {
            icetSparseImageSplit(working_image,
                                 piece_offset,
                                 num_receivers,
                                 total_num_partitions/num_local_partitions,
                                 image_pieces,
                                 piece_offsets);
        } else {
            image_pieces[0] = working_image;
        }

        send_requests
            = icetGetStateBuffer(RADIXK_SEND_REQUEST_BUFFER,
                                 num_receivers * sizeof(IceTCommRequest));
        for (receiver_idx = 0; receiver_idx < num_receivers; receiver_idx++) {
            IceTVoid *package_buffer;
            IceTSizeType package_size;
            IceTInt receiver_rank;

            icetSparseImagePackageForSend(image_pieces[receiver_idx],
                                          &package_buffer, &package_size);
            receiver_rank = receiver_ranks[receiver_idx];

            send_requests[receiver_idx]
                = icetCommIsend(package_buffer,
                                package_size,
                                ICET_BYTE,
                                receiver_rank,
                                RADIXK_TELESCOPE_IMAGE_TAG);
        }

        icetCommWaitall(num_receivers, send_requests);
    } else {
        /* In the sub group. */
        icetRadixkTelescopeComposeSend(main_group,
                                       main_group_size,
                                       sub_group,
                                       sub_group_size,
                                       total_num_partitions,
                                       input_image);
    }
}

static void icetRadixkTelescopeCompose(const IceTInt *compose_group,
                                       IceTInt group_size,
                                       IceTInt image_dest,
                                       IceTSparseImage input_image,
                                       IceTSparseImage *result_image,
                                       IceTSizeType *piece_offset)
{
    const IceTInt *main_group;
    IceTInt main_group_size;
    const IceTInt *sub_group;
    IceTInt sub_group_size;
    IceTBoolean main_in_front;
    IceTBoolean use_interlace;
    IceTInt main_group_rank;
    IceTInt total_num_partitions;
    IceTInt save_max_image_split;

    IceTSparseImage working_image = input_image;
    IceTSizeType original_image_size = icetSparseImageGetNumPixels(input_image);

    main_group_size = radixkFindPower2(group_size);
    sub_group_size = group_size - main_group_size;
    /* Simple optimization to put image_dest in main group so that it has at
       least some data. */
    if (image_dest < main_group_size) {
        main_group = compose_group;
        sub_group = compose_group + main_group_size;
        main_in_front = ICET_TRUE;
    } else {
        sub_group = compose_group;
        main_group = compose_group + sub_group_size;
        main_in_front = ICET_FALSE;
    }
    main_group_rank = icetFindMyRankInGroup(main_group, main_group_size);

    /* Here is a convenient place to determine the final number of
       partitions. */
    {
        /* Group rank does not matter for our purposes. */
        radixkInfo info = radixkGetK(main_group_size, 0);
        total_num_partitions = radixkGetTotalNumPartitions(&info);
    }

    /* This is a corner case I found.  It is possible for the main group to have
       a smaller number of partitions than the sub group when the sub group
       causes a smaller k in the last room that fits within the max image split.
       To prevent this from happening, temporarily set the max image split to
       the total number of partitions we know we are using. */
    icetGetIntegerv(ICET_MAX_IMAGE_SPLIT, &save_max_image_split);
    icetStateSetInteger(ICET_MAX_IMAGE_SPLIT, total_num_partitions);

    /* Since we know the number of final pieces we will create, now is a good
       place to interlace the image (and then later adjust the offset. */
    {
        IceTInt magic_k;
        use_interlace = icetIsEnabled(ICET_INTERLACE_IMAGES);

        icetGetIntegerv(ICET_MAGIC_K, &magic_k);
        use_interlace &= (total_num_partitions > magic_k);
    }

    if (use_interlace) {
        IceTSparseImage interlaced_image = icetGetStateBufferSparseImage(
                                       RADIXK_INTERLACED_IMAGE_BUFFER,
                                       icetSparseImageGetWidth(working_image),
                                       icetSparseImageGetHeight(working_image));
        icetSparseImageInterlace(working_image,
                                 total_num_partitions,
                                 RADIXK_SPLIT_OFFSET_ARRAY_BUFFER,
                                 interlaced_image);
        working_image = interlaced_image;
    }

    /* Do the actual compositing. */
    if (main_group_rank >= 0) {
        /* In the main group. */
        icetRadixkTelescopeComposeReceive(main_group,
                                          main_group_size,
                                          sub_group,
                                          sub_group_size,
                                          total_num_partitions,
                                          main_in_front,
                                          working_image,
                                          result_image,
                                          piece_offset);
    } else {
        /* In the sub group. */
        icetRadixkTelescopeComposeSend(main_group,
                                       main_group_size,
                                       sub_group,
                                       sub_group_size,
                                       total_num_partitions,
                                       working_image);
        *result_image = icetSparseImageNull();
        *piece_offset = 0;
    }

    /* Restore the true image split. */
    icetStateSetInteger(ICET_MAX_IMAGE_SPLIT, save_max_image_split);

    /* If we interlaced the image and are actually returning something,
       correct the offset. */
    if (use_interlace && (0 < icetSparseImageGetNumPixels(*result_image))) {
        radixkInfo info;
        IceTInt global_partition;

        if (main_group_rank < 0) {
            icetRaiseError("Process not in main group got image piece.",
                           ICET_SANITY_CHECK_FAIL);
            return;
        }

        info = radixkGetK(main_group_size, main_group_rank);

        global_partition = radixkGetFinalPartitionIndex(&info);
        *piece_offset = icetGetInterlaceOffset(global_partition,
                                               total_num_partitions,
                                               original_image_size);
    }

    return;
}



void icetRadixkCompose(const IceTInt *compose_group,
                       IceTInt group_size,
                       IceTInt image_dest,
                       IceTSparseImage input_image,
                       IceTSparseImage *result_image,
                       IceTSizeType *piece_offset)
{
    icetRadixkTelescopeCompose(compose_group,
                               group_size,
                               image_dest,
                               input_image,
                               result_image,
                               piece_offset);
}

#else

void icetRadixkCompose(const IceTInt *compose_group,
                       IceTInt group_size,
                       IceTInt image_dest,
                       IceTSparseImage input_image,
                       IceTSparseImage *result_image,
                       IceTSizeType *piece_offset)
{
    IceTInt group_rank = icetFindMyRankInGroup(compose_group, group_size);
    radixkInfo info = radixkGetK(group_size, group_rank);
    IceTInt total_num_partitions = radixkGetTotalNumPartitions(&info);
    IceTBoolean use_interlace = icetIsEnabled(ICET_INTERLACE_IMAGES);
    IceTSparseImage working_image = input_image;
    IceTSizeType original_image_size = icetSparseImageGetNumPixels(input_image);

    (void)image_dest; /* Not used. */

    if (use_interlace) {
        use_interlace = (info.num_rounds > 1);
    }

    if (use_interlace) {
        IceTSparseImage interlaced_image = icetGetStateBufferSparseImage(
                                       RADIXK_INTERLACED_IMAGE_BUFFER,
                                       icetSparseImageGetWidth(working_image),
                                       icetSparseImageGetHeight(working_image));
        icetSparseImageInterlace(working_image,
                                 total_num_partitions,
                                 RADIXK_SPLIT_OFFSET_ARRAY_BUFFER,
                                 interlaced_image);
        working_image = interlaced_image;
    }

    icetRadixkBasicCompose(&info,
                           compose_group,
                           group_size,
                           total_num_partitions,
                           working_image,
                           piece_offset);

    *result_image = working_image;

    if (use_interlace && (0 < icetSparseImageGetNumPixels(working_image))) {
        IceTInt global_partition = radixkGetFinalPartitionIndex(&info);
        *piece_offset = icetGetInterlaceOffset(global_partition,
                                               total_num_partitions,
                                               original_image_size);
    }
}

#endif

static IceTBoolean radixkTryPartitionLookup(IceTInt group_size)
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
        radixkInfo info;
        IceTInt rank_assignment;

        info = radixkGetK(group_size, group_rank);
        partition_index = radixkGetFinalPartitionIndex(&info);
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
            = radixkGetGroupRankForFinalPartitionIndex(&info, partition_index);
        if (rank_assignment != group_rank) {
            printf("Rank %d reports partition %d, but partition reports rank %d.\n",
                   group_rank, partition_index, rank_assignment);
            return ICET_FALSE;
        }
    }

    {
        radixkInfo info;
        info = radixkGetK(group_size, 0);
        if (num_partitions != radixkGetTotalNumPartitions(&info)) {
            printf("Expected %d partitions, found %d\n",
                   radixkGetTotalNumPartitions(&info),
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

ICET_EXPORT IceTBoolean icetRadixkPartitionLookupUnitTest(void)
{
    const IceTInt group_sizes_to_try[] = {
        2,                              /* Base case. */
        ICET_MAGIC_K_DEFAULT,           /* Largest direct send. */
        ICET_MAGIC_K_DEFAULT*2,         /* Changing group sizes. */
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

            if (!radixkTryPartitionLookup(group_size)) {
                return ICET_FALSE;
            }
        }
    }

    return ICET_TRUE;
}

#ifdef RADIXK_USE_TELESCOPE

#define MAIN_GROUP_RANK(idx)    (10000 + idx)
#define SUB_GROUP_RANK(idx)     (20000 + idx)
static IceTBoolean radixkTryTelescopeSendReceive(IceTInt *main_group,
                                                 IceTInt main_group_size,
                                                 IceTInt *sub_group,
                                                 IceTInt sub_group_size)
{
    IceTInt rank;
    IceTInt sub_group_idx;

    icetGetIntegerv(ICET_RANK, &rank);

    /* Check the receives for each entry in sub_group. */
    /* Fill initial sub_group. */
    for (sub_group_idx = 0; sub_group_idx < sub_group_size; sub_group_idx++) {
        IceTInt *receiver_ranks;
        IceTInt num_receivers;
        IceTInt receiver_idx;
        /* The FindLowerGroupReceivers uses local rank to identify in group.
           Temporarily replace the rank in the group. */
        sub_group[sub_group_idx] = rank;
        icetRadixkTelescopeFindLowerGroupReceivers(main_group,
                                                   main_group_size,
                                                   sub_group,
                                                   sub_group_size,
                                                   &receiver_ranks,
                                                   &num_receivers);
        sub_group[sub_group_idx] = SUB_GROUP_RANK(sub_group_idx);

        /* For each receiver, check to make sure the correct sender is
           identified. */
        for (receiver_idx = 0; receiver_idx < num_receivers; receiver_idx++)
        {
            IceTInt receiver_rank = receiver_ranks[receiver_idx];
            IceTInt receiver_group_rank;
            IceTInt send_rank;

            receiver_group_rank = icetFindRankInGroup(main_group,
                                                      main_group_size,
                                                      receiver_rank);
            if (   (receiver_group_rank < 0)
                || (main_group_size <= receiver_group_rank)) {
                printf("Receiver %d for sub group rank %d is %d"
                       " but is not in main group.\n",
                       receiver_idx,
                       sub_group_idx,
                       receiver_rank);
                return ICET_FALSE;
            }

            /* FindUpperGroupSend uses local rank to identify in group.
               Temporarily replace the rank in the group. */
            main_group[receiver_group_rank] = rank;
            send_rank = icetRadixkTelescopeFindUpperGroupSender(main_group,
                                                                main_group_size,
                                                                sub_group,
                                                                sub_group_size);
            main_group[receiver_group_rank] = receiver_rank;

            if (send_rank != SUB_GROUP_RANK(sub_group_idx)) {
                printf("Receiver %d reported sender %d "
                       "but expected %d.\n",
                       receiver_rank,
                       send_rank,
                       SUB_GROUP_RANK(sub_group_idx));
                return ICET_FALSE;
            }
        }
    }

    return ICET_TRUE;
}

ICET_EXPORT IceTBoolean icetRadixkTelescopeSendReceiveTest(void)
{
    IceTInt main_group_size;

    printf("\nTesting send/receive of telescope groups.\n");

    for (main_group_size = 2; main_group_size <= 512; main_group_size *= 2) {
        IceTInt *main_group = malloc(main_group_size * sizeof(IceTInt));
        IceTInt main_group_idx;
        IceTInt sub_group_size;

        printf("Main group size %d\n", main_group_size);

        /* Fill initial main_group. */
        for (main_group_idx = 0;
             main_group_idx < main_group_size;
             main_group_idx++) {
            main_group[main_group_idx] = MAIN_GROUP_RANK(main_group_idx);
        }

        for (sub_group_size = 1;
             sub_group_size < main_group_size;
             sub_group_size *= 2) {
            IceTInt *sub_group = malloc(sub_group_size * sizeof(IceTInt));
            IceTInt sub_group_idx;
            IceTInt max_image_split;

            printf("  Sub group size %d\n", sub_group_size);

            /* Fill initial sub_group. */
            for (sub_group_idx = 0;
                 sub_group_idx < sub_group_size;
                 sub_group_idx++) {
                sub_group[sub_group_idx] = SUB_GROUP_RANK(sub_group_idx);
            }

            for (max_image_split = 1;
                 max_image_split <= main_group_size;
                 max_image_split *= 2) {
                IceTInt result;

                printf("    Max image split %d\n", max_image_split);

                icetStateSetInteger(ICET_MAX_IMAGE_SPLIT, max_image_split);

                result = radixkTryTelescopeSendReceive(main_group,
                                                       main_group_size,
                                                       sub_group,
                                                       sub_group_size);

                if (!result) { return ICET_FALSE; }
            }

            free(sub_group);
        }

        free(main_group);
    }

    return ICET_TRUE;
}

#else /*!RADIXK_USE_TELESCOPE*/

ICET_EXPORT IceTBoolean icetRadixkTelescopeSendReceiveTest(void)
{
    /* Telescope method disabled. */
    return ICET_TRUE;
}

#endif /*!RADIXK_USE_TELESCOPE*/
