/* -*- c -*- *******************************************************/
/*
 * Copyright (C) 2003 Sandia Corporation
 * Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 * the U.S. Government retains certain rights in this software.
 *
 * This source code is released under the New BSD License.
 */

#ifndef _ICET_STRATEGY_COMMON_H_
#define _ICET_STRATEGY_COMMON_H_

#include <IceT.h>
#include <IceTDevImage.h>

/* icetRenderTransferFullImages

   Renders all the tiles that are specified in the ICET_CONTAINED_TILES
   state array and sends them to the processors with ranks specified in
   tile_image_dest.  This method is guaranteed not to deadlock.  It only
   uses memory given with the buffer arguments, and will make its best
   efforts to get the graphics and network hardware to run in parallel.

   image - An image big enough to hold color and/or depth values
        that is ICET_MAX_PIXELS big.  The results will be put in this
        image.
   inSparseImageBuffer - A buffer big enough to hold sparse color and
        depth information for an image that is ICET_MAX_PIXELS big.
        The size can be determined with the icetSparseImageBufferSize
        function in image.h.
   outSparseImage - A sparse image big enough to hold color and/or depth
        values that is ICET_MAX_PIXELS big.
   tile_image_dest - if tile t is in ICET_CONTAINED_TILES, then the
        rendered image for tile t is sent to tile_image_dest[t].

   This function returns an image object (using the image) containing the
   composited image send to this process.  The contents are undefined if nothing
   sent to this process.  */
void icetRenderTransferFullImages(IceTImage image,
                                  IceTVoid *inSparseImageBuffer,
                                  IceTSparseImage outSparseImage,
                                  IceTInt *tile_image_dest);

/* icetRenderTransferSparseImages

   Same as icetRenderTransferSparseImage except that it returns a sparse image
   rather than a full image.  Using this method can be more efficient if the
   resulting image is compressed afterward.

   compositeImage1, compositeImage2 - Sparse images big enough to hold color
        and/or depth values that are ICET_MAX_PIXELS big.  These buffers are
        used store composite results.
   inImageBuffer - A buffer big enough to hold sparse color and
        depth information for an image that is ICET_MAX_PIXELS big.
        The size can be determined with the icetSparseImageBufferSize
        function in image.h.
   outSparseImage - A sparse image big enough to hold color and/or depth values
        that is ICET_MAX_PIXELS big.
   tile_image_dest - if tile t is in ICET_CONTAINED_TILES, then the
        rendered image for tile t is sent to tile_image_dest[t].
   resultImage - Will be set to the image with the final results.  It will point
        to either compositeImage1 or compositeImage2 depending on which buffer
        the result happened to end in.

   This function returns an image object (using the imageBuffer)
   containing the composited image send to this process.  The contents
   are undefined if nothing sent to this process.
*/
void icetRenderTransferSparseImages(IceTSparseImage compositeImage1,
                                    IceTSparseImage compositeImage2,
                                    IceTVoid *inImageBuffer,
                                    IceTSparseImage outSparseImage,
                                    IceTInt *tile_image_dest,
                                    IceTSparseImage *resultImage);


/* icetSendRecvLargeMessages

   Similar to icetRenderTransferFullImages except that it works with
   generic data, data generators, and data handlers.  It takes a count of a
   number of messages to be sent and an array of ranks to send to.  Two
   callbacks are required.  One generates the data (so large data may be
   generated JIT to save memory) and the other handles incoming data.  The
   generate callback is run right before the data it returns is sent to a
   particular destination.  This callback will not be called again until
   the memory it returned is no longer in use, so the memory may be reused.
   As large messages come in, the handle callback is called.  As an
   optimization, if a process sends to itself, then that will be the first
   message created.  This gives the callback an opertunity to build its
   local data while waiting for incoming data.  The handle callback returns
   a pointer to a buffer to be used for the next large message receive.  It
   should be common for this message buffer to be reused too.

   numMessagesSending - A count of the number of large messages this
        processor is sending out.
   messageDestinations - An array of size numMessagesSending that contains
        the ranks of message destinations.
   messagesInOrder - If true, then messages will be received in the order
        specified by the ICET_PROCESS_ORDERS state variable.  The locally
        generated message will come in first, followed by messages adjacent to
        either the left or right.  If messagesInOrder is false, messages may
        come in an arbitrary order.
   generateDataFunc - A callback function that generates messages.  The
        function is given the index in messageDestinations and the rank of
        the destination as arguments.  The data of the message and the size
        of the message (in bytes) are returned.  The generateDataFunc will
        not be called again until the returned data is no longer in use.
        Thus the data may be reused.
   handleDataFunc - A callback function that processes messages.  The
        function is given the data buffer and the rank of the process that
        sent it.  The function is expected to return a buffer to use for
        the next message receive.  If the callback is finished with the
        buffer it was given, it is perfectly acceptable to return it again
        for reuse.
   incomingBuffer - A buffer to use for the first incoming message.
   bufferSize - The maximum size of a message.
   
*/
typedef IceTVoid *(*IceTGenerateData)(IceTInt id, IceTInt dest,
                                      IceTSizeType *size);
