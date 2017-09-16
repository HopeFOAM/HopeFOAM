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

#include "gaussQuadrilateralIntegration.H"
#include "lineBaseFunction.H"
#include "addToRunTimeSelectionTable.H"
#include "Legendre.H"
// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(gaussQuadrilateralIntegration, 0);
    addToRunTimeSelectionTable(gaussIntegration, gaussQuadrilateralIntegration, gaussIntegration);
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::gaussQuadrilateralIntegration::gaussQuadrilateralIntegration(const baseFunction& base)
:
    gaussIntegration(base)
{
    //------------ set cell integration data (tensor product of 1D)------------//
    scalarListList cellJacobiGQ(Legendre::JacobiGQ(0,0, volIntOrder_/2));
    label GQ1D_Size = cellJacobiGQ[0].size();
    nDofPerCell_ = GQ1D_Size * GQ1D_Size;
    cellIntLocation_.setSize(nDofPerCell_);
    cellIntWeights_.setSize(nDofPerCell_);
    for(int i=0, ptr=0; i<GQ1D_Size; i++){
        for(int j=0; j<GQ1D_Size; j++, ptr++){
            cellIntLocation_[ptr] = vector(cellJacobiGQ[0][i], cellJacobiGQ[0][j], 0);
            cellIntWeights_[ptr] = cellJacobiGQ[1][i] * cellJacobiGQ[1][j];
        }
    }
    // set cellVandermonde_ matrix

    cellVandermonde_.setSize(nDofPerCell_, base.nDofPerCell_);
    denseMatrix<scalar> cellV(nDofPerCell_, base.nDofPerCell_, 0);
    const denseMatrix<scalar> cellGaussV_1D (Legendre::Vandermonde1D(base.nOrder_, cellJacobiGQ[0]));
    label baseSize = base.nOrder_+1;
    for(int i=0, ptr1=0, ptr2=0; i<GQ1D_Size; i++, ptr2+=baseSize){
        for(int j=0, ptr3=0; j<GQ1D_Size; j++, ptr3+=baseSize){
            for(int m=0, mPtr=ptr2; m<baseSize; m++, mPtr++){
                for(int n=0, nPtr=ptr3; n<baseSize; n++, ptr1++, nPtr++){
                    cellV[ptr1] = cellGaussV_1D[mPtr]*cellGaussV_1D[nPtr];
                }
            }
        }
    }

    const denseMatrix<scalar>& invV = base.invV_;
    //- set cellDr_
    denseMatrix<vector> gradV1D(Legendre::GradVandermonde1D(base.nOrder_, cellJacobiGQ[0]));
    cellDr_.setSize(nDofPerCell_, base.nDofPerCell_);
    denseMatrix<vector> cellDr(nDofPerCell_, base.nDofPerCell_, vector::zero);
    for(int i=0, ptr1=0, ptr2=0; i<GQ1D_Size; i++, ptr2+=baseSize){
        for(int j=0, ptr3=0; j<GQ1D_Size; j++, ptr3+=baseSize){
            for(int m=0, mPtr=ptr2; m<baseSize; m++, mPtr++){
                for(int n=0, nPtr=ptr3; n<baseSize; n++, ptr1++, nPtr++){
                    cellDr[ptr1] = vector(gradV1D[nPtr].x()*cellGaussV_1D[mPtr], cellGaussV_1D[nPtr] * gradV1D[mPtr].x(), 0.0);
                }
            }
        }
    }

    //cellVandermonde_ = cellVandermonde_ * invV;
    denseMatrix<scalar>::MatMatMult(cellV, invV, cellVandermonde_);
    //cellDr_ = cellDr_ * invV;
    denseMatrix<vector>::MatMatMult(cellDr, invV, cellDr_);

    //---------set face integration data------------//
    scalarListList faceJacobiGQ(Legendre::JacobiGQ(0,0, faceIntOrder_/2));
    nDofPerFace_ = faceJacobiGQ[0].size();
    faceIntLocation_.setSize(nDofPerFace_);
    forAll(faceIntLocation_, pointI){
        faceIntLocation_[pointI] = vector(faceJacobiGQ[0][pointI], 0, 0);
    }
    faceIntWeights_ = faceJacobiGQ[1];

    //- set faceInterp_
    faceInterp_.setSize(nDofPerFace_, base.nDofPerFace_);
    const denseMatrix<scalar> faceGaussV_1D (Legendre::Vandermonde1D(base.nOrder_, faceIntLocation_));
    lineBaseFunction faceLineBase(base.nOrder_, "line");
    const denseMatrix<scalar>& faceInvV_1D = faceLineBase.invV_;
    //faceInterp_ = faceGaussV_1D * faceInvV_1D;
    denseMatrix<scalar>::MatMatMult(faceGaussV_1D, faceInvV_1D, faceInterp_);

    initFaceRotateIndex();
}

