
//init field to sin(x)
//scalar pi = constant::mathematical::pi;
scalar lam = (0.5/nu.value()) - std::sqrt( (0.25/(sqr(nu.value()))) + 4*pi*pi );
dgTree<physicalCellElement>::iteratorEnd end = mesh.cellElementsTree().end();
int cellI = 0;
for(dgTree<physicalCellElement>::leafIterator iter = mesh.cellElementsTree().leafBegin(); iter != end; ++iter, ++cellI)
{
    const physicalCellElement& cellEle = iter()->value();
    label nDof = cellEle.nDof();
    label dofStart = cellEle.dofStart();
    const List<vector>& px = cellEle.dofLocation();
    
    SubField<vector> cellDataU(U.internalField(), nDof, dofStart);
    SubField<scalar> cellDataP(p.internalField(), nDof, dofStart);
    for(int pointI=0; pointI<nDof; pointI++){
        cellDataP[pointI] = -std::cos(2*pi*px[pointI].x()) * std::cos(2*pi*px[pointI].y());
        cellDataU[pointI] = vector(-std::sin(2*pi*px[pointI].y()), std::sin(2*pi*px[pointI].x()), 0.0);
    }
}

//set boundary value
const List<shared_ptr<dgTreeUnit<physicalFaceElement>>>& faceElement = mesh.faceElements();
forAll(p.boundaryFieldRef(), patchI){
    if(p.boundaryField()[patchI].patch().name() == "inlet"){
        inFlag = patchI;
        U.boundaryField()[patchI].patchInternalField(U.boundaryFieldRef()[patchI]);
    }
    else if(p.boundaryFieldRef()[patchI].patch().name() == "outlet"){
        outFlag = patchI;
        p.boundaryField()[patchI].patchInternalField(p.boundaryFieldRef()[patchI]);

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

            Field<vector>& gradient = U.boundaryFieldRef()[patchI].gradient();
            for(int pointI=0; pointI<nFaceDof; pointI++){
                gradient[shift + pointI] = vector(
                                                2*pi*faceNormal.y()*(-std::cos(2*pi*px[faceDofMapping[pointI]].y())),
                                                2*pi*faceNormal.x()*std::cos(2*pi*px[faceDofMapping[pointI]].x()),
                                                0
                                            );

            }
        }
    }
}
U.correctBoundaryConditions();
p.correctBoundaryConditions();
