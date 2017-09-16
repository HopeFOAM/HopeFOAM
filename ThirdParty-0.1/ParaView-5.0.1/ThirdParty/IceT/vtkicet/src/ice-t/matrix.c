/* -*- c -*- *******************************************************/
/*
 * Copyright (C) 2010 Sandia Corporation
 * Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 * the U.S. Government retains certain rights in this software.
 *
 * This source code is released under the New BSD License.
 */

#include <IceTDevMatrix.h>

#include <math.h>

#ifndef M_PI
#define M_PI        3.14159265358979323846264338327950288   /* pi */
#endif

#define MAT(matrix, row, column) ICET_MATRIX(matrix, row, column)

void icetMatrixMultiply(IceTDouble *C, const IceTDouble *A, const IceTDouble *B)
{
    int row, column, k;

    for (row = 0; row < 4; row++) {
        for (column = 0; column < 4; column++) {
            MAT(C, row, column) = 0.0;
            for (k = 0; k < 4; k++) {
                MAT(C, row, column) += MAT(A, row, k) * MAT(B, k, column);
            }
        }
    }
}

void icetMatrixPostMultiply(IceTDouble *A, const IceTDouble *B)
{
    IceTDouble product[16];

    icetMatrixMultiply(product, (const IceTDouble *)A, B);
    icetMatrixCopy(A, (const IceTDouble *)product);
}

void icetMatrixVectorMultiply(IceTDouble *out,
                              const IceTDouble *A,
                              const IceTDouble *v)
{
    int row, k;
    for (row = 0; row < 4; row++) {
        out[row] = 0.0;
        for (k = 0; k < 4; k++) {
            out[row] += MAT(A, row, k) * v[k];
        }
    }
}

void icetMatrixCopy(IceTDouble *matrix_dest, const IceTDouble *matrix_src)
{
    int i;
    for (i = 0; i < 16; i++) {
        matrix_dest[i] = matrix_src[i];
    }
}

ICET_EXPORT void icetMatrixIdentity(IceTDouble *mat_out)
{
    mat_out[ 0] = 1.0;
    mat_out[ 1] = 0.0;
    mat_out[ 2] = 0.0;
    mat_out[ 3] = 0.0;

    mat_out[ 4] = 0.0;
    mat_out[ 5] = 1.0;
    mat_out[ 6] = 0.0;
    mat_out[ 7] = 0.0;

    mat_out[ 8] = 0.0;
    mat_out[ 9] = 0.0;
    mat_out[10] = 1.0;
    mat_out[11] = 0.0;

    mat_out[12] = 0.0;
    mat_out[13] = 0.0;
    mat_out[14] = 0.0;
    mat_out[15] = 1.0;
}

ICET_EXPORT void icetMatrixOrtho(IceTDouble left, IceTDouble right,
                                 IceTDouble bottom, IceTDouble top,
                                 IceTDouble znear, IceTDouble zfar,
                                 IceTDouble *mat_out)
{
    mat_out[ 0] = 2.0/(right-left);
    mat_out[ 1] = 0.0;
    mat_out[ 2] = 0.0;
    mat_out[ 3] = 0.0;

    mat_out[ 4] = 0.0;
    mat_out[ 5] = 2.0/(top-bottom);
    mat_out[ 6] = 0.0;
    mat_out[ 7] = 0.0;

    mat_out[ 8] = 0.0;
    mat_out[ 9] = 0.0;
    mat_out[10] = -2.0/(zfar - znear);
    mat_out[11] = 0.0;

    mat_out[12] = -(right+left)/(right-left);
    mat_out[13] = -(top+bottom)/(top-bottom);
    mat_out[14] = -(zfar+znear)/(zfar-znear);
    mat_out[15] = 1.0;
}

ICET_EXPORT void icetMatrixFrustum(IceTDouble left, IceTDouble right,
                                   IceTDouble bottom, IceTDouble top,
                                   IceTDouble znear, IceTDouble zfar,
                                   IceTDouble *mat_out)
{
    mat_out[ 0] = 2.0*znear/(right-left);
    mat_out[ 1] = 0.0;
    mat_out[ 2] = 0.0;
    mat_out[ 3] = 0.0;

    mat_out[ 4] = 0.0;
    mat_out[ 5] = 2.0*znear/(top-bottom);
    mat_out[ 6] = 0.0;
    mat_out[ 7] = 0.0;

    mat_out[ 8] = (right+left)/(right-left);
    mat_out[ 9] = (top+bottom)/(top-bottom);
    mat_out[10] = -(zfar+znear)/(zfar-znear);
    mat_out[11] = -1.0;

    mat_out[12] = 0.0;
    mat_out[13] = 0.0;
    mat_out[14] = -2.0*zfar*znear/(zfar-znear);
    mat_out[15] = 0.0;
}

