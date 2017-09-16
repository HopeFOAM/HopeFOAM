#ifndef setBoundaryValues_H
#define setBoundaryValues_H

#include "dgCFD.H"

namespace Foam
{

void setBoundaryValues(const GeometricDofField<scalar, dgPatchField, dgGeoMesh>& rho,
                          const GeometricDofField<vector, dgPatchField, dgGeoMesh>& rhoU, 
                          const GeometricDofField<scalar, dgPatchField, dgGeoMesh>& Ener,
                          dimensionedScalar gamma,
                          dimensionedScalar time)
{
	//init field to sin(x)
	scalar pi = constant::mathematical::pi;
	scalar gTime = (1+20*time.value())/std::sqrt(3.0);
	scalar position;

	const dgMesh& mesh = rho.mesh();
	const List<shared_ptr<dgTreeUnit<physicalFaceElement>>>& faceElement = mesh.faceElements();
	const typename Foam::GeometricField<scalar, dgPatchField, dgGeoMesh>::Boundary& bDataRho =  rho.boundaryField();
	const typename Foam::GeometricField<vector, dgPatchField, dgGeoMesh>::Boundary& bDataRhoU = rhoU.boundaryField();
	const typename Foam::GeometricField<scalar, dgPatchField, dgGeoMesh>::Boundary& bDataEner = Ener.boundaryField();
	label nFaceDof;
	forAll(bDataRho, patchI){
        if(bDataRho[patchI].patch().name() == "wall" || bDataRho[patchI].processored()) continue;
        const labelList bdFaceIndex = mesh.boundary()[patchI].dgFaceIndex();
        forAll(bdFaceIndex, faceI){
            label bdFaceI = bdFaceIndex[faceI];
            const physicalFaceElement& dgFaceI = faceElement[bdFaceI]->value();
            nFaceDof = dgFaceI.ownerEle_->value().baseFunction().nDofPerFace();
            const List<vector>& px = dgFaceI.ownerEle_->value().dofLocation();
            SubList<scalar> subBoundaryRho (bDataRho [patchI], nFaceDof, dgFaceI.sequenceIndex().second());
            SubList<vector> subBoundaryRhoU(bDataRhoU[patchI], nFaceDof, dgFaceI.sequenceIndex().second());
            SubList<scalar> subBoundaryEner(bDataEner[patchI], nFaceDof, dgFaceI.sequenceIndex().second());
            const labelList& mapping = const_cast<physicalFaceElement&>(dgFaceI).ownerDofMapping();
            for(int i=0; i<nFaceDof; i++){
            	scalar x = px[mapping[i]].x();
            	scalar y = px[mapping[i]].y();
            	position = 1.0/gTime*(x-1.0/6.0)-y;

            	if(position < 0){
			        subBoundaryRho[i] = 8.0;

					subBoundaryRhoU[i] = vector(8.25*std::cos(pi/6.0)*8.0, -8.25*std::sin(pi/6.0)*8.0, 0.0);

					subBoundaryEner[i] = 116.5/(gamma.value()-1.0) + (magSqr(subBoundaryRhoU[i]))/(2*subBoundaryRho[i]);
				}
				else{
			        subBoundaryRho[i] = 1.4;

					subBoundaryRhoU[i] = vector::zero;
					
					subBoundaryEner[i] = 1.0/(gamma.value() -1.0) + (magSqr(subBoundaryRhoU[i]))/(2*subBoundaryRho[i]);			
				}
            }
        }
    }
}

}

#endif