// * * * * * * * * * * * * * * * * Member Functions * * * * * * * * * * * * * * //

Foam::vectorList Foam::gaussQuadrilateralIntegration::gaussCellNodesLoc(const Foam::vectorList& cellPoints)const
{
    vectorList ans(nDofPerCell_, vector::zero);
    forAll(ans, pointI){
        ans[pointI] = 
             (1-cellIntLocation_[pointI].x())*(1-cellIntLocation_[pointI].y())*0.25*cellPoints[0]
            +(1+cellIntLocation_[pointI].x())*(1-cellIntLocation_[pointI].y())*0.25*cellPoints[1]
            +(1+cellIntLocation_[pointI].x())*(1+cellIntLocation_[pointI].y())*0.25*cellPoints[2]
            +(1-cellIntLocation_[pointI].x())*(1+cellIntLocation_[pointI].y())*0.25*cellPoints[3];
    }
    return ans;
}

void Foam::gaussQuadrilateralIntegration::initFaceRotateIndex()
{
    faceRotateIndex_.setSize(2, labelList(nDofPerFace_));

    for(int i=0; i<nDofPerFace_; i++){
        //if point 0 of the faces overlap
        faceRotateIndex_[0][i] = i;
        //if point 1 of owner face overlap neighbor face
        faceRotateIndex_[1][i] = nDofPerFace_ -1 -i;
    }
}

Foam::tensorList Foam::gaussQuadrilateralIntegration::cellDrdx(const Foam::tensorList& xr)const
{
    tensorList ans(nDofPerCell_, tensor::zero);

    forAll(ans, item){
        scalar J = xr[item][0]*xr[item][4] - xr[item][1]*xr[item][3];
        ans[item][0] =  xr[item][4] / J;
        ans[item][1] = -xr[item][1] / J;
        ans[item][3] = -xr[item][3] / J;
        ans[item][4] =  xr[item][0] / J;
    }

    return ans;
}

Foam::scalarList Foam::gaussQuadrilateralIntegration::cellWJ(const Foam::tensorList& xr)const
{
    scalarList ans(nDofPerCell_);

    forAll(ans, item){
        ans[item] = (xr[item][0]*xr[item][4] - xr[item][1]*xr[item][3]) * cellIntWeights_[item];
    }

    return ans;
}

void Foam::gaussQuadrilateralIntegration::faceNxWJ(const tensorList& xr, label faceI, vectorList& normal, scalarList& wj)const
{
    tensorList gaussXr(faceInterp_.rowSize());
    denseMatrix<tensor>::MatVecMult(faceInterp_, xr, gaussXr);
    normal.setSize(gaussXr.size());
    wj.setSize(gaussXr.size());
    scalar nx, ny, J, Js;

    switch(faceI){
    case 0:
        forAll(wj, pointI){
            nx = gaussXr[pointI][1];// yr
            ny = -gaussXr[pointI][0];// -xr
            Js = sqrt(sqr(nx) + sqr(ny));
            J = gaussXr[pointI][0]*gaussXr[pointI][4] - gaussXr[pointI][1]*gaussXr[pointI][3];
            normal[pointI] = vector(nx, ny, 0.0) / Js;
            wj[pointI] =  Js * (faceIntWeights_[pointI]);
        }
        break;
    case 1:
        forAll(wj, pointI){
            nx = gaussXr[pointI][4];// ys
            ny = -gaussXr[pointI][3];// -xs
            Js = sqrt(sqr(nx) + sqr(ny));
            J = gaussXr[pointI][0]*gaussXr[pointI][4] - gaussXr[pointI][1]*gaussXr[pointI][3];
            normal[pointI] = vector(nx, ny, 0.0) / Js;
            wj[pointI] =  Js * (faceIntWeights_[pointI]);
        }
        break;
    case 2:
        forAll(wj, pointI){
            nx = -gaussXr[pointI][1];// -yr
            ny = gaussXr[pointI][0];// xr
            Js = sqrt(sqr(nx) + sqr(ny));
            J = gaussXr[pointI][0]*gaussXr[pointI][4] - gaussXr[pointI][1]*gaussXr[pointI][3];
            normal[pointI] = vector(nx, ny, 0.0) / Js;
            wj[pointI] =  Js * (faceIntWeights_[pointI]);
        }
        break;
    case 3:
        forAll(wj, pointI){
            nx = -gaussXr[pointI][4];// -ys
            ny = gaussXr[pointI][3];// xs
            Js = sqrt(sqr(nx) + sqr(ny));
            J = gaussXr[pointI][0]*gaussXr[pointI][4] - gaussXr[pointI][1]*gaussXr[pointI][3];
            normal[pointI] = vector(nx, ny, 0.0) / Js;
            wj[pointI] =  Js * (faceIntWeights_[pointI]);
        }
        break;
    }
}
// ************************************************************************* //