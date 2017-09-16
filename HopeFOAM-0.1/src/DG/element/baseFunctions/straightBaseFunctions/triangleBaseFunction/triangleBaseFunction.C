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

#include "triangleBaseFunction.H"
#include "addToRunTimeSelectionTable.H"
#include "Legendre.H"
#include "constants.H"
#include "SubList.H"
// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(triangleBaseFunction, 0);
    addToRunTimeSelectionTable(baseFunction, triangleBaseFunction, straightBaseFunction);
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::triangleBaseFunction::triangleBaseFunction(label nOrder, word cellShape)
:
    baseFunction(nOrder, cellShape)
{
    nDofPerFace_ = nOrder + 1;
    nFacesPerCell_ = 3;
    nDofPerCell_ = (nOrder+1)*(nOrder+2)/2;

    // set dofLocation_
    initDofLocation();

    // set Vandermonde matrix
    V_ = Foam::Legendre::Vandermonde2D(nOrder, dofLocation_);
    invV_ = Foam::Legendre::matrixInv(V_);

    //set invFaceMatrix_
    autoPtr<baseFunction> base_1D = Foam::baseFunction::New(nOrder_, "line");
    invFaceMatrix_ = base_1D -> invV_;

    // set faceDofLocation_
    faceDofLocation_ = base_1D -> dofLocation_;

    initFaceRotateIndex();
    initFaceToCellIndex();
    initFaceVertex();
}

// * * * * * * * * * * * * * * * * Member Functions * * * * * * * * * * * * * * //

void Foam::triangleBaseFunction::initFaceToCellIndex()
{
    faceToCellIndex_.setSize(nFacesPerCell_,labelListList(2, labelList(nDofPerFace_)));

    //init table for face 0-1-2, anti-clockwise
    for(int i=0; i<nDofPerFace_; i++){
        faceToCellIndex_[0][0][i] = i;
    }
    faceToCellIndex_[1][0][0] = nOrder_;
    faceToCellIndex_[2][0][0] = nDofPerCell_-1;
    for(int i=1; i<nDofPerFace_; i++){
        faceToCellIndex_[1][0][i] = faceToCellIndex_[1][0][i-1] + (nDofPerFace_-i);
        faceToCellIndex_[2][0][i] = faceToCellIndex_[2][0][i-1] - i -1;
    }
    for(int i=0; i<nDofPerFace_; i++){
        faceToCellIndex_[0][1][i] = faceToCellIndex_[0][0][nOrder_-i];
        faceToCellIndex_[1][1][i] = faceToCellIndex_[1][0][nOrder_-i];
        faceToCellIndex_[2][1][i] = faceToCellIndex_[2][0][nOrder_-i];
    }
}

void Foam::triangleBaseFunction::initFaceRotateIndex()
{
    faceRotateIndex_.setSize(2, labelList(nDofPerFace_));

    for(int i=0; i<nDofPerFace_; i++){
        //if point 0 of the faces overlap
        faceRotateIndex_[0][i] = i;
        //if point 1 of owner face overlap neighbor face
        faceRotateIndex_[1][i] = nOrder_ -i;
    }
}

void Foam::triangleBaseFunction::initFaceVertex()
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
        faceVertex_[2][1] = 0;
}

void Foam::triangleBaseFunction::initDofLocation()
{
    dofLocation_.setSize(nDofPerCell_);
    scalar alpopt[15] = {0.0000, 0.0000, 1.4152, 0.1001, 0.2751, 0.9800, 1.0999,
        1.2832, 1.3648, 1.4773, 1.4959, 1.5743, 1.5770, 1.6223, 1.6258};

    //Set optimized parameter, alpha, depending on order N
    scalar alpha;
    if(nOrder_<16) alpha = alpopt[nOrder_-1];
    else alpha = 5.0/3.0;

    //Create equidistributed nodes on equilateral triangle
    scalarList L1(nDofPerCell_, 0.0), L2(nDofPerCell_, 0.0), L3(nDofPerCell_, 0.0);
    label sk =0;
    for(int n=1; n<=nOrder_+1; n++){
        for(int m=1; m<=(nOrder_+2-n); m++){
            L1[sk] = (double)(n-1)/nOrder_;
            L3[sk] = (double)(m-1)/nOrder_;
            sk++;
        }
    }
    for(int i=0; i<nDofPerCell_; i++){
        L2[i] = 1.0 - L1[i] - L3[i];
        dofLocation_[i] = vector( L3[i] - L2[i],
                                  (2*L1[i] - L2[i] - L3[i])/sqrt(3.0),
                                  0);
    }

    //Compute blending function at each node for each edge
    scalarList blend1(nDofPerCell_, 0.0), blend2(nDofPerCell_, 0.0), blend3(nDofPerCell_, 0.0);
    for(int i=0; i<nDofPerCell_; i++){
        blend1[i] = 4*L2[i]*L3[i];
        blend2[i] = 4*L1[i]*L3[i];
        blend3[i] = 4*L1[i]*L2[i];
    }

    //Amount of warp for each node, for each edge
    scalarList warp1(warpFactor(nOrder_, L3-L2));
    scalarList warp2(warpFactor(nOrder_, L1-L3));
    scalarList warp3(warpFactor(nOrder_, L2-L1));

    // Combine blend & warp
    for(int i=0; i<nDofPerCell_; i++)warp1[i] = blend1[i]*warp1[i]*(1+sqr(alpha*L1[i]));
    for(int i=0; i<nDofPerCell_; i++)warp2[i] = blend2[i]*warp2[i]*(1+sqr(alpha*L2[i]));
    for(int i=0; i<nDofPerCell_; i++)warp3[i] = blend3[i]*warp3[i]*(1+sqr(alpha*L3[i]));

    //Accumulate deformations associated with each edge
    scalar cos1 = cos(2*Foam::constant::mathematical::pi/3);
    scalar cos2 = cos(4*Foam::constant::mathematical::pi/3);
    scalar sin1 = sin(2*Foam::constant::mathematical::pi/3);
    scalar sin2 = sin(4*Foam::constant::mathematical::pi/3);
    for(int i=0; i<nDofPerCell_; i++){
        dofLocation_[i] += vector( 1*warp1[i] + cos1*warp2[i] + cos2*warp3[i],
                                   0*warp1[i] + sin1*warp2[i] + sin2*warp3[i],
                                   0);
    }

    for(int i=0; i<nDofPerCell_; i++){
        scalar L1 = (sqrt(3.0)*dofLocation_[i].y()+1.0)/3.0;
        scalar L2 = (-3.0*dofLocation_[i].x()-sqrt(3.0)*dofLocation_[i].y()+2.0)/6.0;
        scalar L3 = ( 3.0*dofLocation_[i].x()-sqrt(3.0)*dofLocation_[i].y()+2.0)/6.0;

        dofLocation_[i] = vector( L3-L2-L1, L1-L2-L3, 0);
    }
}


Foam::scalarList Foam::triangleBaseFunction::warpFactor(label N, scalarList rout)
{
    //Compute LGL and equidistant node distribution
    scalarList LGLr(Foam::Legendre::JacobiGL(0,0,N));
    scalarList req(N+1);
    scalar interval = 2.0/N;
    for(int i=0; i<=N; i++){
        req[i] = interval*i-1;
    }

    //Compute V based on req
    scalarListList Veq(Foam::Legendre::Vandermonde1D(N, req));

    //Evaluate Lagrange polynomial at rout
    scalarListList Pmat(N+1, scalarList(rout.size()));
    for(int i=0; i<=N; i++)Pmat[i] = Foam::Legendre::JacobiP(rout, 0, 0, i);

    //transpose Veq
    scalar tmp;
    for(int i=0; i<Veq.size(); i++){
        for(int j=0; j<i; j++){
            tmp = Veq[i][j];
            Veq[i][j] = Veq[j][i];
            Veq[j][i] = tmp;
        }
    }

    //calc Lmat, Lmat = inv(Veq')*Pmat
    denseMatrix<scalar> invVeq(Foam::Legendre::matrixInv(denseMatrix<scalar>(Veq)));
    denseMatrix<scalar> Lmat(N+1, rout.size(), 0.0);
    denseMatrix<scalar>::MatMatMult(invVeq, denseMatrix<scalar>(Pmat), Lmat);

    //calc warp, warp = Lmat' * (LGLr - req)
    scalarList warp(rout.size());
    denseMatrix<scalar>::MatVecMult(Lmat.T(), (LGLr - req)(), warp);

    //scalr factor
    scalarList zerof(rout);
    for(int i=0; i<rout.size(); i++)zerof[i] = (abs(rout[i])<(1.0 - 1.0e-10));
    scalarList sf(rout);
    for(int i=0; i<rout.size(); i++)sf[i] = 1.0- (zerof[i]*rout[i])*(zerof[i]*rout[i]);

    for(int i=0; i<rout.size(); i++)warp[i] = warp[i]/sf[i] + warp[i]*(zerof[i]-1.0);

    return warp;
}

