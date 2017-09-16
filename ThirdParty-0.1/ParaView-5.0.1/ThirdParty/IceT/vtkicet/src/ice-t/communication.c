/* -*- c -*- *******************************************************/
/*
 * Copyright (C) 2010 Sandia Corporation
 * Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 * the U.S. Government retains certain rights in this software.
 *
 * This source code is released under the New BSD License.
 */

#include <IceTDevCommunication.h>

#include <IceT.h>
#include <IceTDevContext.h>
#include <IceTDevDiagnostics.h>
#include <IceTDevPorting.h>

#define icetAddSentBytes(num_sending)                                   \
    icetStateSetInteger(ICET_BYTES_SENT,                                \
                        icetUnsafeStateGetInteger(ICET_BYTES_SENT)[0]   \
                        + (num_sending))

#define icetAddSent(count, datatype)                                    \
    icetAddSentBytes((IceTInt)count*icetTypeWidth(datatype))

#define icetCommCheckCount(count)                                       \
    if (count > 1073741824) {                                           \
        icetRaiseWarning("Encountered a ridiculously large message.",   \
                         ICET_INVALID_VALUE);                           \
    }

IceTCommunicator icetCommDuplicate()
{
    IceTCommunicator comm = icetGetCommunicator();
    return comm->Duplicate(comm);
}

IceTCommunicator icetCommSubset(int count, const IceTInt32 *ranks)
{
    IceTCommunicator comm = icetGetCommunicator();
    return comm->Subset(comm, count, ranks);
}

void icetCommBarrier()
{
    IceTCommunicator comm = icetGetCommunicator();
    comm->Barrier(comm);
}

void icetCommSend(const void *buf,
                  IceTSizeType count,
                  IceTEnum datatype,
                  int dest,
                  int tag)
{
    IceTCommunicator comm = icetGetCommunicator();
    icetCommCheckCount(count);
    icetAddSent(count, datatype);
    comm->Send(comm, buf, (int)count, datatype, dest, tag);
}

void icetCommRecv(void *buf,
                  IceTSizeType count,
                  IceTEnum datatype,
                  int src,
                  int tag)
{
    IceTCommunicator comm = icetGetCommunicator();
    icetCommCheckCount(count);
    comm->Recv(comm, buf, (int)count, datatype, src, tag);
}

void icetCommSendrecv(const void *sendbuf,
                      IceTSizeType sendcount,
                      IceTEnum sendtype,
                      int dest,
                      int sendtag,
                      void *recvbuf,
                      IceTSizeType recvcount,
                      IceTEnum recvtype,
                      int src,
                      int recvtag)
{
    IceTCommunicator comm = icetGetCommunicator();
    icetCommCheckCount(sendcount);
    icetCommCheckCount(recvcount);
    icetAddSent(sendcount, sendtype);
    comm->Sendrecv(comm, sendbuf, (int)sendcount, sendtype, dest, sendtag,
                   recvbuf, (int)recvcount, recvtype, src, recvtag);
}

void icetCommGather(const void *sendbuf,
                    IceTSizeType sendcount,
                    IceTEnum datatype,
                    void *recvbuf,
                    int root)
{
    IceTCommunicator comm = icetGetCommunicator();
    icetCommCheckCount(sendcount);
    if (root != icetCommRank()) {
        icetAddSent(sendcount, datatype);
    }
#ifdef DEBUG
    comm->Barrier(comm);
#endif
    comm->Gather(comm, sendbuf, sendcount, datatype, recvbuf, root);
}

void icetCommGatherv(const void *sendbuf,
                     IceTSizeType sendcount,
                     IceTEnum datatype,
                     void *recvbuf,
                     const IceTSizeType *recvcounts,
                     const IceTSizeType *recvoffsets,
                     int root)
{
    IceTCommunicator comm = icetGetCommunicator();
    int *int_recvcounts;
    int *int_recvoffsets;
    icetCommCheckCount(sendcount);
    if (root == icetCommRank()) {
        if (sizeof(int) == sizeof(IceTSizeType)) {
            int_recvcounts = (int *)recvcounts;
            int_recvoffsets = (int *)recvoffsets;
        } else {
            int numproc = icetCommSize();
            int proc;
            int_recvcounts = icetGetStateBuffer(ICET_COMM_COUNT_BUF,
                                                numproc*sizeof(int));
            int_recvoffsets = icetGetStateBuffer(ICET_COMM_OFFSET_BUF,
                                                 numproc*sizeof(int));
            for (proc = 0; proc < numproc; proc++) {
                icetCommCheckCount(recvcounts[proc]);
                int_recvcounts[proc] = recvcounts[proc];
                icetCommCheckCount(recvoffsets[proc]);
                int_recvoffsets[proc] = recvoffsets[proc];
            }
        }
    } else {
        icetAddSent(sendcount, datatype);
        int_recvcounts = NULL;
        int_recvoffsets = NULL;
    }
#ifdef DEBUG
    comm->Barrier(comm);
#endif
    comm->Gatherv(comm,
                  sendbuf,
                  sendcount,
                  datatype,
                  recvbuf,
                  int_recvcounts,
                  int_recvoffsets,
                  root);
}

void icetCommAllgather(const void *sendbuf,
                       IceTSizeType sendcount,
                       IceTEnum datatype,
                       void *recvbuf)
{
    IceTCommunicator comm = icetGetCommunicator();
    icetCommCheckCount(sendcount);
    icetAddSent(sendcount, datatype);
    comm->Allgather(comm, sendbuf, (int)sendcount, datatype, recvbuf);
}

void icetCommAlltoall(const void *sendbuf,
                      IceTSizeType sendcount,
                      IceTEnum datatype,
                      void *recvbuf)
{
    IceTCommunicator comm = icetGetCommunicator();
    icetCommCheckCount(sendcount);
    icetAddSent(sendcount, datatype);
    comm->Alltoall(comm, sendbuf, (int)sendcount, datatype, recvbuf);
}

IceTCommRequest icetCommIsend(const void *buf,
                              IceTSizeType count,
                              IceTEnum datatype,
                              int dest,
                              int tag)
{
    IceTCommunicator comm = icetGetCommunicator();
    icetCommCheckCount(count);
    icetAddSent(count, datatype);
    return comm->Isend(comm, buf, (int)count, datatype, dest, tag);
}

IceTCommRequest icetCommIrecv(void *buf,
                              IceTSizeType count,
                              IceTEnum datatype,
                              int src,
                              int tag)
{
    IceTCommunicator comm = icetGetCommunicator();
    icetCommCheckCount(count);
    return comm->Irecv(comm, buf, (int)count, datatype, src, tag);
}

void icetCommWait(IceTCommRequest *request)
{
    IceTCommunicator comm = icetGetCommunicator();
    comm->Wait(comm, request);
}

int icetCommWaitany(int count, IceTCommRequest *array_of_requests)
{
    IceTCommunicator comm = icetGetCommunicator();
    return comm->Waitany(comm, count, array_of_requests);
}

void icetCommWaitall(int count, IceTCommRequest *array_of_requests)
{
    int i;
    for (i = 0; i < count; i++) {
        icetCommWait(&array_of_requests[i]);
    }
}

int icetCommSize()
{
    IceTCommunicator comm = icetGetCommunicator();
    return comm->Comm_size(comm);
}

int icetCommRank()
{
    IceTCommunicator comm = icetGetCommunicator();
    return comm->Comm_rank(comm);
}


int icetFindRankInGroup(const int *group,
                        IceTSizeType group_size,
                        int rank_to_find)
{
    IceTSizeType i;
    for (i = 0; i < group_size; i++) {
        if (group[i] == rank_to_find) {
            return i;
        }
    }

    return -1;
}

int icetFindMyRankInGroup(const int *group,
                          IceTSizeType group_size)
{
    IceTInt rank;

    icetGetIntegerv(ICET_RANK, &rank);
    return icetFindRankInGroup(group, group_size, rank);
}
