/* -*- c -*- *******************************************************/
/*
 * Copyright (C) 2003 Sandia Corporation
 * Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 * the U.S. Government retains certain rights in this software.
 *
 * This source code is released under the New BSD License.
 */

#include "common.h"

#include <IceT.h>
#include <IceTDevCommunication.h>
#include <IceTDevDiagnostics.h>
#include <IceTDevState.h>
#include <IceTDevStrategySelect.h>
#include <IceTDevTiming.h>

#include <stdlib.h>
#include <string.h>

#define FULL_IMAGE_DATA 20

#define LARGE_MESSAGE 23

static IceTImage rtfi_image;
static IceTSparseImage rtfi_outSparseImage;
static IceTBoolean rtfi_first;
static IceTVoid *rtfi_generateDataFunc(IceTInt id, IceTInt dest,
                                       IceTSizeType *size) {
    IceTInt rank;
    const IceTInt *tile_list
        = icetUnsafeStateGetInteger(ICET_CONTAINED_TILES_LIST);
    IceTVoid *outBuffer;

    icetGetIntegerv(ICET_RANK, &rank);
    if (dest == rank) {
      /* Special case: sending to myself.
         Just get directly to color and depth buffers. */
        icetGetTileImage(tile_list[id], rtfi_image);
        *size = 0;
        return NULL;
    }
    icetGetCompressedTileImage(tile_list[id], rtfi_outSparseImage);
    icetSparseImagePackageForSend(rtfi_outSparseImage, &outBuffer, size);
    return outBuffer;
}
static void rtfi_handleDataFunc(void *inSparseImageBuffer, IceTInt src) {
    if (inSparseImageBuffer == NULL) {
      /* Superfluous call from send to self. */
        if (!rtfi_first) {
            icetRaiseError("Unexpected callback order"
                           " in icetRenderTransferFullImages.",
                           ICET_SANITY_CHECK_FAIL);
        }
    } else {
        IceTSparseImage inSparseImage
            = icetSparseImageUnpackageFromReceive(inSparseImageBuffer);
        if (rtfi_first) {
            icetDecompressImage(inSparseImage, rtfi_image);
        } else {
            IceTInt rank;
            const IceTInt *process_orders;
            icetGetIntegerv(ICET_RANK, &rank);
            process_orders = icetUnsafeStateGetInteger(ICET_PROCESS_ORDERS);
            icetCompressedComposite(rtfi_image, inSparseImage,
                                    process_orders[src] < process_orders[rank]);
        }
    }
    rtfi_first = ICET_FALSE;
}
void icetRenderTransferFullImages(IceTImage image,
                                  IceTVoid *inSparseImageBuffer,
                                  IceTSparseImage outSparseImage,
                                  IceTInt *tile_image_dest)
{
    IceTInt num_sending;
    const IceTInt *tile_list;
    IceTInt num_tiles;
    IceTInt width, height;
    IceTInt *imageDestinations;

    IceTInt i;

    rtfi_image = image;
    rtfi_outSparseImage = outSparseImage;
    rtfi_first = ICET_TRUE;

    icetGetIntegerv(ICET_NUM_CONTAINED_TILES, &num_sending);
    tile_list = icetUnsafeStateGetInteger(ICET_CONTAINED_TILES_LIST);
    icetGetIntegerv(ICET_TILE_MAX_WIDTH, &width);
    icetGetIntegerv(ICET_TILE_MAX_HEIGHT, &height);
    icetGetIntegerv(ICET_NUM_TILES, &num_tiles);

    imageDestinations = malloc(num_tiles * sizeof(IceTInt));

  /* Make each element imageDestinations point to the processor to send the
     corresponding image in tile_list. */
    for (i = 0; i < num_sending; i++) {
        imageDestinations[i] = tile_image_dest[tile_list[i]];
    }

    icetSendRecvLargeMessages(num_sending, imageDestinations,
                              icetIsEnabled(ICET_ORDERED_COMPOSITE),
                              rtfi_generateDataFunc, rtfi_handleDataFunc,
                              inSparseImageBuffer,
                              icetSparseImageBufferSize(width, height));

    free(imageDestinations);
}

