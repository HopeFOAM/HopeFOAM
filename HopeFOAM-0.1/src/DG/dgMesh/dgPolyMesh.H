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

Class
    Foam::dgPolyMesh

Description
    Mesh consisting of general polyhedral cells.

SourceFiles
    dgPolyMesh.C

\*---------------------------------------------------------------------------*/

#ifndef dgPolyMesh_H
#define dgPolyMesh_H

#include "polyMesh.H"
#include "objectRegistry.H"
#include "dgMeshCtrDict.H"

#include "dgPolyCell.H"
#include "dgPolyFace.H"
#include "dgTree.H"
#include <memory>
#include <vector>

using std::shared_ptr;
using std::make_shared;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{



/*---------------------------------------------------------------------------*\
                          Class dgPolyMesh Declaration
\*---------------------------------------------------------------------------*/

class dgPolyMesh
	:
	public polyMesh,
	public dgMeshCtrDict

{
	
private:

	
	// Permanent data
	label meshDim_;

	//the number  per cell
	labelList nPointsPerCell_;

	//the label list of reorder points in each cell
	labelListList cellsPoints_;

	//the face label of each cell  //edge label when 2D
	cellList cellsFaces_;

	//label sign  Z = 0 the face on the z=0 in openfoam;
	labelList cellFaceLabels_;

	//the reorder face label of each cell using gambit
	//cellList dgCellsFaces_;
	
	//the gobal point label of each face index by cell->face->point
	labelListListList facesPoints_;

	//label of point address in the neighbour face, thus rotate number
	labelListList firstPointIndex_;

	//the point list of each cell after reorder
	List<List<point>>  dgPoints_;

	//face of neighbour cell ID index by cell--face
	labelListList dgFaceNeighbourID_;

	//shape of each cell
	List<word> cellShapes_;


	//the label of the face in the neighbour cell , edge when 2D
	labelListList neighbourFace_;

	//only in 2D  new edge id index by cell->edges
	labelListList dgCellFaceNewID_;

	//the neighbour cell of the edge index by cell -->faces (only valid in 2D)
	labelListList edgeNeighbour_;

	//the owner cell of the edge index by cell faces
	labelListList edgeOwner_;

	//the foam edge number index by new edge number
	labelList dgEdgeToFoamEdge_;
	std::vector<label> vecdgEdgeToFoamEdge_;

	// face owner index by face label
	labelList dgFaceOwner_;
	std::vector<label> vecdgFaceOwner_;

	//face neighbour index by face label		
	labelList dgFaceNeighbour_;
	std::vector<label> vecdgFaceNeighbour_;

	//face number in local cell index by face label  or new edge label when 2D		
	labelList faceIndexInOwner_;

	//face number in neighbour cell index by face label		
	labelList faceIndexInNeighbour_;

	//face point first index in neighbour index by face label
	labelList facePointFirstIndex_;

	//dg face number
	label dgFaceSize_;


	//dg cell element
	List<shared_ptr<dgTreeUnit<dgPolyCell>>> dgCells_;

	//dg cell tree
	dgTree<dgPolyCell> cellTree_;

	//dg face list
	List<shared_ptr<dgTreeUnit<dgPolyFace>>> dgFaces_;

	//dg face tree
	dgTree<dgPolyFace> faceTree_;


public:
	ClassName("dgPolyMesh");
	// Constructors
	//- Construct from polymesh
	explicit dgPolyMesh(const IOobject& io);


	//- Destructor
	virtual ~dgPolyMesh() {}

	//member function
	inline const labelList& nPointsPerCell() const
	{
		return nPointsPerCell_;
	}

	inline const labelListList& cellsPoints() const
	{
		return cellsPoints_;
	}

	inline const cellList& cellsFaces() const
	{
		return cellsFaces_;
	}

	inline label meshDim()const
	{
		return meshDim_;
	}

	//-check if z min is equal to zero when meshDim_ == 2
	void checkPointZ();


	inline const List<List<point>>& dgPoints() const
	{
		return dgPoints_;
	}

	inline const word& cellShapeName( label cellI) const
	{
		return cellShapes_[cellI];
	}
	inline const List<word>& cellShapeName() const
	{
		return cellShapes_;
	}

	inline const labelListList& neighbourFace() const
	{
		return neighbourFace_;
	}


	inline const List<shared_ptr<dgTreeUnit<dgPolyCell>>>& dgCells()
	{
		return dgCells_;
	}

	inline dgTree<dgPolyCell>& cellTree()
	{
		return cellTree_;
	}

	inline const dgTree<dgPolyCell>& cellTree() const
	{
		return cellTree_;
	}

	inline const List<shared_ptr<dgTreeUnit<dgPolyFace>>>& dgFaces()
	{
		return dgFaces_;
	}

	inline dgTree<dgPolyFace>& faceTree()
	{
		return faceTree_;
	}

	inline const dgTree<dgPolyFace>& faceTree() const
	{
		return faceTree_;
	}


	virtual bool writeObjects
	(
	    IOstream::streamFormat fmt,
	    IOstream::versionNumber ver,
	    IOstream::compressionType cmp
	) const;


	// calculate the neighbour cell of every edge in the cell in 2D
	//thus calculate edgeNeighbour_
	void calEdgeNeighbour() ;

	void calFacesPoints(int dimension) ;

	void reorderCellPoints(int dimension);

	void reorderCellsFaces(int dimension);

	void calNeighbourFace(int dimension);
	
	void calFirstPointIndex(int dimension);

	void calDgFaceNeighbourID();

	void initialDgFace();


	void foamToDG();

	inline const labelListList& dgCellFaceNewID() const
	{
		return dgCellFaceNewID_;
	}
		
	inline const labelListList& edgeNeighbour() const
	{
		return edgeNeighbour_;
	}

	inline const labelListListList& facesPoints() const
	{
		return facesPoints_;
	}

	inline const labelListList& firstPointIndex() const
	{
		return firstPointIndex_;
	}

	inline const labelListList& dgFaceNeighbourID() const
	{
		return dgFaceNeighbourID_;
	}

	inline label dgFaceSize() const
		{
		return dgFaceSize_;
	}

	inline const labelListList& edgeOwner() const
		{
			return edgeOwner_;
		}

	inline const labelList& dgEdgeToFoamEdge() const
		{
			return dgEdgeToFoamEdge_;
	}

	// face owner index by face label
	inline const labelList& dgFaceOwner() const
		{
			return dgFaceOwner_;
	}

	//face neighbour index by face label		
	inline const labelList& dgFaceNeighbour() const
	{	
		return dgFaceNeighbour_;
	}

	//face number in local cell index by face label		
	inline const labelList& faceIndexInOwner() const
	{
		return faceIndexInOwner_;
	}

	//face number in neighbour cell index by face label		
	inline const labelList& faceIndexInNeighbour() const
	{
		return faceIndexInNeighbour_;
	}

	//face point first index in neighbour index by face label
	inline const labelList& facePointFirstIndex() const
	{
		return facePointFirstIndex_;
	}


	inline const labelList& cellFaceLabels() const
		{
		return cellFaceLabels_;
	}


};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
