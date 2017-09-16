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
    Foam::dgLduMatrix

Description
    dgLduMatrix is a general matrix class in which the coefficients are
    stored as three arrays, one for the upper triangle, one for the
    lower triangle and a third for the diagonal.

    diagnoal matrix are addressed by cell index, while upper and lower
    matrix are addressed by face index.

SourceFiles
    dgLduMatrix.C
    
Author
    Xu Liyang (xucloud77@gmail.com)

\*---------------------------------------------------------------------------*/

#ifndef dgLduMatrix_H
#define dgLduMatrix_H

#include "dgMesh.H"
#include "denseMatrix.H"
#include "primitiveFieldsFwd.H"
#include "FieldField.H"
#include "typeInfo.H"
#include "autoPtr.H"
#include "runTimeSelectionTables.H"
#include "InfoProxy.H"
#include <petscksp.h> 

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// Forward declaration of friend functions and operators

template<class Type>
class dgLduMatrix;

template<class Type>
Ostream& operator<<(Ostream&, const dgLduMatrix<Type>&);
template<class Type>
Ostream& operator<<(Ostream&, const InfoProxy<dgLduMatrix<Type>>&);


/*---------------------------------------------------------------------------*\
                           Class dgLduMatrix Declaration
\*---------------------------------------------------------------------------*/
template<class Type>
class dgLduMatrix
{
    // private data

    	//- dgMesh reference
		const dgMesh& mesh_;

        //- operator order
        label operatorOrder_;

        //- diagCoeff_ * massMatrix, for dgm::ddt, Sp, and fields
        scalar diagCoeff_;

        //- Coefficients of diagnal
        denseMatrix<scalar> diag_;

        //- lu for dgm::convection, contains information of neighbor faces
        //- lu2 for dgm::laplacian, contains information of neighbor cells
        List<denseMatrix<scalar>> lu_,  lu2_;


        //set size of cell matrix and preallocate data
            //input: order of the operator
            void matrixPreallocation(label);

public:

        //- unIntegrated Source term, arranged as psi field
        //- will be integrated by calc MassMatrix * source_
        label sourceSize_;
        Field<Type> source_;
         //- source term
        Field<Type> b_;
        
    // Static data

        // Declare name of the class and its debug switch
        ClassName("dgLduMatrix");

    // Constructors

        //- Construct given an dgMesh.
        //  The coefficients are initially empty for subsequent setting.
        dgLduMatrix(const dgMesh&, label);

        //- Construct as copy
        dgLduMatrix(const dgLduMatrix<Type>&);

        //- Construct as copy or re-use as specified.
        dgLduMatrix(dgLduMatrix<Type>&, bool reuse);

        //- Construct given an dgMesh and an Istream
        //  from which the coefficients are read
        dgLduMatrix(const dgMesh&, Istream&, label);


    //- Destructor
    ~dgLduMatrix();


    // Member functions

        //- Return the dgMesh from which the addressing is obtained
        const dgMesh& mesh() const
        {
            return mesh_;
        }

        label operatorOrder()const{
            return operatorOrder_;
        }

        //- convert to PetscMatrix
        //void generatePetscMat(Mat& );
        void addSourceToB(const physicalCellElement&);


        // Access to coefficients
            inline scalar& diagCoeff(){return diagCoeff_;}
            List<denseMatrix<scalar>>& lu();
            List<denseMatrix<scalar>>& lu2();
            denseMatrix<scalar>& diag();

            inline const scalar diagCoeff()const {return diagCoeff_;};
            const List<denseMatrix<scalar>>& lu() const;
            const List<denseMatrix<scalar>>& lu2() const;
            const denseMatrix<scalar>& diag() const;


            bool hasDiag() const
            {
                return (diag_.actualSize());
            }

            bool hasLu() const
            {
                return (lu_.size());
            }
            bool hasLu2() const
            {
                return (lu2_.size());
            }

            inline label sourceSize()const{
                return sourceSize_;
            }


            //- Return info proxy.
            //  Used to print matrix information to a stream
            InfoProxy<dgLduMatrix<Type>> info() const
            {
                return *this;
            }

    // Member operators
        //void refreshPreallocate(const dgLduMatrix&);

        void operator=(const dgLduMatrix<Type>&);

        void negate();

        //- Relax matrix (for steady-state solution).
            //  alpha = 1 : diagonally equal
            //  alpha < 1 : diagonally dominant
            //  alpha = 0 : do nothing
            //  Note: Requires positive diagonal.
            //void relax(const scalar alpha);

        //- Return the matrix scalar diagonal
        //tmp<scalarField> D() const;

        void operator+=(const dgLduMatrix<Type>&);
        void operator-=(const dgLduMatrix<Type>&);

        void operator*=(scalar);

    // Ostream operator

        friend Ostream& operator<<(Ostream&, const dgLduMatrix<Type>&);
        friend Ostream& operator<<(Ostream&, const InfoProxy<dgLduMatrix<Type>>&);
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

#ifdef NoRepository
#   include "dgLduMatrix.C"
#endif
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
