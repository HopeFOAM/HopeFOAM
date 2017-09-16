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

#include "gaussLineIntegration.H"
#include "addToRunTimeSelectionTable.H"
#include "Legendre.H"
// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(gaussLineIntegration, 0);
    addToRunTimeSelectionTable(gaussIntegration, gaussLineIntegration, gaussIntegration);
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::gaussLineIntegration::gaussLineIntegration(const baseFunction& base)
:
    gaussIntegration(base)
{
    //------------ set cell integration data ------------//
    scalarListList JacobiGQ(Legendre::JacobiGQ(0,0, volIntOrder_/2));
    nDofPerCell_ = JacobiGQ[0].size();
    cellIntLocation_.setSize(nDofPerCell_);
    forAll(cellIntLocation_, pointI){
        cellIntLocation_[pointI] = vector(JacobiGQ[0][pointI], 0, 0);
    }
    cellIntWeights_ = JacobiGQ[1];

    //- set cellVandermonde_
    cellVandermonde_.setSize(nDofPerCell_, base.nDofPerCell_, 0.0);
    denseMatrix<scalar> V1D (Legendre::Vandermonde1D(base.nOrder_, cellIntLocation_));
    const denseMatrix<scalar>& invV = base.invV_;
    //cellVandermonde_ = V1D * invV;
    denseMatrix<scalar>::MatMatMult(V1D, invV, cellVandermonde_);

    //- set cellDr_
    denseMatrix<vector> Vr(Legendre::GradVandermonde1D(base.nOrder_, cellIntLocation_));
    cellDr_.setSize(nDofPerCell_, invV.size());
    //cellDr_ = Vr * invV;
    denseMatrix<vector>::MatMatMult(Vr, invV, cellDr_);

    //---------set face integration data------------//
    nDofPerFace_ = 1;

    faceIntLocation_.setSize(1, vector(0,0,0));
    faceIntWeights_.setSize(1, 1.0);
    faceInterp_.setSize(1, 1, 1.0);

    initFaceRotateIndex();
}

// * * * * * * * * * * * * * * * * Member Functions * * * * * * * * * * * * * * //
void Foam::gaussLineIntegration::initFaceRotateIndex()
{
    faceRotateIndex_.setSize(1, labelList(1));
    faceRotateIndex_[0][0] = 0;
}

Foam::vectorList Foam::gaussLineIntegration::gaussCellNodesLoc(const Foam::vectorList& cellPoints)const
{
    vectorList ans(nDofPerCell_, vector::zero);

    scalar start = cellPoints[0].x();
    scalar end = cellPoints[1].x();
    scalar tmp = (end-start)*0.5;
    for(int i=0; i<nDofPerCell_; i++)
        ans[i][0] = tmp*(cellIntLocation_[i].x()+1) + start;

    return ans;
}

Foam::tensorList Foam::gaussLineIntegration::cellDrdx(const Foam::tensorList& xr)const
{
    tensorList ans(nDofPerCell_, tensor::zero);

    for(int i=0; i<nDofPerCell_; i++)
        ans[i][0] = 1.0/xr[i][0];

    return ans;
}

Foam::scalarList Foam::gaussLineIntegration::cellWJ(const Foam::tensorList& xr)const
{
    scalarList ans(nDofPerCell_);

    for(int i=0; i<nDofPerCell_; i++)
        ans[i] = 1.0 / xr[i][0] * cellIntWeights_[i];

    return ans;
}

void Foam::gaussLineIntegration::faceNxWJ(const tensorList& xr, label faceI, vectorList& normal, scalarList& wj)const
{
    tensorList gaussXr(faceInterp_.rowSize());
    denseMatrix<tensor>::MatVecMult(faceInterp_, xr, gaussXr);
    normal.setSize(gaussXr.size());
    wj.setSize(gaussXr.size());

    switch(faceI){
    case 0:
        forAll(wj, pointI){
            normal[pointI] = vector(-1.0, 0.0, 0.0);
            wj[pointI] = -(faceIntWeights_[pointI])/gaussXr[pointI][0];
        }
        break;
    case 1:
        forAll(wj, pointI){
            normal[pointI] = vector(1.0, 0.0, 0.0);
            wj[pointI] = faceIntWeights_[pointI]/gaussXr[pointI][0];
        }
        break;
    }
}
// ************************************************************************* //