typedef void (*IceTHandleData)(void *buffer, IceTInt src);
void icetSendRecvLargeMessages(IceTInt numMessagesSending,
                               const IceTInt *messageDestinations,
                               IceTBoolean messagesInOrder,
                               IceTGenerateData generateDataFunc,
                               IceTHandleData handleDataFunc,
                               IceTVoid *incomingBuffer,
                               IceTSizeType bufferSize);

/* icetSingleImageCompose

   Performs a composition of a single image using the current
   ICET_SINGLE_IMAGE_STRATEGY.  The composition happens in the subset of
   processes given in compose_group (and group_size).  If ordered compositing is
   on, the images will be composited in the order determined by ranks in
   compose_group with the first process on top.  The resulting image is left
   partitioned amongst processes.  Use icetSingleImageCollect to combine the
   images.

   compose_group - A mapping of processors from the MPI ranks to the "group"
        ranks.  The composed image ends up in the processor with rank
        compose_group[image_dest].
   group_size - The number of processors in the group.  The compose_group
        array should have group_size entries.
   image_dest - The location of where the final composed image should be
        placed.  It is an index into compose_group, not the actual rank
        of the process.  This is a hint to the composite algorithm rather
        than a determination of where data will end up.
   input_image - The input image colors and/or depth to be used.  This buffer
        may (and probably will) be changed during the composition.  The end data
        is undefined.
   result_image - An image containing the results of the composition.  The image
        need not be (should not be) allocated before its pointer is passed to
        this function.  The results may be distributed amongst all the processes
        in the group.  That is, the final image will be partitioned amongst the
        processes and each process will have a separate piece.  The piece in
        each process is always a contiguous set of pixels starting at the offset
        set in piece_offset and the length of result_image.  result_image may or
        may not be set to input_image.  Use icetSparseImageEqual() to find out.
   piece_offset - The offset to the start of the valid pixels will be placed
        in this argument.
*/
void icetSingleImageCompose(const IceTInt *compose_group,
                            IceTInt group_size,
                            IceTInt image_dest,
                            IceTSparseImage input_image,
                            IceTSparseImage *result_image,
                            IceTSizeType *piece_offset);

/* icetSingleImageCollect

   Collects image partitions distributed amongst processes.  The intension is to
   call this function after a call to icetSingleImageCompose to complete the
   image composition.  Unlike icetSingleImageCompose, however, this function
   must be called on all processes, not just those in a group.  Processes that
   have no piece of the image should pass 0 for piece_offset and a null or other
   zero-size image for input_image.

   input_image - Contains the composited image partition returned from
        icetSingleImageCompose.
   dest - The rank of the process to where the image should be collected.  Be
        aware that this is generally a different value than the image_dest
        parameter of icetSingleImageCompose.
   piece_offset - The offset to the start of the valid pixels will be placed
        in this argument.  Same value as returned from icetSingleImageCompose.
   result_image - an allocated and sized image in which to place the
        uncompressed results of the collection.  */
void icetSingleImageCollect(const IceTSparseImage input_image,
                            IceTInt dest,
                            IceTSizeType piece_offset,
                            IceTImage result_image);

#endif /*_ICET_STRATEGY_COMMON_H_*/
