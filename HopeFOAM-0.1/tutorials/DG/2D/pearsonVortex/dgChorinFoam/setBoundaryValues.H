paraT1 = std::exp(-nu.value()*8*std::pow(pi,2)*runTime.value());
paraT2 = std::exp(-nu.value()*4*std::pow(pi,2)*runTime.value());

const labelList& outFaceIndex = p.boundaryFieldRef()[outFlag].patch().dgFaceIndex();
forAll(outFaceIndex, faceI){
	label bdFaceI = outFaceIndex[faceI];
	const physicalFaceElement& dgFaceI = faceElement[bdFaceI]->value();
	const vector faceNormal = dgFaceI.faceNx_[0];
	const labelList& faceDofMapping = const_cast<physicalFaceElement&>(dgFaceI).ownerDofMapping();
	const physicalCellElement& cellEle = dgFaceI.ownerEle_->value();
	const List<vector>& px = cellEle.dofLocation();
	label nFaceDof = cellEle.baseFunction().nDofPerFace();
	label shift = dgFaceI.sequenceIndex().second();

	Field<vector>& gradient = U.boundaryFieldRef()[outFlag].gradient();
	Field<scalar>& value = p.boundaryFieldRef()[outFlag];
	for(int pointI=0; pointI<nFaceDof; pointI++){
		gradient[shift + pointI] = vector(
										2*pi*faceNormal.y()*(-std::cos(2*pi*px[faceDofMapping[pointI]].y()))*paraT2,
										2*pi*faceNormal.x()*std::cos(2*pi*px[faceDofMapping[pointI]].x())*paraT2,
										0
									);
		value[shift + pointI] = -std::cos(2*pi*px[faceDofMapping[pointI]].x())
								*std::cos(2*pi*px[faceDofMapping[pointI]].y())
								*paraT1;
	}

}
const labelList& inFaceIndex = p.boundaryFieldRef()[inFlag].patch().dgFaceIndex();
forAll(inFaceIndex, faceI){
	label bdFaceI = inFaceIndex[faceI];
	const physicalFaceElement& dgFaceI = faceElement[bdFaceI]->value();
	const labelList& faceDofMapping = const_cast<physicalFaceElement&>(dgFaceI).ownerDofMapping();
	const physicalCellElement& cellEle = dgFaceI.ownerEle_->value();
	const List<vector>& px = cellEle.dofLocation();
	label nFaceDof = cellEle.baseFunction().nDofPerFace();
	label shift = dgFaceI.sequenceIndex().second();

	Field<vector>& value = U.boundaryFieldRef()[inFlag];
	for(int pointI=0; pointI<nFaceDof; pointI++){
		value[shift + pointI] = vector(
										-std::sin(2*pi*px[faceDofMapping[pointI]].y())*paraT2,
										std::sin(2*pi*px[faceDofMapping[pointI]].x())*paraT2,
										0
									);
	}

}

