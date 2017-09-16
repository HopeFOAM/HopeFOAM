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
	vector origin(5.0, 0, 0);
	scalar beta=5.0;
    vector timeLoc(time.value(), 0, 0);

	const dgMesh& mesh = rho.mesh();
	const List<shared_ptr<dgTreeUnit<physicalFaceElement>>>& faceElement = mesh.faceElements();
	const typename Foam::GeometricField<scalar, dgPatchField, dgGeoMesh>::Boundary& bDataRho =  rho.boundaryField();
	const typename Foam::GeometricField<vector, dgPatchField, dgGeoMesh>::Boundary& bDataRhoU = rhoU.boundaryField();
	const typename Foam::GeometricField<scalar, dgPatchField, dgGeoMesh>::Boundary& bDataEner = Ener.boundaryField();
	label nFaceDof;
	forAll(bDataRho, patchI){
        const labelList bdFaceIndex = mesh.boundary()[patchI].dgFaceIndex();
        if(!bDataRho[patchI].fixesValue()) continue;
        forAll(bdFaceIndex, faceI){
            label bdFaceI = bdFaceIndex[faceI];
            const physicalFaceElement& dgFaceI = faceElement[bdFaceI]->value();
            nFaceDof = dgFaceI.ownerEle_->value().baseFunction().nDofPerFace();
            const List<vector>& px = dgFaceI.ownerEle_->value().dofLocation();
            SubList<scalar> subBoundaryRho (bDataRho [patchI], nFaceDof, dgFaceI.sequenceIndex().second());
            SubList<vector> subBoundaryRhoU(bDataRhoU[patchI], nFaceDof, dgFaceI.sequenceIndex().second());
            SubList<scalar> subBoundaryEner(bDataEner[patchI], nFaceDof, dgFaceI.sequenceIndex().second());
            const labelList& mapping = const_cast<physicalFaceElement&>(dgFaceI).ownerDofMapping();
            for(int pointI=0; pointI<nFaceDof; pointI++){
            	scalar r = magSqr(px[mapping[pointI]] - origin - timeLoc);
		        subBoundaryRho[pointI] = std::pow(1.0-(gamma.value()-1.0)*(beta*beta)*std::exp(2.0*(1.0-r))/(16.0*gamma.value()*pi*pi), 1.0/(gamma.value()-1.0));

		        subBoundaryRhoU[pointI] = vector(1-beta*std::exp(1-r)*(px[mapping[pointI]].y()-0)/(2.0*pi),
		                                beta*std::exp(1-r)*(px[mapping[pointI]].x()-5 - time.value())/(2.0*pi), 0.0) * subBoundaryRho[pointI];

		        subBoundaryEner[pointI] = std::pow(subBoundaryRho[pointI], gamma.value())/(gamma.value()-1.0)+0.5*magSqr(subBoundaryRhoU[pointI])/subBoundaryRho[pointI];
            }
        }
    }
}

}

#endif