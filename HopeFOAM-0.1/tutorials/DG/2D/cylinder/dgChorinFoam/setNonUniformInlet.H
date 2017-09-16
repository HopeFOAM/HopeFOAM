
//init field to sin(x)
//scalar pi = constant::mathematical::pi;
//scalar lam = (0.5/nu.value()) - std::sqrt( (0.25/(sqr(nu.value()))) + 4*pi*pi );

//set boundary value
const List<shared_ptr<dgTreeUnit<physicalFaceElement>>>& faceElement = mesh.faceElements();
forAll(p.boundaryFieldRef(), patchI){
    if(p.boundaryFieldRef()[patchI].patch().name() == "inlet"){
        inFlag = patchI;

        const labelList bdFaceIndex = mesh.boundary()[patchI].dgFaceIndex();
        forAll(bdFaceIndex, faceI){
            label bdFaceI = bdFaceIndex[faceI];
            const physicalFaceElement& dgFaceI = faceElement[bdFaceI]->value();
            const vector faceNormal = dgFaceI.faceNx_[0];
            const labelList& faceDofMapping = const_cast<physicalFaceElement&>(dgFaceI).ownerDofMapping();
            const physicalCellElement& cellEle = dgFaceI.ownerEle_->value();
            const List<vector>& px = cellEle.dofLocation();
            label nFaceDof = cellEle.baseFunction().nDofPerFace();
            label shift = dgFaceI.sequenceIndex().second();

            Field<vector>& value = U.boundaryFieldRef()[patchI];
            for(int pointI=0; pointI<nFaceDof; pointI++){
                value[shift + pointI] = vector(std::pow(1/0.41, 2)*6*(px[faceDofMapping[pointI]].y()+0.2)*(0.21-px[faceDofMapping[pointI]].y()),
                                            0.0,
                                            0.0
                                        );
            }
        }
    }
}
U.correctBoundaryConditions();
p.correctBoundaryConditions();