void Foam::triangleBaseFunction::initDrMatrix()
{
    //GradVandermonde
    const Foam::denseMatrix<vector> Vr( Legendre::GradVandermonde2D(nOrder_, dofLocation_));

    drMatrix_.setSize(nDofPerCell_, nDofPerCell_);

    //Dr = Vr / V
    //drMatrix_ = Vr * invV_;
    denseMatrix<vector>::MatMatMult(Vr, invV_, drMatrix_);
}

void Foam::triangleBaseFunction::initLift()
{
    Lift_.setSize(nDofPerCell_, nDofPerFace_*3);

    if(invMassMatrix_.size()<=0)
        initInvMassMatrix();

    autoPtr<baseFunction> base_1D = Foam::baseFunction::New(nOrder_, "line");
    if(base_1D->massMatrix_.size()<=0)base_1D->initMassMatrix();
    const denseMatrix<scalar>& M_1D = base_1D->massMatrix_;

    denseMatrix<scalar> LiftFace0(faceToCellIndex_[0][0].size(), nDofPerCell_);
    denseMatrix<scalar> LiftFace1(faceToCellIndex_[1][0].size(), nDofPerCell_);
    denseMatrix<scalar> LiftFace2(faceToCellIndex_[2][0].size(), nDofPerCell_);
    denseMatrix<scalar>::MatMatMult(invMassMatrix_.subColumes(faceToCellIndex_[0][0]), M_1D, LiftFace0);
    denseMatrix<scalar>::MatMatMult(invMassMatrix_.subColumes(faceToCellIndex_[1][0]), M_1D, LiftFace1);
    denseMatrix<scalar>::MatMatMult(invMassMatrix_.subColumes(faceToCellIndex_[2][0]), M_1D, LiftFace2);

    for(int i=0, ptr=0; i<nDofPerCell_; i++){
        for(int j=0, ptr1=i*nDofPerFace_; j<nDofPerFace_; j++, ptr++, ptr1++)
            Lift_[ptr] = LiftFace0[ptr1];
        for(int j=0, ptr1=i*nDofPerFace_; j<nDofPerFace_; j++, ptr++, ptr1++)
            Lift_[ptr] = LiftFace1[ptr1];
        for(int j=0, ptr1=i*nDofPerFace_; j<nDofPerFace_; j++, ptr++, ptr1++)
            Lift_[ptr] = LiftFace2[ptr1];
    }

    base_1D.clear();
}

void Foam::triangleBaseFunction::initCentreInterpolateMatrix()
{
    vectorList cellCentre(1, vector(0.0, 0.0, 0.0));
    denseMatrix<scalar> cellVandermondeMatrix(Foam::Legendre::Vandermonde2D(nOrder_, cellCentre));

    cellCentreInterpolateMatrix_.setSize(1, nDofPerCell_);
    denseMatrix<scalar>::MatMatMult(cellVandermondeMatrix, invV_, cellCentreInterpolateMatrix_);

    denseMatrix<scalar> faceVandermondeMatrix(Foam::Legendre::Vandermonde1D(nOrder_, cellCentre));
    faceCentreInterpolateMatrix_.setSize(1, nDofPerFace_);
    denseMatrix<scalar>::MatMatMult(faceVandermondeMatrix, invFaceMatrix_, faceCentreInterpolateMatrix_);
}

Foam::denseMatrix<Foam::scalar> Foam::triangleBaseFunction::cellVandermonde(const Foam::vectorList& loc)const
{
    denseMatrix<scalar> ans(Foam::Legendre::Vandermonde2D(nOrder_, loc));
    return ans;
}

Foam::denseMatrix<Foam::scalar> Foam::triangleBaseFunction::faceVandermonde(const Foam::vectorList& loc)const
{
    denseMatrix<scalar> ans(Foam::Legendre::Vandermonde1D(nOrder_, loc));
    return ans;
}