static IceTSparseImage rtsi_workingImage;
static IceTSparseImage rtsi_availableImage;
static IceTSparseImage rtsi_outSparseImage;
static IceTBoolean rtsi_first;
static IceTVoid *rtsi_generateDataFunc(IceTInt id, IceTInt dest,
                                       IceTSizeType *size) {
    IceTInt rank;
    const IceTInt *tile_list
        = icetUnsafeStateGetInteger(ICET_CONTAINED_TILES_LIST);
    IceTVoid *outBuffer;

    icetGetIntegerv(ICET_RANK, &rank);
    if (dest == rank) {
      /* Special case: sending to myself.
         Just get directly to color and depth buffers. */
        icetGetCompressedTileImage(tile_list[id], rtsi_workingImage);
        *size = 0;
        return NULL;
    }
    icetGetCompressedTileImage(tile_list[id], rtsi_outSparseImage);
    icetSparseImagePackageForSend(rtsi_outSparseImage, &outBuffer, size);
    return outBuffer;
}
static void rtsi_handleDataFunc(void *inSparseImageBuffer, IceTInt src) {
    if (inSparseImageBuffer == NULL) {
      /* Superfluous call from send to self. */
        if (!rtsi_first) {
            icetRaiseError("Unexpected callback order"
                           " in icetRenderTransferSparseImages.",
                           ICET_SANITY_CHECK_FAIL);
        }
    } else {
        IceTSparseImage inSparseImage
            = icetSparseImageUnpackageFromReceive(inSparseImageBuffer);
        if (rtsi_first) {
            IceTSizeType num_pixels
                = icetSparseImageGetNumPixels(inSparseImage);
            icetSparseImageCopyPixels(inSparseImage,
                                      0,
                                      num_pixels,
                                      rtsi_workingImage);
        } else {
            IceTInt rank;
            const IceTInt *process_orders;
            IceTSparseImage old_workingImage;

            icetGetIntegerv(ICET_RANK, &rank);
            process_orders = icetUnsafeStateGetInteger(ICET_PROCESS_ORDERS);
            if (process_orders[src] < process_orders[rank]) {
                icetCompressedCompressedComposite(inSparseImage,
                                                  rtsi_workingImage,
                                                  rtsi_availableImage);
            } else {
                icetCompressedCompressedComposite(rtsi_workingImage,
                                                  inSparseImage,
                                                  rtsi_availableImage);
            }

            old_workingImage = rtsi_workingImage;
            rtsi_workingImage = rtsi_availableImage;
            rtsi_availableImage = old_workingImage;
        }
    }
    rtsi_first = ICET_FALSE;
}
void icetRenderTransferSparseImages(IceTSparseImage compositeImage1,
                                    IceTSparseImage compositeImage2,
                                    IceTVoid *inImageBuffer,
                                    IceTSparseImage outSparseImage,
                                    IceTInt *tile_image_dest,
                                    IceTSparseImage *resultImage)
{
    IceTInt num_sending;
    const IceTInt *tile_list;
    IceTInt num_tiles;
    IceTInt width, height;
    IceTInt *imageDestinations;

    IceTInt i;

    rtsi_workingImage = compositeImage1;
    rtsi_availableImage = compositeImage2;
    rtsi_outSparseImage = outSparseImage;
    rtsi_first = ICET_TRUE;

    icetGetIntegerv(ICET_NUM_CONTAINED_TILES, &num_sending);
    tile_list = icetUnsafeStateGetInteger(ICET_CONTAINED_TILES_LIST);
    icetGetIntegerv(ICET_TILE_MAX_WIDTH, &width);
    icetGetIntegerv(ICET_TILE_MAX_HEIGHT, &height);
    icetGetIntegerv(ICET_NUM_TILES, &num_tiles);

    imageDestinations = malloc(num_tiles * sizeof(IceTInt));

  /* Make each element imageDestinations point to the processor to send the
     corresponding image in tile_list. */
    for (i = 0; i < num_sending; i++) {
        imageDestinations[i] = tile_image_dest[tile_list[i]];
    }

    icetSendRecvLargeMessages(num_sending, imageDestinations,
                              icetIsEnabled(ICET_ORDERED_COMPOSITE),
                              rtsi_generateDataFunc, rtsi_handleDataFunc,
                              inImageBuffer,
                              icetSparseImageBufferSize(width, height));

    *resultImage = rtsi_workingImage;

    free(imageDestinations);
}

