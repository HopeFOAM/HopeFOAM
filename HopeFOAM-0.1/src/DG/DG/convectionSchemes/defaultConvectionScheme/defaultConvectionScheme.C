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

\*---------------------------------------------------------------------------*/

#include "defaultConvectionScheme.H"
#include "calculatedDgPatchField.H"
#include "dgMatrices.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace dg
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

template<class Type>
void defaultConvectionScheme<Type>::dgcDivCalculateCell(
    dgTree<physicalCellElement>::leafIterator iter, 
    shared_ptr<dgLduMatrix<Type>> ldu,
    const dgGaussVectorField& U, 
    const GeometricDofField<Type, dgPatchField, dgGeoMesh> * psi,
    const Field<Type>& flux)
{
    const physicalCellElement& ele = iter()->value();
    label nDof = ele.nDof();
    label nGauss = ele.baseFunction().gaussNDofPerCell();
    label dofStart = ele.gaussStart();

    // prepare field data
    const Field<Type>& vf = psi->gaussField().cellField();
    const Field<vector>& uvf = U.cellField();

    // prepare lduMatrix data
    ldu->diagCoeff() = 0.0;
    Field<Type>& source = ldu->source_;
    Field<Type>& b = ldu->b_;

    // prepare cell quadrature data
    const scalarList& gWJ = ele.jacobianWeights();
    const denseMatrix<vector>& cellD1dx = ele.cellD1dx();

    //- init value
    for(int i=0; i<nDof; ++i){
        source[i] = pTraits<Type>::zero;
        b[i] = pTraits<Type>::zero;
    }
    ldu->sourceSize_ = nDof;

    //- cell integration
    for(int i=0, dofI = dofStart; i<nGauss; ++i, ++dofI){
        tempX[i] = uvf[dofI][0] * vf[dofI] * gWJ[i];
        tempY[i] = uvf[dofI][1] * vf[dofI] * gWJ[i];
        tempZ[i] = uvf[dofI][2] * vf[dofI] * gWJ[i];
    }
    for(int i=0, ptr=0; i<nGauss; ++i){
        for(int j=0; j<nDof; ++j, ++ptr){
            b[j] += (cellD1dx[ptr][0] * tempX[i] + cellD1dx[ptr][1] * tempY[i] + cellD1dx[ptr][2] * tempZ[i]);
        }
    }

    //- face integration
    const List<shared_ptr<physicalFaceElement>>& faceEleInCell = ele.faceElementList();
    forAll(faceEleInCell, faceI){
        //- prepare face quadrature data
        const scalarList& faceWJ = faceEleInCell[faceI]->faceWJ_;
        label nFaceGauss = faceEleInCell[faceI]->nGaussPoints_;
        const SubField<Type> subFlux(flux, nFaceGauss, faceEleInCell[faceI]->ownerFaceStart_);
        forAll(faceWJ, pointI){
            tempX[pointI] = faceWJ[pointI] * subFlux[pointI];
        }
        if(faceEleInCell[faceI]->ownerEle_ == iter()){
            //- owner cell
            const stdElement& ownerEle = faceEleInCell[faceI]->ownerEle_->value().baseFunction();
            label nFaceDof = ownerEle.nDofPerFace();
            const denseMatrix<scalar>& faceVand = const_cast<stdElement&>(ownerEle).gaussFaceInterpolateMatrix(faceEleInCell[faceI]->gaussOrder_, 0);
            const labelList& ownerDofMapping = faceEleInCell[faceI]->ownerDofMapping();
            for(int i=0, ptr=0; i<nFaceGauss; ++i){
                for(int j=0; j<nFaceDof; ++j, ++ptr){
                    b[ownerDofMapping[j]] -= tempX[i] * faceVand[ptr];
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
                    b[neighborDofMapping[j]] += tempX[i] * faceVand[ptr];
                }
            }
        }
    }
}

template<class Type>
void defaultConvectionScheme<Type>::dgcDivCalculateCell(
    dgTree<physicalCellElement>::leafIterator iter, 
    shared_ptr<dgLduMatrix<Type>> ldu,
    const dgGaussVectorField& U, 
    const dgGaussField<Type>& psi,
    const Field<Type>& flux)
{
    const physicalCellElement& ele = iter()->value();
    label nDof = ele.nDof();
    label nGauss = ele.baseFunction().gaussNDofPerCell();
    label dofStart = ele.gaussStart();

    // prepare field data
    const Field<Type>& vf = psi.cellField();
    const Field<vector>& uvf = U.cellField();

    // prepare lduMatrix data
    ldu->diagCoeff() = 0.0;
    Field<Type>& source = ldu->source_;
    Field<Type>& b = ldu->b_;

    // prepare cell quadrature data
    const scalarList& gWJ = ele.jacobianWeights();
    const denseMatrix<vector>& cellD1dx = ele.cellD1dx();

    //- init value
    for(int i=0; i<nDof; ++i){
        source[i] = pTraits<Type>::zero;
        b[i] = pTraits<Type>::zero;
    }
    ldu->sourceSize_ = nDof;

    //- cell integration
    for(int i=0, dofI = dofStart; i<nGauss; ++i, ++dofI){
        tempX[i] = uvf[dofI][0] * vf[dofI] * gWJ[i];
        tempY[i] = uvf[dofI][1] * vf[dofI] * gWJ[i];
        tempZ[i] = uvf[dofI][2] * vf[dofI] * gWJ[i];
    }
    for(int i=0, ptr=0; i<nGauss; ++i){
        for(int j=0; j<nDof; ++j, ++ptr){
            b[j] += (cellD1dx[ptr][0] * tempX[i] + cellD1dx[ptr][1] * tempY[i] + cellD1dx[ptr][2] * tempZ[i]);
        }
    }

    //- face integration
    const List<shared_ptr<physicalFaceElement>>& faceEleInCell = ele.faceElementList();
    forAll(faceEleInCell, faceI){
        //- prepare face quadrature data
        const scalarList& faceWJ = faceEleInCell[faceI]->faceWJ_;
        label nFaceGauss = faceEleInCell[faceI]->nGaussPoints_;
        const SubField<Type> subFlux(flux, nFaceGauss, faceEleInCell[faceI]->ownerFaceStart_);
        forAll(faceWJ, pointI){
            tempX[pointI] = faceWJ[pointI] * subFlux[pointI];
        }
        if(&(faceEleInCell[faceI]->ownerEle_->value()) == &(iter()->value())){
            //- owner cell
            const stdElement& ownerEle = faceEleInCell[faceI]->ownerEle_->value().baseFunction();
            label nFaceDof = ownerEle.nDofPerFace();
            const denseMatrix<scalar>& faceVand = const_cast<stdElement&>(ownerEle).gaussFaceInterpolateMatrix(faceEleInCell[faceI]->gaussOrder_, 0);
            const labelList& ownerDofMapping = faceEleInCell[faceI]->ownerDofMapping();
            for(int i=0, ptr=0; i<nFaceGauss; ++i){
                for(int j=0; j<nFaceDof; ++j, ++ptr){
                    b[ownerDofMapping[j]] -= tempX[i] * faceVand[ptr];
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
                    b[neighborDofMapping[j]] += tempX[i] * faceVand[ptr];
                }
            }
        }
    }
}

