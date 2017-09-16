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

Author
    Xu Liyang (xucloud77@gmail.com)
\*---------------------------------------------------------------------------*/

#include "baseFunction.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(baseFunction, 0);
    defineRunTimeSelectionTable(baseFunction, straightBaseFunction);
}

Foam::autoPtr<Foam::baseFunction> Foam::baseFunction::New
(
    label nOrder,
    word cellShape
)
{
    straightBaseFunctionConstructorTable::iterator cstrIter =
        straightBaseFunctionConstructorTablePtr_->find(cellShape);

    if (cstrIter == straightBaseFunctionConstructorTablePtr_->end())
    {
        Info<< "Unknown cellShape " << cellShape << nl << nl
            << "Valid cell shape of baseFunction are :" << endl
            << straightBaseFunctionConstructorTablePtr_->sortedToc()
            << exit(FatalIOError);
    }

    return autoPtr<baseFunction>(cstrIter()(nOrder, cellShape));
}

// * * * * * * * * * * * * * * * * Member Functions * * * * * * * * * * * * * * //

void Foam::baseFunction::initMassMatrix()
{
    //M = invV' * invV
    massMatrix_.setSize(nDofPerCell_, nDofPerCell_);

    massMatrix_ = invV_.transposeMult();
}

void Foam::baseFunction::initInvMassMatrix()
{
    //invM = V * V'
    invMassMatrix_.setSize(nDofPerCell_, nDofPerCell_);

    invMassMatrix_ = V_.multTranspose();
}
// ************************************************************************* //