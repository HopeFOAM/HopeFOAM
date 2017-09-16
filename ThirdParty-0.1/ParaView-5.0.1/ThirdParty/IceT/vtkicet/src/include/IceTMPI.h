/* -*- c -*- *******************************************************/
/*
 * Copyright (C) 2003 Sandia Corporation
 * Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 * the U.S. Government retains certain rights in this software.
 *
 * This source code is released under the New BSD License.
 */

#ifndef __IceTMPI_h
#define __IceTMPI_h

#include <IceT.h>
#include <mpi.h>

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

ICET_MPI_EXPORT IceTCommunicator icetCreateMPICommunicator(MPI_Comm mpi_comm);
ICET_MPI_EXPORT void icetDestroyMPICommunicator(IceTCommunicator comm);

#ifdef __cplusplus
}
#endif

#endif /*__IceTMPI_h*/
