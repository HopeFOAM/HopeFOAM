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

#include "dgPatch.H"
#include "addToRunTimeSelectionTable.H"
#include "dgBoundaryMesh.H"
#include "dgMesh.H"
#include "primitiveMesh.H"
#include <memory>

using std::shared_ptr;
using std::make_shared;

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(dgPatch, 0);
    defineRunTimeSelectionTable(dgPatch, polyPatch);
    addToRunTimeSelectionTable(dgPatch, dgPatch, polyPatch);
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::dgPatch::dgPatch(const polyPatch& p, const dgBoundaryMesh& bm)
:
    polyPatch_(p),
    boundaryMesh_(bm),
    facesPoints_(p.size()),
    dgFaceIndex_(),
    size_(p.size())
{
	
	labelList edgeIndex(size());
	const dgMesh& mesh=boundaryMesh_.mesh();
	label dim = mesh.meshDim();
	const cellList& cellfaces = mesh.cellsFaces();
	if(dim == 3){
		dgFaceIndex_.setSize(size());
		forAll(polyPatch_,faceI){
			label faceLabel = this->start()+faceI;
			dgFaceIndex_[faceI] = faceLabel;
		}
	}
	else if(dim ==2)
	{
		if(polyPatch_.type() != "empty"){
			dgFaceIndex_.setSize(size());
			forAll(polyPatch_,faceI){
				label faceLabel = this->start()+faceI;
				label ownerLabel = mesh.faceOwner()[faceLabel];
				const labelList& edgeList=mesh.faceEdges()[faceLabel];
				const List<edge>& pointLabelList=mesh.edges();
				const List<point>& pointLists = mesh.points();
			
				forAll(edgeList, edgeI){
					label pointLabel0 = pointLabelList[edgeList[edgeI]][0];
					label pointLabel1 = pointLabelList[edgeList[edgeI]][1];
					point point0 = pointLists[pointLabel0]; 
					point point1 = pointLists[pointLabel1]; 
					if((point0.z() == 0) && (point1.z() == 0)){
						edgeIndex[faceI] = edgeList[edgeI];

						break;
					}
				}
				const labelList& dgFaceNewID = mesh.dgCellFaceNewID()[ownerLabel];
				forAll(cellfaces[ownerLabel],faceJ){
					if(cellfaces[ownerLabel][faceJ] == edgeIndex[faceI]){
						dgFaceIndex_[faceI] = dgFaceNewID[faceJ];
						break;
					}
				}
			}
		}

	}
	else
		FatalErrorIn("dgPatch::dgPatch()")
		        << "meshDimension is set error to be "
		        <<dim << " in file dgMeshCtrDict"<<endl
		        << abort(FatalError);
	
	forAll(dgFaceIndex_, faceI){
		dgPolyFace tmp = mesh.faceTree().baseLst()[dgFaceIndex_[faceI]]->value();
		facesPoints_[faceI] = tmp.points();
	}
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::dgPatch::~dgPatch()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

const Foam::labelUList& Foam::dgPatch::faceCells() const
{
    return polyPatch_.faceCells();
}

const Foam::List<shared_ptr<Foam::dgTreeUnit<Foam::dgPolyCell>>>& Foam::dgPatch::dgFaceCells() const
{
    //return polyPatch_.faceCells();
    const dgMesh& mesh = boundaryMesh_.mesh();
    const labelList& dgFaceIndex = this->dgFaceIndex();

    const dgTree<dgPolyFace>& faceTree = mesh.faceTree();
    List<shared_ptr<dgTreeUnit<dgPolyCell>>> patchCell(size_);

    label cellI = 0;
    for(dgTree<dgPolyFace>::leafIterator iter = faceTree.leafBegin(dgFaceIndex) ; iter != faceTree.end() ; ++iter){
    	patchCell[cellI] = iter()->value().owner();
    	++cellI;
    }
    return patchCell;
}

void Foam::dgPatch::updateTotalDofNum()
{
	totalDofNum_ = 0;
	const dgMesh& mesh = this->boundaryMesh().mesh();

	const labelList& dgFaceIndex = this->dgFaceIndex();
	const dgTree<physicalFaceElement>&  dgFaceTree = mesh.faceElementsTree();

	if(dgFaceIndex.size()>0){
		for(dgTree<physicalFaceElement>::leafIterator iter = dgFaceTree.leafBegin(dgFaceIndex); iter != dgFaceTree.end(); ++iter){
			totalDofNum_ += iter()->value().nOwnerDof();
		}
	}

}

void Foam::dgPatch::makeWeights(scalarField& w) const
{
    w = 1.0;
}


void Foam::dgPatch::initMovePoints()
{}


void Foam::dgPatch::movePoints()
{}


const Foam::scalarField& Foam::dgPatch::deltaCoeffs() const
{
    //return boundaryMesh().mesh().deltaCoeffs().boundaryField()[index()];
    return scalarField();
}


const Foam::scalarField& Foam::dgPatch::weights() const
{
    //return boundaryMesh().mesh().weights().boundaryField()[index()];
    return scalarField();
}




// ************************************************************************* //