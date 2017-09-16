/* -*- c -*- *******************************************************/
/*
 * Copyright (C) 2003 Sandia Corporation
 * Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 * the U.S. Government retains certain rights in this software.
 *
 * This source code is released under the New BSD License.
 */

#include <IceTMPI.h>

#include <IceTDevCommunication.h>
#include <IceTDevDiagnostics.h>
#include <IceTDevPorting.h>
#include <IceTDevState.h>

#include <stdlib.h>
#include <string.h>

#ifdef DEBUG
#define BREAK_ON_MPI_ERROR
#endif

#if MPI_VERSION >= 2
#define ICET_USE_MPI_IN_PLACE
#endif

#define ICET_MPI_REQUEST_MAGIC_NUMBER ((IceTEnum)0xD7168B00)

#define ICET_MPI_TEMP_BUFFER_0  (ICET_COMMUNICATION_LAYER_START | (IceTEnum)0x00)

static IceTCommunicator MPIDuplicate(IceTCommunicator self);
static IceTCommunicator MPISubset(IceTCommunicator self,
                                  int count,
                                  const IceTInt32 *ranks);
static void MPIDestroy(IceTCommunicator self);
static void MPIBarrier(IceTCommunicator self);
static void MPISend(IceTCommunicator self,
                    const void *buf,
                    int count,
                    IceTEnum datatype,
                    int dest,
                    int tag);
static void MPIRecv(IceTCommunicator self,
                    void *buf,
                    int count,
                    IceTEnum datatype,
                    int src,
                    int tag);
static void MPISendrecv(IceTCommunicator self,
                        const void *sendbuf,
                        int sendcount,
                        IceTEnum sendtype,
                        int dest,
                        int sendtag,
                        void *recvbuf,
                        int recvcount,
                        IceTEnum recvtype,
                        int src,
                        int recvtag);
static void MPIGather(IceTCommunicator self,
                      const void *sendbuf,
                      int sendcount,
                      IceTEnum datatype,
                      void *recvbuf,
                      int root);
static void MPIGatherv(IceTCommunicator self,
                       const void *sendbuf,
                       int sendcount,
                       IceTEnum datatype,
                       void *recvbuf,
                       const int *recvcounts,
                       const int *recvoffsets,
                       int root);
static void MPIAllgather(IceTCommunicator self,
                         const void *sendbuf,
                         int sendcount,
                         IceTEnum datatype,
                         void *recvbuf);
static void MPIAlltoall(IceTCommunicator self,
                        const void *sendbuf,
                        int sendcount,
                        IceTEnum datatype,
                        void *recvbuf);
static IceTCommRequest MPIIsend(IceTCommunicator self,
                                const void *buf,
                                int count,
                                IceTEnum datatype,
                                int dest,
                                int tag);
static IceTCommRequest MPIIrecv(IceTCommunicator self,
                                void *buf,
                                int count,
                                IceTEnum datatype,
                                int src,
                                int tag);
static void MPIWaitone(IceTCommunicator self, IceTCommRequest *request);
static int  MPIWaitany(IceTCommunicator self,
                       int count, IceTCommRequest *array_of_requests);
static int MPIComm_size(IceTCommunicator self);
static int MPIComm_rank(IceTCommunicator self);

typedef struct IceTMPICommRequestInternalsStruct {
    MPI_Request request;
} *IceTMPICommRequestInternals;

static MPI_Request getMPIRequest(IceTCommRequest icet_request)
{
    if (icet_request == ICET_COMM_REQUEST_NULL) {
        return MPI_REQUEST_NULL;
    }

    if (icet_request->magic_number != ICET_MPI_REQUEST_MAGIC_NUMBER) {
        icetRaiseError("Request object is not from the MPI communicator.",
                       ICET_INVALID_VALUE);
        return MPI_REQUEST_NULL;
    }

    return (((IceTMPICommRequestInternals)icet_request->internals)->request);
}

static void setMPIRequest(IceTCommRequest icet_request, MPI_Request mpi_request)
{
    if (icet_request == ICET_COMM_REQUEST_NULL) {
        icetRaiseError("Cannot set MPI request in null request.",
                       ICET_SANITY_CHECK_FAIL);
        return;
    }

    if (icet_request->magic_number != ICET_MPI_REQUEST_MAGIC_NUMBER) {
        icetRaiseError("Request object is not from the MPI communicator.",
                       ICET_SANITY_CHECK_FAIL);
        return;
    }

    (((IceTMPICommRequestInternals)icet_request->internals)->request)
        = mpi_request;
}

