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
    Ye Shuai (yeshuai09@163.com)

\*---------------------------------------------------------------------------*/

#include "Trianglelimite.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
namespace Foam
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace dg
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

void Trianglelimite::limite
( 
	dgScalarField&,
	dgVectorField&,
	dgScalarField& ,
	const dgGaussScalarField&,
	const dgGaussVectorField&,
	const dgGaussScalarField&,
	dgScalarField&
)const
{

}



void Trianglelimite::limite
( 
	dgScalarField& rho ,
	dgVectorField& rhoU,
	dgScalarField& Ener,
	const dgGaussScalarField& gaussRho,
	const dgGaussVectorField& gaussRhoU,
	const dgGaussScalarField& gaussEner
)const
{

	dgScalarField rhou(rhoU.component(0));
	dgScalarField rhov(rhoU.component(1));
    scalar ther_gamma=1.4;
	const dgMesh& mesh=rho.mesh();
	
	//const vectorListList& faceNx = const_cast<dgMesh&>(mesh).gaussFaceNx();
	

	//1.calculate the  average value of each cell
	label nElement = mesh.cellElementsTree().leafSize();
	//scalar Z = (mesh.points()[mesh.points().size()-1].z() != 0)?mesh.points()[mesh.points().size()-1].z():(mesh.points()[0].z());
	
	label boundaryFaceSize =0;

	forAll(rho.boundaryField(), patchI){
		const dgPatchField<scalar>& tmprho = rho.boundaryField()[patchI];
		if(tmprho.size() <= 0) continue;
		boundaryFaceSize += mesh.boundary()[patchI].size();
	}

	label totalCellSize = boundaryFaceSize + mesh.cellElementsTree().leafSize();  //Constructs a virtual triangular element on the boundary

	scalarList AVEPerCell(totalCellSize,0.0);
	scalarList AVEPerCell1(totalCellSize,0.0);
	scalarList AVEPerCell2(totalCellSize,0.0);
	scalarList AVEPerCell3(totalCellSize,0.0);	
	scalarList cellcenterX(totalCellSize,0.0);
    scalarList cellcenterY(totalCellSize,0.0);

    label cellLabel=0;

    dgTree<physicalCellElement>& cellPhysicalEleTree = const_cast<dgMesh&>(mesh).cellElementsTree();
    scalarList A0(nElement, 0.0);

	scalarList matrixPerPoint(mesh.maxDofPerCell(), 0.0);

    for(dgTree<physicalCellElement>::leafIterator iter = cellPhysicalEleTree.leafBegin(); iter != cellPhysicalEleTree.end(); ++iter){
    	iter()->value().updateSequenceIndex(cellLabel);
    	physicalCellElement& cellPhyEle = iter()->value();
    	const List<vector>& cellDofLoc = cellPhyEle.dofLocation();
		label start = cellPhyEle.dofStart(), pointNum = cellDofLoc.size();
    	//scalarList matrixPerPoint(pointNum, 0.0);

    	const denseMatrix<scalar>& Mmatrix = const_cast<stdElement&>(cellPhyEle.baseFunction()).massMatrix();
		//matrixPerPoint = 0;
		for(int j=0 ; j<pointNum ; j++) {
    		matrixPerPoint[j] = 0;
    	}
    	for(int i=0, k=0; i<pointNum; i++){
    		for(int j=0 ; j<pointNum ; j++, k++){
    			matrixPerPoint[j] += Mmatrix[k]/2;
    		}
    	}
    	const List<tensor>& tdxdr = iter()->value().dxdr();
    	const scalarList& J = iter()->value().baseFunction().jacobian(tdxdr); 
    	for(int i=0 ; i<pointNum; i++){
    		A0[cellLabel] += matrixPerPoint[i] * J[i] * 2/3;
    	}

    	for(int i = start, j=0; i<start+pointNum ; i++, j++){
    		AVEPerCell[cellLabel] += rho[i] * matrixPerPoint[j];
    		AVEPerCell1[cellLabel] += rhou[i] * matrixPerPoint[j];
    		AVEPerCell2[cellLabel] += rhov[i] * matrixPerPoint[j];
    		AVEPerCell3[cellLabel] += Ener[i] * matrixPerPoint[j];
    		cellcenterX[cellLabel] += cellDofLoc[j].x() * matrixPerPoint[j];
    		cellcenterY[cellLabel] += cellDofLoc[j].y() * matrixPerPoint[j];
    	}

    	cellLabel++;

    }

	//PstreamBuffers pBufs(Pstream::defaultCommsType);
	dgTree<physicalFaceElement>& facePhysicalEleTree = const_cast<dgMesh&>(mesh).faceElementsTree();
	
	label neighborCellIndex = nElement;
	labelListList dofFaceInBoundary(mesh.boundary().size());

	Pair<label> boundaryFaceIndex(0, 0);

	forAll(mesh.boundary(), patchI){

		const dgPatch& patch = mesh.boundary()[patchI];
		const dgPatchField<scalar>& tPatchf = rho.boundaryField()[patchI];
		dofFaceInBoundary[patchI].setSize(tPatchf.size());

		if(tPatchf.size()<=0){
			boundaryFaceIndex[0]++;
			continue;
		}

		const dgPatchField<scalar>& tPatchf1 = rhou.boundaryField()[patchI];
		const dgPatchField<scalar>& tPatchf2 = rhov.boundaryField()[patchI];
		const dgPatchField<scalar>& tPatchf3 = Ener.boundaryField()[patchI];

		const labelList& dgFaceIndex = patch.dgFaceIndex();

		if(tPatchf.coupled()){
			//For parallel
		}

		if(tPatchf.reflective()){
			for(dgTree<physicalFaceElement>::leafIterator iter = facePhysicalEleTree.leafBegin(dgFaceIndex); iter!=facePhysicalEleTree.end(); ++iter){
				physicalFaceElement& facePhyEle = iter()->value();
				facePhyEle.updateSequenceIndex(boundaryFaceIndex);

				label ownerIndex = facePhyEle.ownerEle_->value().sequenceIndex();
				vector normal = facePhyEle.faceNx_[0];
				scalar A = normal.x();
				scalar B = normal.y();
				vector tmpPoint = facePhyEle.ownerEle_->value().dofLocation()[facePhyEle.ownerDofMapping()[0]];
				scalar C = -tmpPoint.x()*A - tmpPoint.y()*B;

				cellcenterX[neighborCellIndex] = (B*B-A*A)*cellcenterX[ownerIndex]-2*A*B*cellcenterY[ownerIndex]-2*A*C;
				cellcenterY[neighborCellIndex] = (-B*B+A*A)*cellcenterY[ownerIndex]-2*A*B*cellcenterX[ownerIndex]-2*B*C;
				AVEPerCell[neighborCellIndex] = AVEPerCell[ownerIndex]    ;
	            AVEPerCell1[neighborCellIndex] = AVEPerCell1[ownerIndex] 
									            -A*(A*AVEPerCell1[ownerIndex]
										        +B*AVEPerCell2[ownerIndex])  ;
	            AVEPerCell2[neighborCellIndex] = AVEPerCell2[ownerIndex] 
									            -B*(A*AVEPerCell1[ownerIndex]
									            +B*AVEPerCell2[ownerIndex])  ;
	            AVEPerCell3[neighborCellIndex] = AVEPerCell3[ownerIndex]   ;

	            for(int i=0 ; i<facePhyEle.nOwnerDof(); i++){
	            	dofFaceInBoundary[patchI][boundaryFaceIndex[1]] = neighborCellIndex;
	            	boundaryFaceIndex[1] ++;
	            }
	            neighborCellIndex++; 
	            
			}
			boundaryFaceIndex[0]++;
			boundaryFaceIndex[1] = 0;
		}
		else if(tPatchf.fixesValue()){
			for(dgTree<physicalFaceElement>::leafIterator iter = facePhysicalEleTree.leafBegin(dgFaceIndex); iter!=facePhysicalEleTree.end(); ++iter){
				physicalFaceElement& facePhyEle = iter()->value();

				facePhyEle.updateSequenceIndex(boundaryFaceIndex);
				label ownerIndex = facePhyEle.ownerEle_->value().sequenceIndex();
				vector normal = facePhyEle.faceNx_[0];
				scalar A = normal.x();
				scalar B = normal.y();
				vector tmpPoint = facePhyEle.ownerEle_->value().dofLocation()[facePhyEle.ownerDofMapping()[0]];
				scalar C = -tmpPoint.x()*A - tmpPoint.y()*B;

				cellcenterX[neighborCellIndex] = (B*B-A*A)*cellcenterX[ownerIndex]-2*A*B*cellcenterY[ownerIndex]-2*A*C;
				cellcenterY[neighborCellIndex] = (-B*B+A*A)*cellcenterY[ownerIndex]-2*A*B*cellcenterX[ownerIndex]-2*B*C;		
				AVEPerCell[neighborCellIndex] = tPatchf[0];
	            AVEPerCell3[neighborCellIndex] = tPatchf3[0] ;
				AVEPerCell1[neighborCellIndex] = tPatchf1[0];
	            AVEPerCell2[neighborCellIndex] = tPatchf2[0];

	            for(int i=0 ; i<facePhyEle.nOwnerDof(); i++){
	            	dofFaceInBoundary[patchI][boundaryFaceIndex[1]] = neighborCellIndex;
	            	boundaryFaceIndex[1] ++;
	            }
	            neighborCellIndex++; 
			}
			boundaryFaceIndex[0]++;
			boundaryFaceIndex[1] = 0;
		}
		else{
			for(dgTree<physicalFaceElement>::leafIterator iter = facePhysicalEleTree.leafBegin(dgFaceIndex); iter!=facePhysicalEleTree.end(); ++iter){
				physicalFaceElement& facePhyEle = iter()->value();
				facePhyEle.updateSequenceIndex(boundaryFaceIndex);
				label ownerIndex = facePhyEle.ownerEle_->value().sequenceIndex();
				vector normal = facePhyEle.faceNx_[0];
				scalar A = normal.x();
				scalar B = normal.y();
				vector tmpPoint = facePhyEle.ownerEle_->value().dofLocation()[facePhyEle.ownerDofMapping()[0]];
				scalar C = -tmpPoint.x()*A - tmpPoint.y()*B;

				cellcenterX[neighborCellIndex] = (B*B-A*A)*cellcenterX[ownerIndex]-2*A*B*cellcenterY[ownerIndex]-2*A*C;
				cellcenterY[neighborCellIndex] = (-B*B+A*A)*cellcenterY[ownerIndex]-2*A*B*cellcenterX[ownerIndex]-2*B*C;		
				AVEPerCell[neighborCellIndex] = AVEPerCell[ownerIndex] ;
	            AVEPerCell1[neighborCellIndex] = AVEPerCell1[ownerIndex] ;
										
	            AVEPerCell2[neighborCellIndex] = AVEPerCell2[ownerIndex] ;
										
	            AVEPerCell3[neighborCellIndex] = AVEPerCell3[ownerIndex] ;  

	            for(int i=0 ; i<facePhyEle.nOwnerDof(); i++){
	            	dofFaceInBoundary[patchI][boundaryFaceIndex[1]] = neighborCellIndex;
	            	boundaryFaceIndex[1] ++;
	            }
	            neighborCellIndex++; 
			}
			boundaryFaceIndex[0]++;
			boundaryFaceIndex[1] = 0;
		}
	}
	
	//pBufs.finishedSends();
	/*  
	forAll(mesh.boundary(), patchI){
		
		const dgPatch& patch= mesh.boundary()[patchI];
		const Foam::dgPatchField<scalar>& tPatchf = rho.boundaryField()[patchI];
		if(tPatchf.size()<=0) continue;
		
		const labelList& dgFaceIndex = patch.dgFaceIndex();
		const labelListList& dgPointLabel = patch.facesPoints();
		if (tPatchf.coupled()){
			scalarList cellreceive(6*dgFaceIndex.size(),0.0); 
			patch.receivePstream(cellreceive,pBufs);			 
			forAll(dgFaceIndex,faceI){			
				label faceLabel = dgFaceIndex[faceI];
				label offset = neighborlist[faceLabel];
				//label ownerLabel = dgFaceOwnerList[faceLabel];
				cellcenterX[offset] = cellreceive[faceI*6 + 0];
				cellcenterY[offset] = cellreceive[faceI*6 + 1];		
				AVEPerCell[offset]  = cellreceive[faceI*6 + 2];	
				AVEPerCell1[offset] = cellreceive[faceI*6 + 3];
				AVEPerCell2[offset] = cellreceive[faceI*6 + 4];		
				AVEPerCell3[offset] = cellreceive[faceI*6 + 5];				
			}
			
		}		 
		 
	}
	*/
	//boundary face cell ghost cell
	//2.calculate the average primitive value of each cell
	scalarList AVEPerCellU(totalCellSize,0.0);
	scalarList AVEPerCellV(totalCellSize,0.0);
	scalarList AVEPerCellP(totalCellSize,0.0);

	forAll(AVEPerCellU,cellI)
	{
		AVEPerCellU[cellI] =   AVEPerCell1[cellI]/ AVEPerCell[cellI];
	    AVEPerCellV[cellI] =   AVEPerCell2[cellI]/ AVEPerCell[cellI];
	    AVEPerCellP[cellI] =  (ther_gamma-1)*( AVEPerCell3[cellI]-0.5*(std::pow(AVEPerCell1[cellI],2)+std::pow(AVEPerCell2[cellI],2))/AVEPerCell[cellI]);

	}

	//3.calculate the average ending point value of each face
	label size = 0;

	for(dgTree<physicalCellElement>::leafIterator iter = cellPhysicalEleTree.leafBegin(); iter != cellPhysicalEleTree.end(); ++iter){
		const List<shared_ptr<physicalFaceElement>>& faceInCell = iter()->value().faceElementList();
		forAll(faceInCell, faceI){
			if(iter() == faceInCell[faceI]->ownerEle_) ++size;
		}
    }

	scalarList AVEPerFace0(size,0.0);
	scalarList AVEPerFace1(size,0.0);

	scalarList AVEPerFace10(size,0.0);
	scalarList AVEPerFace11(size,0.0);

	scalarList AVEPerFace20(size,0.0);
	scalarList AVEPerFace21(size,0.0);

	scalarList AVEPerFace30(size,0.0);
	scalarList AVEPerFace31(size,0.0);

	vectorList endingPoint0(size,vector::zero);
	vectorList endingPoint1(size,vector::zero);
	  
	scalarList A(size,0.0);
	scalarList A_2(size,0.0);
	  
	  //coupled patch need to be processed
	label faceIndex = 0;
	label neighbourLabel = 0;


	for(dgTree<physicalCellElement>::leafIterator iter = cellPhysicalEleTree.leafBegin(); iter != cellPhysicalEleTree.end(); ++iter){
		physicalCellElement& cellEle = iter()->value();
		List<shared_ptr<physicalFaceElement>>& facePtr = cellEle.faceElementList();
		label start = cellEle.dofStart();
		forAll(facePtr, faceI){
			if(facePtr[faceI]->ownerEle_ == iter()){
				label fdof = facePtr[faceI]->nOwnerDof();

				label ownerLabelS = facePtr[faceI]->ownerDofMapping()[0] + start;
				label ownerLabelE = facePtr[faceI]->ownerDofMapping()[fdof-1] + start;

				//scalar rho_OI_S = rho[ownerLabelS];
				//scalar rho_OI_E = rho[ownerLabelE];
				scalar rho_NI_S, rho_NI_E;
				scalar rhou_NI_S, rhou_NI_E;
				scalar rhov_NI_S, rhov_NI_E;
				scalar Ener_NI_S, Ener_NI_E;
				
				if(!facePtr[faceI]->isPatch_){
					label nfdof = facePtr[faceI]->nNeighborDof();
					label neighborStart = facePtr[faceI]->neighborEle_->value().dofStart();
					label neighborLabelS = facePtr[faceI]->neighborDofMapping()[0] + neighborStart;
					label neighborLabelE = facePtr[faceI]->neighborDofMapping()[nfdof-1] + neighborStart;

					rho_NI_S = rho[neighborLabelS];
					rho_NI_E = rho[neighborLabelE];

					rhou_NI_S = rhou[neighborLabelS];
					rhou_NI_E = rhou[neighborLabelE];

					rhov_NI_S = rhov[neighborLabelS];
					rhov_NI_E = rhov[neighborLabelE];

					Ener_NI_S = Ener[neighborLabelS];
					Ener_NI_E = Ener[neighborLabelE];					
				}
				else{
					Pair<label> faceBoundaryIndex = facePtr[faceI]->sequenceIndex();
					//label patchIndex = facePtr[faceI]->neighborDofMapping()[0];

					rho_NI_S = rho.boundaryField()[faceBoundaryIndex[0]][faceBoundaryIndex[1]];
					rho_NI_E = rho.boundaryField()[faceBoundaryIndex[0]][faceBoundaryIndex[1] + fdof-1];

					rhou_NI_S = rhou.boundaryField()[faceBoundaryIndex[0]][faceBoundaryIndex[1]];
					rhou_NI_E = rhou.boundaryField()[faceBoundaryIndex[0]][faceBoundaryIndex[1] + fdof-1];

					rhov_NI_S = rhov.boundaryField()[faceBoundaryIndex[0]][faceBoundaryIndex[1]];
					rhov_NI_E = rhov.boundaryField()[faceBoundaryIndex[0]][faceBoundaryIndex[1] + fdof-1];

					Ener_NI_S = Ener.boundaryField()[faceBoundaryIndex[0]][faceBoundaryIndex[1]];
					Ener_NI_E = Ener.boundaryField()[faceBoundaryIndex[0]][faceBoundaryIndex[1] + fdof-1];
				}

				AVEPerFace0[faceIndex] = rho[ownerLabelS]*0.5 + rho_NI_S*0.5;
				AVEPerFace1[faceIndex] = rho[ownerLabelE]*0.5 + rho_NI_E*0.5;
				AVEPerFace10[faceIndex] = rhou[ownerLabelS]*0.5 + rhou_NI_S*0.5;
				AVEPerFace11[faceIndex] = rhou[ownerLabelE]*0.5 + rhou_NI_E*0.5;
				AVEPerFace20[faceIndex] = rhov[ownerLabelS]*0.5 + rhov_NI_S*0.5;
				AVEPerFace21[faceIndex] = rhov[ownerLabelE]*0.5 + rhov_NI_E*0.5;
				AVEPerFace30[faceIndex] = Ener[ownerLabelS]*0.5 + Ener_NI_S*0.5;
				AVEPerFace31[faceIndex] = Ener[ownerLabelE]*0.5 + Ener_NI_E*0.5; 

				endingPoint0[faceIndex] = facePtr[faceI]->ownerEle_->value().dofLocation()[ownerLabelS-start];
				endingPoint1[faceIndex] = facePtr[faceI]->ownerEle_->value().dofLocation()[ownerLabelE-start];

				// caculate the area of the four points  // |x1y2+x2y3+x3y1-x1y3-x2y1-x3y2|*0.5
      			//caculate A = (X1c-X0c)(Y2v-Y1v)-(X2v-X1v)(Y1c-Y0c)
      			
      			label ownerLabel = facePtr[faceI]->ownerEle_->value().sequenceIndex();
      			label neighbourLabel = 0;
      			if(!facePtr[faceI]->isPatch_){
      				neighbourLabel = facePtr[faceI]->neighborEle_->value().sequenceIndex();
      				A_2[faceIndex] = A0[ownerLabel] +A0[neighbourLabel];
      			}
      			else{
      				Pair<label> faceBoundaryIndex = facePtr[faceI]->sequenceIndex();
      				neighbourLabel = dofFaceInBoundary[faceBoundaryIndex[0]][faceBoundaryIndex[1]];
      				A_2[faceIndex] = A0[ownerLabel] +A0[ownerLabel];
      			}
      			
				scalar x1v=endingPoint0[faceIndex].x();
		      	scalar y1v=endingPoint0[faceIndex].y();
		      	scalar x0c=cellcenterX[ownerLabel];
		      	scalar y0c=cellcenterY[ownerLabel];
		      	scalar x2v=endingPoint1[faceIndex].x();
		      	scalar y2v=endingPoint1[faceIndex].y();
			  
		      	scalar x1c=cellcenterX[neighbourLabel];
		      	scalar y1c=cellcenterY[neighbourLabel];

		      	A[faceIndex] =((x1c-x0c)*(y2v-y1v)-(x2v-x1v)*(y1c-y0c))*0.5;
			 
				//calculate primitive variables	 
		   		AVEPerFace10[faceIndex]=AVEPerFace10[faceIndex]/AVEPerFace0[faceIndex];
		   		AVEPerFace11[faceIndex]=AVEPerFace11[faceIndex]/AVEPerFace1[faceIndex];
		   		AVEPerFace20[faceIndex]=AVEPerFace20[faceIndex]/AVEPerFace0[faceIndex];
		   		AVEPerFace21[faceIndex]=AVEPerFace21[faceIndex]/AVEPerFace1[faceIndex];
		   		AVEPerFace30[faceIndex]=(ther_gamma-1)*( 
										AVEPerFace30[faceIndex]
										-0.5*AVEPerFace0[faceIndex]*(
										std::pow(AVEPerFace10[faceIndex],2)
										+std::pow(AVEPerFace20[faceIndex],2)));
		   		AVEPerFace31[faceIndex]=(ther_gamma-1)*( 
										AVEPerFace31[faceIndex]
										-0.5*AVEPerFace1[faceIndex]*(
										std::pow(AVEPerFace11[faceIndex],2)
										+std::pow(AVEPerFace21[faceIndex],2)));
		   		faceIndex++;
			}
		}
    }
	

 /* 
  	forAll(mesh.boundary(), patchI){
       	const dgPatch& patch= mesh.boundary()[patchI];
		const Foam::dgPatchField<scalar>& tPatchf = rho.boundaryField()[patchI];
		if(tPatchf.size()<=0)continue;
		
		const labelList& dgFaceIndex = patch.dgFaceIndex();

		if (tPatchf.coupled()){
			scalarList A2send(dgFaceIndex.size(),0.0); 			 
				forAll(dgFaceIndex,faceI){			
				label faceLabel = dgFaceIndex[faceI];
				//label offset = neighborlist[faceLabel];
				label ownerLabel = dgFaceOwnerList[faceLabel];
				A2send[faceI] = A0[ownerLabel];
			}
			patch.sendPstream(A2send,pBufs);
		}		 
            
 	}
	    
	pBufs.finishedSends();
    forAll(mesh.boundary(), patchI){
        const dgPatch& patch = mesh.boundary()[patchI];
        const Foam::dgPatchField<scalar>& tPatchf = rho.boundaryField()[patchI];
		const labelList& dgFaceIndex = patch.dgFaceIndex();
					
        if(tPatchf.coupled()){
            scalarList A2receive(dgFaceIndex.size(),0.0);
            patch.receivePstream(A2receive,pBufs);
            forAll(dgFaceIndex,faceI){
				label faceLabel = dgFaceIndex[faceI];
				label ownerLabel = dgFaceOwnerList[faceLabel];
                A_2[faceLabel] = A2receive[faceI] + A0[ownerLabel];
            }
		}

    }
 */
	//4.calculate the gradient of each face
  	scalarList Vx(size,0.0);
  	scalarList Vy(size,0.0);
  	scalarList Vx1(size,0.0);
  	scalarList Vy1(size,0.0);
  	scalarList Vx2(size,0.0);
  	scalarList Vy2(size,0.0);
  	scalarList Vx3(size,0.0);
  	scalarList Vy3(size,0.0);
  	scalarList CellA(totalCellSize,0.0);
  	scalarList CellA2(totalCellSize,0.0);

  	label faceI = 0;
	scalar temp1;
	scalar temp2;
  	for(dgTree<physicalCellElement>::leafIterator iter = cellPhysicalEleTree.leafBegin(); iter != cellPhysicalEleTree.end(); ++iter){
		List<shared_ptr<physicalFaceElement>>& facePtr = iter()->value().faceElementList();
		forAll(facePtr, faceJ){
			if(facePtr[faceJ]->ownerEle_ != iter()) continue;
			label ownerLabel = facePtr[faceJ]->ownerEle_->value().sequenceIndex();
			label neighbourLabel = 0;
			if(facePtr[faceJ]->isPatch_){
				Pair<label> faceBoundaryIndex = facePtr[faceJ]->sequenceIndex();
				neighbourLabel = dofFaceInBoundary[faceBoundaryIndex[0]][faceBoundaryIndex[1]]; 
			}
			else{
				neighbourLabel = facePtr[faceJ]->neighborEle_->value().sequenceIndex();
			}
		
			temp1 = (endingPoint1[faceI].x()-endingPoint0[faceI].x());
			temp2 = cellcenterX[neighbourLabel]-cellcenterX[ownerLabel];
			Vx[faceI] = 0.5*((AVEPerCell[neighbourLabel]-AVEPerCell[ownerLabel])*(endingPoint1[faceI].y()-endingPoint0[faceI].y())
	                  +(AVEPerFace0[faceI]-AVEPerFace1[faceI])*(cellcenterY[neighbourLabel]-cellcenterY[ownerLabel]))/A[faceI];

	       	Vy[faceI] = -0.5*((AVEPerCell[neighbourLabel]-AVEPerCell[ownerLabel])*temp1
	                  +(AVEPerFace0[faceI]-AVEPerFace1[faceI])*temp2)/A[faceI];

	       	Vx1[faceI] = 0.5*((AVEPerCellU[neighbourLabel]-AVEPerCellU[ownerLabel])*(endingPoint1[faceI].y()-endingPoint0[faceI].y())
	                  +(AVEPerFace10[faceI]-AVEPerFace11[faceI])*(cellcenterY[neighbourLabel]-cellcenterY[ownerLabel]))/A[faceI];

	       	Vy1[faceI] = -0.5*((AVEPerCellU[neighbourLabel]-AVEPerCellU[ownerLabel])*temp1
	                  +(AVEPerFace10[faceI]-AVEPerFace11[faceI])*temp2)/A[faceI];

	       	Vx2[faceI] = 0.5*((AVEPerCellV[neighbourLabel]-AVEPerCellV[ownerLabel])*(endingPoint1[faceI].y()-endingPoint0[faceI].y())
	                  +(AVEPerFace20[faceI]-AVEPerFace21[faceI])*(cellcenterY[neighbourLabel]-cellcenterY[ownerLabel]))/A[faceI];

	       	Vy2[faceI] = -0.5*((AVEPerCellV[neighbourLabel]-AVEPerCellV[ownerLabel])*temp1
	                  +(AVEPerFace20[faceI]-AVEPerFace21[faceI])*temp2)/A[faceI];

	       	Vx3[faceI] = 0.5*((AVEPerCellP[neighbourLabel]-AVEPerCellP[ownerLabel])*(endingPoint1[faceI].y()-endingPoint0[faceI].y())
	                  +(AVEPerFace30[faceI]-AVEPerFace31[faceI])*(cellcenterY[neighbourLabel]-cellcenterY[ownerLabel]))/A[faceI];

	       	Vy3[faceI] = -0.5*((AVEPerCellP[neighbourLabel]-AVEPerCellP[ownerLabel])*temp1
	                  +(AVEPerFace30[faceI]-AVEPerFace31[faceI])*temp2)/A[faceI];


	      	CellA[ownerLabel] +=  A[faceI];
	      	CellA[neighbourLabel] -= A[faceI];
		
	      	CellA2[ownerLabel] +=  A_2[faceI];

		  	if(neighbourLabel < nElement)
	      		CellA2[neighbourLabel] += A_2[faceI];
	      	faceI++;
		}
	}
	
	//5.calculate the gradient of each cell 
	scalarList CVx(totalCellSize,0.0);
    scalarList CVy(totalCellSize,0.0);
    scalarList CVx1(totalCellSize,0.0);
    scalarList CVy1(totalCellSize,0.0);
    scalarList CVx2(totalCellSize,0.0);
    scalarList CVy2(totalCellSize,0.0);
    scalarList CVx3(totalCellSize,0.0);
    scalarList CVy3(totalCellSize,0.0);

    faceI = 0;
  	for(dgTree<physicalCellElement>::leafIterator iter = cellPhysicalEleTree.leafBegin(); iter != cellPhysicalEleTree.end(); ++iter){
		List<shared_ptr<physicalFaceElement>>& facePtr = iter()->value().faceElementList();
		forAll(facePtr, faceJ){
			if(facePtr[faceJ]->ownerEle_ != iter()) continue;
			label ownerLabel = facePtr[faceJ]->ownerEle_->value().sequenceIndex();
			label neighbourLabel = 0;
			if(facePtr[faceJ]->isPatch_){
				Pair<label> faceBoundaryIndex = facePtr[faceJ]->sequenceIndex();
				neighbourLabel = dofFaceInBoundary[faceBoundaryIndex[0]][faceBoundaryIndex[1]]; 
			}
			else{
				neighbourLabel = facePtr[faceJ]->neighborEle_->value().sequenceIndex();
			}

			if(neighbourLabel<nElement){
				CVx[ownerLabel] =CVx[ownerLabel]+ (A_2[faceI])*Vx[faceI]/CellA2[ownerLabel] ;
	        	//A   neighour sub !!!!!!!!!!!!!!!!! 
	        	CVx[neighbourLabel] =CVx[neighbourLabel]+ (A_2[faceI])*Vx[faceI]/CellA2[neighbourLabel] ;

	        	CVy[ownerLabel] = CVy[ownerLabel]+(A_2[faceI])*Vy[faceI]/CellA2[ownerLabel] ;
	        	CVy[neighbourLabel] =  CVy[neighbourLabel]+ (A_2[faceI])*Vy[faceI]/CellA2[neighbourLabel] ;

	        //u
	        	CVx1[ownerLabel] =CVx1[ownerLabel]+ (A_2[faceI])*Vx1[faceI]/CellA2[ownerLabel] ;
	        	CVx1[neighbourLabel] =CVx1[neighbourLabel]+ (A_2[faceI])*Vx1[faceI]/CellA2[neighbourLabel] ;

	        	CVy1[ownerLabel] = CVy1[ownerLabel]+(A_2[faceI])*Vy1[faceI]/CellA2[ownerLabel] ;
	        	CVy1[neighbourLabel] =  CVy1[neighbourLabel]+ (A_2[faceI])*Vy1[faceI]/CellA2[neighbourLabel] ;
	        //v
	        	CVx2[ownerLabel] =CVx2[ownerLabel]+ (A_2[faceI])*Vx2[faceI]/CellA2[ownerLabel] ;
	        	CVx2[neighbourLabel] =CVx2[neighbourLabel]+ (A_2[faceI])*Vx2[faceI]/CellA2[neighbourLabel] ;

	        	CVy2[ownerLabel] = CVy2[ownerLabel]+(A_2[faceI])*Vy2[faceI]/CellA2[ownerLabel] ;
	        	CVy2[neighbourLabel] =  CVy2[neighbourLabel]+ (A_2[faceI])*Vy2[faceI]/CellA2[neighbourLabel] ;

	        //p
	        	CVx3[ownerLabel] =CVx3[ownerLabel]+ (A_2[faceI])*Vx3[faceI]/CellA2[ownerLabel] ;
	        	CVx3[neighbourLabel] =CVx3[neighbourLabel]+ (A_2[faceI])*Vx3[faceI]/CellA2[neighbourLabel] ;

	        	CVy3[ownerLabel] = CVy3[ownerLabel]+(A_2[faceI])*Vy3[faceI]/CellA2[ownerLabel] ;
	        	CVy3[neighbourLabel] =  CVy3[neighbourLabel]+ (A_2[faceI])*Vy3[faceI]/CellA2[neighbourLabel] ;
			
			}

			else{
				CVx[ownerLabel] =CVx[ownerLabel]+ (A_2[faceI])*Vx[faceI]/CellA2[ownerLabel] ;
	        //A   neighour sub !!!!!!!!!!!!!!!!! 
	        	CVx[neighbourLabel] =Vx[faceI] ;


	        	CVy[ownerLabel] = CVy[ownerLabel]+(A_2[faceI])*Vy[faceI]/CellA2[ownerLabel] ;
	       	 	CVy[neighbourLabel] = Vy[faceI] ;

	        //u
	        	CVx1[ownerLabel] =CVx1[ownerLabel]+ (A_2[faceI])*Vx1[faceI]/CellA2[ownerLabel] ;
	        	CVx1[neighbourLabel] =Vx1[faceI] ;

	        	CVy1[ownerLabel] = CVy1[ownerLabel]+(A_2[faceI])*Vy1[faceI]/CellA2[ownerLabel] ;
	        	CVy1[neighbourLabel] =  Vy1[faceI] ;

	        //v
	        	CVx2[ownerLabel] =CVx2[ownerLabel]+ (A_2[faceI])*Vx2[faceI]/CellA2[ownerLabel] ;
	        	CVx2[neighbourLabel] =Vx2[faceI];

	        	CVy2[ownerLabel] = CVy2[ownerLabel]+(A_2[faceI])*Vy2[faceI]/CellA2[ownerLabel] ;
	        	CVy2[neighbourLabel] =  Vy2[faceI] ;

	        //p
	        	CVx3[ownerLabel] =CVx3[ownerLabel]+ (A_2[faceI])*Vx3[faceI]/CellA2[ownerLabel] ;
	        	CVx3[neighbourLabel] =Vx3[faceI] ;

	        	CVy3[ownerLabel] = CVy3[ownerLabel]+(A_2[faceI])*Vy3[faceI]/CellA2[ownerLabel] ;
	        	CVy3[neighbourLabel] = Vy3[faceI] ;
				
			}
			faceI++;
		}
	}

/*	
	// process the coupled patch boundary
	
  	forAll(mesh.boundary(), patchI){
       	const dgPatch& patch= mesh.boundary()[patchI];
		const Foam::dgPatchField<scalar>& tPatchf = rho.boundaryField()[patchI];
		if(tPatchf.size()<=0)continue;
		
		const labelList& dgFaceIndex = patch.dgFaceIndex();

		if (tPatchf.coupled()){
			scalarList CVxsend(8*dgFaceIndex.size(),0.0); 			 
			forAll(dgFaceIndex,faceI){			
				label faceLabel = dgFaceIndex[faceI];
				//label offset = neighborlist[faceLabel];
				label ownerLabel = dgFaceOwnerList[faceLabel];
				CVxsend[faceI*8 + 0] = CVx[ownerLabel];
				CVxsend[faceI*8 + 1] = CVy[ownerLabel];
				CVxsend[faceI*8 + 2] = CVx1[ownerLabel];
				CVxsend[faceI*8 + 3] = CVy1[ownerLabel];
				CVxsend[faceI*8 + 4] = CVx2[ownerLabel];
				CVxsend[faceI*8 + 5] = CVy2[ownerLabel];
				CVxsend[faceI*8 + 6] = CVx3[ownerLabel];
				CVxsend[faceI*8 + 7] = CVy3[ownerLabel];

			}
			
			//Pout<<"cvx:"<<CVxsend<<endl;
		 	patch.sendPstream(CVxsend,pBufs);
		}		 
            
  	}
	pBufs.finishedSends();
    forAll(mesh.boundary(), patchI){
        const dgPatch& patch = mesh.boundary()[patchI];
        const Foam::dgPatchField<scalar>& tPatchf = rho.boundaryField()[patchI];
		const labelList& dgFaceIndex = patch.dgFaceIndex();
			
        if(tPatchf.coupled()){
            scalarList CVxreceive(dgFaceIndex.size()*8,0.0);
            patch.receivePstream(CVxreceive,pBufs);
            forAll(dgFaceIndex,faceI){
				label faceLabel = dgFaceIndex[faceI];
				label offset = neighborlist[faceLabel];
                CVx[offset] = CVxreceive[faceI*8 + 0];
                CVy[offset] = CVxreceive[faceI*8 + 1];
				CVx1[offset] = CVxreceive[faceI*8 + 2];
                CVy1[offset] = CVxreceive[faceI*8 + 3];
				CVx2[offset] = CVxreceive[faceI*8 + 4];
                CVy2[offset] = CVxreceive[faceI*8 + 5];
				CVx3[offset] = CVxreceive[faceI*8 + 6];
                CVy3[offset] = CVxreceive[faceI*8 + 7];
            }
        }

    }
*/
	//6 limite the gradient of each cell
  	scalar w1=0.0,w2=0.0,w3=0.0;
  	scalar g1=0.0,g2=0.0,g3=0.0;
  	scalar w11=0.0,w12=0.0,w13=0.0;
  	scalar g11=0.0,g12=0.0,g13=0.0;
  	scalar w21=0.0,w22=0.0,w23=0.0;
  	scalar g21=0.0,g22=0.0,g23=0.0;
  	scalar w31=0.0,w32=0.0,w33=0.0;
  	scalar g31=0.0,g32=0.0,g33=0.0;
  	scalar epse=1e-10;
  	scalarList LdvdxC(nElement,0.0);
  	scalarList LdvdyC(nElement,0.0);
  	scalarList LdvdxC1(nElement,0.0);
  	scalarList LdvdyC1(nElement,0.0);
  	scalarList LdvdxC2(nElement,0.0);
  	scalarList LdvdyC2(nElement,0.0);
  	scalarList LdvdxC3(nElement,0.0);
  	scalarList LdvdyC3(nElement,0.0);

  	// build the neighbor cell index(including the ghost cell) of every cell
  	label cellI = 0;
  	for(dgTree<physicalCellElement>::leafIterator iter = cellPhysicalEleTree.leafBegin(); iter != cellPhysicalEleTree.end(); ++iter){
		List<shared_ptr<physicalFaceElement>>& facePtr = iter()->value().faceElementList();
		labelList c(facePtr.size());
		forAll(facePtr, faceJ){
			if(facePtr[faceJ]->ownerEle_ == iter()){
				if(!facePtr[faceJ]->isPatch_){
					c[faceJ] = facePtr[faceJ]->neighborEle_->value().sequenceIndex();
				}
				else{
					Pair<label> faceBoundaryIndex = facePtr[faceJ]->sequenceIndex();
					c[faceJ] = dofFaceInBoundary[faceBoundaryIndex[0]][faceBoundaryIndex[1]]; 
				}
			}
			else if(facePtr[faceJ]->neighborEle_ == iter()){
				c[faceJ] = facePtr[faceJ]->ownerEle_->value().sequenceIndex();
			}
		}
		label c1 = c[0];
		label c2 = c[1];
		label c3 = c[2];

		g1=CVx[c1]*CVx[c1]+CVy[c1]*CVy[c1];
        g2=CVx[c2]*CVx[c2]+CVy[c2]*CVy[c2];
        g3=CVx[c3]*CVx[c3]+CVy[c3]*CVy[c3];

        scalar fac=g1*g1+g2*g2+g3*g3;
        w1 = (g2*g3+epse)/(fac+3*epse);
        w2 = (g1*g3+epse)/(fac+3*epse);
        w3 = (g2*g1+epse)/(fac+3*epse);

        LdvdxC[cellI] = w1*CVx[c1]+w2*CVx[c2]+w3*CVx[c3];
        LdvdyC[cellI] = w1*CVy[c1]+w2*CVy[c2]+w3*CVy[c3];


        g11=CVx1[c1]*CVx1[c1]+CVy1[c1]*CVy1[c1];
        g12=CVx1[c2]*CVx1[c2]+CVy1[c2]*CVy1[c2];
        g13=CVx1[c3]*CVx1[c3]+CVy1[c3]*CVy1[c3];

        scalar fac1=g11*g11+g12*g12+g13*g13;
        w11 = (g12*g13+epse)/(fac1+3*epse);
        w12 = (g11*g13+epse)/(fac1+3*epse);
        w13 = (g12*g11+epse)/(fac1+3*epse);

        LdvdxC1[cellI] = w11*CVx1[c1]+w12*CVx1[c2]+w13*CVx1[c3];
        LdvdyC1[cellI] = w11*CVy1[c1]+w12*CVy1[c2]+w13*CVy1[c3];
 
        g21=CVx2[c1]*CVx2[c1]+CVy2[c1]*CVy2[c1];
        g22=CVx2[c2]*CVx2[c2]+CVy2[c2]*CVy2[c2];
        g23=CVx2[c3]*CVx2[c3]+CVy2[c3]*CVy2[c3];

        scalar fac2=g21*g21+g22*g22+g23*g23;
        w21 = (g22*g23+epse)/(fac2+3*epse);
        w22 = (g21*g23+epse)/(fac2+3*epse);
        w23 = (g22*g21+epse)/(fac2+3*epse);

        LdvdxC2[cellI] = w21*CVx2[c1]+w22*CVx2[c2]+w23*CVx2[c3];
        LdvdyC2[cellI] = w21*CVy2[c1]+w22*CVy2[c2]+w23*CVy2[c3];

        g31=CVx3[c1]*CVx3[c1]+CVy3[c1]*CVy3[c1];
        g32=CVx3[c2]*CVx3[c2]+CVy3[c2]*CVy3[c2];
        g33=CVx3[c3]*CVx3[c3]+CVy3[c3]*CVy3[c3];

        scalar fac3=g31*g31+g32*g32+g33*g33;
        w31 = (g32*g33+epse)/(fac3+3*epse);
        w32 = (g31*g33+epse)/(fac3+3*epse);
        w33 = (g32*g31+epse)/(fac3+3*epse);

        LdvdxC3[cellI] = w31*CVx3[c1]+w32*CVx3[c2]+w33*CVx3[c3];
        LdvdyC3[cellI] = w31*CVy3[c1]+w32*CVy3[c2]+w33*CVy3[c3];

        cellI++;
	}

	//7.limite the value of primitive value and reconstruct the conservation value
  	scalar tol =1e-2;
  	cellI = 0;

  	for(dgTree<physicalCellElement>::leafIterator iter = cellPhysicalEleTree.leafBegin(); iter != cellPhysicalEleTree.end(); ++iter){
  		const List<vector>& X = iter()->value().dofLocation();
  		label dofStart = iter()->value().dofStart();
  		forAll(X, pointI){
  			scalar du=0;
        	scalar du1=0;
        	scalar du2=0;
        	scalar du3=0;

        	du = (X[pointI].x()-cellcenterX[cellI])*LdvdxC[cellI]+ (X[pointI].y()-cellcenterY[cellI])*LdvdyC[cellI];
        	du1 = (X[pointI].x()-cellcenterX[cellI])*LdvdxC1[cellI]+ (X[pointI].y()-cellcenterY[cellI])*LdvdyC1[cellI];
        	du2 = (X[pointI].x()-cellcenterX[cellI])*LdvdxC2[cellI]+ (X[pointI].y()-cellcenterY[cellI])*LdvdyC2[cellI];
        	du3 = (X[pointI].x()-cellcenterX[cellI])*LdvdxC3[cellI]+ (X[pointI].y()-cellcenterY[cellI])*LdvdyC3[cellI];

        	scalar Lrho = AVEPerCell[cellI] + du ;

       		while(Lrho<tol){
                Info<<"warning: crroect negative density"<<endl;
                du = du*0.5;
                Lrho = AVEPerCell[cellI]  +du;
         	}

        	rho[dofStart+pointI] = Lrho ;

        	rhou[dofStart+pointI] = AVEPerCell1[cellI] + AVEPerCell[cellI]*du1 + du*AVEPerCellU[cellI] ;

        	rhov[dofStart+pointI] = AVEPerCell2[cellI] +AVEPerCell[cellI]*du2 +du*AVEPerCellV[cellI];

         	scalar dE=0;
         	dE = (1/(ther_gamma-1))*du3+0.5*du*(std::pow(AVEPerCellU[cellI],2)+std::pow(AVEPerCellV[cellI],2))
                    +AVEPerCell[cellI]*(AVEPerCellU[cellI]*du1+AVEPerCellV[cellI]*du2);
         	Ener[dofStart+pointI] = AVEPerCell3[cellI] + dE ;

        	scalar Lp = (ther_gamma-1)*(Ener[dofStart+pointI]
							-0.5*(
							std::pow(rhou[dofStart+pointI],2)
							+std::pow(rhov[dofStart+pointI],2)
							)/rho[dofStart+pointI]);
         	if(Lp<tol){
             	Info<<"warning: crroect negative pressure"<<endl;
          	}
  		}
  		cellI++;
	}
  
	rhoU.replace(0,rhou);
	rhoU.replace(1,rhov);
}


	 

         
} // End namespace dg

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //
