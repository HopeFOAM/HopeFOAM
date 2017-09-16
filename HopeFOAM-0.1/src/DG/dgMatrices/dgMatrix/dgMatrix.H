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
    Foam::dgMatrix

Description
    A special matrix type and solver, designed for DG
    solutions of scalar equations.

SourceFiles
    dgMatrix.C
    dgMatrixSolve.C
    dgScalarMatrix.C

\*---------------------------------------------------------------------------*/
#ifndef dgMatrix_H
#define dgMatrix_H

#include "dgFields.H"
#include "tmp.H"
#include "autoPtr.H"
#include "dimensionedTypes.H"
#include "zero.H"
#include "className.H"
#include "dgLduMatrix.H"
#include "solverPerformance.H"
#include <petscksp.h> 
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
namespace Foam
{
// Forward declaration of friend functions and operators

template<class Type>
class dgMatrix;


template<class Type>
Ostream& operator<<(Ostream&, const dgMatrix<Type>&);

/*---------------------------------------------------------------------------*\
                           Class dgMatrix Declaration
\*---------------------------------------------------------------------------*/
template<class Type>
class dgMatrix
:
    public refCount
{
    // Private data

        //- Const reference to GeometricDofField<Type, dgPatchField, dgGeoMesh>
        //  Converted into a non-const reference at the point of solution.
        const GeometricDofField<Type, dgPatchField, dgGeoMesh>& psi_;

        //- order of operator, pure source--0, sp/ddt--1, div,grad--2, laplacian--3
        label operatorOrder_;

        //- Dimension set
        dimensionSet dimensions_;

        //- KSP solver
        KSP ksp_;

        Mat mat_;
        //- source term
        Field<Type> b_;

        // * * * * * * * * * data for petsc matrix preallocate * * * * * * * * * * * * //
        //- row and column size of matrix related to each cel;
        labelList rowSize_, colSize_, onnSize_;
        //- temp space for petsc matrix dof location
        labelList dofRows_, dofCols_;
        //- different in operators, 1order:ddt; 2order:div/grad; 3order:laplacian
        labelList d_nnz_;//array containing the number of nonzeros in the various rows of the DIAGONAL portion of the local submatrix
        labelList o_nnz_;//array containing the number of nonzeros in the various rows of the OFF-DIAGONAL portion of the local submatrix 

protected:

    //- Declare friendship with the dgSolver class
    //friend class dgSolver;

    // Protected Member Functions
        

public:

    ClassName("dgMatrix");

    // Constructors

        //- Construct given a field to solve for
        dgMatrix
        (
            const GeometricDofField<Type, dgPatchField, dgGeoMesh>&,
            const dimensionSet&,
            label
        );

        //- Construct as copy
        dgMatrix(const dgMatrix<Type>&);

        //- Construct as copy of tmp<dgMatrix<Type> > deleting argument
        #ifdef ConstructFromTmp
        dgMatrix(const tmp<dgMatrix<Type> >&);
        #endif

        //- Construct from Istream given field to solve for
        dgMatrix(const GeometricDofField<Type, dgPatchField, dgGeoMesh>&, Istream&, label);


    //- Destructor
    virtual ~dgMatrix();

    // Member functions

        // Access
            const Mat A()const{
                return mat_;
            }

            const GeometricDofField<Type, dgPatchField, dgGeoMesh>& psi() const
            {
                return psi_;
            }

            Field<Type>& b(){
                return b_;
            }

            const dimensionSet& dimensions() const
            {
                return dimensions_;
            }

            label operatorOrder() const{
                return operatorOrder_;
            }

            KSP kspSolver();

            
        // Operations

            //- Set reference level for solution
            void setReference
            (
                const label celli,
                const Type& value,
                const bool forceReference = false
            );

            //- Set reference level for a component of the solution
            //  on a given patch face
            void setComponentReference
            (
                const label patchi,
                const label facei,
                const direction cmpt,
                const scalar value
            );

            //- Relax matrix (for steady-state solution).
            //  alpha = 1 : diagonally equal
            //  alpha < 1 : diagonally dominant
            //  alpha = 0 : do nothing
            //  Note: Requires positive diagonal.
            //void relax(const scalar alpha);

            //- Relax matrix (for steady-state solution).
            //  alpha is read from controlDict
            //void relax();

            //- Construct and return the solver
            //  Use the given solver controls
            //autoPtr<dgSolver> solver(const dictionary&);

            //- Construct and return the solver
            //  Solver controls read from dgSolution
            //autoPtr<dgSolver> solver();

            //- Solve segregated or coupled returning the solution statistics.
            //  Use the given solver controls
            SolverPerformance<Type> solve(const dictionary&, KSP);

            //- Solve segregated returning the solution statistics.
            //  Use the given solver controls
            SolverPerformance<Type> solveSegregated(KSP);

            //- Solve coupled returning the solution statistics.
            //  Use the given solver controls
            SolverPerformance<Type> solveCoupled(const dictionary&);

            //- Solve returning the solution statistics.
            //  Solver controls read from dgSolution
            SolverPerformance<Type> solve();
            SolverPerformance<Type> solve(KSP);

            //- Return the matrix residual
            tmp<Field<Type> > residual() const;


    // Member operators

            void petscPreAllocate();

            void assembleMatrix(dgLduMatrix<Type>&, const physicalCellElement&, label, label);

            void finalAssemble();


    // Friend operators


    // Ostream operator

        friend Ostream& operator<< <Type>
        (
            Ostream&,
            const dgMatrix<Type>&
        );
};

// * * * * * * * * * * * * * * * Global functions  * * * * * * * * * * * * * //

template<class Type>
void checkMethod
(
    const dgMatrix<Type>&,
    const dgMatrix<Type>&,
    const char*
);

template<class Type>
void checkMethod
(
    const dgMatrix<Type>&,
    const DimensionedField<Type, dgGeoMesh>&,
    const char*
);

template<class Type>
void checkMethod
(
    const dgMatrix<Type>&,
    const dimensioned<Type>&,
    const char*
);


//- Solve returning the solution statistics given convergence tolerance
//  Use the given solver controls
template<class Type>
SolverPerformance<Type> solve(dgMatrix<Type>&, const dictionary&);


//- Solve returning the solution statistics given convergence tolerance,
//  deleting temporary matrix after solution.
//  Use the given solver controls
template<class Type>
SolverPerformance<Type> solve
(
    const tmp<dgMatrix<Type> >&,
    const dictionary&
);


//- Solve returning the solution statistics given convergence tolerance
//  Solver controls read dgSolution
template<class Type>
SolverPerformance<Type> solve(dgMatrix<Type>&);

template<class Type>
SolverPerformance<Type> solve(dgMatrix<Type>&, KSP&);


//- Solve returning the solution statistics given convergence tolerance,
//  deleting temporary matrix after solution.
//  Solver controls read dgSolution
template<class Type>
SolverPerformance<Type> solve(const tmp<dgMatrix<Type> >&);

template<class Type>
SolverPerformance<Type> solve(const tmp<dgMatrix<Type> >&, KSP&);


/*/- Return the correction form of the given matrix
//  by subtracting the matrix multiplied by the current field
template<class Type>
tmp<dgMatrix<Type> > correction(const dgMatrix<Type>&);


//- Return the correction form of the given temporary matrix
//  by subtracting the matrix multiplied by the current field
template<class Type>
tmp<dgMatrix<Type> > correction(const tmp<dgMatrix<Type> >&);*/


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
#   include "dgMatrix.C"
#endif

// Specialisation for scalars
#include "dgScalarMatrix.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
