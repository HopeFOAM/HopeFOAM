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

#include "tetrahedralBaseFunction.H"
#include "addToRunTimeSelectionTable.H"
#include "Legendre.H"
#include "constants.H"
#include "SubList.H"
// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(tetrahedralBaseFunction, 0);
    addToRunTimeSelectionTable(baseFunction, tetrahedralBaseFunction, straightBaseFunction);
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::tetrahedralBaseFunction::tetrahedralBaseFunction(label nOrder, word cellShape)
:
    baseFunction(nOrder, cellShape)
{
    nDofPerFace_ = (nOrder+1)*(nOrder+2)/2;
    nFacesPerCell_ = 4;
    nDofPerCell_ = (nOrder+1)*(nOrder+2)*(nOrder+3)/6;

    // set dofLocation_
    initDofLocation();

    // set Vandermonde matrix
    V_ = Foam::Legendre::Vandermonde3D(nOrder, dofLocation_);
    invV_ = Foam::Legendre::matrixInv(V_);

    //set invFaceMatrix_
    autoPtr<baseFunction> base_2D = Foam::baseFunction::New(nOrder_, "tri");
    invFaceMatrix_ = base_2D -> invV_;

    // set faceDofLocation_
    faceDofLocation_ = base_2D -> dofLocation_;

    initFaceRotateIndex();
    initFaceToCellIndex();
    initFaceVertex();
}

// * * * * * * * * * * * * * * * * Member Functions * * * * * * * * * * * * * * //

void Foam::tetrahedralBaseFunction::initFaceToCellIndex()
{
    faceToCellIndex_.setSize(nFacesPerCell_,labelListList(3, labelList(nDofPerFace_)));

    //init table for face 1-2-3
    for(int i=0, ptr=nOrder_, index=0; i<=nOrder_; i++){
        for(int j=0; j<=(nOrder_-i); j++, index++){
            faceToCellIndex_[0][0][index] = ptr;
            ptr+=(nOrder_-i-j);
        }
        ptr+=nOrder_-i;
    }
    //init table for face 0-3-2
    for(int i=0, ptr=0; i<=nOrder_; i++){
        for(int j=0, index=i; j<=(nOrder_-i); j++, index+=(nOrder_-j+2)){
            faceToCellIndex_[1][0][index] = ptr;
            ptr+=(nOrder_-i-j+1);
        }
    }
    //init tabel for face 0-1-3
    for(int i=0, ptr=0, index=0; i<=nOrder_; i++){
        for(int j=0; j<=(nOrder_-i); j++, index++){
            faceToCellIndex_[2][0][index] = ptr;
            ptr++;
        }
        label size=nOrder_-i;
        ptr+= (size+1)*size/2;
    }
    //init table for face 0-2-1
    for(int i=0, ptr=0; i<=nOrder_; i++){
        for(int j=0, index=i; j<=(nOrder_-i); j++, index+=(nOrder_-j+2)){
            faceToCellIndex_[3][0][index] = ptr;
            ptr++;
        }
    }

    for(int i=0; i<nDofPerFace_; i++){
        faceToCellIndex_[0][1][i] = faceToCellIndex_[0][0][faceRotateIndex_[1][i]];
        faceToCellIndex_[1][1][i] = faceToCellIndex_[1][0][faceRotateIndex_[1][i]];
        faceToCellIndex_[2][1][i] = faceToCellIndex_[2][0][faceRotateIndex_[1][i]];
        faceToCellIndex_[3][1][i] = faceToCellIndex_[3][0][faceRotateIndex_[1][i]];

        faceToCellIndex_[0][2][i] = faceToCellIndex_[0][0][faceRotateIndex_[2][i]];
        faceToCellIndex_[1][2][i] = faceToCellIndex_[1][0][faceRotateIndex_[2][i]];
        faceToCellIndex_[2][2][i] = faceToCellIndex_[2][0][faceRotateIndex_[2][i]];
        faceToCellIndex_[3][2][i] = faceToCellIndex_[3][0][faceRotateIndex_[2][i]];
    }
}

void Foam::tetrahedralBaseFunction::initFaceRotateIndex()
{
    faceRotateIndex_.setSize(3, labelList(nDofPerFace_));

    //point 0 over lap point 0 of neighbor
    for(int i=0, index=0; i<=nOrder_; i++){
        for(int j=0, ptr=i; j<=(nOrder_-i); j++, ptr+=(nOrder_-j+2), index++){
            faceRotateIndex_[0][index] = ptr;
        }
    }

    //point 0 over lap point 1 of neighbor
    for(int i=0, index=0, ptr2=nOrder_; i<=nOrder_; i++, ptr2+=(nOrder_-i+1)){
        for(int j=0, ptr=ptr2; j<=(nOrder_-i); j++, ptr--, index++){
            faceRotateIndex_[1][index] = ptr;
        }
    }

    //point 0 over lap point 2 of neighbor
    for(int i=0, index=0, ptr2=nDofPerFace_-1; i<=nOrder_; i++, ptr2-=(i+1)){
        for(int j=0, ptr=ptr2; j<=(nOrder_-i); j++, index++){
            faceRotateIndex_[2][index] = ptr;
            ptr -= (i+j+1);
        }
    }
}

void Foam::tetrahedralBaseFunction::initFaceVertex()
{
    faceVertex_.setSize(nFacesPerCell_);
    faceVertex_[0].setSize(3);//1 2 3
        faceVertex_[0][0] = 1;
        faceVertex_[0][1] = 2;
        faceVertex_[0][2] = 3;
    faceVertex_[1].setSize(3);//0 3 2
        faceVertex_[0][0] = 0;
        faceVertex_[0][1] = 3;
        faceVertex_[0][2] = 2;
    faceVertex_[2].setSize(3);//0 1 3
        faceVertex_[0][0] = 0;
        faceVertex_[0][1] = 1;
        faceVertex_[0][2] = 3;
    faceVertex_[3].setSize(3);//0 2 1
        faceVertex_[0][0] = 0;
        faceVertex_[0][1] = 2;
        faceVertex_[0][2] = 1;
}

void Foam::tetrahedralBaseFunction::initDofLocation()
{
    dofLocation_.setSize(nDofPerCell_);
    scalar alpopt[15] = {0, 0, 0, 0.1002, 1.1332, 1.5608, 1.3413, 1.2577, 1.1603,
                1.10153, 0.6080, 0.4523, 0.8856, 0.8717, 0.9655};

    //Set optimized parameter, alpha, depending on order N
    scalar alpha, tol = 1e-10;
    if(nOrder_<16) alpha = alpopt[nOrder_-1];
    else alpha = 1.0;

    //Create equidistributed nodes on equilateral triangle
    
    label sk =0;
    scalarList r(nDofPerCell_), s(nDofPerCell_), t(nDofPerCell_);
    for(int n=1; n<=nOrder_+1; n++){
        for(int m=1; m<=(nOrder_+2-n); m++){
            for(int q=1; q<=(nOrder_+3-n-m); q++){
                r[sk] = -1.0 + (double)(q-1)*2/nOrder_;
                s[sk] = -1.0 + (double)(m-1)*2/nOrder_;
                t[sk] = -1.0 + (double)(n-1)*2/nOrder_;
                sk++;
            }
        }
    }
    scalarList L1(nDofPerCell_, 0.0), L2(nDofPerCell_, 0.0), L3(nDofPerCell_, 0.0), L4(nDofPerCell_, 0.0);
    for(int i=0; i<nDofPerCell_; i++){
        L1[i] = (1 + t[i])/2;
        L2[i] = (1 + s[i])/2;
        L3[i] =-(1 + r[i] + s[i] + t[i])/2;
        L4[i] = (1 + r[i])/2;
    }

    // set vertices of tetrahedron
    vector v1(-1, -1/sqrt(3.0), -1/sqrt(6.0));
    vector v2( 1, -1/sqrt(3.0), -1/sqrt(6.0));
    vector v3( 0,  2/sqrt(3.0), -1/sqrt(6.0));
    vector v4( 0,  0,            3/sqrt(6.0));
    // orthogonal axis tangents on faces 1-4
    List<vector> t1(4);t1[0]=v2-v1;t1[1]=v2-v1;t1[2]=v3-v2;t1[3]=v3-v1;
    List<vector> t2(4);t2[0]=v3-0.5*(v1+v2);t2[1]=v4-0.5*(v1+v2);t2[2]=v4-0.5*(v2+v3);t2[3]=v4-0.5*(v1+v3);
    //normalize tangents 
    for(int i=0; i<4; i++){
        t1[i] = t1[i]/sqrt(sqr(t1[i].x()) + sqr(t1[i].y()) + sqr(t1[i].z()));
        t2[i] = t2[i]/sqrt(sqr(t2[i].x()) + sqr(t2[i].y()) + sqr(t2[i].z()));
    }

    //Warp and blend for each face (accumulated in dofLocation_)
    for(int i=0; i<nDofPerCell_; i++){
        dofLocation_[i] = vector(L3[i]*v1.x() + L4[i]*v2.x()+L2[i]*v3.x()+L1[i]*v4.x(),
                                 L3[i]*v1.y() + L4[i]*v2.y()+L2[i]*v3.y()+L1[i]*v4.y(),
                                 L3[i]*v1.z() + L4[i]*v2.z()+L2[i]*v3.z()+L1[i]*v4.z());
    }
    List<vector> shift(nDofPerCell_);
    scalarList* La, *Lb, *Lc, *Ld;
    for(int face=0; face<4; face++){
        if(face==0){La = &L1; Lb = &L2; Lc = &L3; Ld = &L4;}
        if(face==1){La = &L2; Lb = &L1; Lc = &L3; Ld = &L4;}
        if(face==2){La = &L3; Lb = &L1; Lc = &L4; Ld = &L2;} 
        if(face==3){La = &L4; Lb = &L1; Lc = &L3; Ld = &L2;}
        // compute warp tangential to face
        scalarListList warp(evalshift(nOrder_, alpha, *Lb, *Lc, *Ld));
        scalarList blend(nDofPerCell_);
        for(int i=0; i<nDofPerCell_; i++){
            blend[i] = (*Lb)[i]*(*Lc)[i]*(*Ld)[i];
            scalar denom = ((*Lb)[i] + 0.5*(*La)[i])*((*Lc)[i]+0.5*(*La)[i])*((*Ld)[i]+0.5*(*La)[i]);
            if(denom > tol)
                blend[i] = (1+sqr(alpha*(*La)[i]))*blend[i]/denom;
        }
        //compute warp & blend
        for(int i=0; i<nDofPerCell_; i++){
            shift[i] = shift[i] + (blend[i]*warp[0][i])*t1[face] + (blend[i]*warp[1][i])*t2[face];
            if((*La)[i]<tol && ((*Lb)[i]<=tol || (*Lc)[i]<=tol || (*Ld)[i]<=tol))
                shift[i] = warp[0][i]*t1[face] + warp[1][i]*t2[face];
        }
    }
    for(int i=0; i<nDofPerCell_; i++){
        dofLocation_[i] += shift[i];
    }

    xytors();
}


Foam::scalarListList Foam::tetrahedralBaseFunction::evalshift(label p, scalar pval, scalarList& L1, scalarList& L2, scalarList& L3)
{
    //compute Gauss-Lobatto-Legendre node distribution
    scalarList gaussX(Foam::Legendre::JacobiGL(0,0,p));
    for(int i=0; i<gaussX.size(); i++)gaussX[i] = -gaussX[i];
    scalarListList shift(2,scalarList(nDofPerCell_));

    //compute blending function at each node for each edge
    scalarList blend1(nDofPerCell_), blend2(nDofPerCell_), blend3(nDofPerCell_);
    for(int i=0; i<nDofPerCell_; i++){
        blend1[i] = L2[i]*L3[i];
        blend2[i] = L1[i]*L3[i];
        blend3[i] = L1[i]*L2[i];
    }
    //amount of warp for each node, for each edge
    scalarList warpfactor1(evalwarp(p, gaussX, L3-L2));
    scalarList warpfactor2(evalwarp(p, gaussX, L1-L3));
    scalarList warpfactor3(evalwarp(p, gaussX, L2-L1));

    //combine blend & warp
    scalarList warp1(nDofPerCell_), warp2(nDofPerCell_), warp3(nDofPerCell_);
    for(int i=0; i<nDofPerCell_; i++){
        warp1[i] = blend1[i] * warpfactor1[i] * (1+sqr(pval*L1[i]));
        warp2[i] = blend2[i] * warpfactor2[i] * (1+sqr(pval*L2[i]));
        warp3[i] = blend3[i] * warpfactor3[i] * (1+sqr(pval*L3[i]));
    }

    //evaluate shift in equilateral triangle
    scalar cos1 = cos(2*Foam::constant::mathematical::pi/3);
    scalar cos2 = cos(4*Foam::constant::mathematical::pi/3);
    scalar sin1 = sin(2*Foam::constant::mathematical::pi/3);
    scalar sin2 = sin(4*Foam::constant::mathematical::pi/3);
    for(int i=0; i<nDofPerCell_; i++){
        shift[0][i] = 1*warp1[i] + cos1*warp2[i] + cos2*warp3[i];
        shift[1][i] = 0*warp1[i] + sin1*warp2[i] + sin2*warp3[i];
    }

    return shift;
}

