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

#include "defaultLaplacianScheme.H"
#include "dgMatrices.H"
#include "dgGaussField.H"
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace dg
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
//laplacian algorithm (Internal punishment)
//refer to book <<DG>> chapter 7.2, page 232


template<class Type, class GType>
void
defaultLaplacianScheme<Type, GType>::laplacianFunc
(
    dgTree<physicalCellElement>::leafIterator iter, 
    shared_ptr<dgLduMatrix<Type>> ldu,
    const GeometricDofField<Type, dgPatchField, dgGeoMesh> *vf,
    const GeometricDofField<scalar, dgPatchField, dgGeoMesh>& gamma
)
{

}

template<class Type, class GType>
void
defaultLaplacianScheme<Type, GType>::laplacianFunc
(
    dgTree<physicalCellElement>::leafIterator iter, 
    shared_ptr<dgLduMatrix<Type>> ldu,
    const GeometricDofField<Type, dgPatchField, dgGeoMesh> *psi
)
{
    const physicalCellElement& cellEle = iter()->value();
    label nDof = cellEle.nDof();
    label nGauss = cellEle.baseFunction().gaussNDofPerCell();
    label dofStart = cellEle.dofStart();

    // prepare lduMatrix data
    Field<Type>& source = ldu->source_;
    Field<Type>& b = ldu->b_;
    //- init value
    for(int i=0; i<nDof; ++i){
        source[i] = pTraits<Type>::zero;
        b[i] = pTraits<Type>::zero;
    }
    ldu->sourceSize_ = nDof;

    // prepare cell quadrature data
    const scalarList& gWJ = cellEle.jacobianWeights();
    const denseMatrix<vector>& cellD1dx = cellEle.cellD1dx();

    //- integrate didj_ part
    denseMatrix<scalar>& diag = ldu->diag();
    ldu->diagCoeff() = 0.0;
    diag.resetLabel(nDof, nDof);
    diag.setToZeroMatrix();
    // diag = Vr' * diag(WJ) * Vr
    for(int k=0, ps=0; k<nGauss; k++, ps+=nDof){
        for(int i=0, ptr=0, pi=ps; i<nDof; i++, pi++){
            for(int j=0, pj=ps; j<nDof; j++, ptr++, pj++){
                diag[ptr] -= cellD1dx[pi] & cellD1dx[pj] * gWJ[k];
            }
        }
    }

    //- face integration
    const denseMatrix<scalar>* faceVand1, *faceVand2, *faceVand3, *faceVand4;
    const denseMatrix<scalar>* faceDn1, *faceDn2;
    label nFaceDof1, nFaceDof2, nCellDof1, nCellDof2;
    const stdElement* faceEle1, *faceEle2;
    const labelList* faceDofMapping1, *faceDofMapping2;
    const List<shared_ptr<physicalFaceElement>>& faceEleInCell = cellEle.faceElementList();
    List<denseMatrix<scalar>>& lu2 = ldu->lu2();
    forAll(faceEleInCell, faceI){
        //- prepare face quadrature data
        scalar gtau = (faceEleInCell[faceI]->fscale_)*20*std::pow(faceEleInCell[faceI]->gaussOrder_+1, 2);
        const scalarList& faceWJ = faceEleInCell[faceI]->faceWJ_;
        label nFaceGauss = faceEleInCell[faceI]->nGaussPoints_;

        if(faceEleInCell[faceI]->ownerEle_ == iter()){
            //- owner cell
            faceEle1 = &(faceEleInCell[faceI]->ownerEle_->value().baseFunction());
            faceEle2 = (faceEleInCell[faceI]->neighborEle_)?&(faceEleInCell[faceI]->neighborEle_->value().baseFunction()):NULL;
            nFaceDof1 = faceEle1->nDofPerFace();
            nFaceDof2 = faceEle2?(faceEle2->nDofPerFace()):-1;
            nCellDof1 = faceEle1->nDofPerCell();
            nCellDof2 = faceEle2?(faceEle2->nDofPerCell()):-1;
            faceVand1 = &(const_cast<stdElement*>(faceEle1)->gaussFaceInterpolateMatrix(faceEleInCell[faceI]->gaussOrder_, 0));
            faceVand2 = faceEle2?&(const_cast<stdElement*>(faceEle2)->gaussFaceInterpolateMatrix(faceEleInCell[faceI]->gaussOrder_, 0)):NULL;
            faceVand3 = &(const_cast<stdElement*>(faceEle1)->gaussFaceInterpolateMatrix(faceEleInCell[faceI]->gaussOrder_, 0));
            faceVand4 = faceEle2?&(const_cast<stdElement*>(faceEle2)->gaussFaceInterpolateMatrix(faceEleInCell[faceI]->gaussOrder_, 0)):NULL;
            faceDn1 = &(faceEleInCell[faceI]->gaussOwnerFaceDn_);
            faceDn2 = &(faceEleInCell[faceI]->gaussNeighborFaceDnRotate_);
            faceDofMapping1 = &(faceEleInCell[faceI]->ownerDofMapping());
            faceDofMapping2 = faceEle2?&(faceEleInCell[faceI]->neighborDofMapping()):NULL;
        }
        else
        {
            //- neighbor cell
            faceEle2 = &(faceEleInCell[faceI]->ownerEle_->value().baseFunction());
            faceEle1 = &(faceEleInCell[faceI]->neighborEle_->value().baseFunction());
            nFaceDof2 = faceEle1->nDofPerFace();
            nFaceDof1 = faceEle2->nDofPerFace();
            nCellDof2 = faceEle1->nDofPerCell();
            nCellDof1 = faceEle2->nDofPerCell();
            faceVand2 = &(const_cast<stdElement*>(faceEle1)->gaussFaceInterpolateMatrix(faceEleInCell[faceI]->gaussOrder_, 0));
            faceVand4 = &(const_cast<stdElement*>(faceEle1)->gaussFaceInterpolateMatrix(faceEleInCell[faceI]->gaussOrder_, faceEleInCell[faceI]->faceRotate_));
            faceVand3 = &(const_cast<stdElement*>(faceEle2)->gaussFaceInterpolateMatrix(faceEleInCell[faceI]->gaussOrder_, 0));
            faceVand1 = &(const_cast<stdElement*>(faceEle2)->gaussFaceInterpolateMatrix(faceEleInCell[faceI]->gaussOrder_, faceEleInCell[faceI]->faceRotate_));
            faceDn2 = &(faceEleInCell[faceI]->gaussOwnerFaceDn_);
            faceDn1 = &(faceEleInCell[faceI]->gaussNeighborFaceDn_);
            faceDofMapping2 = &(faceEleInCell[faceI]->ownerDofMapping());
            faceDofMapping1 = &(faceEleInCell[faceI]->neighborDofMapping());
        }
        if(faceEle2 == NULL){
            lu2[faceI].resetLabel(0, 0);
            const typename Foam::GeometricField<Type, dgPatchField, dgGeoMesh>::Boundary& bData = psi->boundaryField();
            label patchI = faceEleInCell[faceI]->sequenceIndex().first();
            if(bData[patchI].fixesValue()){
                SubList<Type> subBoundary(bData[patchI],  nFaceDof1, faceEleInCell[faceI]->sequenceIndex().second());
                    //- faceEle1, add to diag
                    //- part1, faceDiag to source
                    tmpMatrix_.resetLabel(nFaceDof1, nFaceDof1);
                    denseMatrix<scalar>::MatDiagMatMult(*faceVand1, faceWJ, *faceVand1, tmpMatrix_);
                    tmpMatrix_ *= (gtau);
                    for(int i=0, ptr=0; i<nFaceDof1; i++){
                        label ptr1 = (*faceDofMapping1)[i] * nCellDof1;
                        for(int j=0; j<nFaceDof1; j++, ptr++){
                            diag[ptr1 + (*faceDofMapping1)[j]] -= tmpMatrix_[ptr];
                        }
                    }
                    //- add boundary data to source
                    SubList<Type> subTmpData(tmpData, nFaceDof1);
                    denseMatrix<Type>::MatTVecMult(tmpMatrix_, subBoundary, subTmpData);
                    for(int i=0; i<nFaceDof1; i++){
                        b[(*faceDofMapping1)[i]] -= subTmpData[i];
                    }

                    //- part2, faceVmDnM
                    tmpMatrix_.resetLabel(nFaceDof1, nCellDof1);
                    denseMatrix<scalar>::MatDiagMatMult(*faceVand1, faceWJ, *faceDn1, tmpMatrix_);
                    for(int i=0, ptr=0; i<nFaceDof1; i++){
                        label ptr1 = (*faceDofMapping1)[i] * nCellDof1;
                        for(int j=0; j<nCellDof1; j++, ptr++, ptr1++){
                            diag[ptr1] += tmpMatrix_[ptr];
                        }
                    }

                    //- part3, faceDnMVm
                    tmpMatrix_.resetLabel(nCellDof1, nFaceDof1);
                    denseMatrix<scalar>::MatDiagMatMult(*faceDn1, faceWJ, *faceVand1, tmpMatrix_);
                    for(int i=0, ptr=0; i<nCellDof1; i++){
                        label ptr1 = i * nCellDof1;
                        for(int j=0; j<nFaceDof1; j++, ptr++){
                            diag[ptr1 + (*faceDofMapping1)[j]] += tmpMatrix_[ptr];
                        }
                    }
                    //add boundary data to source
                    SubList<Type> subTmpData2(tmpData, nCellDof1);
                    denseMatrix<Type>::MatVecMult(tmpMatrix_, subBoundary, subTmpData2);
                    for(int i=0; i<nCellDof1; i++){
                        b[i] += subTmpData2[i];
                    }
                    
                    // Info << "patch "<<patchI<<", face "<<faceI<<endl;
                    // Info << "faceDn"<<endl<<faceDn<<endl;
                    // Info << "faceWJ"<<endl<<faceWJ<<endl;
                    // Info << "finterp"<<endl<<faceVand<<endl;
                    // Info << "index "<<dgFaceI.sequenceIndex().second()<<endl;
                    // Info << "subBoundary"<<endl<<subBoundary<<endl;
            }
            else if(bData[patchI].fixesGradient()){
                SubList<Type> subBoundary(bData[patchI].gradient(), nFaceDof1, faceEleInCell[faceI]->sequenceIndex().second());
                tmpMatrix_.resetLabel(nFaceDof1, nFaceDof1);
                denseMatrix<scalar>::MatDiagMatMult(*faceVand1, faceWJ, *faceVand1, tmpMatrix_);
                //- add boundary data to source
                SubList<Type> subTmpData(tmpData, nFaceDof1);
                denseMatrix<Type>::MatTVecMult(tmpMatrix_, subBoundary, subTmpData);
                for(int i=0; i<nFaceDof1; i++){
                    b[(*faceDofMapping1)[i]] -= subTmpData[i];
                }
            }
        }
        else{
        //- faceEle1, add to diag
            //- part1, faceDiagMatrix
            tmpMatrix_.resetLabel(nFaceDof1, nFaceDof1);
            denseMatrix<scalar>::MatDiagMatMult(*faceVand1, faceWJ, *faceVand1, tmpMatrix_);
            tmpMatrix_ *= (gtau * 0.5);
            for(int i=0, ptr=0; i<nFaceDof1; i++){
                label ptr1 = (*faceDofMapping1)[i] * nCellDof1;
                for(int j=0; j<nFaceDof1; j++, ptr++){
                    diag[ptr1 + (*faceDofMapping1)[j]] -= tmpMatrix_[ptr];
                }
            }

            //- part2, faceVmDnM
            tmpMatrix_.resetLabel(nFaceDof1, nCellDof1);
            denseMatrix<scalar>::MatDiagMatMult(*faceVand1, faceWJ, *faceDn1, tmpMatrix_);
            for(int i=0, ptr=0; i<nFaceDof1; i++){
                label ptr1 = (*faceDofMapping1)[i] * nCellDof1;
                for(int j=0; j<nCellDof1; j++, ptr++, ptr1++){
                    diag[ptr1] += tmpMatrix_[ptr] * 0.5;
                }
            }
            //- part3, faceDnMVm
            tmpMatrix_.resetLabel(nCellDof1, nFaceDof1);
            denseMatrix<scalar>::MatDiagMatMult(*faceDn1, faceWJ, *faceVand1, tmpMatrix_);
            for(int i=0, ptr=0; i<nCellDof1; i++){
                label ptr1 = i * nCellDof1;
                for(int j=0; j<nFaceDof1; j++, ptr++){
                    diag[ptr1 + (*faceDofMapping1)[j]] += tmpMatrix_[ptr] * 0.5;
                }
            }

        //- faceEle2, add to lu2

            lu2[faceI].resetLabel(nCellDof1, nCellDof2);
            lu2[faceI].setToZeroMatrix();
            //- part1, faceDiagMatrix
            tmpMatrix_.resetLabel(nFaceDof1, nFaceDof2);
            denseMatrix<scalar>::MatDiagMatMult(*faceVand3, faceWJ, *faceVand2, tmpMatrix_);
            tmpMatrix_ *= (gtau * 0.5);
            for(int i=0, ptr=0; i<nFaceDof1; i++){
                label ptr1 = (*faceDofMapping1)[i] * nCellDof1;
                for(int j=0; j<nFaceDof1; j++, ptr++){
                    lu2[faceI][ptr1 + (*faceDofMapping2)[j]] += tmpMatrix_[ptr];
                }
            }
            //- part2, faceVmDnM
            tmpMatrix_.resetLabel(nFaceDof1, nCellDof2);
            denseMatrix<scalar>::MatDiagMatMult(*faceVand3, faceWJ, *faceDn2, tmpMatrix_);
            for(int i=0, ptr=0; i<nFaceDof1; i++){
                label ptr1 = (*faceDofMapping1)[i] * nCellDof2;
                for(int j=0; j<nCellDof2; j++, ptr++, ptr1++){
                    lu2[faceI][ptr1] -= tmpMatrix_[ptr] * 0.5;
                }
            }
            //- part3, faceDnMVm
            tmpMatrix_.resetLabel(nCellDof1, nFaceDof2);
            denseMatrix<scalar>::MatDiagMatMult(*faceDn1, faceWJ, *faceVand4, tmpMatrix_);
            for(int i=0, ptr=0; i<nCellDof1; i++){
                label ptr1 = i * nCellDof1;
                for(int j=0; j<nFaceDof2; j++, ptr++){
                    lu2[faceI][ptr1 + (*faceDofMapping2)[j]] -= tmpMatrix_[ptr] * 0.5;
                }
            }
        }
    }
}
/*
template<class Type, class GType>
void defaultLaplacianScheme<Type, GType>::applyBoundaryConditions(
    shared_ptr<dgMatrix<Type>> matrix,
    const GeometricDofField<Type, dgPatchField, dgGeoMesh> *psi)
{
    const dgMesh& mesh = psi->mesh();
    const typename Foam::GeometricField<Type, dgPatchField, dgGeoMesh>::Boundary& bData = psi->boundaryField();
    Field<Type>& matrix_b = matrix->b();
    List<Type> tmpData(mesh.maxGaussPerCell());
    List<Type> tmpData2(mesh.maxGaussPerCell());
    const stdElement* faceEle;
    label nCellDof, nFaceDof, nFaceGauss;
    const List<shared_ptr<dgTreeUnit<physicalFaceElement>>>& faceElement = mesh.faceElements();

    forAll(bData, patchI){
        if(bData[patchI].fixesValue()){
            const labelList bdFaceIndex = mesh.boundary()[patchI].dgFaceIndex();
            forAll(bdFaceIndex, faceI){
                label bdFaceI = bdFaceIndex[faceI];
                const physicalFaceElement& dgFaceI = faceElement[bdFaceI]->value();
                faceEle = &(dgFaceI.ownerEle_->value().baseFunction());
                const denseMatrix<scalar>& faceVand = (const_cast<stdElement*>(faceEle)->gaussFaceInterpolateMatrix(dgFaceI.gaussOrder_, 0));
                const denseMatrix<scalar>& faceDn = (dgFaceI.gaussOwnerFaceDn_);
                const labelList& faceDofMapping = const_cast<physicalFaceElement&>(dgFaceI).ownerDofMapping();

                scalar gtau = (dgFaceI.fscale_)*20*pow(dgFaceI.gaussOrder_+1, 2);
                const scalarList& faceWJ = dgFaceI.faceWJ_;
                nFaceGauss = dgFaceI.nGaussPoints_;
                nCellDof = faceEle->nDofPerCell();
                nFaceDof = faceEle->nDofPerFace();

                SubList<Type> subTmpData(tmpData, nFaceGauss);
                SubList<Type> subBoundary(bData[patchI], nFaceDof, dgFaceI.sequenceIndex().second());
                denseMatrix<Type>::MatVecMult(faceVand, subBoundary, subTmpData);
                for(int i=0; i<nFaceGauss; i++){
                    subTmpData[i] *= (faceWJ[i]*0.5);
                }

                SubField<Type> subMatrix_b(matrix_b, nCellDof, dgFaceI.ownerEle_->value().dofStart());
                //- part1, faceDiag
                SubList<Type> subTmpData2(tmpData2, nFaceDof);
                denseMatrix<Type>::MatTVecMult(faceVand, subTmpData, subTmpData2);
                for(int i=0; i<nFaceDof; i++){
                    subMatrix_b[faceDofMapping[i]] += subTmpData2[i]*gtau;
                }
                
                //- part2, faceDnMVm
                Info << "patch "<<patchI<<", face "<<faceI<<endl;
                Info << "faceDn"<<endl<<faceDn<<endl;
                Info << "faceWJ"<<endl<<faceWJ<<endl;
                Info << "finterp"<<endl<<faceVand<<endl;
                Info << "index "<<dgFaceI.sequenceIndex().second()<<endl;
                Info << "subBoundary"<<endl<<subBoundary<<endl;
                SubList<Type> subTmpData3(tmpData2, nCellDof);
                denseMatrix<Type>::MatTVecMult(faceDn, subTmpData, subTmpData3);
                for(int i=0; i<nCellDof; i++){
                    subMatrix_b[i] -= subTmpData3[i];
                }
            }
        }
    }
} */
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

template<class Type, class GType>
void defaultLaplacianScheme<Type, GType>::dgcLaplacianCalculateCell(
    dgTree<physicalCellElement>::leafIterator iter, 
    shared_ptr<dgLduMatrix<Type>> ldu, 
    const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf)
{}

template<class Type, class GType>
void defaultLaplacianScheme<Type, GType>::dgmLaplacianCalculateCell(
    dgTree<physicalCellElement>::leafIterator iter, 
    shared_ptr<dgLduMatrix<Type>> ldu,
    const GeometricDofField<Type, dgPatchField, dgGeoMesh> *vf)
{
    this->laplacianFunc(iter, ldu, vf);
}

template<class Type, class GType>
void defaultLaplacianScheme<Type, GType>::dgmLaplacianCalculateCell(
    dgTree<physicalCellElement>::leafIterator iter, 
    shared_ptr<dgLduMatrix<Type>> ldu,
    const GeometricDofField<Type, dgPatchField, dgGeoMesh> *vf,
    const GeometricDofField<scalar, dgPatchField, dgGeoMesh>& gamma)
{
    this->laplacianFunc(iter, ldu, vf, gamma);
}


} // End namespace fv

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //
