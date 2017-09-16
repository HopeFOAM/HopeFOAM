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

#include "physicalElementData.H"

using std::shared_ptr;
using std::make_shared;

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::physicalElementData::physicalElementData(const dgPolyMesh& mesh)
:
	mesh_(mesh),
	elementSets_(),
	cellElementsTree_(),
	faceElementsTree_()
{
	cellElements_.setSize(mesh.nCells());
	faceElements_.setSize(mesh.dgFaceSize());
}


// * * * * * * * * * * * * * * * * Destructors  * * * * * * * * * * * * * * //
Foam::physicalElementData::~physicalElementData()
{
	cellElements_.clear();
	faceElements_.clear();
}

// * * * * * * * * *  Member Functions (data access from base function, including linear integration matrix)  * * * * * * * * //

void Foam::physicalElementData::initElements(label order)
{

	const List<word>& cellShape = mesh_.cellShapeName();

	label start = 0;
	forAll(cellShape, cellI){
		shared_ptr<physicalCellElement> tmpCellEle = make_shared<physicalCellElement>();
		
		tmpCellEle->initElement(elementSets_.getElement(order, cellShape[cellI]), mesh_.dgPoints()[cellI]);
		cellElements_[cellI] = make_shared<dgTreeUnit<physicalCellElement>>(tmpCellEle);
		cellElements_[cellI]->value().updateGaussStart(start);
		cellElements_[cellI]->value().updateSequenceIndex(cellI);
		start += cellElements_[cellI]->value().baseFunction().gaussNDofPerCell();
	}

	const labelList& dgFaceOwner = mesh_.dgFaceOwner();
	const labelList& dgFaceNeighbour = mesh_.dgFaceNeighbour();
	const labelList& faceIndexInOwner = mesh_.faceIndexInOwner();
	const labelList& faceIndexInNeighbour = mesh_.faceIndexInNeighbour();
	const labelListList& faceRotate = mesh_.firstPointIndex();
	forAll(faceElements_, faceI){
		
		shared_ptr<physicalFaceElement> dgFaceI = make_shared<physicalFaceElement>();

		dgFaceI->ownerEle_ = cellElements_[dgFaceOwner[faceI]];
		dgFaceI->neighborEle_ = dgFaceNeighbour[faceI]>=0? (cellElements_[dgFaceNeighbour[faceI]]) : NULL;
		dgFaceI->faceOwnerLocalID_ = faceIndexInOwner[faceI];
		dgFaceI->faceNeighborLocalID_ = faceIndexInNeighbour[faceI];
		dgFaceI->faceRotate_ = faceRotate[dgFaceOwner[faceI]][faceIndexInOwner[faceI]];
		if(dgFaceI->neighborEle_){
			dgFaceI->isPatch_ = false;
			dgFaceI->gaussOrder_ = max(dgFaceI->ownerEle_->value().baseFunction().nOrder(), dgFaceI->neighborEle_->value().baseFunction().nOrder());
			dgFaceI->nGaussPoints_ = max((dgFaceI->ownerEle_)->value().baseFunction().gaussNDofPerFace(),(dgFaceI->neighborEle_)->value().baseFunction().gaussNDofPerFace());
		}
		else{
			dgFaceI->isPatch_ = true;
			dgFaceI->gaussOrder_ = dgFaceI->ownerEle_->value().baseFunction().nOrder();
			dgFaceI->nGaussPoints_ = (dgFaceI->ownerEle_)->value().baseFunction().gaussNDofPerFace();
		}

		dgFaceI->nGaussPoints_ = (dgFaceI->ownerEle_)->value().baseFunction().gaussNDofPerFace();

		faceElements_[faceI] = make_shared<dgTreeUnit<physicalFaceElement>>(dgFaceI);

		// set faceElement to cellElement
		dgFaceI->ownerEle_->value().setFaceElement(dgFaceI->faceOwnerLocalID_, dgFaceI);
		if(!dgFaceI->isPatch_){
			dgFaceI->neighborEle_->value().setFaceElement(dgFaceI->faceNeighborLocalID_, dgFaceI);
		}

	}

	cellElementsTree_.initial(cellElements_.size(), cellElements_);
	faceElementsTree_.initial(faceElements_.size(), faceElements_);
	updateDofMapping();
	initGaussFaceNx();

	maxDofPerCell_ = 0;
	maxGaussPerCell_ = 0;
	maxNFacesPerCell_ = 0;
	maxDofPerFace_  = 0;
	for(dgTree<physicalCellElement>::leafIterator iter = cellElementsTree_.leafBegin(); iter != cellElementsTree_.end(); ++iter){

		List<shared_ptr<physicalFaceElement>>& faceEleInCell = iter()->value().faceElementList();


		maxDofPerCell_ = max(maxDofPerCell_, iter()->value().nDof());
		maxGaussPerCell_ = max(maxGaussPerCell_, iter()->value().baseFunction().gaussNDofPerCell());
		maxNFacesPerCell_ = max(maxNFacesPerCell_, iter()->value().baseFunction().nFacesPerCell());
		//List<shared_ptr<physicalFaceElement>>& faceEleInCell = iter()->value().faceElementList();
		forAll(faceEleInCell, faceI){
			maxDofPerFace_ = max(maxDofPerFace_, faceEleInCell[faceI]->nGaussPoints_);
		}
	}
	
	initGaussFaceDn();

	totalDof_ = 0;
	totalGaussCellDof_ = 0;
	label total = 0;

	for(dgTree<physicalCellElement>::leafIterator iter = cellElementsTree_.leafBegin(); iter != cellElementsTree_.end(); ++iter){
		totalDof_ += iter()->value().nDof();
		totalGaussCellDof_ += iter()->value().baseFunction().gaussNDofPerCell();

		List<shared_ptr<physicalFaceElement>>& faceEleInCell = iter()->value().faceElementList();
		forAll(faceEleInCell, faceI){
			if(faceEleInCell[faceI]->ownerEle_ == iter()){
				faceEleInCell[faceI]->ownerFaceStart_ = total;
				

				//- If the face is patch face, it does not have neighbour cell, 
				// thus determine the start index in owner cell condition
				faceEleInCell[faceI]->neighbourFaceStart_ = total;
				total += faceEleInCell[faceI]->nGaussPoints_;
			}
		}
	}
	totalGaussFaceDof_ = total;
	
}

void Foam::physicalElementData::updatePatchDofIndexMapping(const dgBoundaryMesh& bMesh)
{
	Pair<label> index(0, 0);

	forAll(bMesh, patchI){
		//dgPatch& patch = bMesh[patchI];
		
		const labelList& dgFaceIndex = bMesh[patchI].dgFaceIndex();
		if(dgFaceIndex.size()<=0){
			++index[0];
			continue;
		}

		for(dgTree<physicalFaceElement>::leafIterator iter = faceElementsTree_.leafBegin(dgFaceIndex); iter != faceElementsTree_.end() ; ++iter){
			physicalFaceElement& faceI = iter()->value();
			faceI.updateSequenceIndex(index);
			index[1] += faceI.nOwnerDof();
			
		}
		index[0]++;
		index[1]=0;

		//- Init the dof location of points on curve boundary
		if(bMesh[patchI].curvedPatch()){
			const labelList& dgFaceIndex = bMesh[patchI].dgFaceIndex();
			const label totalDofNum = bMesh[patchI].totalDofNum();
			vectorList stdDofLoc(totalDofNum);
			List<bool> isEnd(totalDofNum);

			label numBase = 0;
			forAll(dgFaceIndex, faceI){
				physicalFaceElement& dgFaceI = faceElements_[dgFaceIndex[faceI]]->value();
				const label ownerDof = dgFaceI.nOwnerDof();
				const List<point>& px = (dgFaceI.ownerEle_)->value().dofLocation();

				const labelList& faceDofMapping = dgFaceI.ownerDofMapping();
				forAll(faceDofMapping, dofI){
					stdDofLoc[numBase] = px[faceDofMapping[dofI]];
					if(dofI==0 || dofI==ownerDof-1) isEnd[numBase] = true;
					else isEnd[numBase] = false;
					++numBase;
				}
			}

			vectorList tmpPatchShift = bMesh[patchI].positions(stdDofLoc, isEnd);

			forAll(tmpPatchShift, pointI){
				tmpPatchShift[pointI] -= stdDofLoc[pointI];
			}

			label stPos = 0;
			forAll(dgFaceIndex, faceI){
				physicalFaceElement& dgFaceI = faceElements_[dgFaceIndex[faceI]]->value();
				physicalCellElement& owner = dgFaceI.ownerEle_->value();
				label nDof = dgFaceI.nOwnerDof();
				SubList<vector> tmpFaceShift(tmpPatchShift, nDof, stPos);
				stPos += nDof;
				vectorList& dofLocation = const_cast<vectorList&>(owner.dofLocation());
				stdElement& ownerBase = const_cast<stdElement&>(owner.baseFunction());
				ownerBase.addFaceShiftToCell(tmpFaceShift, dgFaceI.faceOwnerLocalID_, dofLocation);
			}
		}
	}
}

void Foam::physicalElementData::initGaussFaceNx()
{
	List<tensor> dxdrFace;
	label faceLocalID;
	int cellI = 0;
	for(dgTree<physicalCellElement>::leafIterator iter = cellElementsTree_.leafBegin(); iter != cellElementsTree_.end(); ++iter, ++cellI){
		List<shared_ptr<physicalFaceElement>>& faceEleInCell = iter()->value().faceElementList();
		const List<tensor>& celldxdr = iter()->value().dxdr();

		forAll(faceEleInCell, faceI){
			if(faceEleInCell[faceI]->ownerEle_ != iter()) continue;
			//if(faceEleInCell[faceI]->gaussOrder_ == faceEleInCell[faceI]->ownerEle_->value().baseFunction().nOrder()){
				const labelList& ownerDofMap = faceEleInCell[faceI]->ownerDofMapping();
				dxdrFace.setSize(ownerDofMap.size());
				for(int i=0 ; i<ownerDofMap.size() ; i++){
					dxdrFace[i] = celldxdr[ownerDofMap[i]];
				}
				faceLocalID = faceEleInCell[faceI]->faceOwnerLocalID_;
				faceEleInCell[faceI]->ownerEle_->value().baseFunction().gaussFaceNxWJ(dxdrFace, faceLocalID, faceEleInCell[faceI]->faceNx_, faceEleInCell[faceI]->faceWJ_);
				faceEleInCell[faceI]->ownerEle_->value().baseFunction().faceFscale(dxdrFace, faceLocalID, faceEleInCell[faceI]->fscale_);
			//}
			if(!faceEleInCell[faceI]->isPatch_){
				const List<tensor>& ncelldxdr = faceEleInCell[faceI]->neighborEle_->value().dxdr();
				const labelList& neighborDofMap = faceEleInCell[faceI]->neighborDofMapping();
				dxdrFace.setSize(neighborDofMap.size());
				for(int i=0 ; i<neighborDofMap.size() ; i++){
					dxdrFace[i] = ncelldxdr[neighborDofMap[i]];
				}
				faceLocalID = faceEleInCell[faceI]->faceNeighborLocalID_;
				scalar tmpFscale;
				faceEleInCell[faceI]->neighborEle_->value().baseFunction().faceFscale(dxdrFace, faceLocalID, tmpFscale);
				faceEleInCell[faceI]->fscale_ = max(faceEleInCell[faceI]->fscale_, tmpFscale);
			}
		}
	}
}

void Foam::physicalElementData::initGaussFaceDn()
{
	List<tensor> drdxFace(this->maxDofPerFace());
	List<tensor> gaussDrdxFace(this->maxDofPerFace() * 3);
	denseMatrix<vector> tmpFaceDr(this->maxDofPerFace() * 3, this->maxDofPerCell() * 3);

	for(dgTree<physicalCellElement>::leafIterator iter = cellElementsTree_.leafBegin(); iter != cellElementsTree_.end(); ++iter){
		List<shared_ptr<physicalFaceElement>>& faceEleInCell = iter()->value().faceElementList();
		const List<tensor>& celldrdx = iter()->value().drdx();

		forAll(faceEleInCell, faceI){
			if(faceEleInCell[faceI]->ownerEle_ == iter()){
				const labelList& ownerDofMap = faceEleInCell[faceI]->ownerDofMapping();
				label nGauss = faceEleInCell[faceI]->nGaussPoints_;
				label nDof = iter()->value().baseFunction().nDofPerCell();
				label nFaceDof = faceEleInCell[faceI]->nOwnerDof();
				const denseMatrix<scalar>& faceVand = const_cast<stdElement&>(iter()->value().baseFunction()).gaussFaceInterpolateMatrix(faceEleInCell[faceI]->gaussOrder_, 0);
				for(int i=0 ; i < nFaceDof; i++){
					drdxFace[i] = celldrdx[ownerDofMap[i]];
				}
				SubList<tensor> subDrdxFace(drdxFace, nFaceDof);
				SubList<tensor> subGaussDrdxFace(gaussDrdxFace, nGauss);
				denseMatrix<tensor>::MatVecMult(faceVand, subDrdxFace, subGaussDrdxFace);

				denseMatrix<vector> faceDr = const_cast<stdElement&>(iter()->value().baseFunction()).drMatrix().subRows(ownerDofMap);
				tmpFaceDr.resetLabel(nGauss, nDof);
				denseMatrix<vector>::MatScalarMatMult(faceVand, faceDr, tmpFaceDr);

				vector tmp;
				faceEleInCell[faceI]->gaussOwnerFaceDn_.setSize(nGauss, nDof);
				for(int i=0, ptr=0; i < nGauss; i++){
					for(int j=0; j < nDof; j++, ptr++){
						tmp[0] = tmpFaceDr[ptr] & vector(subGaussDrdxFace[i][0], subGaussDrdxFace[i][1], subGaussDrdxFace[i][2]);
						tmp[1] = tmpFaceDr[ptr] & vector(subGaussDrdxFace[i][3], subGaussDrdxFace[i][4], subGaussDrdxFace[i][5]);
						tmp[2] = tmpFaceDr[ptr] & vector(subGaussDrdxFace[i][6], subGaussDrdxFace[i][7], subGaussDrdxFace[i][8]);

						faceEleInCell[faceI]->gaussOwnerFaceDn_[ptr] = faceEleInCell[faceI]->faceNx_[i] & tmp;
					}
				}

				if(!faceEleInCell[faceI]->isPatch_){
					const labelList& faceRotate = faceEleInCell[faceI]->neighborEle_->value().baseFunction().gaussFaceRotateIndex()[faceEleInCell[faceI]->faceRotate_];
					faceEleInCell[faceI]->gaussOwnerFaceDnRotate_ = faceEleInCell[faceI]->gaussOwnerFaceDn_.subRows(faceRotate);
				}
			}
			else{
				const labelList& ownerDofMap = faceEleInCell[faceI]->neighborNoRotateDofMapping();
				label nGauss = faceEleInCell[faceI]->nGaussPoints_;
				label nDof = iter()->value().baseFunction().nDofPerCell();
				label nFaceDof = faceEleInCell[faceI]->nNeighborDof();
				const denseMatrix<scalar>& faceVand = const_cast<stdElement&>(iter()->value().baseFunction()).gaussFaceInterpolateMatrix(faceEleInCell[faceI]->gaussOrder_, 0);
				for(int i=0 ; i < nFaceDof; i++){
					drdxFace[i] = celldrdx[ownerDofMap[i]];
				}
				SubList<tensor> subDrdxFace(drdxFace, nFaceDof);
				SubList<tensor> subGaussDrdxFace(gaussDrdxFace, nGauss);
				denseMatrix<tensor>::MatVecMult(faceVand, subDrdxFace, subGaussDrdxFace);

				denseMatrix<vector> faceDr = const_cast<stdElement&>(iter()->value().baseFunction()).drMatrix().subRows(ownerDofMap);
				tmpFaceDr.resetLabel(nGauss, nDof);
				denseMatrix<vector>::MatScalarMatMult(faceVand, faceDr, tmpFaceDr);

				vector tmp;
				faceEleInCell[faceI]->gaussNeighborFaceDn_.setSize(nGauss, nDof);
				for(int i=0, ptr=0; i < nGauss; i++){
					for(int j=0; j < nDof; j++, ptr++){
						tmp[0] = tmpFaceDr[ptr] & vector(subGaussDrdxFace[i][0], subGaussDrdxFace[i][1], subGaussDrdxFace[i][2]);
						tmp[1] = tmpFaceDr[ptr] & vector(subGaussDrdxFace[i][3], subGaussDrdxFace[i][4], subGaussDrdxFace[i][5]);
						tmp[2] = tmpFaceDr[ptr] & vector(subGaussDrdxFace[i][6], subGaussDrdxFace[i][7], subGaussDrdxFace[i][8]);

						faceEleInCell[faceI]->gaussNeighborFaceDn_[ptr] = -faceEleInCell[faceI]->faceNx_[i] & tmp;
					}
				}
				const labelList& faceRotate = faceEleInCell[faceI]->neighborEle_->value().baseFunction().gaussFaceRotateIndex()[faceEleInCell[faceI]->faceRotate_];
				faceEleInCell[faceI]->gaussNeighborFaceDnRotate_ = faceEleInCell[faceI]->gaussNeighborFaceDn_.subRows(faceRotate);
			}
		}
	}
}


Foam::label Foam::physicalElementData::updateTotalDof()
{
	totalDof_ = 0;
	for(dgTree<physicalCellElement>::leafIterator iter = cellElementsTree_.leafBegin(); iter != cellElementsTree_.end(); ++iter){
		totalDof_ += iter()->value().nDof();
	}

	return totalDof_;
}


Foam::label Foam::physicalElementData::updateTotalGaussCellDof()
{
	totalGaussCellDof_ = 0;
	for(dgTree<physicalCellElement>::leafIterator iter = cellElementsTree_.leafBegin(); iter != cellElementsTree_.end(); ++iter){
		totalGaussCellDof_ += iter()->value().baseFunction().gaussNDofPerCell();
	}
	return totalGaussCellDof_;
}


Foam::label Foam::physicalElementData::uptateTotalGaussFaceDof()
{
	label total = 0;
	for(dgTree<physicalCellElement>::leafIterator iter = cellElementsTree_.leafBegin(); iter != cellElementsTree_.end(); ++iter){
		List<shared_ptr<physicalFaceElement>>& faceEleInCell = iter()->value().faceElementList();
		forAll(faceEleInCell, faceI){
			if(faceEleInCell[faceI]->ownerEle_ == iter()){
				faceEleInCell[faceI]->ownerFaceStart_ = total;
				

				//- If the face is patch face, it does not have neighbour cell, 
				// thus determine the start index in owner cell condition
				faceEleInCell[faceI]->neighbourFaceStart_ = total;
				total += faceEleInCell[faceI]->nGaussPoints_;
			}
		}
	}
	totalGaussFaceDof_ = total;
	return total;
}

void Foam::physicalElementData::updateDofMapping()
{
	label start = 0;


	for(dgTree<physicalCellElement>::leafIterator iter = cellElementsTree_.leafBegin(); iter != cellElementsTree_.end(); ++iter){

		iter()->value().updateDofStart(start);
		start += iter()->value().nDof();

	}
	

}

// *********************************** interfaces for dgLduMatrix ************************************** //

Foam::label Foam::physicalElementData::updateMaxDofPerCell()
{
	maxDofPerCell_ = 0;
	for(dgTree<physicalCellElement>::leafIterator iter = cellElementsTree_.leafBegin(); iter != cellElementsTree_.end(); ++iter){
		maxDofPerCell_ = max(maxDofPerCell_, iter()->value().nDof());
	}
	return maxDofPerCell_;
}

Foam::label Foam::physicalElementData::updateMaxGaussPerCell()
{
	maxGaussPerCell_ = 0;
	for(dgTree<physicalCellElement>::leafIterator iter = cellElementsTree_.leafBegin(); iter != cellElementsTree_.end(); ++iter){
		maxGaussPerCell_ = max(maxGaussPerCell_, iter()->value().baseFunction().gaussNDofPerCell());
	}
	return maxGaussPerCell_;
}

Foam::label Foam::physicalElementData::updateMaxNFacesPerCell()
{
	maxNFacesPerCell_ = 0;
	for(dgTree<physicalCellElement>::leafIterator iter = cellElementsTree_.leafBegin(); iter != cellElementsTree_.end(); ++iter){
		maxNFacesPerCell_ = max(maxNFacesPerCell_, iter()->value().baseFunction().nFacesPerCell());
	}
	return maxNFacesPerCell_;
}

Foam::label Foam::physicalElementData::updateMaxDofPerFace()
{
	maxDofPerFace_  = 0;
	for(dgTree<physicalCellElement>::leafIterator iter = cellElementsTree_.leafBegin(); iter != cellElementsTree_.end(); ++iter){
		List<shared_ptr<physicalFaceElement>>& faceEleInCell = iter()->value().faceElementList();
		forAll(faceEleInCell, faceI){
			maxDofPerFace_ = max(maxDofPerFace_, faceEleInCell[faceI]->nGaussPoints_);
		}
	}

	return maxDofPerFace_;
}

// ************************************************************************* //