#define ICET_SEND_RECV_LARGE_SEND_IDS_BUF       ICET_STRATEGY_COMMON_BUF_0
#define ICET_SEND_RECV_LARGE_DEST_MASK_BUF      ICET_STRATEGY_COMMON_BUF_1
#define ICET_SEND_RECV_LARGE_SRC_MASK_BUF       ICET_STRATEGY_COMMON_BUF_2

enum IceTIterState {
    ICET_SEND_RECV_LARGE_ITER_FORWARD = 0x1101,
    ICET_SEND_RECV_LARGE_ITER_BACKWARD,
    ICET_SEND_RECV_LARGE_ITER_DONE
};

static void icetSendRecvLargeBuildMessageMasks(
                                             IceTInt numMessagesSending,
                                             const IceTInt *messageDestinations,
                                             IceTInt **sendIds_p,
                                             IceTBoolean **myDestMask_p,
                                             IceTBoolean **mySrcMask_p)
{
    IceTInt comm_size;

    IceTInt *sendIds;
    IceTBoolean *myDestMask;
    IceTBoolean *mySrcMask;

    icetGetIntegerv(ICET_NUM_PROCESSES, &comm_size);

    sendIds = icetGetStateBuffer(ICET_SEND_RECV_LARGE_SEND_IDS_BUF,
                                 comm_size * sizeof(IceTInt));
    myDestMask = icetGetStateBuffer(ICET_SEND_RECV_LARGE_DEST_MASK_BUF,
                                    comm_size * sizeof(IceTBoolean));
    mySrcMask = icetGetStateBuffer(ICET_SEND_RECV_LARGE_SRC_MASK_BUF,
                                   comm_size * sizeof(IceTBoolean));

    /* Convert array of ranks to a mask of ranks. */ {
        IceTInt message_id;
        memset(myDestMask, 0, comm_size * sizeof(IceTBoolean));
        for (message_id = 0; message_id < numMessagesSending; message_id++) {
            myDestMask[messageDestinations[message_id]] = ICET_TRUE;
            sendIds[messageDestinations[message_id]] = message_id;
        }
    }

    /* Transpose myDestMask array to get mask of srcs on each process. */
    icetCommAlltoall(myDestMask, 1, ICET_BOOLEAN, mySrcMask);

    *sendIds_p = sendIds;
    *myDestMask_p = myDestMask;
    *mySrcMask_p = mySrcMask;
}

static void icetSendRecvLargePostReceive(const IceTBoolean *mySrcMask,
                                         IceTBoolean messagesInOrder,
                                         IceTInt order_rank,
                                         IceTVoid *incomingBuffer,
                                         IceTSizeType bufferSize,
                                         IceTInt *recv_order_idx_p,
                                         enum IceTIterState *recv_iter_state_p,
                                         IceTCommRequest *recv_request_p)
{
    IceTInt comm_size;
    const IceTInt *composite_order;
    IceTInt src_rank = -1;

    /* If we are still waiting for a receive to finish, do nothing. */
    if (*recv_request_p != ICET_COMM_REQUEST_NULL) return;

    /* If we are done receiving all messages, do nothing. */
    if (*recv_iter_state_p == ICET_SEND_RECV_LARGE_ITER_DONE) return;

    if (messagesInOrder) {
        composite_order = icetUnsafeStateGetInteger(ICET_COMPOSITE_ORDER);
    } else {
        composite_order = NULL;
    }

    icetGetIntegerv(ICET_NUM_PROCESSES, &comm_size);

    while (*recv_iter_state_p == ICET_SEND_RECV_LARGE_ITER_BACKWARD) {
        (*recv_order_idx_p)--;
        if (*recv_order_idx_p < 0) {
            if (messagesInOrder) {
                /* If receiving messages in order, start iterating down from
                   self. */
                *recv_iter_state_p = ICET_SEND_RECV_LARGE_ITER_FORWARD;
                *recv_order_idx_p = order_rank;
                break;
            } else {
                /* If receiving messages in arbitrary order, continue iteration
                   from bottom. */
                *recv_order_idx_p = comm_size - 1;
            }
        }
        if (*recv_order_idx_p == order_rank) {
            if (messagesInOrder) {
                icetRaiseError("Somehow flipped rank in ordered messages.",
                               ICET_SANITY_CHECK_FAIL);
            }
            *recv_iter_state_p = ICET_SEND_RECV_LARGE_ITER_DONE;
            break;
        }
        /* Check to see if we have this message to send. */
        if (messagesInOrder) {
            src_rank = composite_order[*recv_order_idx_p];
        } else {
            src_rank = *recv_order_idx_p;
        }
        if (mySrcMask[src_rank]) break;
    }

    while (*recv_iter_state_p == ICET_SEND_RECV_LARGE_ITER_FORWARD) {
        (*recv_order_idx_p)++;
        if (*recv_order_idx_p >= comm_size) {
            if (!messagesInOrder) {
                icetRaiseError("Reversed iteration in unordered messages.",
                               ICET_SANITY_CHECK_FAIL);
            }
            /* We have iterated over everything at this point. */
            *recv_iter_state_p = ICET_SEND_RECV_LARGE_ITER_DONE;
            break;
        }
        /* Check to see if we have this message to send. */
        /* If we are here, messages are in order. */
        src_rank = composite_order[*recv_order_idx_p];
        if (mySrcMask[src_rank]) break;
    }

    /* If we have not finished, then we must be ready to send a message. */
    if (*recv_iter_state_p != ICET_SEND_RECV_LARGE_ITER_DONE) {
        if (src_rank < 0) {
            icetRaiseError("Computed invalid src_rank", ICET_SANITY_CHECK_FAIL);
        }
        *recv_request_p = icetCommIrecv(incomingBuffer,
                                        bufferSize,
                                        ICET_BYTE,
                                        src_rank,
                                        LARGE_MESSAGE);
    }
}

static void icetSendRecvLargePostSend(const IceTInt *sendIds,
                                      const IceTBoolean *myDestMask,
                                      IceTBoolean messagesInOrder,
                                      IceTGenerateData generateDataFunc,
                                      IceTInt order_rank,
                                      IceTInt *send_order_idx_p,
                                      enum IceTIterState *send_iter_state_p,
                                      IceTCommRequest *send_request_p)
{
    IceTInt comm_size;
    const IceTInt *composite_order;
    IceTInt dest_rank = -1;

    /* If we are still waiting for a send to finish, do nothing. */
    if (*send_request_p != ICET_COMM_REQUEST_NULL) return;

    /* If we are done sending all messages, do nothing. */
    if (*send_iter_state_p == ICET_SEND_RECV_LARGE_ITER_DONE) return;

    if (messagesInOrder) {
        composite_order = icetUnsafeStateGetInteger(ICET_COMPOSITE_ORDER);
    } else {
        composite_order = NULL;
    }

    icetGetIntegerv(ICET_NUM_PROCESSES, &comm_size);

    while (*send_iter_state_p == ICET_SEND_RECV_LARGE_ITER_FORWARD) {
        (*send_order_idx_p)++;
        if (*send_order_idx_p >= comm_size) {
            if (messagesInOrder) {
                /* If receiving messages in order, start iterating up from
                   self. */
                *send_iter_state_p = ICET_SEND_RECV_LARGE_ITER_BACKWARD;
                *send_order_idx_p = order_rank;
                break;
            } else {
                /* If receiving messages in arbitrary order, continue iteration
                   from top. */
                *send_order_idx_p = 0;
            }
        }
        if (*send_order_idx_p == order_rank) {
            if (messagesInOrder) {
                icetRaiseError("Somehow flipped rank in ordered messages.",
                               ICET_SANITY_CHECK_FAIL);
            }
            *send_iter_state_p = ICET_SEND_RECV_LARGE_ITER_DONE;
            break;
        }
        /* Check to see if we have this message to send. */
        if (messagesInOrder) {
            dest_rank = composite_order[*send_order_idx_p];
        } else {
            dest_rank = *send_order_idx_p;
        }
        if (myDestMask[dest_rank]) break;
    }

    while (*send_iter_state_p == ICET_SEND_RECV_LARGE_ITER_BACKWARD) {
        (*send_order_idx_p)--;
        if (*send_order_idx_p < 0) {
            if (!messagesInOrder) {
                icetRaiseError("Reversed iteration in unordered messages.",
                               ICET_SANITY_CHECK_FAIL);
            }
            /* We have iterated over everything at this point. */
            *send_iter_state_p = ICET_SEND_RECV_LARGE_ITER_DONE;
            break;
        }
        /* Check to see if we have this message to send. */
        /* If we are here, messages are in order. */
        dest_rank = composite_order[*send_order_idx_p];
        if (myDestMask[dest_rank]) break;
    }

    /* If we have not finished, then we must be ready to send a message. */
    if (*send_iter_state_p != ICET_SEND_RECV_LARGE_ITER_DONE) {
        IceTSizeType data_size;
        IceTVoid *data;
        if (dest_rank < 0) {
            icetRaiseError("Computed invalid dest_rank",ICET_SANITY_CHECK_FAIL);
        }
        data = (*generateDataFunc)(sendIds[dest_rank], dest_rank, &data_size);
        *send_request_p = icetCommIsend(data,
                                        data_size,
                                        ICET_BYTE,
                                        dest_rank,
                                        LARGE_MESSAGE);
    }
}

static void icetDoSendRecvLarge(const IceTInt *sendIds,
                                const IceTBoolean *myDestMask,
                                const IceTBoolean *mySrcMask,
                                IceTBoolean messagesInOrder,
                                IceTGenerateData generateDataFunc,
                                IceTHandleData handleDataFunc,
                                IceTVoid *incomingBuffer,
                                IceTSizeType bufferSize)
{
    IceTInt comm_size;
    IceTInt rank;
    const IceTInt *composite_order;
    const IceTInt *process_orders;

    IceTInt order_rank;
    IceTInt recv_order_idx;
    IceTInt send_order_idx;

    enum IceTCommIndices { RECV_IDX = 0, SEND_IDX = 1 };
    IceTCommRequest requests[2];

    enum IceTIterState recv_iter_state;
    enum IceTIterState send_iter_state;

    icetGetIntegerv(ICET_NUM_PROCESSES, &comm_size);
    icetGetIntegerv(ICET_RANK, &rank);

    if (messagesInOrder) {
        composite_order = icetUnsafeStateGetInteger(ICET_COMPOSITE_ORDER);
        process_orders = icetUnsafeStateGetInteger(ICET_PROCESS_ORDERS);
    } else {
        composite_order = NULL;
        process_orders = NULL;
    }

    /* We'll just handle send to self as a special case. */
    if (myDestMask[rank]) {
        IceTSizeType data_size;
        IceTVoid *data;
        icetRaiseDebug("Sending to self.");
        data = (*generateDataFunc)(sendIds[rank], rank, &data_size);
        (*handleDataFunc)(data, rank);
    }

    /* We have to create a communication pattern that is guaranteed not to
       deadlock even if we don't know what everyone is sending.  To do this, we
       use a particular type of communication pattern.  If order does not
       matter, then each process asynchronously sends to rank+1 and receives
       from rank-1 (with wraparound between 0 and num_proc-1).  Because there
       every sender has receive and vice versa, this is guaranteed by MPI to
       complete.  This is then repeated sending to rank+2, rank+3, and so on.
       The ordered communication is similar except that there is no wraparound.
       If the sender or receiver is outside the range, no communication happens.
       The communication is then repeated in the other direction.  Again, all
       send/receives are matched and eventually everyone sends/receives to/from
       everyone.

       The trick we are going to pull is realize that the actual messages are
       probably sparse.  Thus, we won't actually do a send/receive if no message
       is being passed.  However, this does not change the fact that all sends
       and receives are matched up. */

    if (messagesInOrder) {
        order_rank = process_orders[rank];
    } else {
        order_rank = rank;
    }

    recv_order_idx = send_order_idx = order_rank;
    recv_iter_state = ICET_SEND_RECV_LARGE_ITER_BACKWARD;
    send_iter_state = ICET_SEND_RECV_LARGE_ITER_FORWARD;
    requests[0] = requests[1] = ICET_COMM_REQUEST_NULL;

    while (ICET_TRUE) {
        icetSendRecvLargePostReceive(mySrcMask,
                                     messagesInOrder,
                                     order_rank,
                                     incomingBuffer,
                                     bufferSize,
                                     &recv_order_idx,
                                     &recv_iter_state,
                                     &requests[RECV_IDX]);
        icetSendRecvLargePostSend(sendIds,
                                  myDestMask,
                                  messagesInOrder,
                                  generateDataFunc,
                                  order_rank,
                                  &send_order_idx,
                                  &send_iter_state,
                                  &requests[SEND_IDX]);

        /* If finished with all messages, quit. */ {
            IceTBoolean finished;
            finished  = (send_iter_state == ICET_SEND_RECV_LARGE_ITER_DONE);
            finished &= (recv_iter_state == ICET_SEND_RECV_LARGE_ITER_DONE);
            if (finished) break;
        }

        /* Wait for some some message to come in before continuing. */ {
            int request_finished_idx = icetCommWaitany(2, requests);
            if (request_finished_idx == RECV_IDX) {
                IceTInt src_rank;
                if (messagesInOrder) {
                    src_rank = composite_order[recv_order_idx];
                } else {
                    src_rank = recv_iter_state;
                }
                (*handleDataFunc)(incomingBuffer, src_rank);
            }
        }
    }
}

