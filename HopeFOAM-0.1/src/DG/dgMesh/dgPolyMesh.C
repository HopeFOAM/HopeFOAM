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
#include "dgPolyMesh.H"
#include "prismMatcher.H"
#include "hexMatcher.H"

using std::shared_ptr;
using std::make_shared;

//constructor
Foam::dgPolyMesh::dgPolyMesh(const IOobject& io)
	:
	polyMesh(io),
	dgMeshCtrDict(static_cast<const objectRegistry&>(*this)),
	nPointsPerCell_(this->nCells()),
	cellsPoints_(nCells()),
	dgPoints_(nCells()),
	cellsFaces_(nCells()),
	neighbourFace_(nCells()),
	edgeOwner_(nCells()),
	edgeNeighbour_(nCells()),
	facesPoints_(nCells()),
	firstPointIndex_(nCells()),
	cellFaceLabels_(0),
	dgFaceNeighbourID_(nCells()),
	dgFaceSize_(0),
	dgFaceOwner_(),
	dgFaceNeighbour_(),
	faceIndexInOwner_(),
	faceIndexInNeighbour_(),
	dgEdgeToFoamEdge_(),
	facePointFirstIndex_(),
	dgCellFaceNewID_(nCells()),
	
	cellShapes_(nCells()),
	dgCells_(nCells()),
	dgFaces_(0),
	vecdgEdgeToFoamEdge_(),
	vecdgFaceOwner_(),
	vecdgFaceNeighbour_()
{



	const dictionary& dgDict = dgMeshCtrDict::subDict("DG");
	if(!dgDict.found("meshDimension") ) {
		FatalErrorIn("dgMesh::dgMesh()")
		        << "DG parameters meshDimension have not been set"<<endl
		        << abort(FatalError);
	}
	meshDim_ = dgDict.lookupOrDefault<int>("meshDimension", 2);

	const cellShapeList& cellShapes = this->cellShapes();



	if(meshDim_ == 3) {
			dgFaceOwner_.setSize(faceOwner().size());
			dgFaceNeighbour_.setSize(faceOwner().size());

		forAll(nPointsPerCell_,CellI) {
			nPointsPerCell_[CellI] = cellShapes[CellI].nPoints();
			cellShapes_[CellI] = cellShapes[CellI].model().name();

		}
		cellsPoints_ = cellPoints();
		cellsFaces_ = cells();
		

		dgFaceSize_ = faceOwner().size();
			dgFaceOwner_ = faceOwner();
			
			forAll(dgFaceNeighbour_, faceI){
				if(faceI< faceNeighbour().size())
					dgFaceNeighbour_[faceI] = faceNeighbour()[faceI];
				else
					dgFaceNeighbour_[faceI] = -1;
				}

	} else if(meshDim_ == 2) {
		checkPointZ();
		label isFace = 1;
		cellFaceLabels_.setSize(nCells());
		forAll(nPointsPerCell_,CellI) {
			if(cellShapes[CellI].model().name() == "hex" ) {
				cellShapes_[CellI] = "quad";
				nPointsPerCell_[CellI] = 4 ;
				cellsPoints_[CellI].setSize(4);
				
				cellsFaces_[CellI].setSize(4);
				neighbourFace_[CellI].setSize(4);
				dgCellFaceNewID_[CellI].setSize(4);

				forAll(cells()[CellI],faceI) {
					isFace = 1;

					face tmpface = faces()[cells()[CellI][faceI]];
					forAll(tmpface,pointI) {
						point tmpPoint = points()[tmpface[pointI]];
						if(tmpPoint.z() != 0) {
							isFace = 0;
							
						}


					}
					if(isFace) {
						label faceLabel = cells()[CellI][faceI];
						cellFaceLabels_[CellI] = faceLabel;
						const labelList& edgeList = faceEdges()[faceLabel];
						cellsFaces_[CellI][0] = edgeList[0];
						cellsFaces_[CellI][1] = edgeList[1];
						cellsFaces_[CellI][2] = edgeList[2];
						cellsFaces_[CellI][3] = edgeList[3];

						cellsPoints_[CellI] = faces()[faceLabel];
						face tmpface = faces()[faceLabel];
						forAll(tmpface,pointI) {
							point tmpPoint = points()[tmpface[pointI]];
							
						}


					}
				}



			} else if(cellShapes[CellI].model().name() == "prism") {
				cellShapes_[CellI] = "tri";
				nPointsPerCell_[CellI] = 3 ;
				cellsPoints_[CellI].setSize(3);
				cellsFaces_[CellI].setSize(3);
				
				neighbourFace_[CellI].setSize(3);
				dgCellFaceNewID_[CellI].setSize(3);
				forAll(cells()[CellI],faceI) {
					isFace = 1;
					face tmpface = faces()[cells()[CellI][faceI]];
					forAll(tmpface,pointI) {
						point tmpPoint = points()[tmpface[pointI]];

						if(tmpPoint.z() != 0) {
							isFace = 0;
							
						}
					}
					if(isFace) {
						label faceLabel = cells()[CellI][faceI];
						const labelList& edgeList = faceEdges()[faceLabel];
						cellsFaces_[CellI][0] = edgeList[0];
						cellsFaces_[CellI][1] = edgeList[1];
						cellsFaces_[CellI][2] = edgeList[2];
						
						cellFaceLabels_[CellI] = faceLabel;
						cellsPoints_[CellI] = faces()[faceLabel];
						face tmpface = faces()[faceLabel];
						forAll(tmpface,pointI) {
							point tmpPoint = points()[tmpface[pointI]];
							
						}

					}
				}


			}

			else
				FatalErrorIn("dgPolyMesh::dgPolyMesh()")
				        << "not implement yet for "
				        <<cellShapes[CellI].model().name() << " in dg"<<endl
				        << abort(FatalError);
		}




	} else if(meshDim_ == 1) {
		forAll(nPointsPerCell_,CellI) {
			cellShapes_[CellI] = "Line";
			nPointsPerCell_[CellI] = 2;

		}

	} else
		FatalErrorIn("dgPolyMesh::dgPolyMesh()")
		        << "meshDimension is set error to be "
		        <<meshDim_ << " in file dgMeshCtrDict"<<endl
		        << abort(FatalError);


	reorderCellPoints(meshDim_);

	reorderCellsFaces(meshDim_);

	calFacesPoints(meshDim_);

	calNeighbourFace(meshDim_);

	calFirstPointIndex(meshDim_);

	calDgFaceNeighbourID();

	initialDgFace();

	foamToDG();

}

// * * * * * * * * * * * * * * * * Member Function  * * * * * * * * * * * * * * * //

bool Foam::dgPolyMesh::writeObjects
(
    Foam::IOstream::streamFormat fmt,
    Foam::IOstream::versionNumber ver,
    Foam::IOstream::compressionType cmp
) const
{

	return Foam::polyMesh::writeObject(fmt, ver, cmp);

}

void Foam::dgPolyMesh::checkPointZ()
{
	const labelListList& tmpPointList = cellPoints();
	const pointField& tmpPoints = points();
	label isZero = 0;
	forAll(tmpPointList, cellI) {
		isZero = 0;
		forAll(tmpPointList[cellI],pointI) {
			point tmpPoint = tmpPoints[tmpPointList[cellI][pointI]];
			if(tmpPoint.z() == 0)
				isZero = 1;
		}
		if(isZero == 0)
			FatalErrorIn("dgPolyMesh::checkPointZ()")
			        << "the Z axis of the mesh must start at 0 when the mesh is a 2-D mesh  "
			        <<endl
			        << abort(FatalError);

	}



}

void listAppend(Foam::labelList & list, std::vector<Foam::label> & vec)
{
	Foam::label start = list.size();
	Foam::label end = list.size() + vec.size();
	list.setSize(list.size() + vec.size());
	for (Foam::label i = start, j = 0; i < end; i++, j++)
	{
		list[i] = vec[j];
	}
}

void Foam::dgPolyMesh::calEdgeNeighbour()
{
	label isAdjFace = 0;
	label zzero = 0;
	label znot = 0;
	const cellShapeList& cellShapes = this->cellShapes();
	const List<point>& pointsList = points();
	forAll(nPointsPerCell_, cellI) {
		if(cellShapes[cellI].model().name() == "hex" ) {
			edgeNeighbour_[cellI].setSize(4);
			edgeOwner_[cellI].setSize(4);
			//label Face = cellsFaces()[cellI][0];
			const labelList& faceEdge = cellsFaces_[cellI];
			forAll(faceEdge, edgeI) {
				label tmpEdge = faceEdge[edgeI];
				const labelList& edgeFace = edgeFaces()[tmpEdge];
				forAll(edgeFace, faceI) {
					isAdjFace = 0;
					zzero = 0;
					znot = 0;
					label tmpFaceLabel = edgeFace[faceI];
					face tmpFace = faces()[tmpFaceLabel];
					forAll(tmpFace, pointI) {
						point tmpPoint = pointsList[tmpFace[pointI]];
						if(tmpPoint.z() != 0)
							znot =  1;
						if(tmpPoint.z() == 0)
							zzero = 1;

					}
					isAdjFace = znot && zzero; // to judge if a face is a adjacent face
					if(isAdjFace) {
						//dgFaceSize_ ++;
						//dgEdgeToFoamEdge_.append(tmpEdge);
						label faceowner = faceOwner()[tmpFaceLabel];
						edgeOwner_[cellI][edgeI] = faceowner;
						if(tmpFaceLabel < faceNeighbour().size())  //some adjacent face is boundary face
							edgeNeighbour_[cellI][edgeI] =(faceNeighbour()[tmpFaceLabel] == cellI)?
							                              faceowner
							                              :faceNeighbour()[tmpFaceLabel]; // the neighbour is not the same as face neighbour
						//in openfoam
						else
							{
								edgeNeighbour_[cellI][edgeI] = -1;
								//dgEdgeToFoamEdge_.append( tmpEdge);
								//dgFaceSize_ ++;
							}
						if(edgeOwner_[cellI][edgeI] != edgeNeighbour_[cellI][edgeI])  // one face 
							{
								
								vecdgEdgeToFoamEdge_.push_back( tmpEdge);
								dgFaceSize_ ++;
								vecdgFaceNeighbour_.push_back(edgeNeighbour_[cellI][edgeI]);
								vecdgFaceOwner_.push_back(faceowner);
						}
						
						break;
					}
				}
			}

		} else if(cellShapes[cellI].model().name() == "prism") {
			edgeNeighbour_[cellI].setSize(3);
			edgeOwner_[cellI].setSize(3);
			//label Face = cellsFaces()[cellI][0];
			const labelList& faceEdge = cellsFaces_[cellI];
			forAll(faceEdge, edgeI) {
				label tmpEdge = faceEdge[edgeI];
				const labelList& edgeFace = edgeFaces()[tmpEdge];
				forAll(edgeFace, faceI) {
					isAdjFace = 0;
					zzero = 0;
					znot = 0;
					label tmpFaceLabel = edgeFace[faceI];
					face tmpFace = faces()[tmpFaceLabel];
					forAll(tmpFace, pointI) {
						point tmpPoint = pointsList[tmpFace[pointI]];
						if(tmpPoint.z() != 0)
							znot =  1;
						if(tmpPoint.z() == 0)
							zzero = 1;

					}
					isAdjFace = znot && zzero;
					if(isAdjFace) {
						//dgFaceSize_ ++;
						//dgEdgeToFoamEdge_.append(tmpEdge);
						label faceowner = faceOwner()[tmpFaceLabel];
						edgeOwner_[cellI][edgeI] = faceowner;
						if(tmpFaceLabel < faceNeighbour().size())
							edgeNeighbour_[cellI][edgeI] = (faceNeighbour()[tmpFaceLabel] == cellI)?
							                               faceowner
							                               :faceNeighbour()[tmpFaceLabel];
						else
							{
							edgeNeighbour_[cellI][edgeI] = -1;
							//dgEdgeToFoamEdge_.append( tmpEdge);
							//	dgFaceSize_ ++;
							}
						if(edgeOwner_[cellI][edgeI] != edgeNeighbour_[cellI][edgeI])
							{
								
								vecdgEdgeToFoamEdge_.push_back( tmpEdge);
								dgFaceSize_ ++;
								vecdgFaceNeighbour_.push_back(edgeNeighbour_[cellI][edgeI]);
								vecdgFaceOwner_.push_back(faceowner);
						}
						
						break;
					}
				}
			}
			
		} else
			FatalErrorIn("dgPolyMesh::dgPolyMesh()")
			        << "not implement yet for "
			        <<cellShapes[cellI].model().name() << " in dg"<<endl
			        << abort(FatalError);
	}
	listAppend(dgEdgeToFoamEdge_, vecdgEdgeToFoamEdge_);
	listAppend(dgFaceNeighbour_, vecdgFaceNeighbour_);
	listAppend(dgFaceOwner_, vecdgFaceOwner_);
	vecdgEdgeToFoamEdge_.clear();
	vecdgFaceNeighbour_.clear();
	vecdgFaceOwner_.clear();
}

