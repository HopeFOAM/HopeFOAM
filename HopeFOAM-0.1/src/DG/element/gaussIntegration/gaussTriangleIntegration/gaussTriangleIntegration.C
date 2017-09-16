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

#include "gaussTriangleIntegration.H"
#include "lineBaseFunction.H"
#include "addToRunTimeSelectionTable.H"
#include "Legendre.H"
// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(gaussTriangleIntegration, 0);
    addToRunTimeSelectionTable(gaussIntegration, gaussTriangleIntegration, gaussIntegration);
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::gaussTriangleIntegration::gaussTriangleIntegration(const baseFunction& base)
:
    gaussIntegration(base)
{
    //------------ set cell integration data ------------//
    if(volIntOrder_ <= 28){
        scalarListList cubData = gaussTriangleIntegration::dataTable(volIntOrder_);
        nDofPerCell_ = cubData[0].size();
        cellIntLocation_.setSize(nDofPerCell_);
        forAll(cellIntLocation_, pointI){
            cellIntLocation_[pointI] = vector(cubData[0][pointI], cubData[1][pointI], 0);
        }
        cellIntWeights_ = cubData[2];
    }
    else{
        FatalErrorIn("Foam::gaussBase2D::Cubature")
            << "volIntOrder_ = " << volIntOrder_
            << " is not implemented "
            << abort(FatalError);
    }

    // set cellVandermonde_
    cellVandermonde_.setSize(nDofPerCell_, base.nDofPerCell_);
    denseMatrix<scalar> gaussV2D (Legendre::Vandermonde2D(base.nOrder_, cellIntLocation_));
    const denseMatrix<scalar>& invV = base.invV_;
    //cellVandermonde_ = gaussV2D * invV;
    denseMatrix<scalar>::MatMatMult(gaussV2D, invV, cellVandermonde_);

    //- set cellDr_
    denseMatrix<vector> Vr(Legendre::GradVandermonde2D(base.nOrder_, cellIntLocation_));
    cellDr_.setSize(nDofPerCell_, invV.colSize());
    //cellDr_ = Vr * invV;
    denseMatrix<vector>::MatMatMult(Vr, invV, cellDr_);

    //---------set face integration data------------//
    scalarListList JacobiGQ(Legendre::JacobiGQ(0,0, faceIntOrder_/2));
    nDofPerFace_ = JacobiGQ[0].size();
    faceIntLocation_.setSize(nDofPerFace_);
    forAll(faceIntLocation_, pointI){
        faceIntLocation_[pointI] = vector(JacobiGQ[0][pointI], 0, 0);
    }
    faceIntWeights_ = JacobiGQ[1];

    //- set faceInterp_
    faceInterp_.setSize(nDofPerFace_, base.nDofPerFace_);
    const denseMatrix<scalar> gaussV_1D (Legendre::Vandermonde1D(base.nOrder_, faceIntLocation_));
    lineBaseFunction lineBase(base.nOrder_, "line");
    const denseMatrix<scalar>& invV_1D = lineBase.invV_;
    //faceInterp_ = gaussV_1D * invV_1D;
    denseMatrix<scalar>::MatMatMult(gaussV_1D, invV_1D, faceInterp_);

    initFaceRotateIndex();
}

// * * * * * * * * * * * * * * * * Member Functions * * * * * * * * * * * * * * //

Foam::vectorList Foam::gaussTriangleIntegration::gaussCellNodesLoc(const Foam::vectorList& cellPoints)const
{
    vectorList ans(nDofPerCell_, vector::zero);

    forAll(ans, pointI){
        ans[pointI] = 
            -(cellIntLocation_[pointI].x() + cellIntLocation_[pointI].y())*0.5*cellPoints[0]
            +(cellIntLocation_[pointI].x() + 1)*0.5*cellPoints[1]
            +(cellIntLocation_[pointI].y() + 1)*0.5*cellPoints[2];
    }
    
    return ans;
}

void Foam::gaussTriangleIntegration::initFaceRotateIndex()
{
    faceRotateIndex_.setSize(2, labelList(nDofPerFace_));

    for(int i=0; i<nDofPerFace_; i++){
        //if point 0 of the faces overlap
        faceRotateIndex_[0][i] = i;
        //if point 1 of owner face overlap neighbor face
        faceRotateIndex_[1][i] = nDofPerFace_ -1 -i;
    }
}

Foam::tensorList Foam::gaussTriangleIntegration::cellDrdx(const Foam::tensorList& xr)const
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

Foam::scalarList Foam::gaussTriangleIntegration::cellWJ(const Foam::tensorList& xr)const
{
    scalarList ans(nDofPerCell_);

    forAll(ans, item){
        ans[item] = (xr[item][0]*xr[item][4] - xr[item][1]*xr[item][3]) * cellIntWeights_[item];
    }

    return ans;
}

void Foam::gaussTriangleIntegration::faceNxWJ(const tensorList& xr, label faceI, vectorList& normal, scalarList& wj)const
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
            nx = gaussXr[pointI][4] - gaussXr[pointI][1];// ys-yr
            ny = gaussXr[pointI][0] - gaussXr[pointI][3];// -xs+xr
            Js = sqrt(sqr(nx) + sqr(ny));
            J = gaussXr[pointI][0]*gaussXr[pointI][4] - gaussXr[pointI][1]*gaussXr[pointI][3];
            normal[pointI] = vector(nx, ny, 0.0) / Js;
            wj[pointI] =  Js * (faceIntWeights_[pointI]);
        }
        break;
    case 2:
        forAll(wj, pointI){
            nx = -gaussXr[pointI][4];// -ys
            ny = gaussXr[pointI][3];// xs
            Js = sqrt(sqr(nx) + sqr(ny));
            J = gaussXr[pointI][0]*gaussXr[pointI][4] - gaussXr[pointI][1]*gaussXr[pointI][3];
            normal[pointI] = vector(nx, ny, 0.0) / Js;
            wj[pointI] =  Js * (faceIntWeights_[pointI]);
        }
    }
}
// ************************************************************************* //