ICET_EXPORT void icetMatrixScale(IceTDouble x, IceTDouble y, IceTDouble z,
                                 IceTDouble *mat_out)
{
    mat_out[ 0] = x;
    mat_out[ 1] = 0.0;
    mat_out[ 2] = 0.0;
    mat_out[ 3] = 0.0;

    mat_out[ 4] = 0.0;
    mat_out[ 5] = y;
    mat_out[ 6] = 0.0;
    mat_out[ 7] = 0.0;

    mat_out[ 8] = 0.0;
    mat_out[ 9] = 0.0;
    mat_out[10] = z;
    mat_out[11] = 0.0;

    mat_out[12] = 0.0;
    mat_out[13] = 0.0;
    mat_out[14] = 0.0;
    mat_out[15] = 1.0;
}

ICET_EXPORT void icetMatrixTranslate(IceTDouble x, IceTDouble y, IceTDouble z,
                                     IceTDouble *mat_out)
{
    mat_out[ 0] = 1.0;
    mat_out[ 1] = 0.0;
    mat_out[ 2] = 0.0;
    mat_out[ 3] = 0.0;

    mat_out[ 4] = 0.0;
    mat_out[ 5] = 1.0;
    mat_out[ 6] = 0.0;
    mat_out[ 7] = 0.0;

    mat_out[ 8] = 0.0;
    mat_out[ 9] = 0.0;
    mat_out[10] = 1.0;
    mat_out[11] = 0.0;

    mat_out[12] = x;
    mat_out[13] = y;
    mat_out[14] = z;
    mat_out[15] = 1.0;
}

ICET_EXPORT void icetMatrixRotate(IceTDouble angle,
                                  IceTDouble x, IceTDouble y, IceTDouble z,
                                  IceTDouble *mat_out)
{
    IceTDouble v[3];
    IceTDouble length;
    IceTDouble c;
    IceTDouble s;

    v[0] = x;  v[1] = y;  v[2] = z;
    length = sqrt(icetDot3(v, v));
    v[0] /= length;  v[1] /= length;  v[2] /= length;

    c = cos((M_PI/180.0)*angle);
    s = sin((M_PI/180.0)*angle);

    mat_out[ 0] = v[0]*v[0]*(1-c) + c;
    mat_out[ 1] = v[0]*v[1]*(1-c) + v[2]*s;
    mat_out[ 2] = v[0]*v[2]*(1-c) - v[1]*s;
    mat_out[ 3] = 0.0;

    mat_out[ 4] = v[0]*v[1]*(1-c) - v[2]*s;
    mat_out[ 5] = v[1]*v[1]*(1-c) + c;
    mat_out[ 6] = v[1]*v[2]*(1-c) + v[0]*s;
    mat_out[ 7] = 0.0;

    mat_out[ 8] = v[0]*v[2]*(1-c) + v[1]*s;
    mat_out[ 9] = v[1]*v[2]*(1-c) - v[0]*s;
    mat_out[10] = v[2]*v[2]*(1-c) + c;
    mat_out[11] = 0.0;

    mat_out[12] = 0.0;
    mat_out[13] = 0.0;
    mat_out[14] = 0.0;
    mat_out[15] = 1.0;
}

void icetMatrixMultiplyScale(IceTDouble *mat_out,
                             IceTDouble x,
                             IceTDouble y,
                             IceTDouble z)
{
    IceTDouble transform[16];
    icetMatrixScale(x, y, z, transform);
    icetMatrixPostMultiply(mat_out, (const IceTDouble *)transform);
}

void icetMatrixMultiplyTranslate(IceTDouble *mat_out,
                                 IceTDouble x,
                                 IceTDouble y,
                                 IceTDouble z)
{
    IceTDouble transform[16];
    icetMatrixTranslate(x, y, z, transform);
    icetMatrixPostMultiply(mat_out, (const IceTDouble *)transform);
}

void icetMatrixMultiplyRotate(IceTDouble *mat_out,
                              IceTDouble angle,
                              IceTDouble x,
                              IceTDouble y,
                              IceTDouble z)
{
    IceTDouble transform[16];
    icetMatrixRotate(angle, x, y, z, transform);
    icetMatrixPostMultiply(mat_out, (const IceTDouble *)transform);
}

/*
 * The inverse matrix is calculated by performing the Gaussian matrix reduction
 * with a partial pivoting with back substitution embedded.
 *
 * Much of this algorithm was inspired by the matrix inverse function in the
 * Mesa 3D library originally written by Jacques Leroy (jle@stare.be).  However,
 * for maintainability, many of the loops are re-rolled and many of the
 * identifiers are treated differently.  Also, the back substitution takes
 * place while the elimination takes place to shorten the code.
 */