void Foam::dgPolyMesh::reorderCellPoints(int dim)
{
	const pointField& _polyPoints = points();

	const cellShapeList& cellShapes = this->cellShapes();
	if(dim == 3) {
		forAll(cellsPoints_, cellI) {
			if(cellShapes[cellI].model().name() == "hex" ) {
				FatalErrorIn("dgPolyMesh::dgPolyMesh()")
				        << "not implement yet for reordercellPoints "
				        <<cellShapes[cellI].model().name() << " in dg"<<endl
				        << abort(FatalError);
			} else if(cellShapes[cellI].model().name() == "prism" ) {
				FatalErrorIn("dgPolyMesh::dgPolyMesh()")
				        << "not implement yet for reordercellPoints "
				        <<cellShapes[cellI].model().name() << " in dg"<<endl
				        << abort(FatalError);
			} else if(cellShapes[cellI].model().name() == "tet") {

				dgPoints_[cellI].setSize(cellsPoints_[cellI].size());

				vector A(_polyPoints[cellsPoints_[cellI][1]] - _polyPoints[cellsPoints_[cellI][0]]);//vector 0->1
				vector B(_polyPoints[cellsPoints_[cellI][2]] - _polyPoints[cellsPoints_[cellI][0]]);//vector 0->2
				vector C(_polyPoints[cellsPoints_[cellI][3]] - _polyPoints[cellsPoints_[cellI][0]]);//vector 0->3
				vector D(A^B);//vector cross product, normal of face 012
				//use inner product to determin the anticlockwise
				if( (C&D) < 0) {
					label swap = cellsPoints_[cellI][2];
					cellsPoints_[cellI][2] = cellsPoints_[cellI][1];
					cellsPoints_[cellI][1] = swap;
				}

				dgPoints_[cellI][0] = _polyPoints[cellsPoints_[cellI][0]];
				dgPoints_[cellI][1] = _polyPoints[cellsPoints_[cellI][1]];
				dgPoints_[cellI][2] = _polyPoints[cellsPoints_[cellI][2]];
				dgPoints_[cellI][3] = _polyPoints[cellsPoints_[cellI][3]];



			} else
				FatalErrorIn("dgPolyMesh::dgPolyMesh()")
				        << "not implement yet for "
				        <<cellShapes[cellI].model().name() << " in dg"<<endl
				        << abort(FatalError);




		}

	} else if(dim == 2) {
		forAll(cellsPoints_, cellI) {
			dgPoints_[cellI].setSize(cellsPoints_[cellI].size());
			if(cellShapes[cellI].model().name() == "hex" ) {
				vector A(_polyPoints[cellsPoints_[cellI][1]] - _polyPoints[cellsPoints_[cellI][0]]);
				vector B(_polyPoints[cellsPoints_[cellI][3]] - _polyPoints[cellsPoints_[cellI][0]]);
				vector C(A^B);
				vector cellCenter(cellCentres()[cellI]);
				vector faceCenter(faceCentres()[cellFaceLabels_[cellI]]);
				//vector D(faceCenter - cellCenter);
				vector D(0, 0, 1.0);

				//use vector cross product to determin the anticlockwise
				//label anticlockwise = ((A[0]*B[1] - A[1]*B[0]) < 0) && (signZ<0) || ((A[0]*B[1] - A[1]*B[0]) > 0) && (signZ>0);
				if( (C&D) < 0) {
					label swap = cellsPoints_[cellI][3];
					cellsPoints_[cellI][3] = cellsPoints_[cellI][1];
					cellsPoints_[cellI][1] = swap;


				}


				dgPoints_[cellI][0] = _polyPoints[cellsPoints_[cellI][0]];
				dgPoints_[cellI][1] = _polyPoints[cellsPoints_[cellI][1]];
				dgPoints_[cellI][2] = _polyPoints[cellsPoints_[cellI][2]];
				dgPoints_[cellI][3] = _polyPoints[cellsPoints_[cellI][3]];

			} else if(cellShapes[cellI].model().name() == "prism" ) {
				//the points of triangle should be anticlockwise
				vector A(_polyPoints[cellsPoints_[cellI][1]] - _polyPoints[cellsPoints_[cellI][0]]);
				vector B(_polyPoints[cellsPoints_[cellI][2]] - _polyPoints[cellsPoints_[cellI][0]]);
				vector C(A^B);
				vector cellCenter(cellCentres()[cellI]);
				vector faceCenter(faceCentres()[cellFaceLabels_[cellI]]);
				//vector D(faceCenter - cellCenter);
				vector D(0, 0, 1.0);
				//use vector cross product to determin the anticlockwise
				//label anticlockwise = ((A[0]*B[1] - A[1]*B[0]) < 0) && (signZ<0) || ((A[0]*B[1] - A[1]*B[0]) > 0) && (signZ>0);
				if( (C&D) < 0) {
					label swap = cellsPoints_[cellI][2];
					cellsPoints_[cellI][2] = cellsPoints_[cellI][1];
					cellsPoints_[cellI][1] = swap;
				}
				
				dgPoints_[cellI][0] = _polyPoints[cellsPoints_[cellI][0]];
				dgPoints_[cellI][1] = _polyPoints[cellsPoints_[cellI][1]];
				dgPoints_[cellI][2] = _polyPoints[cellsPoints_[cellI][2]];
				
			} else
				FatalErrorIn("dgPolyMesh::dgPolyMesh()")
				        << "not implement yet for "
				        <<cellShapes[cellI].model().name() << " in dg"<<endl
				        << abort(FatalError);
		}

	} else if(dim == 1) {
		FatalErrorIn("dgPolyMesh::dgPolyMesh()")
		        << "meshDimension is set error to be "
		        <<meshDim_ << " in file dgMeshCtrDict"<<endl
		        << abort(FatalError);

	} else
		FatalErrorIn("dgPolyMesh::dgPolyMesh()")
		        << "meshDimension is set error to be "
		        <<meshDim_ << " in file dgMeshCtrDict"<<endl
		        << abort(FatalError);

}