Foam::scalarList Foam::tetrahedralBaseFunction::evalwarp(label p, scalarList& xnodes, scalarList xout)
{
    scalarList warp(xout.size(), 0.0);
    scalarList xeq(p+1);
    for(int i=0; i<=p; i++)
        xeq[i] = -1.0 + 2.0*(p-i)/p;

    for(int i=0; i<=p; i++){
        scalarList d(xout.size(), xnodes[i] - xeq[i]);
        for(int j=1; j<p; j++){
            if(i!=j){
                for(int n=0; n<d.size(); n++)
                    d[n] = d[n] * (xout[n]-xeq[j])/(xeq[i]-xeq[j]);
            }
        }

        if(i!=0){
            for(int n=0; n<d.size(); n++)
                d[n] = -d[n] / (xeq[i] - xeq[0]);
        }

        if(i!=p){
            for(int n=0; n<d.size(); n++)
                d[n] = d[n] / (xeq[i] - xeq[p]);
        }

        for(int n=0; n<d.size(); n++)
            warp[n] = warp[n] + d[n];
    }

    for(int i=0; i<warp.size(); i++)
        warp[i] = warp[i] * 4;

    return warp;
}

void Foam::tetrahedralBaseFunction::xytors()
{
    vector v1(-1, -1/sqrt(3.0), -1/sqrt(6.0));
    vector v2( 1, -1/sqrt(3.0), -1/sqrt(6.0));
    vector v3( 0,  2/sqrt(3.0), -1/sqrt(6.0));
    vector v4( 0,  0,            3/sqrt(6.0));

    vector Av1 = 0.5*(v2-v1);
    vector Av2 = 0.5*(v3-v1);
    vector Av3 = 0.5*(v4-v1);

    tensor A;
    A[0] = Av1.x(); A[1] = Av2.x(); A[2] = Av3.x();
    A[3] = Av1.y(); A[4] = Av2.y(); A[5] = Av3.y();
    A[6] = Av1.z(); A[7] = Av2.z(); A[8] = Av3.z();

    tensor invA = A.inv();

    vector delta = 0.5*(v2+v3+v4-v1);
    vector rhs;

    for(int i=0; i<nDofPerCell_; i++){
        rhs = dofLocation_[i] - delta;

        dofLocation_[i] = invA & rhs;
    }
}

void Foam::tetrahedralBaseFunction::initDrMatrix()
{
    //GradVandermonde
    const Foam::denseMatrix<vector> Vr( Foam::Legendre::GradVandermonde3D(nOrder_, dofLocation_));

    drMatrix_.setSize(nDofPerCell_, nDofPerCell_);

    //Dr = Vr / V
    //drMatrix_ = Vr * invV_;
    denseMatrix<vector>::MatMatMult(Vr, invV_, drMatrix_);
}

void Foam::tetrahedralBaseFunction::initLift()
{
    Lift_.setSize(nDofPerCell_, nDofPerFace_*4);

    if(invMassMatrix_.size()<=0)
        initInvMassMatrix();

    autoPtr<baseFunction> base_2D = Foam::baseFunction::New(nOrder_, "tri");
    if(base_2D->massMatrix_.size()<=0){
        base_2D->initMassMatrix();
    }
    const denseMatrix<scalar>& M_2D = base_2D->massMatrix_;

    denseMatrix<scalar> LiftFace0(faceToCellIndex_[0][0].size(), nDofPerCell_);
    denseMatrix<scalar> LiftFace1(faceToCellIndex_[1][0].size(), nDofPerCell_);
    denseMatrix<scalar> LiftFace2(faceToCellIndex_[2][0].size(), nDofPerCell_);
    denseMatrix<scalar> LiftFace3(faceToCellIndex_[3][0].size(), nDofPerCell_);
    denseMatrix<scalar>::MatMatMult(invMassMatrix_.subColumes(faceToCellIndex_[0][0]), M_2D, LiftFace0);
    denseMatrix<scalar>::MatMatMult(invMassMatrix_.subColumes(faceToCellIndex_[1][0]), M_2D, LiftFace1);
    denseMatrix<scalar>::MatMatMult(invMassMatrix_.subColumes(faceToCellIndex_[2][0]), M_2D, LiftFace2);
    denseMatrix<scalar>::MatMatMult(invMassMatrix_.subColumes(faceToCellIndex_[3][0]), M_2D, LiftFace3);

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

    base_2D.clear();
}

void Foam::tetrahedralBaseFunction::initCentreInterpolateMatrix()
{
    vectorList cellCentre(1, vector(0.0, 0.0, 0.0));
    denseMatrix<scalar> cellVandermondeMatrix(Foam::Legendre::Vandermonde3D(nOrder_, cellCentre));
    //cellCentreInterpolateMatrix_ = cellVandermondeMatrix * invV_;
    cellCentreInterpolateMatrix_.setSize(1, nDofPerCell_);
    denseMatrix<scalar>::MatMatMult(cellVandermondeMatrix, invV_, cellCentreInterpolateMatrix_);

    denseMatrix<scalar> faceVandermondeMatrix(Foam::Legendre::Vandermonde2D(nOrder_, cellCentre));
    //faceCentreInterpolateMatrix_ = faceVandermondeMatrix * invFaceMatrix_;
    faceCentreInterpolateMatrix_.setSize(1, nDofPerFace_);
    denseMatrix<scalar>::MatMatMult(faceVandermondeMatrix, invFaceMatrix_, faceCentreInterpolateMatrix_);
}

Foam::denseMatrix<Foam::scalar> Foam::tetrahedralBaseFunction::cellVandermonde(const Foam::vectorList& loc)const
{
    denseMatrix<scalar> ans(Foam::Legendre::Vandermonde3D(nOrder_, loc));
    return ans;
}

Foam::denseMatrix<Foam::scalar> Foam::tetrahedralBaseFunction::faceVandermonde(const Foam::vectorList& loc)const
{
    denseMatrix<scalar> ans(Foam::Legendre::Vandermonde2D(nOrder_, loc));
    return ans;
}

Foam::vectorList Foam::tetrahedralBaseFunction::physicalNodesLoc(const Foam::vectorList& cellPoints)const
{
    vectorList ans(nDofPerCell_, vector::zero);
    forAll(ans, pointI){
        ans[pointI] = 0.5*(
            -(1 + dofLocation_[pointI].x() + dofLocation_[pointI].y() + dofLocation_[pointI].z())*cellPoints[0]
            +(1 + dofLocation_[pointI].x())*cellPoints[1]
            +(1 + dofLocation_[pointI].y())*cellPoints[2]
            +(1 + dofLocation_[pointI].z())*cellPoints[3]
        );
    }
    return ans;
}

Foam::tensorList Foam::tetrahedralBaseFunction::dxdr(const Foam::vectorList& physicalNodes)
{
    if(drMatrix_.size()<=0)
        initDrMatrix();
    
    tensorList ans(nDofPerCell_, tensor::zero);
    //ans = drMatrix_ ^ physicalNodes;
    denseMatrix<vector>::MatVecOuterProduct(drMatrix_, physicalNodes, ans);

    return ans;
}

Foam::tensorList Foam::tetrahedralBaseFunction::drdx(const Foam::tensorList& xr)const
{
    tensorList ans(nDofPerCell_, tensor::zero);

    forAll(ans, item){
        scalar J = det(xr[item]);
        tensor t = cof(xr[item]).T();
        ans[item] = t / J;
    }

    return ans;
}

Foam::scalarList Foam::tetrahedralBaseFunction::jacobian(const Foam::tensorList& xr)const
{
    scalarList ans(nDofPerCell_);
    forAll(ans, item){
        ans[item] = det(xr[item]);
    }
    return ans;
}

void Foam::tetrahedralBaseFunction::faceNxFscale(const tensorList& xr, label faceI, vectorList& normal, scalarList& fscale)const
{
    normal.setSize(nDofPerFace_);
    fscale.setSize(nDofPerFace_);
    scalar nx, ny, nz, Js;
    tensor t;

    switch(faceI){
    case 0:
        forAll(normal, pointI){
            t = (cof(xr[pointI]).T())/det(xr[pointI]);
            nx = -t[6];// -tx
            ny = -t[7];// -ty
            nz = -t[8];// -tz
            Js = sqrt(sqr(nx) + sqr(ny) + sqr(nz));
            normal[pointI] = vector(nx, ny, nz)/Js;
            fscale[pointI] = Js;
        }
        break;
    case 1:
        forAll(normal, pointI){
            t = (cof(xr[pointI]).T())/det(xr[pointI]);
            nx = -t[3];// -sx
            ny = -t[4];// -sy
            nz = -t[5];// -sz
            Js = sqrt(sqr(nx) + sqr(ny) + sqr(nz));
            normal[pointI] = vector(nx, ny, nz)/Js;
            fscale[pointI] = Js;
        }
        break;
    case 2:
        forAll(normal, pointI){
            t = (cof(xr[pointI]).T())/det(xr[pointI]);
            nx = t[0]+t[3]+t[6];// rx+sx+tx
            ny = t[1]+t[4]+t[7];// ry+sy+ty
            nz = t[2]+t[5]+t[8];// rz+sz+tz
            Js = sqrt(sqr(nx) + sqr(ny) + sqr(nz));
            normal[pointI] = vector(nx, ny, nz)/Js;
            fscale[pointI] = Js;
        }
        break;
    case 3:
        forAll(normal, pointI){
            t = (cof(xr[pointI]).T())/det(xr[pointI]);
            nx = -t[0];// -rx
            ny = -t[1];// -ry
            nz = -t[2];// -rz
            Js = sqrt(sqr(nx) + sqr(ny) + sqr(nz));
            normal[pointI] = vector(nx, ny, nz)/Js;
            fscale[pointI] = Js;
        }
        break;
    }
}

void Foam::tetrahedralBaseFunction::faceFscale(const tensorList& xr, label faceI, scalar& fscale)const
{
    scalar nx, ny, nz, Js;
    tensor t;

    switch(faceI){
    case 0:
        t = (cof(xr[0]).T())/det(xr[0]);
        nx = -t[6];// -tx
        ny = -t[7];// -ty
        nz = -t[8];// -tz
        Js = sqrt(sqr(nx) + sqr(ny) + sqr(nz));
        fscale = Js;
        break;
    case 1:
        t = (cof(xr[0]).T())/det(xr[0]);
        nx = -t[3];// -sx
        ny = -t[4];// -sy
        nz = -t[5];// -sz
        Js = sqrt(sqr(nx) + sqr(ny) + sqr(nz));
        fscale = Js;
        break;
    case 2:
        t = (cof(xr[0]).T())/det(xr[0]);
        nx = t[0]+t[3]+t[6];// rx+sx+tx
        ny = t[1]+t[4]+t[7];// ry+sy+ty
        nz = t[2]+t[5]+t[8];// rz+sz+tz
        Js = sqrt(sqr(nx) + sqr(ny) + sqr(nz));
        fscale = Js;
        break;
    case 3:
        t = (cof(xr[0]).T())/det(xr[0]);
        nx = -t[0];// -rx
        ny = -t[1];// -ry
        nz = -t[2];// -rz
        Js = sqrt(sqr(nx) + sqr(ny) + sqr(nz));
        fscale = Js;
        break;
    }
}
// ************************************************************************* //