Foam::vectorList Foam::triangleBaseFunction::physicalNodesLoc(const Foam::vectorList& cellPoints)const
{
    vectorList ans(nDofPerCell_, vector::zero);
    forAll(ans, pointI){
        ans[pointI] = 
            -(dofLocation_[pointI].x() + dofLocation_[pointI].y())*0.5*cellPoints[0]
            +(dofLocation_[pointI].x() + 1)*0.5*cellPoints[1]
            +(dofLocation_[pointI].y() + 1)*0.5*cellPoints[2];
    }
    return ans;
}

Foam::tensorList Foam::triangleBaseFunction::dxdr(const Foam::vectorList& physicalNodes)
{
    if(drMatrix_.size()<=0)
        initDrMatrix();

    tensorList ans(nDofPerCell_, tensor::zero);
    //ans = drMatrix_ ^ physicalNodes;
    denseMatrix<vector>::MatVecOuterProduct(drMatrix_, physicalNodes, ans);
    //forAll(ans, i){
    //    ans[i] = ans[i].T();
    //}

    return ans;
}

Foam::tensorList Foam::triangleBaseFunction::drdx(const Foam::tensorList& xr)const
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

Foam::scalarList Foam::triangleBaseFunction::jacobian(const Foam::tensorList& xr)const
{
    scalarList ans(nDofPerCell_);
    forAll(ans, item){
        ans[item] = xr[item][0]*xr[item][4] - xr[item][1]*xr[item][3];
    }
    return ans;
}

void Foam::triangleBaseFunction::faceNxFscale(const tensorList& xr, label faceI, vectorList& normal, scalarList& fscale)const
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
            nx = xr[pointI][4] - xr[pointI][1];// ys-yr
            ny = xr[pointI][0] - xr[pointI][3];// -xs+xr
            Js = sqrt(sqr(nx) + sqr(ny));
            J = xr[pointI][0]*xr[pointI][4] - xr[pointI][1]*xr[pointI][3];
            normal[pointI] = vector(nx, ny, 0.0) / Js;
            fscale[pointI] = Js / J;
        }
        break;
    case 2:
        forAll(normal, pointI){
            nx = -xr[pointI][4];// -ys
            ny = xr[pointI][3];// xs
            Js = sqrt(sqr(nx) + sqr(ny));
            J = xr[pointI][0]*xr[pointI][4] - xr[pointI][1]*xr[pointI][3];
            normal[pointI] = vector(nx, ny, 0.0) / Js;
            fscale[pointI] = Js / J;
        }
    }
}

void Foam::triangleBaseFunction::faceFscale(const tensorList& xr, label faceI, scalar& fscale)const
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
        nx = xr[0][4] - xr[0][1];// ys-yr
        ny = xr[0][0] - xr[0][3];// -xs+xr
        Js = sqrt(sqr(nx) + sqr(ny));
        J = xr[0][0]*xr[0][4] - xr[0][1]*xr[0][3];
        fscale = Js / J;
        break;
    case 2:
        nx = -xr[0][4];// -ys
        ny = xr[0][3];// xs
        Js = sqrt(sqr(nx) + sqr(ny));
        J = xr[0][0]*xr[0][4] - xr[0][1]*xr[0][3];
        fscale = Js / J;
    }
}

void Foam::triangleBaseFunction::addFaceShiftToCell(const vectorList& shift, label faceI, vectorList& dofLocation)const
{
    //for face 2, the shift should be reverged
    vectorList tmpFaceShift(shift);
    if(faceI == 2){
        for(int i=0,j=tmpFaceShift.size()-1;i<j; i++, j--){
            vector tmp = tmpFaceShift[i];
            tmpFaceShift[i] = tmpFaceShift[j];
            tmpFaceShift[j] = tmp;
        }
    }

    // build vr
    vectorList vr(nDofPerCell_);
    forAll(vr, pointI){
        if(faceI==0)
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
        if( fabs(1.0 - vr[pointI][0]) < 1.e-7)
            continue;
        scalar blend;
        if(faceI==1){
            blend = (dofLocation_[pointI].x()+1)/(1-vr[pointI][0]);
        }
        else
            blend = -(dofLocation_[pointI].x()+dofLocation_[pointI].y())/(1-vr[pointI][0]);
        vertexD[pointI] *= blend;
    }

    //- update dof location
    forAll(vertexD, pointI){
        dofLocation[pointI] += vertexD[pointI];
    }
}
// ************************************************************************* //