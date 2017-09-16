
nbpOld = nbp;

typename Foam::GeometricField<Foam::vector, Foam::dgPatchField, Foam::dgGeoMesh>::Boundary& ubData = U.boundaryFieldRef();
Field<vector> tmpCellData(mesh.maxDofPerCell());
Field<scalar> tmpcurlData(mesh.maxDofPerCell());
const List<shared_ptr<dgTreeUnit<physicalFaceElement>>>& faceElement = mesh.faceElements();

forAll(ubData, patchI){
    if(!ubData[patchI].fixesValue())
        continue;
    const labelList bdFaceIndex = mesh.boundary()[patchI].dgFaceIndex();
    const stdElement* faceEle;
    forAll(bdFaceIndex, faceI){
        label bdFaceI = bdFaceIndex[faceI];
        const physicalFaceElement& dgFaceI = faceElement[bdFaceI]->value();
        const physicalCellElement& dgCellI = dgFaceI.ownerEle_->value();
        faceEle = &(dgCellI.baseFunction());
        const denseMatrix<vector>& Dr = const_cast<stdElement*>(faceEle)->drMatrix();
        const List<tensor>& drdx = dgCellI.drdx();

        //- curl
        int size = faceEle->nDofPerCell();
        SubField<vector> data_UOld(U.oldTime().primitiveField(), size, dgCellI.dofStart());
        SubField<vector> data_U(U.primitiveField(), size, dgCellI.dofStart());
        SubField<vector> data_Convection(convecU.primitiveField(), size, dgCellI.dofStart());
        SubField<vector> data_curlU(curlU.primitiveField(), size, dgCellI.dofStart());
        for(int i=0; i<size; i++){
            tmpcurlData[i] = 0;
            tmpCellData[i] = vector::zero;
        }
        for(int i=0, ptr=0; i<size; i++){
            for(int j=0; j<size; j++, ptr++){
                tmpcurlData[i] += vector(
                        -Dr[ptr] & vector(drdx[i][3], drdx[i][4], drdx[i][5]),
                        Dr[ptr] & vector(drdx[i][0], drdx[i][1], drdx[i][2]), 
                        0)
                    &(data_UOld[j]);
            }
        }
        for(int i=0, ptr=0; i<size; i++){
            for(int j=0; j<size; j++, ptr++){
                tmpCellData[i].x() += (Dr[ptr] & vector(drdx[i][3], drdx[i][4], drdx[i][5])) * tmpcurlData[j];
                tmpCellData[i].y() -= (Dr[ptr] & vector(drdx[i][0], drdx[i][1], drdx[i][2])) * tmpcurlData[j];
            }
        }

        const labelList& dofMapping = const_cast<physicalFaceElement&>(dgFaceI).ownerDofMapping();
        forAll(dofMapping, dofI){

            data_curlU[dofMapping[dofI]] = data_Convection[dofMapping[dofI]] + nu.value()*tmpCellData[dofMapping[dofI]];
        }
    }
    curlU.boundaryField()[patchI].patchInternalField(curlU.boundaryFieldRef()[patchI]);
    
}

shared_ptr<dg::Equation<scalar>> pEqnCorrect = make_shared<EquationPCorrect<scalar>>(U, curlU, nbp, nbpOld, b0, b1, paraT1);
