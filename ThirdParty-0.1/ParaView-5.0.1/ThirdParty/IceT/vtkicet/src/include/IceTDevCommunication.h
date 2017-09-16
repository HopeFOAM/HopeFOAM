/* -*- c -*- *******************************************************/
/*
 * Copyright (C) 2010 Sandia Corporation
 * Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 * the U.S. Government retains certain rights in this software.
 *
 * This source code is released under the New BSD License.
 */

#ifndef __IceTDevCommunication_h
#define __IceTDevCommunication_h

#include <IceT.h>

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

/* All of these methods call the associated method in the communicator for
   the current context. */
ICET_EXPORT IceTCommunicator icetCommDuplicate();
ICET_EXPORT IceTCommunicator icetCommSubset(int count,
                                            const IceTInt32 *ranks);
ICET_EXPORT void icetCommBarrier();
ICET_EXPORT void icetCommSend(const void *buf,
                              IceTSizeType count,
                              IceTEnum datatype,
                              int dest,
                              int tag);
ICET_EXPORT void icetCommRecv(void *buf,
                              IceTSizeType count,
                              IceTEnum datatype,
                              int src,
                              int tag);
ICET_EXPORT void icetCommSendrecv(const void *sendbuf,
                                  IceTSizeType sendcount,
                                  IceTEnum sendtype,
                                  int dest,
                                  int sendtag,
                                  void *recvbuf,
                                  IceTSizeType recvcount,
                                  IceTEnum recvtype,
                                  int src,
                                  int recvtag);
ICET_EXPORT void icetCommGather(const void *sendbuf,
                                IceTSizeType sendcount,
                                IceTEnum datatype,
                                void *recvbuf,
                                int root);
ICET_EXPORT void icetCommGatherv(const void *sendbuf,
                                 IceTSizeType sendcount,
                                 IceTEnum datatype,
                                 void *recvbuf,
                                 const IceTSizeType *recvcounts,
                                 const IceTSizeType *recvoffsets,
                                 int root);
ICET_EXPORT void icetCommAllgather(const void *sendbuf,
                                   IceTSizeType sendcount,
                                   IceTEnum type,
                                   void *recvbuf);
ICET_EXPORT void icetCommAlltoall(const void *sendbuf,
                                  IceTSizeType sendcount,
                                  IceTEnum type,
                                  void *recvbuf);
ICET_EXPORT IceTCommRequest icetCommIsend(const void *buf,
                                          IceTSizeType count,
                                          IceTEnum datatype,
                                          int dest,
                                          int tag);
ICET_EXPORT IceTCommRequest icetCommIrecv(void *buf,
                                          IceTSizeType count,
                                          IceTEnum datatype,
                                          int src,
                                          int tag);
ICET_EXPORT void icetCommWait(IceTCommRequest *request);
ICET_EXPORT int icetCommWaitany(int count, IceTCommRequest *array_of_requests);
ICET_EXPORT void icetCommWaitall(int count, IceTCommRequest *array_of_requests);
ICET_EXPORT int icetCommSize();
ICET_EXPORT int icetCommRank();

/* When used in place of sendbuf in one of the gathers, then this means that
 * the local process should skip sending to itself.  Instead, the correct
 * data is already in the destbuf.  For icetCommGather and icetCommGatherV,
 * using this only makes sense on the root process.  Implementations of
 * the gather functions in an IceTCommunicatorStruct should check for
 * the existance of this. */
#define ICET_IN_PLACE_COLLECT   ((void *)(-1))

/* These are convenience methods for finding a particular rank in a group (array
 * of process ids).  This is a common operation as IceT frequently uses an array
 * of process ids to represent a lightweight group.  If the specified rank is
 * not in the group, -1 is returned. */
ICET_EXPORT int icetFindRankInGroup(const int *group,
                                    IceTSizeType group_size,
                                    int rank_to_find);
ICET_EXPORT int icetFindMyRankInGroup(const int *group,
                                      IceTSizeType group_size);

#ifdef __cplusplus
}
#endif

#endif /*__icetDevCommunication_h*/
