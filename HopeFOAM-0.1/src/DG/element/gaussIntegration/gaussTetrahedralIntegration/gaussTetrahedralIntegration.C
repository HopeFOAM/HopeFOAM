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

#include "gaussTetrahedralIntegration.H"
#include "addToRunTimeSelectionTable.H"
#include "Legendre.H"
#include "constants.H"
// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(gaussTetrahedralIntegration, 0);
    addToRunTimeSelectionTable(gaussIntegration, gaussTetrahedralIntegration, gaussIntegration);
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::gaussTetrahedralIntegration::gaussTetrahedralIntegration(const baseFunction& base)
:
    gaussIntegration(base)
{
    //------------ set cell integration data ------------//

    // set cellVandermonde_

    //- set cellDr_

    //---------set face integration data------------//

    //- set faceInterp_

    //initFaceRotateIndex();
}

// * * * * * * * * * * * * * * * * Member Functions * * * * * * * * * * * * * * //

Foam::vectorList Foam::gaussTetrahedralIntegration::gaussCellNodesLoc(const Foam::vectorList& cellPoints)const
{
    vectorList ans(nDofPerCell_, vector::zero);
    
    return ans;
}


void Foam::gaussTetrahedralIntegration::initFaceRotateIndex()
{
    faceRotateIndex_.setSize(3, labelList(nDofPerFace_));
    label nOrderPerLine = faceIntOrder_;

    //point 0 over lap point 0 of neighbor
    for(int i=0, index=0; i<=nOrderPerLine; i++){
        for(int j=0, ptr=i; j<=(nOrderPerLine-i); j++, ptr+=(nOrderPerLine-j+2), index++){
            faceRotateIndex_[0][index] = ptr;
        }
    }

    //point 0 over lap point 1 of neighbor
    for(int i=0, index=0, ptr2=nOrderPerLine; i<=nOrderPerLine; i++, ptr2+=(nOrderPerLine-i+1)){
        for(int j=0, ptr=ptr2; j<=(nOrderPerLine-i); j++, ptr--, index++){
            faceRotateIndex_[1][index] = ptr;
        }
    }

    //point 0 over lap point 2 of neighbor
    for(int i=0, index=0, ptr2=nDofPerFace_-1; i<=nOrderPerLine; i++, ptr2-=(i+1)){
        for(int j=0, ptr=ptr2; j<=(nOrderPerLine-i); j++, index++){
            faceRotateIndex_[2][index] = ptr;
            ptr -= (i+j+1);
        }
    }
}

Foam::tensorList Foam::gaussTetrahedralIntegration::cellDrdx(const Foam::tensorList& xr)const
{
    tensorList ans(nDofPerCell_, tensor::zero);

    forAll(ans, item){
        scalar J = det(xr[item]);
        tensor t = cof(xr[item]).T();
        ans[item] = t / J;
    }

    return ans;
}

Foam::scalarList Foam::gaussTetrahedralIntegration::cellWJ(const Foam::tensorList& xr)const
{
    scalarList ans(nDofPerCell_);

    forAll(ans, item){
        ans[item] = det(xr[item]) * cellIntWeights_[item];
    }

    return ans;
}

void Foam::gaussTetrahedralIntegration::faceNxWJ(const tensorList& xr, label faceI, vectorList& normal, scalarList& wj)const
{
    tensorList gaussXr(faceInterp_ * xr);
    normal.setSize(gaussXr.size());
    wj.setSize(gaussXr.size());
    scalar nx, ny, nz, Js;
    tensor t;

    switch(faceI){
    case 0:
        forAll(wj, pointI){
            t = (cof(gaussXr[pointI]).T())/det(gaussXr[pointI]);
            nx = -t[6];// -tx
            ny = -t[7];// -ty
            nz = -t[8];// -tz
            Js = sqrt(sqr(nx) + sqr(ny) + sqr(nz));
            normal[pointI] = vector(nx, ny, nz)/Js;
            wj[pointI] = Js * (faceIntWeights_[pointI]);
        }
        break;
    case 1:
        forAll(wj, pointI){
            t = (cof(gaussXr[pointI]).T())/det(gaussXr[pointI]);
            nx = -t[3];// -sx
            ny = -t[4];// -sy
            nz = -t[5];// -sz
            Js = sqrt(sqr(nx) + sqr(ny) + sqr(nz));
            normal[pointI] = vector(nx, ny, nz)/Js;
            wj[pointI] = Js * (faceIntWeights_[pointI]);
        }
        break;
    case 2:
        forAll(wj, pointI){
            t = (cof(gaussXr[pointI]).T())/det(gaussXr[pointI]);
            nx = t[0]+t[3]+t[6];// rx+sx+tx
            ny = t[1]+t[4]+t[7];// ry+sy+ty
            nz = t[2]+t[5]+t[8];// rz+sz+tz
            Js = sqrt(sqr(nx) + sqr(ny) + sqr(nz));
            normal[pointI] = vector(nx, ny, nz)/Js;
            wj[pointI] = Js * (faceIntWeights_[pointI]);
        }
        break;
    case 3:
        forAll(wj, pointI){
            t = (cof(gaussXr[pointI]).T())/det(gaussXr[pointI]);
            nx = -t[0];// -rx
            ny = -t[1];// -ry
            nz = -t[2];// -rz
            Js = sqrt(sqr(nx) + sqr(ny) + sqr(nz));
            normal[pointI] = vector(nx, ny, nz)/Js;
            wj[pointI] = Js * (faceIntWeights_[pointI]);
        }
        break;
    }

}
// ************************************************************************* //