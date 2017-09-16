/*--------------------------------------------------------------------------------------
|     __  ______  ____  ______ |                                                       |
|    / / / / __ \/ __ \/ ____/ | HopeFOAM: High Order Parallel Extensible CFD Software |
|   / /_/ / / / / /_/ / __/    |                                                       |
|  / __  / /_/ / ____/ /___    |                                                       |
| /_/ /_/\____/_/   /_____/    | Copyright(c) 2017-2017 The Exercise Group, AMS, China.|
|                              |                                                       |
----------------------------------------------------------------------------------------

License
    This file is part of HopeFOAM, which is developed by Exercise Group, Innovation 
    Institute for Defence Science and Technology, the Academy of Military Science (AMS), China.

    HopeFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation, either version 3 of the License, or (at your
    option) any later version.

    HopeFOAM is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with HopeFOAM.  If not, see <http://www.gnu.org/licenses/>.

Class
    Foam::denseMatrix

Description
    small dense matrix for the calculation of base functions

SourceFiles
    denseMatrix.C

Author
    Xu Liyang (xucloud77@gmail.com)

\*---------------------------------------------------------------------------*/
#ifndef denseMatrix_H
#define denseMatrix_H

#include "labelList.H"
#include "UPtrList.H"
#include "tensor.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// Forward declaration of friend functions and operators

template<class Type>
class denseMatrix;

//- solve matrix and source
template<class Type>
List<Type> solve(const denseMatrix<scalar>&, const List<Type>&);

template<class Type>
Ostream& operator<<
(
    Ostream&,
    const denseMatrix<Type>&
);
/*---------------------------------------------------------------------------*\
                       Class denseMatrix Declaration
\*---------------------------------------------------------------------------*/

template<class Type>
class denseMatrix
:
    public List<Type>
{
	// Private data

		//- number of row of the matrix
		label rowSize_;
		//- number of colume of the matrix
		label colSize_;

public:

    // Constructors

        //- Null constructor
        inline denseMatrix();

        //- Construct with given size
        explicit denseMatrix(const label, const label);

        //- Construct with given size and value for all elements
        denseMatrix(const label, const label, const Type&);

        //- Copy constructor
        denseMatrix(const denseMatrix<Type>&);

        //- Construct as copy of ListList<T>
        explicit denseMatrix(const List<List<Type>>&);

    //- Destructor
    ~denseMatrix()
    {}

    // Member Functions

        //- Return the number of rows
        inline label rowSize() const{return rowSize_;}

        //- Return the number of columes
        inline label colSize() const{return colSize_;}

        //- Return the actual size of matrix
        inline label actualSize() const{return rowSize_*colSize_;}

        // Edit

            //- Alias for setSize(const label)
            inline void resize(const label, const label);

            //- Alias for setSize(const label, const T&)
            inline void resize(const label, const label, const Type&);

            //- Reset size of List
            void setSize(const label, const label);

            //- Reset size of List and value for new elements
            void setSize(const label, const label, const Type&);

            //- Reset the size label only, not change the list size
            void resetLabel(const label, const label);

            //- set the matrix to critical size unit matrix
            void setToUnitMatrix(const label);

            //- set the matrix contents to zero
            void setToZeroMatrix();

            //- Clear the list, i.e. set size to zero
            void clear();


    // Member operators

        //- Assignment from denseMatrix operator. Takes linear time
        void operator=(const denseMatrix<Type>&);

        //- Assignment from SLList operator. Takes linear time
        void operator=(const List<List<Type>>&);

        //- Assignment of all entries to the given value
        inline void operator=(const Type&);

    // function members

        //- return the Transpose matrix
        denseMatrix<Type> T()const;

        //- M.T() * M, matrix transpose
        denseMatrix<Type> transposeMult()const;
        //- M * M.T, matrix transpose
        denseMatrix<Type> multTranspose()const;
        //- return a matrix with some columes from this matrix
        denseMatrix<Type> subColumes(const labelList&)const;
        //- return a matrix with some rows from this matrix
        denseMatrix<Type> subRows(const labelList&)const;

        List<Type> solve(const List<Type>& source)const;

    // calculation operator
        void operator+=(const denseMatrix<Type>&);
        void operator-=(const denseMatrix<Type>&);
        void operator*=(scalar);

        //- mat_C = mat_A.T() * diag(vec_x) * mat_B, mat_C cound not be mat_A or mat_B;
        static void MatDiagMatMult(const denseMatrix<scalar>& mat_A, const UList<scalar> vec_x, 
                                   const denseMatrix<scalar>& mat_B, denseMatrix<scalar>& mat_C);

        //- mat_C = mat_A * mat_B, mat_C cound not be mat_A or mat_B;
        static void MatMatMult(const denseMatrix<Type>& mat_A, const denseMatrix<scalar>& mat_B,
                               denseMatrix<Type>& mat_C);

        static void MatScalarMatMult(const denseMatrix<scalar>& mat_A, const denseMatrix<Type>& mat_B,
                               denseMatrix<Type>& mat_C);

        //- vec_y = mat_A * vec_x, vec_y cound not be vec_x;
        static void MatVecMult(const denseMatrix<scalar>& mat_A, const UList<Type> vec_x,
                               UList<Type> vec_y);

        //- vec_y = mat_A.T() * vec_x, vec_y cound not be vec_x;
        static void MatTVecMult(const denseMatrix<scalar>& mat_A, const UList<Type> vec_x,
                               UList<Type> vec_y);

        //- vec_y += mat_A * vec_x, vec_y cound not be vec_x;
        static void MatVecMultAdd(const denseMatrix<scalar>& mat_A, const UList<Type> vec_x,
                               UList<Type> vec_y);

        //- vec_y = mat_A * vec_x
        static void MatVecOuterProduct(const denseMatrix<vector>& mat_A, const UList<scalar> vec_x,
                                        UList<vector> vec_y);
        static void MatVecOuterProduct(const denseMatrix<vector>& mat_A, const UList<vector> vec_x,
                                        UList<tensor> vec_y);

        //- vec_y = mat_A * vec_x
        static void MatVecInnerProductProduct(const denseMatrix<vector>& mat_A, const UList<vector> vec_x,
                                        UList<scalar> vec_y);
        static void MatVecInnerProductProduct(const denseMatrix<vector>& mat_A, const UList<tensor> vec_x,
                                        UList<vector> vec_y);

    // Ostream operators

        friend Ostream& operator<< <Type>
        (
            Ostream&,
            const denseMatrix<Type>&
        );
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

#ifdef NoRepository
    #include "denseMatrix.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif
