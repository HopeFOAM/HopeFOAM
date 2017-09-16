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

#include "gaussIntegration.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(gaussIntegration, 0);
    defineRunTimeSelectionTable(gaussIntegration, gaussIntegration);
}

Foam::autoPtr<Foam::gaussIntegration> Foam::gaussIntegration::New
(
    const baseFunction& base
)
{
    gaussIntegrationConstructorTable::iterator cstrIter =
        gaussIntegrationConstructorTablePtr_->find(base.cellShape_);

    if (cstrIter == gaussIntegrationConstructorTablePtr_->end())
    {
        Info<< "Unknown cellShape " << base.cellShape_ << nl << nl
            << "Valid cell shape of gaussIntegration are :" << endl
            << gaussIntegrationConstructorTablePtr_->sortedToc()
            << exit(FatalIOError);
    }

    return autoPtr<gaussIntegration>(cstrIter()(base));
}

// * * * * * * * * * * * * * * * * Constructors * * * * * * * * * * * * * * //

Foam::gaussIntegration::gaussIntegration(const baseFunction& base)
:
    baseFunction_(base)
{
    volIntOrder_ = (base.nOrder_ + 1) * 3;

    faceIntOrder_ = (base.nOrder_ + 1) * 2;
}

// * * * * * * * * * * * * * * * * Member Functions * * * * * * * * * * * * * * //
Foam::tensorList Foam::gaussIntegration::cellDxdr(const Foam::vectorList& physicalNodes)const
{
    tensorList ans(nDofPerCell_, tensor::zero);
    
    denseMatrix<vector>::MatVecOuterProduct(cellDr_, physicalNodes, ans);

    return ans;
}
// ************************************************************************* //
