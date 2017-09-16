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

#include "quadrilateralBaseFunction.H"
#include "addToRunTimeSelectionTable.H"
#include "Legendre.H"
#include "constants.H"
#include "SubList.H"
// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(quadrilateralBaseFunction, 0);
    addToRunTimeSelectionTable(baseFunction, quadrilateralBaseFunction, straightBaseFunction);
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::quadrilateralBaseFunction::quadrilateralBaseFunction(label nOrder, word cellShape)
:
    baseFunction(nOrder, cellShape)
{
    nDofPerFace_ = nOrder + 1;
    nFacesPerCell_ = 4;
    nDofPerCell_ = (nOrder+1)*(nOrder+1);

    // set data of 1D Legendre
    LGL1D_ = Foam::Legendre::JacobiGL(0, 0, nOrder);
    V1D_ = Foam::Legendre::Vandermonde1D(nOrder, LGL1D_);
    invV1D_ = Foam::Legendre::matrixInv(V1D_);
    gradV1D_ = Foam::Legendre::GradVandermonde1D(nOrder, LGL1D_);

    // set dofLocation_
    initDofLocation();

    // set faceDofLocation_
    faceDofLocation_.setSize(LGL1D_.size());
    forAll(faceDofLocation_, pointI){
        faceDofLocation_[pointI] = vector(LGL1D_[pointI], 0.0, 0.0);
    }

    // set Vandermonde matrix, Vij = Pj(ei)
    V_.setSize(nDofPerCell_, nDofPerCell_);
    for(int i=0, ptr1=0, ptr2=0; i<=nOrder; i++, ptr2+=nDofPerFace_){
        for(int j=0, ptr3=0; j<=nOrder; j++, ptr3+=nDofPerFace_){
            for(int m=0, mPtr=ptr2; m<=nOrder; m++, mPtr++){
                for(int n=0, nPtr=ptr3; n<=nOrder; n++, ptr1++, nPtr++){
                    V_[ptr1] = V1D_[mPtr]*V1D_[nPtr];
                }
            }
        }
    }

    invV_ = Foam::Legendre::matrixInv(V_);

    //set invFaceMatrix_
    invFaceMatrix_ = invV1D_;

    initFaceRotateIndex();
    initFaceToCellIndex();
    initFaceVertex();
}

// * * * * * * * * * * * * * * * * Member Functions * * * * * * * * * * * * * * //

void Foam::quadrilateralBaseFunction::initFaceToCellIndex()
{
    faceToCellIndex_.setSize(nFacesPerCell_,labelListList(2, labelList(nDofPerFace_)));

    faceToCellIndex_[0][0][0] = 0;
    faceToCellIndex_[1][0][0] = nDofPerFace_-1;
    faceToCellIndex_[2][0][0] = nDofPerCell_-1;
    faceToCellIndex_[3][0][0] = nDofPerCell_ - nDofPerFace_;
    for(int i=1; i<nDofPerFace_; i++){
        faceToCellIndex_[0][0][i] = faceToCellIndex_[0][0][i-1] + 1;
        faceToCellIndex_[1][0][i] = faceToCellIndex_[1][0][i-1] + nDofPerFace_;
        faceToCellIndex_[2][0][i] = faceToCellIndex_[2][0][i-1] - 1;
        faceToCellIndex_[3][0][i] = faceToCellIndex_[3][0][i-1] - nDofPerFace_;
    }
    for(int i=0; i<nDofPerFace_; i++){
        faceToCellIndex_[0][1][i] = faceToCellIndex_[0][0][nOrder_-i];
        faceToCellIndex_[1][1][i] = faceToCellIndex_[1][0][nOrder_-i];
        faceToCellIndex_[2][1][i] = faceToCellIndex_[2][0][nOrder_-i];
        faceToCellIndex_[3][1][i] = faceToCellIndex_[3][0][nOrder_-i];
    }
}

void Foam::quadrilateralBaseFunction::initFaceRotateIndex()
{
    faceRotateIndex_.setSize(2, labelList(nDofPerFace_));

    for(int i=0; i<nDofPerFace_; i++){
        //if point 0 of the faces overlap
        faceRotateIndex_[0][i] = i;
        //if point 1 of owner face overlap neighbor face
        faceRotateIndex_[1][i] = nOrder_ -i;
    }
}

void Foam::quadrilateralBaseFunction::initFaceVertex()
{
    faceVertex_.setSize(nFacesPerCell_);
    faceVertex_[0].setSize(2);
        faceVertex_[0][0] = 0;
        faceVertex_[0][1] = 1;
    faceVertex_[1].setSize(2);
        faceVertex_[1][0] = 1;
        faceVertex_[1][1] = 2;
    faceVertex_[2].setSize(2);
        faceVertex_[2][0] = 2;
        faceVertex_[2][1] = 3;
    faceVertex_[3].setSize(2);
        faceVertex_[3][0] = 3;
        faceVertex_[3][1] = 0;
}

void Foam::quadrilateralBaseFunction::initDofLocation()
{
    dofLocation_.setSize(nDofPerCell_);
    for(int i=0, ptr=0; i<nDofPerFace_; i++){
        for(int j=0; j<nDofPerFace_; j++, ptr++)
            dofLocation_[ptr] = vector(LGL1D_[j], LGL1D_[i], 0);
    }
}

void Foam::quadrilateralBaseFunction::initDrMatrix()
{
    denseMatrix<vector> Vr(nDofPerCell_, nDofPerCell_, vector::zero);

    for(int i=0, ptr1=0, ptr2=0; i<=nOrder_; i++, ptr2+=nDofPerFace_){
        for(int j=0, ptr3=0; j<=nOrder_; j++, ptr3+=nDofPerFace_){
            for(int m=0, mPtr=ptr2; m<=nOrder_; m++, mPtr++){
                for(int n=0, nPtr=ptr3; n<=nOrder_; n++, ptr1++, nPtr++){
                    Vr[ptr1] = vector(gradV1D_[nPtr].x()*V1D_[mPtr], V1D_[nPtr] * gradV1D_[mPtr].x(), 0.0);
                }
            }
        }
    }

    //Dr = Vr / V
    drMatrix_.setSize(nDofPerCell_, nDofPerCell_);
    //drMatrix_ = Vr * invV_;
    denseMatrix<vector>::MatMatMult(Vr, invV_, drMatrix_);
}

void Foam::quadrilateralBaseFunction::initLift()
{
    Lift_.setSize(nDofPerCell_, nDofPerFace_*4);

    if(invMassMatrix_.size()<=0)
        initInvMassMatrix();

    autoPtr<baseFunction> base_1D = Foam::baseFunction::New(nOrder_, "line");
    if(base_1D->massMatrix_.size()<=0)base_1D->initMassMatrix();
    const denseMatrix<scalar>& M_1D = base_1D->massMatrix_;

    denseMatrix<scalar> LiftFace0(faceToCellIndex_[0][0].size(), nDofPerCell_);
    denseMatrix<scalar> LiftFace1(faceToCellIndex_[1][0].size(), nDofPerCell_);
    denseMatrix<scalar> LiftFace2(faceToCellIndex_[2][0].size(), nDofPerCell_);
    denseMatrix<scalar> LiftFace3(faceToCellIndex_[3][0].size(), nDofPerCell_);
    denseMatrix<scalar>::MatMatMult(invMassMatrix_.subColumes(faceToCellIndex_[0][0]), M_1D, LiftFace0);
    denseMatrix<scalar>::MatMatMult(invMassMatrix_.subColumes(faceToCellIndex_[1][0]), M_1D, LiftFace1);
    denseMatrix<scalar>::MatMatMult(invMassMatrix_.subColumes(faceToCellIndex_[2][0]), M_1D, LiftFace2);
    denseMatrix<scalar>::MatMatMult(invMassMatrix_.subColumes(faceToCellIndex_[3][0]), M_1D, LiftFace3);

    for(int i=0, ptr=0; i<nDofPerCell_; i++){
        for(int j=0, ptr1=i*nDofPerFace_; j<nDofPerFace_; j++, ptr++, ptr1++)
            Lift_[ptr] = LiftFace0[ptr1];
        for(int j=0, ptr1=i*nDofPerFace_; j<nDofPerFace_; j++, ptr++, ptr1++)
            Lift_[ptr] = LiftFace1[ptr1];
        for(int j=0, ptr1=i*nDofPerFace_; j<nDofPerFace_; j++, ptr++, ptr1++)
            Lift_[ptr] = LiftFace2[ptr1];
        for(int j=0, ptr1=i*nDofPerFace_; j<nDofPerFace_; j++, ptr++, ptr1++)
            Lift_[ptr] = LiftFace3[ptr1];
    }
}

void Foam::quadrilateralBaseFunction::initCentreInterpolateMatrix()
{
    vectorList cellCentre(1, vector(0.0, 0.0, 0.0));
    denseMatrix<scalar> tmpV1D(Foam::Legendre::Vandermonde1D(nOrder_, cellCentre));
    denseMatrix<scalar> cellVandermondeMatrix(1, nDofPerCell_);
    for(int i=0, ptr=0; i<=nOrder_; i++){
        for(int j=0; j<=nOrder_; j++, ptr++){
            cellVandermondeMatrix[ptr] = tmpV1D[i] * tmpV1D[j];
        }
    }
    cellCentreInterpolateMatrix_.setSize(1, nDofPerCell_);
    denseMatrix<scalar>::MatMatMult(cellVandermondeMatrix, invV_, cellCentreInterpolateMatrix_);

    faceCentreInterpolateMatrix_.setSize(1, nDofPerFace_);
    denseMatrix<scalar>::MatMatMult(tmpV1D, invFaceMatrix_, faceCentreInterpolateMatrix_);
}

Foam::denseMatrix<Foam::scalar> Foam::quadrilateralBaseFunction::cellVandermonde(const Foam::vectorList& loc)const
{
    label locSize = loc.size();
    denseMatrix<scalar> ans(locSize, nDofPerCell_);
    scalarList x(locSize), y(locSize);
    forAll(loc, pointI){
        x[pointI] = loc[pointI].x();
        y[pointI] = loc[pointI].y();
    }
    scalarListList vx(Legendre::Vandermonde1D(nOrder_, x));
    scalarListList vy(Legendre::Vandermonde1D(nOrder_, y));

    // set Vandermonde matrix, Vij = Pj(ei)
    for(int i=0, ptr1=0; i<locSize; i++){
        for(int m=0; m<=nOrder_; m++){
            for(int n=0; n<=nOrder_; n++, ptr1++){
                ans[ptr1] = vy[i][m]*vx[i][n];
            }
        }
    }
    return ans;
}

Foam::denseMatrix<Foam::scalar> Foam::quadrilateralBaseFunction::faceVandermonde(const Foam::vectorList& loc)const
{
    denseMatrix<scalar> ans(Foam::Legendre::Vandermonde1D(nOrder_, loc));
    return ans;
}

Foam::vectorList Foam::quadrilateralBaseFunction::physicalNodesLoc(const Foam::vectorList& cellPoints)const
{
    vectorList ans(nDofPerCell_, vector::zero);
    forAll(ans, pointI){
        ans[pointI] = 
             (1-dofLocation_[pointI].x())*(1-dofLocation_[pointI].y())*0.25*cellPoints[0]
            +(1+dofLocation_[pointI].x())*(1-dofLocation_[pointI].y())*0.25*cellPoints[1]
            +(1+dofLocation_[pointI].x())*(1+dofLocation_[pointI].y())*0.25*cellPoints[2]
            +(1-dofLocation_[pointI].x())*(1+dofLocation_[pointI].y())*0.25*cellPoints[3];
    }
    return ans;
}

Foam::tensorList Foam::quadrilateralBaseFunction::dxdr(const Foam::vectorList& physicalNodes)
{
    if(drMatrix_.size()<=0)
        initDrMatrix();

    tensorList ans(nDofPerCell_, tensor::zero);
    //ans = drMatrix_ ^ physicalNodes;
    denseMatrix<vector>::MatVecOuterProduct(drMatrix_, physicalNodes, ans);

    return ans;
}

Foam::tensorList Foam::quadrilateralBaseFunction::drdx(const Foam::tensorList& xr)const
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

Foam::scalarList Foam::quadrilateralBaseFunction::jacobian(const Foam::tensorList& xr)const
{
    scalarList ans(nDofPerCell_);
    forAll(ans, item){
        ans[item] = xr[item][0]*xr[item][4] - xr[item][1]*xr[item][3];
    }
    return ans;
}