void Foam::dgPolyMesh::calFacesPoints(int dim)
{
	const cellShapeList& cellShapes = this->cellShapes();

	if(dim == 3) {
		forAll(facesPoints_, cellI) {
			if(cellShapes[cellI].model().name() == "hex" ) {
				const faceList& hexModelFaces = cellModeller::lookup("hex")->modelFaces();
				labelList cellpoint = cellsPoints_[cellI];
				facesPoints_[cellI].setSize(6);
				forAll(facesPoints_[cellI], faceI) {

					facesPoints_[cellI][faceI].setSize(4);
					forAll(facesPoints_[cellI][faceI],pointI) {
						facesPoints_[cellI][faceI][pointI] = cellpoint[hexModelFaces[faceI][pointI]];
					}
				}

			} else if(cellShapes[cellI].model().name() == "prism" ) {
				const faceList& hexModelFaces = cellModeller::lookup("prism")->modelFaces();
				labelList cellpoint = cellsPoints_[cellI];
				facesPoints_[cellI].setSize(5);
				forAll(facesPoints_[cellI], faceI) {
					if(faceI<2) {
						facesPoints_[cellI][faceI].setSize(3);
						forAll(facesPoints_[cellI][faceI],pointI) {
							facesPoints_[cellI][faceI][pointI] = cellpoint[hexModelFaces[faceI][pointI]];
						}
					} else {
						facesPoints_[cellI][faceI].setSize(4);
						forAll(facesPoints_[cellI][faceI],pointI) {
							facesPoints_[cellI][faceI][pointI] = cellpoint[hexModelFaces[faceI][pointI]];
						}

					}

				}

			} else if(cellShapes[cellI].model().name() == "tet" ) {
				const faceList& hexModelFaces = cellModeller::lookup("tet")->modelFaces();
				const labelList& cellpoint = cellsPoints_[cellI];
				facesPoints_[cellI].setSize(4);
				forAll(facesPoints_[cellI], faceI) {

					facesPoints_[cellI][faceI].setSize(3);
					forAll(facesPoints_[cellI][faceI],pointI) {
						facesPoints_[cellI][faceI][pointI] = cellpoint[hexModelFaces[faceI][pointI]];
					}
				}

			} else
				FatalErrorIn("dgPolyMesh::dgPolyMesh()")
				        << "not implement yet for "
				        <<cellShapes[cellI].model().name() << " in dg"<<endl
				        << abort(FatalError);

		}

	} else if(dim == 2) {
		forAll(facesPoints_, cellI) {

			if(cellShapes[cellI].model().name() == "hex" ) {
				facesPoints_[cellI].setSize(4);
				const labelList& cellpoint = cellsPoints_[cellI];
				forAll(facesPoints_[cellI],faceI) {
					facesPoints_[cellI][faceI].setSize(2);
				}
				facesPoints_[cellI][0][0] = cellpoint[0];
				facesPoints_[cellI][0][1] = cellpoint[1];
				facesPoints_[cellI][1][0] = cellpoint[1];
				facesPoints_[cellI][1][1] = cellpoint[2];
				facesPoints_[cellI][2][0] = cellpoint[2];
				facesPoints_[cellI][2][1] = cellpoint[3];
				facesPoints_[cellI][3][0] = cellpoint[3];
				facesPoints_[cellI][3][1] = cellpoint[0];


			} else if(cellShapes[cellI].model().name() == "prism") {
				facesPoints_[cellI].setSize(3);
				const labelList& cellpoint = cellsPoints_[cellI];
				forAll(facesPoints_[cellI],faceI) {
					facesPoints_[cellI][faceI].setSize(2);
				}
				facesPoints_[cellI][0][0] = cellpoint[0];
				facesPoints_[cellI][0][1] = cellpoint[1];
				facesPoints_[cellI][1][0] = cellpoint[1];
				facesPoints_[cellI][1][1] = cellpoint[2];
				facesPoints_[cellI][2][0] = cellpoint[2];
				facesPoints_[cellI][2][1] = cellpoint[0];

			} else
				FatalErrorIn("dgPolyMesh::dgPolyMesh()")
				        << "not implement yet for "
				        <<cellShapes[cellI].model().name() << " in dg"<<endl
				        << abort(FatalError);

		}

	} else {
		forAll(facesPoints_, cellI) {

			facesPoints_[cellI].setSize(0);
		}


	}



}