static IceTCommRequest create_request(void)
{
    IceTCommRequest request;

    request = (IceTCommRequest)malloc(sizeof(struct IceTCommRequestStruct));
    if (request == NULL) {
        icetRaiseError("Could not allocate memory for IceTCommRequest",
                       ICET_OUT_OF_MEMORY);
        return NULL;
    }

    request->magic_number = ICET_MPI_REQUEST_MAGIC_NUMBER;
    request->internals=malloc(sizeof(struct IceTMPICommRequestInternalsStruct));
    if (request->internals == NULL) {
        free(request);
        icetRaiseError("Could not allocate memory for IceTCommRequest",
                       ICET_OUT_OF_MEMORY);
        return NULL;
    }

    setMPIRequest(request, MPI_REQUEST_NULL);

    return request;
}

static void destroy_request(IceTCommRequest request)
{
    MPI_Request mpi_request = getMPIRequest(request);
    if (mpi_request != MPI_REQUEST_NULL) {
        icetRaiseError("Destroying MPI request that is not NULL."
                       " Probably leaking MPI requests.",
                       ICET_SANITY_CHECK_FAIL);
    }

    free(request->internals);
    free(request);
}

#ifdef BREAK_ON_MPI_ERROR
static void ErrorHandler(MPI_Comm *comm, int *errorno, ...)
{
    char error_msg[MPI_MAX_ERROR_STRING+16];
    int mpi_error_len;
    (void)comm;

    strcpy(error_msg, "MPI ERROR:\n");
    MPI_Error_string(*errorno, error_msg + strlen(error_msg), &mpi_error_len);

    icetRaiseError(error_msg, ICET_INVALID_OPERATION);
    icetDebugBreak();
}
#endif

IceTCommunicator icetCreateMPICommunicator(MPI_Comm mpi_comm)
{
    IceTCommunicator comm;
#ifdef BREAK_ON_MPI_ERROR
    MPI_Errhandler eh;
#endif

    if (mpi_comm == MPI_COMM_NULL) {
        return ICET_COMM_NULL;
    }

    comm = malloc(sizeof(struct IceTCommunicatorStruct));
    if (comm == NULL) {
        icetRaiseError("Could not allocate memory for IceTCommunicator.",
                       ICET_OUT_OF_MEMORY);
        return NULL;
    }

    comm->Duplicate = MPIDuplicate;
    comm->Subset = MPISubset;
    comm->Destroy = MPIDestroy;
    comm->Barrier = MPIBarrier;
    comm->Send = MPISend;
    comm->Recv = MPIRecv;
    comm->Sendrecv = MPISendrecv;
    comm->Gather = MPIGather;
    comm->Gatherv = MPIGatherv;
    comm->Allgather = MPIAllgather;
    comm->Alltoall = MPIAlltoall;
    comm->Isend = MPIIsend;
    comm->Irecv = MPIIrecv;
    comm->Wait = MPIWaitone;
    comm->Waitany = MPIWaitany;
    comm->Comm_size = MPIComm_size;
    comm->Comm_rank = MPIComm_rank;

    comm->data = malloc(sizeof(MPI_Comm));
    if (comm->data == NULL) {
        free(comm);
        icetRaiseError("Could not allocate memory for IceTCommunicator.",
                       ICET_OUT_OF_MEMORY);
        return NULL;
    }
    MPI_Comm_dup(mpi_comm, (MPI_Comm *)comm->data);

#ifdef BREAK_ON_MPI_ERROR
#if MPI_VERSION < 2
    MPI_Errhandler_create(ErrorHandler, &eh);
    MPI_Errhandler_set(*((MPI_Comm *)comm->data), eh);
    MPI_Errhandler_free(&eh);
#else /* MPI_VERSION >= 2 */
    MPI_Comm_create_errhandler(ErrorHandler, &eh);
    MPI_Comm_set_errhandler(*((MPI_Comm *)comm->data), eh);
    MPI_Errhandler_free(&eh);
#endif /* MPI_VERSION >= 2 */
#endif

    return comm;
}

void icetDestroyMPICommunicator(IceTCommunicator comm)
{
    if (comm != ICET_COMM_NULL) {
        comm->Destroy(comm);
    }
}


#define MPI_COMM        (*((MPI_Comm *)self->data))

static IceTCommunicator MPIDuplicate(IceTCommunicator self)
{
    if (self != ICET_COMM_NULL) {
        return icetCreateMPICommunicator(MPI_COMM);
    } else {
        return ICET_COMM_NULL;
    }
}

static IceTCommunicator MPISubset(IceTCommunicator self,
                                  int count,
                                  const IceTInt32 *ranks)
{
    MPI_Group original_group;
    MPI_Group subset_group;
    MPI_Comm subset_comm;
    IceTCommunicator result;

    MPI_Comm_group(MPI_COMM, &original_group);
    MPI_Group_incl(original_group, count, (IceTInt32 *)ranks, &subset_group);
    MPI_Comm_create(MPI_COMM, subset_group, &subset_comm);

    result = icetCreateMPICommunicator(subset_comm);

    if (subset_comm != MPI_COMM_NULL) {
        MPI_Comm_free(&subset_comm);
    }

    MPI_Group_free(&subset_group);
    MPI_Group_free(&original_group);

    return result;
}

static void MPIDestroy(IceTCommunicator self)
{
    MPI_Comm_free((MPI_Comm *)self->data);
    free(self->data);
    free(self);
}

static void MPIBarrier(IceTCommunicator self)
{
    MPI_Barrier(MPI_COMM);
}

#define CONVERT_DATATYPE(icet_type, mpi_type)                                \
    switch (icet_type) {                                                     \
      case ICET_BOOLEAN:mpi_type = MPI_BYTE;    break;                       \
      case ICET_BYTE:   mpi_type = MPI_BYTE;    break;                       \
      case ICET_SHORT:  mpi_type = MPI_SHORT;   break;                       \
      case ICET_INT:    mpi_type = MPI_INT;     break;                       \
      case ICET_FLOAT:  mpi_type = MPI_FLOAT;   break;                       \
      case ICET_DOUBLE: mpi_type = MPI_DOUBLE;  break;                       \
      default:                                                               \
          icetRaiseError("MPI Communicator received bad data type.",         \
                         ICET_INVALID_ENUM);                                 \
          mpi_type = MPI_BYTE;                                               \
          break;                                                             \
    }

static void MPISend(IceTCommunicator self,
                    const void *buf,
                    int count,
                    IceTEnum datatype,
                    int dest,
                    int tag)
{
    MPI_Datatype mpidatatype;
    CONVERT_DATATYPE(datatype, mpidatatype);
    MPI_Send((void *)buf, count, mpidatatype, dest, tag, MPI_COMM);
}

static void MPIRecv(IceTCommunicator self,
                    void *buf,
                    int count,
                    IceTEnum datatype,
                    int src,
                    int tag)
{
    MPI_Datatype mpidatatype;
    CONVERT_DATATYPE(datatype, mpidatatype);
    MPI_Recv(buf, count, mpidatatype, src, tag, MPI_COMM, MPI_STATUS_IGNORE);
}

static void MPISendrecv(IceTCommunicator self,
                        const void *sendbuf,
                        int sendcount,
                        IceTEnum sendtype,
                        int dest,
                        int sendtag,
                        void *recvbuf,
                        int recvcount,
                        IceTEnum recvtype,
                        int src,
                        int recvtag)
{
    MPI_Datatype mpisendtype;
    MPI_Datatype mpirecvtype;
    CONVERT_DATATYPE(sendtype, mpisendtype);
    CONVERT_DATATYPE(recvtype, mpirecvtype);

    MPI_Sendrecv((void *)sendbuf, sendcount, mpisendtype, dest, sendtag,
                 recvbuf, recvcount, mpirecvtype, src, recvtag, MPI_COMM,
                 MPI_STATUS_IGNORE);
}

static void MPIGather(IceTCommunicator self,
                      const void *sendbuf,
                      int sendcount,
                      IceTEnum datatype,
                      void *recvbuf,
                      int root)
{
    MPI_Datatype mpitype;
    CONVERT_DATATYPE(datatype, mpitype);

    if (sendbuf == ICET_IN_PLACE_COLLECT) {
#ifdef ICET_USE_MPI_IN_PLACE
        sendbuf = MPI_IN_PLACE;
#else
        int rank;
        MPI_Comm_rank(MPI_COMM, &rank);
        sendbuf = icetGetStateBuffer(ICET_MPI_TEMP_BUFFER_0,
                                     sendcount*icetTypeWidth(datatype));
        memcpy((void *)sendbuf,
               ((const IceTByte *)recvbuf) + rank*sendcount,
               sendcount);
#endif
    }

    MPI_Gather((void *)sendbuf, sendcount, mpitype,
               recvbuf, sendcount, mpitype, root,
               MPI_COMM);
}

static void MPIGatherv(IceTCommunicator self,
                       const void *sendbuf,
                       int sendcount,
                       IceTEnum datatype,
                       void *recvbuf,
                       const int *recvcounts,
                       const int *recvoffsets,
                       int root)
{
    MPI_Datatype mpitype;
    CONVERT_DATATYPE(datatype, mpitype);

    if (sendbuf == ICET_IN_PLACE_COLLECT) {
#ifdef ICET_USE_MPI_IN_PLACE
        sendbuf = MPI_IN_PLACE;
#else
        int rank;
        MPI_Comm_rank(MPI_COMM, &rank);
        sendcount = recvcounts[rank];
        sendbuf = icetGetStateBuffer(ICET_MPI_TEMP_BUFFER_0,
                                     sendcount*icetTypeWidth(datatype));
        memcpy((void *)sendbuf,
               ((const IceTByte *)recvbuf) + recvoffsets[rank],
               sendcount);
#endif
    }

    MPI_Gatherv((void *)sendbuf, sendcount, mpitype,
                recvbuf, (int *)recvcounts, (int *)recvoffsets, mpitype,
                root, MPI_COMM);
}

static void MPIAllgather(IceTCommunicator self,
                         const void *sendbuf,
                         int sendcount,
                         IceTEnum datatype,
                         void *recvbuf)
{
    MPI_Datatype mpitype;
    CONVERT_DATATYPE(datatype, mpitype);

    if (sendbuf == ICET_IN_PLACE_COLLECT) {
#ifdef ICET_USE_MPI_IN_PLACE
        sendbuf = MPI_IN_PLACE;
#else
        int rank;
        MPI_Comm_rank(MPI_COMM, &rank);
        sendbuf = icetGetStateBuffer(ICET_MPI_TEMP_BUFFER_0,
                                     sendcount*icetTypeWidth(datatype));
        memcpy((void *)sendbuf,
               ((const IceTByte *)recvbuf) + rank*sendcount,
               sendcount);
#endif
    }

    MPI_Allgather((void *)sendbuf, sendcount, mpitype,
                  recvbuf, sendcount, mpitype,
                  MPI_COMM);
}

static void MPIAlltoall(IceTCommunicator self,
                        const void *sendbuf,
                        int sendcount,
                        IceTEnum datatype,
                        void *recvbuf)
{
    MPI_Datatype mpitype;
    CONVERT_DATATYPE(datatype, mpitype);

    MPI_Alltoall((void *)sendbuf, sendcount, mpitype,
                 recvbuf, sendcount, mpitype,
                 MPI_COMM);
}

static IceTCommRequest MPIIsend(IceTCommunicator self,
                                const void *buf,
                                int count,
                                IceTEnum datatype,
                                int dest,
                                int tag)
{
    IceTCommRequest icet_request;
    MPI_Request mpi_request;
    MPI_Datatype mpidatatype;

    CONVERT_DATATYPE(datatype, mpidatatype);
    MPI_Isend((void *)buf, count, mpidatatype, dest, tag, MPI_COMM,
              &mpi_request);

    icet_request = create_request();
    setMPIRequest(icet_request, mpi_request);

    return icet_request;
}

static IceTCommRequest MPIIrecv(IceTCommunicator self,
                                void *buf,
                                int count,
                                IceTEnum datatype,
                                int src,
                                int tag)
{
    IceTCommRequest icet_request;
    MPI_Request mpi_request;
    MPI_Datatype mpidatatype;

    CONVERT_DATATYPE(datatype, mpidatatype);
    MPI_Irecv(buf, count, mpidatatype, src, tag, MPI_COMM,
              &mpi_request);

    icet_request = create_request();
    setMPIRequest(icet_request, mpi_request);

    return icet_request;
}

static void MPIWaitone(IceTCommunicator self, IceTCommRequest *icet_request)
{
    MPI_Request mpi_request;

    /* To remove warning */
    (void)self;

    if (*icet_request == ICET_COMM_REQUEST_NULL) return;

    mpi_request = getMPIRequest(*icet_request);
    MPI_Wait(&mpi_request, MPI_STATUS_IGNORE);
    setMPIRequest(*icet_request, mpi_request);

    destroy_request(*icet_request);
    *icet_request = ICET_COMM_REQUEST_NULL;
}

static int  MPIWaitany(IceTCommunicator self,
                       int count, IceTCommRequest *array_of_requests)
{
    MPI_Request *mpi_requests;
    int idx;

    /* To remove warning */
    (void)self;

    mpi_requests = malloc(sizeof(MPI_Request)*count);
    if (mpi_requests == NULL) {
        icetRaiseError("Could not allocate array for MPI requests.",
                       ICET_OUT_OF_MEMORY);
        return -1;
    }

    for (idx = 0; idx < count; idx++) {
        mpi_requests[idx] = getMPIRequest(array_of_requests[idx]);
    }

    MPI_Waitany(count, mpi_requests, &idx, MPI_STATUS_IGNORE);

    setMPIRequest(array_of_requests[idx], mpi_requests[idx]);
    destroy_request(array_of_requests[idx]);
    array_of_requests[idx] = ICET_COMM_REQUEST_NULL;

    free(mpi_requests);

    return idx;
}

static int MPIComm_size(IceTCommunicator self)
{
    int size;
    MPI_Comm_size(MPI_COMM, &size);
    return size;
}

static int MPIComm_rank(IceTCommunicator self)
{
    int rank;
    MPI_Comm_rank(MPI_COMM, &rank);
    return rank;
}