IceTBoolean icetMatrixInverse(const IceTDouble *matrix_in,
                              IceTDouble *matrix_out)
{
    /* A 4x8 matrix we will do operations on.  The matrix starts as matrix_in
     * adjoined with the identity matrix.  We then do Gaussian elimination and
     * back substitution on the matrix that will result with the identity matrix
     * adjoined with the inverse of the original matrix. */
    IceTDouble matrix[4][8];
    int eliminationIdx;
    int rowIdx;

    /* Initialize the matrix. */
    for (rowIdx = 0; rowIdx < 4; rowIdx++) {
        int columnIdx;
        /* Left half is equal to matrix_in. */
        for (columnIdx = 0; columnIdx < 4; columnIdx++) {
            matrix[rowIdx][columnIdx] = MAT(matrix_in, rowIdx, columnIdx);
        }
        /* Right half is the identity matrix. */
        for (columnIdx = 4; columnIdx < 8; columnIdx++) {
            matrix[rowIdx][columnIdx] = 0.0;
        }
        matrix[rowIdx][rowIdx+4] = 1.0;
    }

    /* Per the Gaussian elimination algorithm, starting at the top row make
     * the leftmost value 1 and zero out the column on all lower rows.  Repeat
     * to rows on down.  If a column will all remaining zeros is encountered,
     * no inverse exists so return failure. */
    for (eliminationIdx = 0; eliminationIdx < 4; eliminationIdx++)
    {
        int pivotIdx;
        int columnIdx;
        IceTDouble scale_value;

        /* Choose the pivot row as the one with the highest absolute value in
         * the elimination column. */
        pivotIdx = eliminationIdx;
        for (rowIdx = eliminationIdx+1; rowIdx < 4; rowIdx++) {
            if (  fabs(matrix[pivotIdx][eliminationIdx])
                < fabs(matrix[rowIdx][eliminationIdx]) ) {
                pivotIdx = rowIdx;
            }
        }

        /* If the entry in pivotIdx is 0, then all the values in the column must
         * be zero and no inverse exists. */
        if (matrix[pivotIdx][eliminationIdx] == 0.0) return ICET_FALSE;

        /* Swap the elimination row with the pivot row to make the pivot row the
         * elimination row. */
        if (pivotIdx != eliminationIdx) {
            for (columnIdx = 0; columnIdx < 8; columnIdx++) {
                IceTDouble old_elim_value = matrix[eliminationIdx][columnIdx];
                matrix[eliminationIdx][columnIdx] = matrix[pivotIdx][columnIdx];
                matrix[pivotIdx][columnIdx] = old_elim_value;
            }
        }

        /* Size the elimination row to scale the eliminationIdx column to 1. */
        scale_value = 1.0/matrix[eliminationIdx][eliminationIdx];
        for (columnIdx = eliminationIdx; columnIdx < 8; columnIdx++) {
            matrix[eliminationIdx][columnIdx] *= scale_value;
        }

        /* Add a multiple of the elimination row to all other rows to zero out
         * that column of values.  Note that this is also doing back-propagation
         * as it is zeroing out values above. */
        for (rowIdx = 0; rowIdx < 4; rowIdx++) {
            if (rowIdx == eliminationIdx) continue;
            scale_value = -matrix[rowIdx][eliminationIdx];
            for (columnIdx = eliminationIdx; columnIdx < 8; columnIdx++) {
                matrix[rowIdx][columnIdx]
                    += scale_value*matrix[eliminationIdx][columnIdx];
            }
        }
    }

    /* At this point, the left 4x4 submatrix of matrix should be the identity
     * matrix and the right 4x4 submatrix is the inverse of matrix_in.  Copy the
     * result to matrix_out. */
    for (rowIdx = 0; rowIdx < 4; rowIdx++) {
        int columnIdx;
        for (columnIdx = 0; columnIdx < 4; columnIdx++) {
            MAT(matrix_out, rowIdx, columnIdx) = matrix[rowIdx][columnIdx+4];
        }
    }

    return ICET_TRUE;
}

void icetMatrixTranspose(const IceTDouble *matrix_in, IceTDouble *matrix_out)
{
    int rowIdx;
    for (rowIdx = 0; rowIdx < 4; rowIdx++) {
        int columnIdx;
        for (columnIdx = 0; columnIdx < 4; columnIdx++) {
            MAT(matrix_out, rowIdx, columnIdx)
                = MAT(matrix_in, columnIdx, rowIdx);
        }
    }
}

IceTBoolean icetMatrixInverseTranspose(const IceTDouble *matrix_in,
                                       IceTDouble *matrix_out)
{
    IceTDouble inverse[16];
    IceTBoolean success;

    success = icetMatrixInverse(matrix_in, inverse);
    if (!success) return ICET_FALSE;

    icetMatrixTranspose((const IceTDouble *)inverse, matrix_out);
    return ICET_TRUE;
}
