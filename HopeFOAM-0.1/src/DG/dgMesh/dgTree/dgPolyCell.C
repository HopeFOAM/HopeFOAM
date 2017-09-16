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

#include "dgPolyCell.H"

using std::shared_ptr;
using std::make_shared;

Foam::dgPolyCell::dgPolyCell()
:
    meshDim_(2),
    nPoints_(0),
    points_(0),
    dgFacesPtr_(0),

    firstPointLabel_(0),
    dgFaceNeighbourPtr_(0),
    dgFaceOwnerPtr_(0),
    neighbourFaceLabel_(0)

{

}

Foam::dgPolyCell::dgPolyCell(const dgPolyCell& cell)
:
    meshDim_(cell.meshDim()),
    nPoints_(cell.nPoints()),
    points_(cell.points()),
    dgFacesPtr_(cell.faces()),

    firstPointLabel_(cell.firstPointLabel()),
    dgFaceNeighbourPtr_(cell.dgFaceNeighbourPtr()),
    dgFaceOwnerPtr_(cell.dgFaceOwnerPtr()),
    neighbourFaceLabel_(cell.neighbourFaceLabel())

{

}

void Foam::dgPolyCell::setCellShape(const word cellShape)
{
    cellShape_ = cellShape;
}

void Foam::dgPolyCell::setMeshDim(const label meshDim)
{
    meshDim_ = meshDim;
}

void Foam::dgPolyCell::setnPoints(const label nPoints)
{
    nPoints_ = nPoints;
}

void Foam::dgPolyCell::setPoints(const List<point>& points)
{
    points_ = points;
}

void Foam::dgPolyCell::setnFaces(const label nFaces)
{
    nFaces_ = nFaces;
    dgFacesPtr_.setSize(nFaces);
    dgFaceOwnerPtr_.setSize(nFaces);
    dgFaceNeighbourPtr_.setSize(nFaces);

    forAll(dgFaceNeighbourPtr_, faceI)
    {
        dgFaceNeighbourPtr_[faceI] = NULL;
        dgFaceOwnerPtr_[faceI] = NULL;
    }

    firstPointLabel_.setSize(nFaces);
    neighbourFaceLabel_.setSize(nFaces);
}

void Foam::dgPolyCell::setDgFaces(const label faceI, shared_ptr<dgPolyFace> dgFaceI)
{
    if(dgFacesPtr_.size() <= faceI){
        FatalErrorIn("dgPolyCell::dgPolyCell::setDgFaces(const label faceI, const dgPolyFace dgFaceI)")
                        << "the size of dgFacesPtr_ is less than "
                        <<faceI<< " in dgPolyCell"<<endl
                        << abort(FatalError);
    }
    dgFacesPtr_[faceI] = dgFaceI;
}

void Foam::dgPolyCell::setFirstPointLabel(const label faceI,  label firstLabel)
{
    if(firstPointLabel_.size() <= faceI){
        FatalErrorIn("dgPolyCell::dgPolyCell::setFirstPointLabel(const label faceI, const label firstLabel)")
                        << "the size of firstPointLabel_ is less than "
                        <<faceI<< " in dgPolyCell"<<endl
                        << abort(FatalError);
    }
    firstPointLabel_[faceI] = firstLabel;
}

void Foam::dgPolyCell::setDgFaceNeighbourPtr(const label faceI,  shared_ptr<dgTreeUnit<dgPolyCell>> cellPtr)
{
    if(dgFaceNeighbourPtr_.size() <= faceI){
        FatalErrorIn("dgPolyCell::setDgFaceNeighbourPtr(const label faceI, const dgPolyCell* cellPtr)")
                        << "the size of dgFaceNeighbourPtr_ is less than "
                        <<faceI<< " in dgPolyCell"<<endl
                        << abort(FatalError);
    }
    dgFaceNeighbourPtr_[faceI] = cellPtr;
}

void Foam::dgPolyCell::setDgFaceOwnerPtr(const label faceI,  shared_ptr<dgTreeUnit<dgPolyCell>> cellPtr)
{
    if(dgFaceNeighbourPtr_.size() <= faceI){
        FatalErrorIn("dgPolyCell::setDgFaceOwnerPtr(const label faceI, const dgPolyCell* cellPtr)")
                        << "the size of dgFaceOwnerPtr_ is less than "
                        <<faceI<< " in dgPolyCell"<<endl
                        << abort(FatalError);
    }
    dgFaceOwnerPtr_[faceI] = cellPtr;
}

void Foam::dgPolyCell::setNeighbourFaceLabel(const label faceI,  label neighbourFaceLabel)
{
    if(neighbourFaceLabel_.size() <= faceI){
        FatalErrorIn("dgPolyCell::setNeighbourFaceLabel(const label faceI, const label neighbourFaceLabel)")
                        << "the size of neighbourFaceLabel_ is less than "
                        <<faceI<< " in dgPolyCell"<<endl
                        << abort(FatalError);
    }
    neighbourFaceLabel_[faceI] = neighbourFaceLabel;
}

const Foam::List<Foam::dgPolyCell>& Foam::dgPolyCell::refine() const
{
    //refine the cell and return the sub cell list
}