void Foam::dgPolyMesh::reorderCellsFaces(int dim)
{
	const cellShapeList& cellShapes = this->cellShapes();
	int tetIndex[7] = {-1,-1,-1,3,2,1,0};
	int hexIndex[4] = {3,0,2,1};
	int prismIndex[4] = {-1,0,2,1};

	if(dim == 3) {
		forAll(cellsFaces_,cellI) {

			cell tmpCellsFaces(cellsFaces_[cellI]);
			if(cellShapes[cellI].model().name() == "tet" ) {
				forAll(cellsFaces_[cellI],faceI) {
					label cellPointsNewIndexSum = 0;

					label faceLabel = cellsFaces_[cellI][faceI];
					const labelList& pointList = faces()[faceLabel];
					forAll(pointList,pointI) {
						label cellPointsNewIndex = -1;
						forAll(cellsPoints_[cellI],pointJ) {
							cellPointsNewIndex ++;
							if(cellsPoints_[cellI][pointJ] == pointList[pointI])
								break;

						}
						cellPointsNewIndexSum +=cellPointsNewIndex;
					}

					tmpCellsFaces[tetIndex[cellPointsNewIndexSum]] = cellsFaces_[cellI][faceI];
				}
				cellsFaces_[cellI] = tmpCellsFaces;
			} else
				FatalErrorIn("dgPolyMesh::dgPolyMesh()")
				        << "not implement yet for "
				        <<cellShapes[cellI].model().name() << " in dg"<<endl
				        << abort(FatalError);
		}
	} else if(dim == 2) {
		forAll(cellsFaces_,cellI) {

			cell tmpCellsFaces(cellsFaces_[cellI]);
			if(cellShapes[cellI].model().name() == "hex" ) {
				forAll(cellsFaces_[cellI],faceI) {
					label cellPointsNewIndexSum = 0;

					label edgeLabel = cellsFaces_[cellI][faceI];
					const edge& pointList = edges()[edgeLabel];
					forAll(pointList,pointI) {
						label cellPointsNewIndex = -1;
						forAll(cellsPoints_[cellI],pointJ) {
							cellPointsNewIndex ++;
							if(cellsPoints_[cellI][pointJ] == pointList[pointI])
								break;

						}
						cellPointsNewIndexSum += (cellPointsNewIndex %3);
					}

					tmpCellsFaces[hexIndex[cellPointsNewIndexSum]] = cellsFaces_[cellI][faceI];
				}
				cellsFaces_[cellI] = tmpCellsFaces;

			} else if(cellShapes[cellI].model().name() == "prism") {
				forAll(cellsFaces_[cellI],faceI) {
					label cellPointsNewIndexSum = 0;

					label edgeLabel = cellsFaces_[cellI][faceI];
					const edge& pointList = edges()[edgeLabel];
					forAll(pointList,pointI) {
						label cellPointsNewIndex = -1;
						forAll(cellsPoints_[cellI],pointJ) {
							cellPointsNewIndex ++;
							if(cellsPoints_[cellI][pointJ] == pointList[pointI])
								break;

						}
						cellPointsNewIndexSum +=cellPointsNewIndex;
					}

					tmpCellsFaces[prismIndex[cellPointsNewIndexSum]] = cellsFaces_[cellI][faceI];
				}
				cellsFaces_[cellI] = tmpCellsFaces;

			} else
				FatalErrorIn("dgPolyMesh::dgPolyMesh()")
				        << "not implement yet for "
				        <<cellShapes[cellI].model().name() << " in dg"<<endl
				        << abort(FatalError);


		}
	} else if(dim ==1 ) {
	} else
		FatalErrorIn("dgPolyMesh::dgPolyMesh()")
		        << "meshDimension is set error to be "
		        <<meshDim_ << " in file dgMeshCtrDict"<<endl
		        << abort(FatalError);

}

void Foam::dgPolyMesh::calNeighbourFace(int dim)
{

	if(meshDim_ == 3) {
		labelList faceIndexInOwner(nFaces(),-1);
		labelList faceIndexInNeighbour(nFaces(),-1);
		const labelList& faceOwners = faceOwner();
		const cellList& cellsList=cellsFaces();

		forAll(faceOwners,faceI) {
			label cellI = faceOwners[faceI];
			forAll(cellsList[cellI],faceJ) {
				faceIndexInOwner[faceI]++;
				if(cellsList[cellI][faceJ] == faceI) {

					break;
				}
			}
		}
		const labelList& faceNeighbours = faceNeighbour();
		forAll(faceNeighbours,faceI) {
			label cellI = faceNeighbours[faceI];
			forAll(cellsList[cellI],faceJ) {
				faceIndexInNeighbour[faceI]++;
				if(cellsList[cellI][faceJ] == faceI) {

					break;
				}
			}
		}

		forAll(cellsList,cellI)
		neighbourFace_[cellI].setSize(cellsList[cellI].size());

		forAll(faceIndexInOwner,faceI) {
			label cellI = faceOwners[faceI];
			neighbourFace_[cellI][faceIndexInOwner[faceI]] = faceIndexInNeighbour[faceI];
		}

		forAll(faceNeighbours,faceI) {
			label cellI = faceNeighbours[faceI];
			neighbourFace_[cellI][faceIndexInNeighbour[faceI]] = faceIndexInOwner[faceI];
		}


	} else if(dim == 2) {


		calEdgeNeighbour();

		const faceList& facesList = faces();
		//const labelListList& faceEdgesList = faceEdges();
		label edgeAddr = -1;
		forAll(neighbourFace_, cellI) {
			//label faceLabel = cellsFaces_[cellI][0];
			const labelList& edgeList = cellsFaces_[cellI];
			forAll(edgeList, edgeI) {
				edgeAddr = -1;
				label neighbour = edgeNeighbour_[cellI][edgeI];
				//label edgeLabel = faceEdgesList[faceLabel][edgeI];
				label edgeLabel = edgeList[edgeI];
				if(neighbour == -1)
					neighbourFace_[cellI][edgeI] = -1;
				else {
					//label neighbourFace = cellsFaces_[neighbour][0];
					//const labelList& edgeList = faceEdgesList[neighbourFace];
					const labelList& edgeList2 = cellsFaces_[neighbour];
					forAll(edgeList2, edgeJ) {
						edgeAddr++;
						if(edgeList2[edgeJ] == edgeLabel) {
							neighbourFace_[cellI][edgeI] = edgeAddr++;
							break;
						}

					}

				}

			}

		}

	} else

		FatalErrorIn("dgPolyMesh::dgPolyMesh()")
		        << "meshDimension is set error to be "
		        <<meshDim_ << " in file dgMeshCtrDict"<<endl
		        << abort(FatalError);

}

void Foam::dgPolyMesh::calFirstPointIndex(int dim)
{
	if(dim == 3) {
		forAll(firstPointIndex_,cellI) {
			firstPointIndex_[cellI].setSize(cellsFaces_[cellI].size());
			label index = -1;
			forAll(firstPointIndex_[cellI],faceI) {
				label faceLabel = cellsFaces_[cellI][faceI];
				label neighbourFaceIndex = neighbourFace_[cellI][faceI];
				if(neighbourFaceIndex== -1)
					{
					firstPointIndex_[cellI][faceI] = -1;
					Info<<"boundary mesh from dgpolymesh:"<<boundaryMesh()<<endl;
				}
				else {
					label neighbourCellLabel = faceNeighbour()[faceLabel];
					if(neighbourCellLabel == cellI)
						neighbourCellLabel = faceOwner()[faceLabel];
					const labelList& neighbourFacePoint = facesPoints_[neighbourCellLabel][neighbourFaceIndex];
					label firstPointLabel = facesPoints_[cellI][faceI][0];
					index = -1;
					forAll(neighbourFacePoint,pointI) {
						index ++ ;
						if(firstPointLabel == neighbourFacePoint[pointI])
							break;
					}
					firstPointIndex_[cellI][faceI] = index;

				}

			}

		}
	} else if(dim == 2) {

		forAll(firstPointIndex_,cellI) {
			firstPointIndex_[cellI].setSize(cellsFaces_[cellI].size());
			label index = -1;
			forAll(firstPointIndex_[cellI],faceI) {
				label faceLabel = cellsFaces_[cellI][faceI];
				label neighbourFaceIndex = neighbourFace_[cellI][faceI];
				if(neighbourFaceIndex== -1)
					{
						firstPointIndex_[cellI][faceI] = -1;
					}
				else {
					label neighbourCellLabel = edgeNeighbour_[cellI][faceI];
					const labelList& neighbourFacePoint = facesPoints_[neighbourCellLabel][neighbourFaceIndex];
					label firstPointLabel = facesPoints_[cellI][faceI][0];
					index = -1;
					forAll(neighbourFacePoint,pointI) {
						index ++ ;
						if(firstPointLabel == neighbourFacePoint[pointI])
							break;
					}
					firstPointIndex_[cellI][faceI] = index;

				}

			}

		}


				


	} else

		FatalErrorIn("dgPolyMesh::dgPolyMesh()")
		        << "meshDimension is set error to be "
		        <<meshDim_ << " in file dgMeshCtrDict"<<endl
		        << abort(FatalError);


}

