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
    Foam::dgMesh
    
Description
    Mesh date needed to do the DG discretisation.

SourceFiles
    dgMesh.C
    dgMeshGeometry.C

\*---------------------------------------------------------------------------*/

#ifndef dgMesh_H
#define dgMesh_H

#include "dgPolyMesh.H"
#include "dgSchemes.H"
#include "dgSolution.H"
#include "data.H"
#include "dgBoundaryMesh.H"
#include "physicalElementData.H"
#include <petscksp.h>

namespace Foam
{
/*---------------------------------------------------------------------------*\
                           Class dgMesh Declaration
\*---------------------------------------------------------------------------*/

class dgMesh
	:
	public dgPolyMesh,
	public dgSchemes,
	public dgSolution,
	public data,
	public physicalElementData
{
	// Private data
	dgBoundaryMesh boundary_;

	//- the points shift and size of local points in global scale
	labelPair localRange_;

	//- inv mass matrix solver for all local cells
	KSP massMatrixSolver_;

	// Private Member Functions

	// Storage management

	//- Clear geometry but not the old-time cell volumes
	void clearGeomNotOldVol();

	//- Clear geometry like clearGeomNotOldVol but recreate any
	//  geometric demand-driven data that was set
	void updateGeomNotOldVol();

	//- Clear geometry
	void clearGeom();

	// Make geometric data

	//- Disallow construct as copy
	dgMesh(const dgMesh&);

	//- Disallow assignment
	void operator=(const dgMesh&);

public:

	// Public typedefs

	typedef dgMesh Mesh;
	typedef dgBoundaryMesh BoundaryMesh;

	// Declare name of the class and its debug switch
	ClassName("dgMesh");

	// Constructors

	//- Construct from IOobject
	explicit dgMesh(const IOobject& io);

	//- Destructor
	virtual ~dgMesh();

	// Member Functions

	// Access
	KSP massMatrixSolver();

	//- Return the top-level database
	const Time& time() const
	{
		return polyMesh::time();
	}

	//- Return the object registry - resolve conflict element/lduMesh
	virtual const objectRegistry& thisDb() const
	{
		return polyMesh::thisDb();
	}

	//- Return reference to name
	//  Note: name() is currently ambiguous due to derivation from
	//  surfaceInterpolation
	const word& name() const
	{
		return polyMesh::name();
	}
	//- Return communicator used for parallel communication
	virtual int comm() const
	{
		return polyMesh::comm();
	}
	const globalMeshData& globalData() const
	{
		return polyMesh::globalData();
	}

	//get dg related data
	const labelListList& eleCellPoint()const
	{
		return cellsPoints();
	}


	const dgBoundaryMesh& boundary() const
	{
		return boundary_;
	}

	//- local range for the proc, first()-global shift; second()- local size;	
	const labelPair localRange() const{
		return localRange_;
	}

	void updateLocalRange();

	//get polyMesh data
	const List<List<point>>& polyPoints() const
	{
		return dgPoints();
	};
	label nPoints()
	{
		return Foam::dgPolyMesh::nPoints();
	};
	const labelList& faceOwner()const
	{
		return Foam::dgPolyMesh::faceOwner();
	}
	const labelList& faceNeighbour()const
	{
		return Foam::dgPolyMesh::faceNeighbour();
	}
	const polyBoundaryMesh& boundaryMesh() const
	{
		return Foam::dgPolyMesh::boundaryMesh();
	}

	// Edit

	//- Clear all geometry and addressing
	void clearOut();

	// Write

	//- Write the underlying element and other data

	virtual bool writeObjects
	(
	    IOstream::streamFormat fmt,
	    IOstream::versionNumber ver,
	    IOstream::compressionType cmp
	) const;

	//- Write mesh using IO settings from time
	virtual bool write() const;

 	const word meshSubDir() const {return  polyMesh::meshSubDir;}
	// Member Operators

	bool operator!=(const dgMesh&) const;
	bool operator==(const dgMesh&) const;
};

}

#ifdef NoRepository
#   include "dgPatchDgMeshTemplates.C"
#endif

#endif
