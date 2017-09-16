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

InClass
    Foam::dgMatrix

Description
    A scalar instance of dgMatrix

SourceFiles
    dgScalarMatrix.C

\*---------------------------------------------------------------------------*/
#ifndef dgScalarMatrix_H
#define dgScalarMatrix_H

#include "dgMatrix.H"
#include "dgMatricesFwd.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

// Set reference level for a component of the solution
// on a given patch face
template<>
void dgMatrix<scalar>::setComponentReference
(
    const label patchi,
    const label facei,
    const direction,
    const scalar value
);

//template<>
//autoPtr<dgMatrix<scalar>::dgSolver> dgMatrix<scalar>::solver
//(
//    const dictionary&
//);
/*
template<>
solverPerformance<Type> dgMatrix<scalar>::dgSolver::solve
(
    scalarField& psi,
    const scalarField& source,
    const direction cmpt
);*/

template<>
SolverPerformance<scalar> dgMatrix<scalar>::solveSegregated
(
    KSP
);

template<>
tmp<scalarField> dgMatrix<scalar>::residual() const;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* /