void Foam::quadrilateralBaseFunction::faceNxFscale(const tensorList& xr, label faceI, vectorList& normal, scalarList& fscale)const
{
    normal.setSize(nDofPerFace_);
    fscale.setSize(nDofPerFace_);
    scalar nx, ny, J, Js;

    switch(faceI){
    case 0:
        forAll(normal, pointI){
            nx = xr[pointI][1];// yr
            ny = -xr[pointI][0];// -xr
            Js = sqrt(sqr(nx) + sqr(ny));
            J = xr[pointI][0]*xr[pointI][4] - xr[pointI][1]*xr[pointI][3];
            normal[pointI] = vector(nx, ny, 0.0) / Js;
            fscale[pointI] = Js / J;
        }
        break;
    case 1:
        forAll(normal, pointI){
            nx = xr[pointI][4];// ys
            ny = -xr[pointI][3];// -xs
            Js = sqrt(sqr(nx) + sqr(ny));
            J = xr[pointI][0]*xr[pointI][4] - xr[pointI][1]*xr[pointI][3];
            normal[pointI] = vector(nx, ny, 0.0) / Js;
            fscale[pointI] = Js / J;
        }
        break;
    case 2:
        forAll(normal, pointI){
            nx = -xr[pointI][1];// -yr
            ny = xr[pointI][0];// xr
            Js = sqrt(sqr(nx) + sqr(ny));
            J = xr[pointI][0]*xr[pointI][4] - xr[pointI][1]*xr[pointI][3];
            normal[pointI] = vector(nx, ny, 0.0) / Js;
            fscale[pointI] = Js / J;
        }
        break;
    case 3:
        forAll(normal, pointI){
            nx = -xr[pointI][4];// -ys
            ny = xr[pointI][3];// xs
            Js = sqrt(sqr(nx) + sqr(ny));
            J = xr[pointI][0]*xr[pointI][4] - xr[pointI][1]*xr[pointI][3];
            normal[pointI] = vector(nx, ny, 0.0) / Js;
            fscale[pointI] = Js / J;
        }
        break;
    }
}

void Foam::quadrilateralBaseFunction::faceFscale(const tensorList& xr, label faceI, scalar& fscale)const
{
    scalar nx, ny, J, Js;
    switch(faceI){
    case 0:
        nx = xr[0][1];// yr
        ny = -xr[0][0];// -xr
        Js = sqrt(sqr(nx) + sqr(ny));
        J = xr[0][0]*xr[0][4] - xr[0][1]*xr[0][3];
        fscale = Js / J;
        break;
    case 1:
        nx = xr[0][4];// ys
        ny = -xr[0][3];// -xs
        Js = sqrt(sqr(nx) + sqr(ny));
        J = xr[0][0]*xr[0][4] - xr[0][1]*xr[0][3];
        fscale = Js / J;
        break;
    case 2:
        nx = -xr[0][1];// -yr
        ny = xr[0][0];// xr
        Js = sqrt(sqr(nx) + sqr(ny));
        J = xr[0][0]*xr[0][4] - xr[0][1]*xr[0][3];
        fscale = Js / J;
        break;
    case 3:
        nx = -xr[0][4];// -ys
        ny = xr[0][3];// xs
        Js = sqrt(sqr(nx) + sqr(ny));
        J = xr[0][0]*xr[0][4] - xr[0][1]*xr[0][3];
        fscale = Js / J;
        break;
    }
}

void Foam::quadrilateralBaseFunction::addFaceShiftToCell(const vectorList& shift, label faceI, vectorList& dofLocation)const
{
    //for face 2 & 3, the shift should be reverged
    vectorList tmpFaceShift(shift);
    if(faceI == 2 || faceI == 3){
        for(int i=0,j=tmpFaceShift.size()-1;i<j; i++, j--){
            vector tmp = tmpFaceShift[i];
            tmpFaceShift[i] = tmpFaceShift[j];
            tmpFaceShift[j] = tmp;
        }
    }

    // build vr
    vectorList vr(nDofPerCell_);
    forAll(vr, pointI){
        if(faceI==0 || faceI==2)
            vr[pointI][0] = dofLocation_[pointI].x();
        else 
            vr[pointI][0] = dofLocation_[pointI].y();
    }

    // calc the unblended influence of face shift to cell, using interpolation
    vectorList tmpRes(invFaceMatrix_.rowSize());
    denseMatrix<vector>::MatVecMult(invFaceMatrix_, tmpFaceShift, tmpRes);
    vectorList vertexD((this->faceVandermonde(vr)).rowSize());
    denseMatrix<vector>::MatVecMult((this->faceVandermonde(vr)), tmpRes, vertexD);
    //vectorList vertexD((this->faceVandermonde(vr))*(invFaceMatrix_*tmpFaceShift));
    
    // calc blend factor
    forAll(vertexD, pointI){
        scalar blend;
        if(faceI==0 || faceI==3){
            blend = (1-vr[pointI][0])*0.5;
        }
        else
            blend = (1+vr[pointI][0])*0.5;
        vertexD[pointI] *= blend;
    }

    //- update dof location
    forAll(vertexD, pointI){
        dofLocation[pointI] += vertexD[pointI];
    }
}
// ************************************************************************* //