void Foam::dgPolyMesh::calDgFaceNeighbourID()
{
	if(meshDim_ == 3) {
		forAll(dgFaceNeighbourID_,cellI) {
			dgFaceNeighbourID_[cellI].setSize(cellsFaces_[cellI].size());
			forAll(cellsFaces_[cellI],faceI) {
				label faceLabel = cellsFaces_[cellI][faceI];
				label neighbourFaceIndex = neighbourFace_[cellI][faceI];
				if(neighbourFaceIndex== -1)
					dgFaceNeighbourID_[cellI][faceI] = -1;
				else {
					label neighbourCellLabel = faceNeighbour()[faceLabel];
					if(neighbourCellLabel == cellI)
						neighbourCellLabel = faceOwner()[faceLabel];

					dgFaceNeighbourID_[cellI][faceI] = neighbourCellLabel;
				}
			}

		}

	} else if(meshDim_ == 2) {

		forAll(dgFaceNeighbourID_,cellI) {
			dgFaceNeighbourID_[cellI].setSize(cellsFaces_[cellI].size());
			forAll(cellsFaces_[cellI],faceI) {
				label faceLabel = cellsFaces_[cellI][faceI];
				label neighbourFaceIndex = neighbourFace_[cellI][faceI];
				if(neighbourFaceIndex== -1)
					dgFaceNeighbourID_[cellI][faceI] = -1;
				else {
					label neighbourCellLabel = edgeNeighbour_[cellI][faceI];
					dgFaceNeighbourID_[cellI][faceI] = neighbourCellLabel;
				}
			}

		}


	}
}


void Foam::dgPolyMesh::initialDgFace(){
		if(meshDim_ == 3){
			
			faceIndexInNeighbour_.setSize(dgFaceSize_);
			faceIndexInOwner_.setSize(dgFaceSize_);
			facePointFirstIndex_.setSize(dgFaceSize_);
			
			
			forAll(dgFaceNeighbour_, faceI){
				
				label Owner = dgFaceOwner_[faceI];
				
				//label index=-1;
				forAll( cellsFaces_[Owner], faceJ){
					label faceLabel = cellsFaces()[Owner][faceJ];
					//index ++ ;
					if(faceLabel == faceI)
					{	faceIndexInOwner_[faceI] = faceJ;
						facePointFirstIndex_[faceI] = firstPointIndex()[Owner][faceJ];
						break;
						}
				}
				
				label neighbour = dgFaceNeighbour_[faceI];
				//index = -1;
				if(neighbour != -1){
				forAll( cellsFaces_[neighbour], faceJ){
					label faceLabel = cellsFaces_[neighbour][faceJ];
					//index ++ ;
					if(faceLabel == faceI)
					{	faceIndexInNeighbour_[faceI] = faceJ;
						
						break;
						}
				}
					}
				else
					faceIndexInNeighbour_[faceI] = -1;
				

			}
				

		}
	else if(meshDim_ == 2){
			
			faceIndexInNeighbour_.setSize(dgFaceSize_);
			faceIndexInOwner_.setSize(dgFaceSize_);
			facePointFirstIndex_.setSize(dgFaceSize_);
			forAll(faceIndexInOwner_, faceI){
				label dgEdgelabel = dgEdgeToFoamEdge_[faceI];
				label owner = dgFaceOwner_[faceI];
				//label index = -1;
				forAll( cellsFaces_[owner], faceJ){
					label foamEdgeLabel = cellsFaces()[owner][faceJ];
					//index ++ ;
					if(foamEdgeLabel == dgEdgelabel)
					{	faceIndexInOwner_[faceI] = faceJ;
						dgCellFaceNewID_[owner][faceJ] = faceI;
						break;
						}
				}
				
				label neighbour = dgFaceNeighbour_[faceI];
				//index = -1;
				if(neighbour != -1){
				forAll( cellsFaces_[neighbour], faceJ){
					label foamEdgeLabel = cellsFaces()[neighbour][faceJ];
					//index ++ ;
					if(foamEdgeLabel == dgEdgelabel)
					{	faceIndexInNeighbour_[faceI] = faceJ;
						dgCellFaceNewID_[neighbour][faceJ] = faceI;
						break;
						}
				}
					}
					else
					faceIndexInNeighbour_[faceI] = -1;

					
				

			}
		}
				
	else
		FatalErrorIn("dgPolyMesh::dgPolyMesh()")
		        << "meshDimension is set error to be "
		        <<meshDim_ << " in file dgMeshCtrDict"<<endl
		        << abort(FatalError);

}


