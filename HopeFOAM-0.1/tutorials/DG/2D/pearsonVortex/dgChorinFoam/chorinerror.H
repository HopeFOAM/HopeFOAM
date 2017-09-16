scalarList averageErr(1, 0.0);
scalarList maxErr(1, 0.0);
labelList pointNum(1, 0);
dgTree<physicalCellElement>& cellEleTree = mesh.cellElementsTree();
for(dgTree<physicalCellElement>::leafIterator iter = cellEleTree.leafBegin() ; iter != cellEleTree.end() ; ++iter){
	physicalCellElement& cellI = iter()->value();
	const List<point>& cellIDof = cellI.dofLocation();
	const label dofStart = cellI.dofStart();
	const label nDof = cellI.nDof();
	forAll(cellIDof, dofI){
		scalar pE = -std::cos(2*pi*cellIDof[dofI].x())
					*std::cos(2*pi*cellIDof[dofI].y())
					*paraT1;

		maxErr[0] = max(maxErr[0], fabs(p[dofStart + dofI] - pE));
		averageErr[0] += fabs(p[dofStart + dofI] - pE);
		pointNum[0] ++;
	}
}

Info<<"average error : "<<gSum(averageErr)/gSum(pointNum)<<endl;
Info<<"max error : "<<gMax(maxErr)<<endl;

