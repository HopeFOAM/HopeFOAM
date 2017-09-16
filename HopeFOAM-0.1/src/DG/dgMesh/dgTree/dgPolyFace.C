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

#include "dgPolyFace.H"
#include <memory>

using std::shared_ptr;
using std::make_shared;


Foam::dgPolyFace::dgPolyFace()
:
    nPoints_(0),
    points_(0),
    neighbour_(NULL),
    neighbourIndex_(-1),
    owner_(NULL),
    ownerIndex_(-1),
    fatherNeighbour_(NULL)
{

}

Foam::dgPolyFace::dgPolyFace(dgPolyFace& face)
:
    nPoints_(face.nPoints()),
    points_(face.points()),
    neighbour_(face.neighbour()),
    neighbourIndex_(face.neighbourIndex()),
    owner_(face.owner()),
    ownerIndex_(face.ownerIndex()),
    fatherNeighbour_(face.fatherNeighbour())
{

}

Foam::dgPolyFace::dgPolyFace(const dgPolyFace& face)
:
    nPoints_(face.nPoints()),
    points_(face.points()),
    neighbour_(face.neighbour()),
    neighbourIndex_(face.neighbourIndex()),
    owner_(face.owner()),
    ownerIndex_(face.ownerIndex()),
    fatherNeighbour_(face.fatherNeighbour())
{

}

void Foam::dgPolyFace::setnPoints(const label num)
{
    nPoints_ = num;
    points_.setSize(num);
}

void Foam::dgPolyFace::setPoints(const List<point>& points)
{
    points_ = points;
}

void Foam::dgPolyFace::setNeighbour(shared_ptr<dgTreeUnit<dgPolyCell>> neighbour)
{
    neighbour_ = neighbour;
}

void Foam::dgPolyFace::setNeighbourIndex(const label index)
{
    neighbourIndex_ = index;
}

void Foam::dgPolyFace::setOwner(shared_ptr<dgTreeUnit<dgPolyCell>> owner)
{
    owner_ = owner;
}

void Foam::dgPolyFace::setOwnerIndex(const label index)
{
    ownerIndex_ = index;
}

void Foam::dgPolyFace::setFatherNeighbour(shared_ptr<dgTreeUnit<dgPolyFace>> fatherNeighbour)
{
    fatherNeighbour_ = fatherNeighbour;
}

const Foam::List<Foam::dgPolyFace>& Foam::dgPolyFace::refine() const
{
    //refine the face, return a list contain the subface splitting from this face
}