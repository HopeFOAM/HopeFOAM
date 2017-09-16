paraT1 = pi/8*std::cos(pi*runTime.value()/8);
paraT2 = std::sin(pi*runTime.value()/8);

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
		value[shift + pointI] = vector(std::pow(1/0.41, 2)*6*(px[faceDofMapping[pointI]].y()+0.2)*(0.21-px[faceDofMapping[pointI]].y())*paraT2,
									0.0,
									0.0
								);
	}

}

