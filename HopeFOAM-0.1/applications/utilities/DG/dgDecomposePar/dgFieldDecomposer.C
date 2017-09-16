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

\*---------------------------------------------------------------------------*/

#include "dgFieldDecomposer.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::dgFieldDecomposer::patchFieldDecomposer::patchFieldDecomposer
(
    const labelUList& addressingSlice,
    const label addressingOffset
)
:
  
    directAddressing_(addressingSlice)
{
/*****directAddressing:patch field per processor addressed in the complete patch address*****/
    forAll(directAddressing_, i)
    {
        // Subtract one to align addressing.
        //  directAddressing_[i] =addressingSlice[i]- addressingOffset -1;
		 directAddressing_[i] -=addressingOffset ;
    }
}


Foam::dgFieldDecomposer::processorVolPatchFieldDecomposer::
processorVolPatchFieldDecomposer
(
    const dgMesh& mesh,
    const labelUList& addressingSlice   // addressingslice is common face label of openfoam
)
:
    directFaceLabel_(addressingSlice.size())
{
	const labelList& own = mesh.faceOwner();
   	const labelList& neighb = mesh.faceNeighbour();
	const labelList& dgOwn = mesh.dgFaceOwner();
	const labelList& dgNeighb = mesh.dgFaceNeighbour();
    
    label faceLabel=0;
	
    forAll(addressingSlice, faceI){
	    label ai = mag(addressingSlice[faceI]) - 1;
        label owner = own[ai];
        label neighbour= neighb[ai];
	
        forAll(dgOwn,faceI){
            label dgOwner = dgOwn[faceI];
            label dgNeighbour = dgNeighb[faceI];
            if(dgOwner == owner && dgNeighbour == neighbour){
                faceLabel = faceI;
                break;
            }
        }
	    directFaceLabel_[faceI][1]  =faceLabel;
		if (ai < neighb.size()){
			if(addressingSlice[faceI] >= 0)
				directFaceLabel_[faceI][0]  = -1; //neighoubr point for boundary
			else
				directFaceLabel_[faceI][0] = 1;  //owner point for boundary
		}
		else
		    directFaceLabel_[faceI][0] = 1;   //the face is a boundary face ,thus the value is the owner field point
	}

}
 




Foam::dgFieldDecomposer::dgFieldDecomposer
(
    const dgMesh& completeMesh,
    const dgMesh& procMesh,
    const labelList& faceAddressing,
    const labelList& cellAddressing,
    const labelList& boundaryAddressing
)
:
    completeMesh_(completeMesh),
    procMesh_(procMesh),
    faceAddressing_(faceAddressing),
    cellAddressing_(cellAddressing),
    cellFieldAddressing_(), //cellAddressing.size() * procMesh.elements()[0]->nDofPerCell()
    boundaryAddressing_(boundaryAddressing),
    boundaryFieldAddressing_(),//boundaryAddressing.size()* procMesh.dgFaceList()[0].nOwnerDof_
    patchFieldDecomposerPtrs_
    (
        procMesh_.boundary().size(),
        static_cast<patchFieldDecomposer*>(NULL)
    ),
    processorVolPatchFieldDecomposerPtrs_
    (
        procMesh_.boundary().size(),
        static_cast<processorVolPatchFieldDecomposer*>(NULL)
    )
{
    label start=0;
	label pointsPerCell;
    const dgTree<physicalCellElement> cellEleTree = completeMesh_.cellElementsTree();



    for(dgTree<physicalCellElement>::leafIterator iter = cellEleTree.leafBegin(cellAddressing_) ; iter != cellEleTree.end() ; ++iter){
        const physicalCellElement& cellEle = iter()->value();
        pointsPerCell = cellEle.dofLocation().size();
        start = cellEle.dofStart();

        for(label i=0 ; i<pointsPerCell ; ++i){
            cellFieldAddressing_.append(start++);
        }
    }
	
    label pointsPerFace = completeMesh_.faceElements()[0]->value().nOwnerDof();//It doesn't matter which face is this face ,since tri and qua face 
                                                                                //have the same dof each face.
    forAll(boundaryAddressing_, patchI){
        if (
            boundaryAddressing_[patchI] >= 0
         && !isA<processorLduInterface>(procMesh_.boundary()[patchI])
        )
        {
            const SubList<label>& patchIFaceAddressing = procMesh_.boundary()[patchI].patchSlice(faceAddressing_);
            labelList patchIFieldFaceAddressing;

            forAll(patchIFaceAddressing, faceI){
                label start=patchIFaceAddressing[faceI]*pointsPerFace;
                for(label i=0;i<pointsPerFace;i++){
                    patchIFieldFaceAddressing.append(start++);
                }   
            }
            patchFieldDecomposerPtrs_[patchI] = new patchFieldDecomposer
            (
               patchIFieldFaceAddressing,
                (completeMesh_.boundaryMesh()
                [
                    boundaryAddressing_[patchI]
                ].start()+1)*pointsPerFace     //patchIFaceAddressing start from label 1......
            );
        }
        else{
            processorVolPatchFieldDecomposerPtrs_[patchI] =new processorVolPatchFieldDecomposer(completeMesh_, procMesh_.boundary()[patchI].patchSlice(faceAddressing_));
        }
    }
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::dgFieldDecomposer::~dgFieldDecomposer()
{
    forAll(patchFieldDecomposerPtrs_, patchi)
    {
        if (patchFieldDecomposerPtrs_[patchi])
        {
            delete patchFieldDecomposerPtrs_[patchi];
        }
    }

    forAll(processorVolPatchFieldDecomposerPtrs_, patchi)
    {
        if (processorVolPatchFieldDecomposerPtrs_[patchi])
        {
            delete processorVolPatchFieldDecomposerPtrs_[patchi];
        }
    }

}

// ************************************************************************* //
