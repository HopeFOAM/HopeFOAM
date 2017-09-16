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

#include "defaultDivScheme.H"
#include "IOobject.H"
//#include "dimensionedType.H"
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace dg
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

template<class Type>
void defaultDivScheme<Type>::dgcDivCalculateCell(
    dgTree<physicalCellElement>::leafIterator iter, 
    shared_ptr<dgLduMatrix<Type>> lduMatrixPtr, 
    const GeometricDofField<typename outerProduct<vector, Type>::type, dgPatchField, dgGeoMesh> * vf)
{
    const physicalCellElement& ele = iter()->value();
    label nDof = ele.nDof();
    const stdElement* baseFunc = &(ele.baseFunction());
    const denseMatrix<vector>& Dr = const_cast<stdElement*>(baseFunc)->drMatrix();
    const List<tensor>& drdx = ele.drdx();
    label dofStart = ele.dofStart();

    // prepare field data
    typedef typename outerProduct<vector, Type>::type GradType;
    const Field<GradType>& data_vf = *vf;

    // prepare lduMatrix data
    lduMatrixPtr->diagCoeff() = 0.0;
    Field<Type>& source = lduMatrixPtr->source_;
    Field<Type>& b = lduMatrixPtr->b_;

    for(int i=0; i<nDof; ++i){
        source[i] = pTraits<Type>::zero;
        b[i] = pTraits<Type>::zero;
    }
    lduMatrixPtr->sourceSize_ = nDof;

    for(int i=0 , ptr=0 ; i<nDof ; ++i){
        for(int j=0 ; j<nDof ; ++j,++ptr){
            source[i] -= vector(
                        Dr[ptr] & vector(drdx[i][0], drdx[i][1], drdx[i][2]),
                        Dr[ptr] & vector(drdx[i][3], drdx[i][4], drdx[i][5]), 
                        0) & data_vf[dofStart + j];
        }
    }
}

template<class Type>
void defaultDivScheme<Type>::dgcDivCalculateCell(
    dgTree<physicalCellElement>::leafIterator iter, 
    shared_ptr<dgLduMatrix<Type>> lduMatrixPtr, 
    const dgGaussField<typename outerProduct<vector, Type>::type>& dgf,
    const Field<typename outerProduct<vector, Type>::type>& flux)
{

    const physicalCellElement& ele = iter()->value();
    label nDof = ele.nDof();
    label nGauss = ele.baseFunction().gaussNDofPerCell();
    label dofStart = ele.gaussStart();

    // prepare field data
    typedef typename outerProduct<vector, Type>::type GradType;
    const Field<GradType>& gvf = dgf.cellField();

    // prepare lduMatrix data
    lduMatrixPtr->diagCoeff() = 0.0;
    Field<Type>& source = lduMatrixPtr->source_;
    Field<Type>& b = lduMatrixPtr->b_;

    // prepare cell quadrature data
    const scalarList& gWJ = ele.jacobianWeights();
    const denseMatrix<vector>& cellD1dx = ele.cellD1dx();

    //- init value
    for(int i=0; i<nDof; ++i){
        source[i] = pTraits<Type>::zero;
        b[i] = pTraits<Type>::zero;
    }
    lduMatrixPtr->sourceSize_ = nDof;

    //- cell integration
    for(int i=0, dofI = dofStart; i<nGauss; ++i, ++dofI){
        tempX[i] = gvf[dofI] * gWJ[i];
    }
    for(int i=0, ptr=0; i<nGauss; ++i){
        for(int j=0; j<nDof; ++j, ++ptr){
            b[j] += (cellD1dx[ptr] & tempX[i]);
        }
    }

    //- face integration
    const List<shared_ptr<physicalFaceElement>>& faceEleInCell = ele.faceElementList();
    forAll(faceEleInCell, faceI){
        //- prepare face quadrature data
        const scalarList& faceWJ = faceEleInCell[faceI]->faceWJ_;
        const vectorList& faceNx = faceEleInCell[faceI]->faceNx_;
        label nFaceGauss = faceEleInCell[faceI]->nGaussPoints_;
        const SubField<GradType> subFlux(flux, nFaceGauss, faceEleInCell[faceI]->ownerFaceStart_);
        forAll(faceWJ, pointI){
            tempY[pointI] = faceWJ[pointI] * (faceNx[pointI] & subFlux[pointI]);
        }
        if(&(faceEleInCell[faceI]->ownerEle_->value()) == &(iter()->value())){
            //- owner cell
            const stdElement& ownerEle = faceEleInCell[faceI]->ownerEle_->value().baseFunction();
            label nFaceDof = ownerEle.nDofPerFace();
            const denseMatrix<scalar>& faceVand = const_cast<stdElement&>(ownerEle).gaussFaceInterpolateMatrix(faceEleInCell[faceI]->gaussOrder_, 0);
            const labelList& ownerDofMapping = faceEleInCell[faceI]->ownerDofMapping();
            for(int i=0, ptr=0; i<nFaceGauss; ++i){
                for(int j=0; j<nFaceDof; ++j, ++ptr){
                    b[ownerDofMapping[j]] -= tempY[i] * faceVand[ptr];
                }
            }
        }
        else
        {
            //- neighbor cell
            const stdElement& neighborEle = faceEleInCell[faceI]->neighborEle_->value().baseFunction();
            label nFaceDof = neighborEle.nDofPerFace();
            const denseMatrix<scalar>& faceVand = 
                const_cast<stdElement&>(neighborEle).gaussFaceInterpolateMatrix(faceEleInCell[faceI]->gaussOrder_, 0);
            const labelList& neighborDofMapping = faceEleInCell[faceI]->neighborDofMapping();
            for(int i=0, ptr=0; i<nFaceGauss; ++i){
                for(int j=0; j<nFaceDof; ++j, ++ptr){
                    b[neighborDofMapping[j]] += tempY[i] * faceVand[ptr];
                }
            }
        }
    }

}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace dg

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //
