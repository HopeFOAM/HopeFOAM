/* -*- c -*- *******************************************************/
/*
 * Copyright (C) 2003 Sandia Corporation
 * Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 * the U.S. Government retains certain rights in this software.
 *
 * This source code is released under the New BSD License.
 */

#ifndef _TEST_MPI_H_
#define _TEST_MPI_H_

#include "test_util.h"
#include <IceTMPI.h>

extern int run_test_base(int (*test_function)(void));

void init_mpi(int *argcp, char ***argvp)
{
    IceTCommunicator comm;

    MPI_Init(argcp, argvp);
    comm = icetCreateMPICommunicator(MPI_COMM_WORLD);

    initialize_test(argcp, argvp, comm);

    icetDestroyMPICommunicator(comm);
}

void finalize_communication(void)
{
    MPI_Finalize();
}

#ifndef ICET_NO_MPI_RENDERING_FUNCTIONS
int run_test(int (*test_function)())
{
    return run_test_base(test_function);
}
void initialize_render_window(int width, int height)
{
    (void)width;
    (void)height;
}
void swap_buffers() {  }
void finalize_rendering() {  }
#endif /* !ICET_NO_MPI_RENDERING_FUNCTIONS */

#endif /*_TEST_MPI_H_*/