void Foam::dgPolyMesh::foamToDG()
{
	List<point> tmp;
	const pointField& polyPoints = points();
	dgFaces_.setSize(dgFaceSize_);

	if(meshDim_ == 3){
		forAll(dgFaces_, faceI){
			shared_ptr<dgPolyFace> tmpFace = make_shared<dgPolyFace>();

			tmp.setSize(faces()[faceI].size());
			forAll(tmp, pointI){
				tmp[pointI] = polyPoints[faces()[faceI][pointI]];
			}
			tmpFace->setnPoints(tmp.size());
			tmpFace->setPoints(tmp);
			label neighbourI = faceNeighbour()[faceI];

			if(neighbourI < 0)
				tmpFace->setNeighbour(NULL);
			else
				tmpFace->setNeighbour(dgCells_[neighbourI]);

			label ownerI = faceOwner()[faceI];
			tmpFace->setOwner(dgCells_[ownerI]);

			tmpFace->setNeighbourIndex(faceIndexInNeighbour_[faceI]);
			tmpFace->setOwnerIndex(faceIndexInOwner_[faceI]);

			dgFaces_[faceI] = make_shared<dgTreeUnit<dgPolyFace>>(tmpFace);
		}

		forAll(dgCells_, cellI){
			shared_ptr<dgPolyCell> tmpCell = make_shared<dgPolyCell>();
			tmpCell->setMeshDim(meshDim_);
			tmpCell->setCellShape(cellShapes_[cellI]);

			tmpCell->setnPoints(dgPoints_[cellI].size());
			tmpCell->setPoints(dgPoints_[cellI]);
			
			labelList& cellFace = cellsFaces_[cellI];

			tmpCell->setnFaces(cellFace.size());

			forAll(cellFace, faceI){
				label faceIndex = cellFace[faceI];
				tmpCell->setDgFaces(faceI, make_shared<dgPolyFace>(dgFaces_[faceIndex]->value()));
				tmpCell->setFirstPointLabel(faceI, firstPointIndex_[cellI][faceI]);

				label ownerI = faceOwner()[faceIndex];
				tmpCell->setDgFaceOwnerPtr(faceI, dgCells_[ownerI]);

				label neighbourI = faceNeighbour()[faceIndex];
				if(neighbourI < 0)
					tmpCell->setDgFaceNeighbourPtr(faceI, NULL);
				else
					tmpCell->setDgFaceNeighbourPtr(faceI, dgCells_[neighbourI]);

				tmpCell->setNeighbourFaceLabel(faceI, faceIndexInNeighbour_[faceIndex]);
			}
			dgCells_[cellI] = make_shared<dgTreeUnit<dgPolyCell>>(tmpCell);
		}
	}
	else if(meshDim_ == 2){
		forAll(dgFaces_, faceI){
			shared_ptr<dgPolyFace> tmpFace = make_shared<dgPolyFace>();

			label faceID = dgEdgeToFoamEdge_[faceI];
			tmp.setSize(edges()[faceID].size());
			forAll(tmp, pointI){
				tmp[pointI] = polyPoints[edges()[faceID][pointI]];
			}
			tmpFace->setnPoints(tmp.size());
			tmpFace->setPoints(tmp);
			label neighbourI = dgFaceNeighbour_[faceI];
			if(neighbourI < 0)
				tmpFace->setNeighbour(NULL);
			else
				tmpFace->setNeighbour(dgCells_[neighbourI]);

			label ownerI = dgFaceOwner_[faceI];
			tmpFace->setOwner(dgCells_[ownerI]);

			tmpFace->setNeighbourIndex(faceIndexInNeighbour_[faceI]);
			tmpFace->setOwnerIndex(faceIndexInOwner_[faceI]);

			dgFaces_[faceI] = make_shared<dgTreeUnit<dgPolyFace>>(tmpFace);
		}

		forAll(dgCells_, cellI){
			shared_ptr<dgPolyCell> tmpCell = make_shared<dgPolyCell>();
			tmpCell->setMeshDim(meshDim_);
			tmpCell->setCellShape(cellShapes_[cellI]);

			tmpCell->setnPoints(dgPoints_[cellI].size());
			tmpCell->setPoints(dgPoints_[cellI]);
			
			labelList& cellFace = dgCellFaceNewID_[cellI];

			tmpCell->setnFaces(cellFace.size());

			forAll(cellFace, faceI){
				label faceIndex = cellFace[faceI];
				tmpCell->setDgFaces(faceI, make_shared<dgPolyFace>(dgFaces_[faceIndex]->value()));
				tmpCell->setFirstPointLabel(faceI, firstPointIndex_[cellI][faceI]);

				label ownerI = dgFaceOwner_[faceIndex];
				tmpCell->setDgFaceOwnerPtr(faceI, dgCells_[ownerI]);

				label neighbourI = dgFaceNeighbour_[faceIndex];
				if(neighbourI < 0)
					tmpCell->setDgFaceNeighbourPtr(faceI, NULL);
				else
					tmpCell->setDgFaceNeighbourPtr(faceI, dgCells_[neighbourI]);

				tmpCell->setNeighbourFaceLabel(faceI, faceIndexInNeighbour_[faceIndex]);
			}
			dgCells_[cellI] = make_shared<dgTreeUnit<dgPolyCell>>(tmpCell);
		}
	}
	else if(meshDim_ == 1) {
		FatalErrorIn("dgPolyMesh::dgPolyMesh()")
		        << "meshDimension is set error to be "
		        <<meshDim_ << " in file dgMeshCtrDict"<<endl
		        << abort(FatalError);

	}
	else
		FatalErrorIn("dgPolyMesh::dgPolyMesh()")
		        << "meshDimension is set error to be "
		        <<meshDim_ << " in file dgMeshCtrDict"<<endl
		        << abort(FatalError);
	
	faceTree_.initial(dgFaceSize(), dgFaces_);

	cellTree_.initial(nCells(), dgCells_);
	
}
