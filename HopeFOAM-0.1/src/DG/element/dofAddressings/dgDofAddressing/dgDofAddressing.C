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
#include "dgDofAddressing.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::dgDofAddressing::dgDofAddressing(){
}

// * * * * * * * * * * * * * * * * Destructors  * * * * * * * * * * * * * * //

Foam::dgDofAddressing::~dgDofAddressing(){
}

// * * * * * * * * * * * * * * Member Functions * * * * * * * * * * * * * * //

void Foam::dgDofAddressing::initCellAddressing(const List<stdElement*> eleList)
{
	//- set cellDofIndex_
	cellDofIndex_.setSize(eleList.size());
	cellDofIndexPair_.setSize(eleList.size());
	label cellPtr =0;
	forAll(cellDofIndex_, cellI){
		cellDofIndex_[cellI].setSize(eleList[cellI]->nDofPerCell());
		cellDofIndexPair_[cellI][0] = cellPtr;
		cellDofIndexPair_[cellI][1] = eleList[cellI]->nDofPerCell();
		for(int i=0; i<eleList[cellI]->nDofPerCell(); i++, cellPtr++){
			cellDofIndex_[cellI][i] = cellPtr;
		}
	}
}
void Foam::dgDofAddressing::initFaceAddressing(const List<stdElement*> eleList, const List<dgFace>& dgFaceList)
{
	//- set ownerFaceDofIndex_ & neighborFaceDofIndex_
	ownerFaceDofIndex_.setSize(dgFaceList.size());
	neighborFaceDofIndex_.setSize(dgFaceList.size());
	forAll(ownerFaceDofIndex_, faceI){
		const dgFace& dgFaceI = dgFaceList[faceI];

		const labelList& faceIndex = (dgFaceI.ownerBase_)->faceToCellIndex();
		ownerFaceDofIndex_[faceI].setSize(dgFaceI.nOwnerDof_);
		for(int i=0, j=dgFaceI.nOwnerDof_*dgFaceI.faceOwnerLocalID_; i<dgFaceI.nOwnerDof_; i++, j++){
			ownerFaceDofIndex_[faceI][i] = cellDofIndex_[dgFaceI.faceOwner_][faceIndex[j]];
		}

		//neighbor face index
		if(dgFaceI.faceNeighbor_>=0){
			neighborFaceDofIndex_[faceI].setSize(dgFaceI.nNeighborDof_);
			const labelList& faceIndexN = (dgFaceI.neighborBase_)->faceToCellIndex();
			labelList tmpList(dgFaceI.nNeighborDof_);
			for(int i=0, j=dgFaceI.nNeighborDof_*dgFaceI.faceNeighborLocalID_; i<dgFaceI.nNeighborDof_; i++, j++){
				tmpList[i] = cellDofIndex_[dgFaceI.faceNeighbor_][faceIndexN[j]];
			}
			const labelList& neighborFaceRotate = ((dgFaceI.neighborBase_)->faceRotateIndex())[dgFaceI.faceRotate_];
			for(int i=0; i<dgFaceI.nNeighborDof_; i++){
				neighborFaceDofIndex_[faceI][i] = tmpList[neighborFaceRotate[i]];
			}
		}
		else{
			// boundary face
			neighborFaceDofIndex_[faceI].setSize(0);
		}
	}

	// * * * * * * * * * * * * * * gauss Index * * * * * * * * * * * * * //
	//- set gaussCellDofStart_
	gaussCellDofIndex_.setSize(eleList.size());
	if(eleList.size() > 0){
		gaussCellDofIndex_[0][0] = 0;
		gaussCellDofIndex_[0][1] = eleList[0]->gaussNDofPerCell();
	}
	for(int i=1; i<eleList.size(); i++){

		gaussCellDofIndex_[i][1] = eleList[i]->gaussNDofPerCell();
		gaussCellDofIndex_[i][0] = gaussCellDofIndex_[i-1][0] + gaussCellDofIndex_[i-1][1];

	}
	//- set gaussFaceDofStart_
	gaussFaceDofIndex_.setSize(dgFaceList.size());
	if(dgFaceList.size() >0){
		gaussFaceDofIndex_[0][0] = 0;
		gaussFaceDofIndex_[0][1] = dgFaceList[0].nGaussDof_;
	}
	for(int i=1; i<dgFaceList.size(); i++){
		gaussFaceDofIndex_[i][1] = dgFaceList[i].nGaussDof_;
		gaussFaceDofIndex_[i][0] = gaussFaceDofIndex_[i-1][0] + dgFaceList[i-1].nGaussDof_;
	}
}

void Foam::dgDofAddressing::updateAddressing(label cellI, const List<stdElement*> eleList, const List<dgFace>& dgFaceList)
{
	/*/ assume that dof addressing of cells with ID less than cellI are correct.
	label cellPtr;
	if(cellI == 0) cellPtr = 0;
	else
		cellPtr = cellDofIndex_[cellI-1][cellDofIndex_[cellI-1].size()-1] + 1;
	cellDofIndex_[cellI].setSize(eleList[cellI]->nDofPerCell());
	for(int i=0; i<eleList[cellI]->nDofPerCell(); i++, cellPtr++){
		cellDofIndex_[cellI][i] = cellPtr;
	}

	const labelList& faceIndex = eleList[cellI]->faceToCellIndex();
	faceDofIndex_[cellI].setSize(faceIndex.size());
	forAll(faceDofIndex_, pointI){
		faceDofIndex_[cellI][pointI] = cellDofIndex_[cellI][faceIndex[pointI]];
	}

	//- set neighborFaceDofIndex_
	const labelListList& neighborCell = mesh.dgFaceNeighbourID();
	const labelListList& neighborFace = mesh.neighbourFace();
	const labelListList& faceRotate = mesh.firstPointIndex();

	label nFace = eleList[cellI]->nFacesPerCell();
	neighborFaceDofIndex_[cellI].setSize(nFace);
	for(int faceI=0; faceI<nFace; faceI++){
		label neighborCellIndex = neighborCell[cellI][faceI];
		if(neighborCellIndex < 0) continue;
		label neighborFaceLocalIndex = neighborFace[cellI][faceI];
		label neighborNDofPerFace = eleList[neighborCellIndex]->nDofPerFace();
		const labelList& neighborFaceRotate = (eleList[neighborCellIndex]->faceRotateIndex())[faceRotate[cellI][faceI]];
		
		neighborFaceDofIndex_[cellI][faceI].setSize(neighborNDofPerFace);
		SubList<label> neighbourFaceDofList(faceDofIndex_[neighborCellIndex], neighborNDofPerFace, neighborNDofPerFace * neighborFaceLocalIndex);
		for(int i=0; i<neighborNDofPerFace; i++){
			neighborFaceDofIndex_[cellI][faceI][i] = neighbourFaceDofList[neighborFaceRotate[i]];
		}
	}*/

}