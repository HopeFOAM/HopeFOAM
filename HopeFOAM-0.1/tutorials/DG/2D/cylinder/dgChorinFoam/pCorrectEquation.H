#ifndef pCorrectEquation_H
#define pCorrectEquation_H


#include "Equation.H"

namespace Foam
{


template<class Type>
class EquationPCorrect
    : public dg::Equation<Type>
{
public:


    EquationPCorrect( const GeometricDofField<vector, dgPatchField, dgGeoMesh>& U,
                        const GeometricDofField<vector, dgPatchField, dgGeoMesh>& curlU,
                        const GeometricDofField<scalar, dgPatchField, dgGeoMesh>& nbp, 
                        const GeometricDofField<scalar, dgPatchField, dgGeoMesh>& nbpOld, scalar b0, scalar b1, scalar Ufactor){
        this->U = &U;
        this->curlU = &curlU;
        this->nbp = &nbp;
        this->nbpOld = &nbpOld;
        this->b0 = b0;
        this->b1 = b1;
        this->Ufactor = Ufactor;

        tmpScalarData.setSize(U.mesh().maxGaussPerCell());
        tmpScalarData2.setSize(U.mesh().maxGaussPerCell());
        tmpScalarData3.setSize(U.mesh().maxGaussPerCell());

        this->lduMatrixPtr = make_shared<dgLduMatrix<Type>>(U.mesh(), 0);
    }

    const GeometricDofField<Type, dgPatchField, dgGeoMesh>* getPSI() {
        return NULL;
    }
    label getMatrixLabel() {
        return 0;
    }

    void calculateCell(dgTree<physicalCellElement>::leafIterator iter)
    {
        scalar pi = constant::mathematical::pi;
        const physicalCellElement& ele = iter()->value();
        const List<vector>& px = ele.dofLocation();
        label nDof = ele.nDof();
        label dofStart = ele.gaussStart();

        // prepare lduMatrix data
        this->lduMatrixPtr->diagCoeff() = 0.0;
        Field<scalar>& source = this->lduMatrixPtr->source_;
        Field<scalar>& b = this->lduMatrixPtr->b_;

        //- init value
        for(int i=0; i<nDof; ++i){
            source[i] = 0.0;
            b[i] = 0.0;
        }
        this->lduMatrixPtr->sourceSize_ = nDof;

        const List<shared_ptr<physicalFaceElement>>& faceEleInCell = ele.faceElementList();
        forAll(faceEleInCell, faceI){
            if(faceEleInCell[faceI]->neighborEle_) continue;

            const typename Foam::GeometricField<vector, dgPatchField, dgGeoMesh>::Boundary& bDataU = U->boundaryField();
            const typename Foam::GeometricField<vector, dgPatchField, dgGeoMesh>::Boundary& bDataCurlU = curlU->boundaryField();
            const typename Foam::GeometricField<scalar, dgPatchField, dgGeoMesh>::Boundary& bDataNbp = nbp->boundaryField();
            const typename Foam::GeometricField<scalar, dgPatchField, dgGeoMesh>::Boundary& bDataNbpOld = nbpOld->boundaryField();
            label nFaceDof = ele.baseFunction().nDofPerFace();
            label nFaceGaussDof = faceEleInCell[faceI]->nGaussPoints_;

            const labelList& faceDofMapping = const_cast<shared_ptr<physicalFaceElement>&>(faceEleInCell[faceI])->ownerDofMapping();
            const vectorList& faceNx = faceEleInCell[faceI]->faceNx_;
            label patchI = faceEleInCell[faceI]->sequenceIndex().first();
            if(bDataU[patchI].fixesValue()){
                SubList<scalar> subNbp(bDataNbp[patchI],  nFaceDof, faceEleInCell[faceI]->sequenceIndex().second());
                SubList<vector> subCurlU(bDataCurlU[patchI],  nFaceDof, faceEleInCell[faceI]->sequenceIndex().second());
                for(int i=0 ; i<nFaceDof ; i++){
                    subNbp[i] = faceNx[0]&subCurlU[i];
                    if(bDataU[patchI].patch().name() == "inlet"){
                        subNbp[i] += -std::pow(1/0.41, 2)*6*(px[faceDofMapping[i]].y()+0.2)*(0.21-px[faceDofMapping[i]].y())*Ufactor;
                    }
                }
                SubList<scalar> subNbpOld(bDataNbpOld[patchI],  nFaceDof, faceEleInCell[faceI]->sequenceIndex().second());
                const denseMatrix<scalar>& faceVand = (const_cast<stdElement&>(ele.baseFunction()).gaussFaceInterpolateMatrix(faceEleInCell[faceI]->gaussOrder_, 0));
                for(int i=0; i<nFaceDof; i++){
                    subNbpOld[i] = b0*subNbp[i] + b1*subNbpOld[i];
                }
                SubList<scalar> subTmpScalarData(tmpScalarData, nFaceGaussDof);
                denseMatrix<scalar>::MatVecMult(faceVand, subNbpOld, subTmpScalarData);

                SubList<scalar> subTmpScalarData2(tmpScalarData2, nFaceGaussDof);
                const scalarList& faceWJ = faceEleInCell[faceI]->faceWJ_;
                
                for(int i=0; i<nFaceGaussDof; i++){
                    subTmpScalarData2[i] = faceWJ[i] * subTmpScalarData[i];
                }

                SubList<scalar> subTmpScalarData3(tmpScalarData3, nFaceDof);
                denseMatrix<scalar>::MatTVecMult(faceVand, subTmpScalarData2, subTmpScalarData3);

                
                for(int i=0; i<nFaceDof; i++){
                    b[faceDofMapping[i]] -= subTmpScalarData3[i];
                }
            }
        }
        //Info<<"dpdn : "<<b<<endl;
    }

    dimensionSet getDimensionSet()
    {
        return dimensionSet(0,0,-2,0,0,0,0);
    }

    const GeometricDofField<vector, dgPatchField, dgGeoMesh>* U;
    const GeometricDofField<vector, dgPatchField, dgGeoMesh>* curlU;
    const GeometricDofField<scalar, dgPatchField, dgGeoMesh>* nbpOld;
    const GeometricDofField<scalar, dgPatchField, dgGeoMesh>* nbp;
    scalar b0, b1, Ufactor;
    Field<scalar> tmpScalarData;
    Field<scalar> tmpScalarData2;
    Field<scalar> tmpScalarData3;

};


}

#endif