template<class Type>
void defaultConvectionScheme<Type>::dgcDivCalculateCell(
    dgTree<physicalCellElement>::leafIterator iter, 
    shared_ptr<dgLduMatrix<Type>> ldu,
    const GeometricDofField<vector, dgPatchField, dgGeoMesh>& dofU,
    const GeometricDofField<Type, dgPatchField, dgGeoMesh>& psi,
    const Field<Type>& flux)
{
    const physicalCellElement& ele = iter()->value();
    label nDof = ele.nDof();
    label nGauss = ele.baseFunction().gaussNDofPerCell();
    label dofStart = ele.dofStart();

    // prepare field data
    const Field<Type>& vf = psi.primitiveField();
    const Field<vector>& uvf = dofU.primitiveField();

    // prepare lduMatrix data
    ldu->diagCoeff() = 0.0;
    Field<Type>& source = ldu->source_;
    Field<Type>& b = ldu->b_;

    // prepare cell quadrature data
    const scalarList& gWJ = ele.jacobianWeights();
    const denseMatrix<vector>& cellD1dx = ele.cellD1dx();

    //- init value
    for(int i=0; i<nDof; ++i){
        source[i] = pTraits<Type>::zero;
        b[i] = pTraits<Type>::zero;
    }
    ldu->sourceSize_ = nDof;

    //- cell integration
    const denseMatrix<scalar>& interploateMatrix = ele.baseFunction().gaussCellVandermonde();
    for(int k=0; k<3; k++){
        for(int i=0, dofI = dofStart; i<nDof; ++i, ++dofI){
            tempX[i] = uvf[dofI][k] * vf[dofI];
        }
        SubList<Type> subTempX(tempX, nDof);
        SubList<Type> subGTempX(gtempX, nGauss);
        denseMatrix<Type>::MatVecMult(interploateMatrix, subTempX, subGTempX);
        for(int i=0; i<nGauss; i++)
            subGTempX[i] *= gWJ[i];
        for(int i=0, ptr=0; i<nGauss; ++i){
            for(int j=0; j<nDof; ++j, ++ptr){
                b[j] += cellD1dx[ptr][k] * subGTempX[i];
            }
        }
    }

    //- face integration
    const List<shared_ptr<physicalFaceElement>>& faceEleInCell = ele.faceElementList();
    forAll(faceEleInCell, faceI){
        //- prepare face quadrature data
        const scalarList& faceWJ = faceEleInCell[faceI]->faceWJ_;
        label nFaceGauss = faceEleInCell[faceI]->nGaussPoints_;
        const SubField<Type> subFlux(flux, nFaceGauss, faceEleInCell[faceI]->ownerFaceStart_);
        forAll(faceWJ, pointI){
            tempX[pointI] = faceWJ[pointI] * subFlux[pointI];
        }
        if(&(faceEleInCell[faceI]->ownerEle_->value()) == &(iter()->value())){
            //- owner cell
            const stdElement& ownerEle = faceEleInCell[faceI]->ownerEle_->value().baseFunction();
            label nFaceDof = ownerEle.nDofPerFace();
            const denseMatrix<scalar>& faceVand = const_cast<stdElement&>(ownerEle).gaussFaceInterpolateMatrix(faceEleInCell[faceI]->gaussOrder_, 0);
            const labelList& ownerDofMapping = faceEleInCell[faceI]->ownerDofMapping();
            for(int i=0, ptr=0; i<nFaceGauss; ++i){
                for(int j=0; j<nFaceDof; ++j, ++ptr){
                    b[ownerDofMapping[j]] -= tempX[i] * faceVand[ptr];
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
                    b[neighborDofMapping[j]] += tempX[i] * faceVand[ptr];
                }
            }
        }
    }
}

template<class Type>
void defaultConvectionScheme<Type>::dgmDivCalculateCell(
    dgTree<physicalCellElement>::leafIterator iter, 
    shared_ptr<dgLduMatrix<Type>> ldu,
    const dgGaussVectorField& U,
    const GeometricDofField<Type, dgPatchField, dgGeoMesh> * psi)
{

}
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace dg

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //
