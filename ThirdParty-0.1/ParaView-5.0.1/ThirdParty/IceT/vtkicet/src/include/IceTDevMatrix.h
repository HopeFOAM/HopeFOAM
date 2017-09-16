/* -*- c -*- *******************************************************/
/*
 * Copyright (C) 2010 Sandia Corporation
 * Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 * the U.S. Government retains certain rights in this software.
 *
 * This source code is released under the New BSD License.
 */

#ifndef __IceTDevMatrix_h
#define __IceTDevMatrix_h

#include <IceT.h>

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

/* Macro to reference an entry in a matrix. */
#define ICET_MATRIX(matrix, row, column) (matrix)[(column)*4+(row)]

/* Multiplies A and B.  Puts the result in C. */
ICET_EXPORT void icetMatrixMultiply(IceTDouble *C,
                                    const IceTDouble *A,
                                    const IceTDouble *B);

/* Performs an in-place post multiply.  Computes A * B and puts the result in
   A. */
ICET_EXPORT void icetMatrixPostMultiply(IceTDouble *A,
                                        const IceTDouble *B);

/* Multiplies A (a 4x4 matrix) and v (a 4 vector) and returns the result (a 4
 * vector) in out. */
ICET_EXPORT void icetMatrixVectorMultiply(IceTDouble *out,
                                          const IceTDouble *A,
                                          const IceTDouble *v);

/* Returns the dot product of two vector 3's. */
#define icetDot3(v1, v2) ((v1)[0]*(v2)[0] + (v1)[1]*(v2)[1] + (v1)[2]*(v2)[2])

/* Returns the dot product of two vector 4's. */
#define icetDot4(v1, v2) (icetDot3(v1, v2) + (v1)[3]*(v2)[3])

/* Copies matrix_src to matrix_dest. */
ICET_EXPORT void icetMatrixCopy(IceTDouble *matrix_dest,
                                const IceTDouble *matrix_src);

/* Returns the identity matrix. */
ICET_EXPORT void icetMatrixIdentity(IceTDouble *mat_out);

/* Returns an orthographic projection that is equivalent to glOrtho. */
ICET_EXPORT void icetMatrixOrtho(IceTDouble left, IceTDouble right,
                                 IceTDouble bottom, IceTDouble top,
                                 IceTDouble znear, IceTDouble zfar,
                                 IceTDouble *mat_out);

/* Returns a frustum projection that is equivalent to glFrustum. */
ICET_EXPORT void icetMatrixFrustum(IceTDouble left, IceTDouble right,
                                   IceTDouble bottom, IceTDouble top,
                                   IceTDouble znear, IceTDouble zfar,
                                   IceTDouble *mat_out);

/* Returns a scaling transform matrix that is equivalent to glScale. */
ICET_EXPORT void icetMatrixScale(IceTDouble x, IceTDouble y, IceTDouble z,
                                 IceTDouble *mat_out);

/* Returns a translation transform matrix that is equivalent to glTranslate. */
ICET_EXPORT void icetMatrixTranslate(IceTDouble x, IceTDouble y, IceTDouble z,
                                     IceTDouble *mat_out);

/* Returns a rotation transform matrix that is equivalent to glRotate. */
ICET_EXPORT void icetMatrixRotate(IceTDouble angle,
                                  IceTDouble x, IceTDouble y, IceTDouble z,
                                  IceTDouble *mat_out);

/* Like the above functions except the result is post multiplied to the
 * given matrix and returned again in that matrix. */
ICET_EXPORT void icetMatrixMultiplyScale(IceTDouble *mat_out,
                                         IceTDouble x,
                                         IceTDouble y,
                                         IceTDouble z);
ICET_EXPORT void icetMatrixMultiplyTranslate(IceTDouble *mat_out,
                                             IceTDouble x,
                                             IceTDouble y,
                                             IceTDouble z);
ICET_EXPORT void icetMatrixMultiplyRotate(IceTDouble *mat_out,
                                          IceTDouble angle,
                                          IceTDouble x,
                                          IceTDouble y,
                                          IceTDouble z);

/* Computes the inverse of the given matrix.  Returns ICET_TRUE if successful,
 * ICET_FALSE if the matrix has no inverse. */
ICET_EXPORT IceTBoolean icetMatrixInverse(const IceTDouble *matrix_in,
                                          IceTDouble *matrix_out);

/* Computes the transpose of the given matrix. */
ICET_EXPORT void icetMatrixTranspose(const IceTDouble *matrix_in,
                                     IceTDouble *matrix_out);

/* Computes the inverse transpose of the given matrix. */
ICET_EXPORT IceTBoolean icetMatrixInverseTranspose(const IceTDouble *matrix_in,
                                                   IceTDouble *matrix_out);

#ifdef __cplusplus
}
#endif

#endif /*__IceTDevMatrix_h*/