void icetSendRecvLargeMessages(IceTInt numMessagesSending,
                               const IceTInt *messageDestinations,
                               IceTBoolean messagesInOrder,
                               IceTGenerateData generateDataFunc,
                               IceTHandleData handleDataFunc,
                               IceTVoid *incomingBuffer,
                               IceTSizeType bufferSize)
{
    IceTInt *sendIds;
    IceTBoolean *myDestMask;
    IceTBoolean *mySrcMask;

    icetSendRecvLargeBuildMessageMasks(numMessagesSending,
                                       messageDestinations,
                                       &sendIds,
                                       &myDestMask,
                                       &mySrcMask);

    icetDoSendRecvLarge(sendIds,
                        myDestMask,
                        mySrcMask,
                        messagesInOrder,
                        generateDataFunc,
                        handleDataFunc,
                        incomingBuffer,
                        bufferSize);
}

void icetSingleImageCompose(const IceTInt *compose_group,
                            IceTInt group_size,
                            IceTInt image_dest,
                            IceTSparseImage input_image,
                            IceTSparseImage *result_image,
                            IceTSizeType *piece_offset)
{
    IceTEnum strategy;

    icetGetEnumv(ICET_SINGLE_IMAGE_STRATEGY, &strategy);
    icetInvokeSingleImageStrategy(strategy,
                                  compose_group,
                                  group_size,
                                  image_dest,
                                  input_image,
                                  result_image,
                                  piece_offset);
}

#define ICET_IMAGE_COLLECT_OFFSET_BUF ICET_STRATEGY_COMMON_BUF_0
#define ICET_IMAGE_COLLECT_SIZE_BUF ICET_STRATEGY_COMMON_BUF_1

