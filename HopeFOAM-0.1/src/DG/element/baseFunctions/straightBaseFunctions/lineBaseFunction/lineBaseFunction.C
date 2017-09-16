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

#include "lineBaseFunction.H"
#include "addToRunTimeSelectionTable.H"
#include "Legendre.H"
// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(lineBaseFunction, 0);
    addToRunTimeSelectionTable(baseFunction, lineBaseFunction, straightBaseFunction);
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::lineBaseFunction::lineBaseFunction(label nOrder, word cellShape)
:
    baseFunction(nOrder, cellShape)
{
    nDofPerFace_ = 1;
    nFacesPerCell_ = 2;
    nDofPerCell_ = nOrder + 1;

    // set dofLocation_
    scalarList LGL(Foam::Legendre::JacobiGL(0, 0, nOrder));
    dofLocation_.setSize(LGL.size());
    forAll(dofLocation_, pointI){
        dofLocation_[pointI] = vector(LGL[pointI], 0.0, 0.0);
    }

    // set faceDofLocation_
    faceDofLocation_.setSize(1, vector::zero);

    // set Vandermonde matrix
    V_ = Foam::Legendre::Vandermonde1D(nOrder, LGL);
    invV_ = Foam::Legendre::matrixInv(V_);

    //set invFaceMatrix_
    invFaceMatrix_.setSize(1, 1, 1.0);

    initFaceToCellIndex();
    initFaceRotateIndex();
    initFaceVertex();
}

// * * * * * * * * * * * * * * * * Member Functions * * * * * * * * * * * * * * //
void Foam::lineBaseFunction::initFaceToCellIndex()
{
    faceToCellIndex_.setSize(2, labelListList(1, labelList(1)));
    faceToCellIndex_[0][0][0] = 0;
    faceToCellIndex_[1][0][0] = nOrder_;
}

void Foam::lineBaseFunction::initFaceRotateIndex()
{
    faceRotateIndex_.setSize(1, labelList(1));
    faceRotateIndex_[0][0] = 0;
}

void Foam::lineBaseFunction::initFaceVertex()
{
    faceVertex_.setSize(nFacesPerCell_);
    faceVertex_[0].setSize(1,0);
    faceVertex_[1].setSize(1,1);
}

void Foam::lineBaseFunction::initDrMatrix()
{
    //GradVandermonde
    const Foam::denseMatrix<vector> Vr( Foam::Legendre::GradVandermonde1D(nOrder_, dofLocation_));

    drMatrix_.setSize(nDofPerCell_, nDofPerCell_, vector::zero);

    //Dr = Vr / V
    //drMatrix_ = Vr * invV_;
    denseMatrix<vector>::MatMatMult(Vr, invV_, drMatrix_);
}

void Foam::lineBaseFunction::initCentreInterpolateMatrix()
{
    vectorList cellCentre(1, vector(0.0, 0.0, 0.0));
    denseMatrix<scalar> cellVandermondeMatrix(Foam::Legendre::Vandermonde1D(nOrder_, cellCentre));
    //cellCentreInterpolateMatrix_ = cellVandermondeMatrix * invV_;
    denseMatrix<scalar>::MatMatMult(cellVandermondeMatrix, invV_, cellCentreInterpolateMatrix_);

    faceCentreInterpolateMatrix_ = denseMatrix<scalar>(1,1,1.0);
}

void Foam::lineBaseFunction::initLift()
{
    Lift_.setSize(nDofPerCell_, nDofPerFace_*2, 0.0);
    if(invMassMatrix_.size()<=0)
        initInvMassMatrix();
    for(int i=0, ptr1=0, ptr2=0; i<nDofPerCell_; i++){
        Lift_[ptr1++] = invMassMatrix_[ptr2];
        ptr2 += nOrder_;
        Lift_[ptr1++] = invMassMatrix_[ptr2++];
    }
}

Foam::denseMatrix<Foam::scalar> Foam::lineBaseFunction::cellVandermonde(const Foam::vectorList& loc)const
{
    denseMatrix<scalar> ans(Foam::Legendre::Vandermonde1D(nOrder_, loc));
    return ans;
}

Foam::denseMatrix<Foam::scalar> Foam::lineBaseFunction::faceVandermonde(const Foam::vectorList& loc)const
{
    denseMatrix<scalar> ans(1,1,1.0);
    return ans;
}

Foam::vectorList Foam::lineBaseFunction::physicalNodesLoc(const Foam::vectorList& cellPoints)const
{
    vectorList ans(nDofPerCell_, vector::zero);

    scalar start = cellPoints[0].x();
    scalar end = cellPoints[1].x();
    scalar tmp = (end-start)*0.5;
    for(int i=0; i<nDofPerCell_; i++)
        ans[i][0] = tmp*(dofLocation_[i].x()+1) + start;

    return ans;
}

Foam::tensorList Foam::lineBaseFunction::dxdr(const Foam::vectorList& physicalNodes)
{
    tensorList ans(nDofPerCell_, tensor::zero);
    if(drMatrix_.size()<=0)
        initDrMatrix();

    //ans = drMatrix_ ^ physicalNodes;
    denseMatrix<vector>::MatVecOuterProduct(drMatrix_, physicalNodes, ans);

    return ans;
}

Foam::tensorList Foam::lineBaseFunction::drdx(const Foam::tensorList& xr)const
{
    tensorList ans(nDofPerCell_, tensor::zero);

    for(int i=0; i<nDofPerCell_; i++)
        ans[i][0] = 1.0/xr[i][0];

    return ans;
}

Foam::scalarList Foam::lineBaseFunction::jacobian(const Foam::tensorList& xr)const
{
    scalarList ans(nDofPerCell_);

    for(int i=0; i<nDofPerCell_; i++)
        ans[i] = 1.0/xr[i][0];
    
    return ans;
}

void Foam::lineBaseFunction::faceNxFscale(const tensorList& xr, label faceI, vectorList& normal, scalarList& fscale)const
{    
    normal.setSize(nDofPerFace_);
    fscale.setSize(nDofPerFace_);

    switch(faceI){
    case 0:
        forAll(normal, pointI){
            normal[pointI] = vector(-1.0, 0.0, 0.0);
            fscale[pointI] = 1.0/xr[pointI][0];
        }
        break;
    case 1:
        forAll(normal, pointI){
            normal[pointI] = vector(1.0, 0.0, 0.0);
            fscale[pointI] = 1.0/xr[pointI][0];
        }
        break;
    }
}

void Foam::lineBaseFunction::faceFscale(const tensorList& xr, label faceI, scalar& fscale)const
{
    switch(faceI){
    case 0:
        fscale = 1.0/xr[0][0];
        break;
    case 1:
        fscale = 1.0/xr[0][0];
        break;
    }
}

// ************************************************************************* //