void icetSingleImageCollect(const IceTSparseImage input_image,
                            IceTInt dest,
                            IceTSizeType piece_offset,
                            IceTImage result_image)
{
    IceTSizeType *offsets;
    IceTSizeType *sizes;
    IceTInt rank;
    IceTInt numproc;

    IceTSizeType piece_size;

    IceTEnum color_format;
    IceTEnum depth_format;
    IceTSizeType color_size = 1;
    IceTSizeType depth_size = 1;

#define DUMMY_BUFFER_SIZE       ((IceTSizeType)(16*sizeof(IceTInt)))
    IceTByte dummy_buffer[DUMMY_BUFFER_SIZE];

    rank = icetCommRank();
    numproc = icetCommSize();

    /* Collect partitions held by each process. */
    piece_size = icetSparseImageGetNumPixels(input_image);
    if (rank == dest) {
        offsets = icetGetStateBuffer(ICET_IMAGE_COLLECT_OFFSET_BUF,
                                     sizeof(IceTSizeType)*numproc);
        sizes = icetGetStateBuffer(ICET_IMAGE_COLLECT_SIZE_BUF,
                                   sizeof(IceTSizeType)*numproc);
    } else {
        offsets = NULL;
        sizes = NULL;
    }
    /* Technically, these gathers are part of the collection process and should
       therefore be timed.  However, unless the compositing is very well load
       balanced (and typically it is not), different processes will arrive here
       at different times.  These gathers act something like a barrier that
       helps separate the time spent collecting final pixels from the time spent
       in transferring and compositing fragments. */
    icetCommGather(&piece_offset, 1, ICET_SIZE_TYPE, offsets, dest);
    icetCommGather(&piece_size, 1, ICET_SIZE_TYPE, sizes, dest);

#ifdef DEBUG
    if (rank == dest) {
        IceTVoid *data;
        IceTSizeType size;
        if (icetImageGetColorFormat(result_image) != ICET_IMAGE_COLOR_NONE) {
            data = icetImageGetColorVoid(result_image, &size);
            memset(data, 0xCD, icetImageGetNumPixels(result_image)*size);
        }
        if (icetImageGetDepthFormat(result_image) != ICET_IMAGE_DEPTH_NONE) {
            data = icetImageGetDepthVoid(result_image, &size);
            memset(data, 0xCD, icetImageGetNumPixels(result_image)*size);
        }
    }
#endif

    if (piece_size > 0) {
        /* Decompress data into appropriate offset of result image. */
        icetDecompressSubImageCorrectBackground(input_image,
                                                piece_offset,
                                                result_image);
    } else if (rank != dest) {
        /* If this function is called for multiple collections, it is likely
           that the local process will not have data for all collections.  To
           prevent making the calling function allocate an image buffer when it
           is neither sending or receiving data, just allocate a dummy buffer to
           satisfy the following code. */
        if (DUMMY_BUFFER_SIZE < icetImageBufferSize(0, 0)) {
            icetRaiseError("Oops.  My dummy buffer is not big enough.",
                           ICET_SANITY_CHECK_FAIL);
            return;
        }
        result_image = icetImageAssignBuffer(dummy_buffer, 0, 0);
    } else {
        /* Collecting data at empty process.  We still want to use the provided
           result_image, but we have nothing of our own to put in it.  Thus, do
           nothing. */
    }

    /* Adjust image for output as some buffers, such as depth, might be
       dropped. */
    icetImageAdjustForOutput(result_image);

    icetTimingCollectBegin();

    color_format = icetImageGetColorFormat(result_image);
    depth_format = icetImageGetDepthFormat(result_image);

    if (color_format != ICET_IMAGE_COLOR_NONE) {
        /* Use IceTByte for byte-based pointer arithmetic. */
        IceTByte *color_buffer
          = icetImageGetColorVoid(result_image, &color_size);
        int proc;

        if (rank == dest) {
            /* Adjust sizes and offsets to sizes of pixel colors. */
            for (proc = 0; proc < numproc; proc++) {
                offsets[proc] *= color_size;
                sizes[proc] *= color_size;
            }
            icetCommGatherv(ICET_IN_PLACE_COLLECT,
                            sizes[rank],
                            ICET_BYTE,
                            color_buffer,
                            sizes,
                            offsets,
                            dest);
        } else {
            icetCommGatherv(color_buffer + piece_offset * color_size,
                            piece_size * color_size,
                            ICET_BYTE,
                            NULL,
                            NULL,
                            NULL,
                            dest);
        }
    }

    if (depth_format != ICET_IMAGE_DEPTH_NONE) {
        /* Use IceTByte for byte-based pointer arithmetic. */
        IceTByte *depth_buffer
          = icetImageGetDepthVoid(result_image, &depth_size);
        int proc;

        if (rank == dest) {
            /* Adjust sizes and offsets to sizes of pixel depths.  Also
               adjust for any adjustments made for color. */
            if (color_size != depth_size) {
                for (proc = 0; proc < numproc; proc++) {
                    offsets[proc] /= color_size;
                    offsets[proc] *= depth_size;
                    sizes[proc] /= color_size;
                    sizes[proc] *= depth_size;
                }
            }
            icetCommGatherv(ICET_IN_PLACE_COLLECT,
                            sizes[rank],
                            ICET_BYTE,
                            depth_buffer,
                            sizes,
                            offsets,
                            dest);
        } else {
            icetCommGatherv(depth_buffer + piece_offset * depth_size,
                            piece_size * depth_size,
                            ICET_BYTE,
                            NULL,
                            NULL,
                            NULL,
                            dest);
        }
    }

    icetTimingCollectEnd();
}
