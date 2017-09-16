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
    Foam::dgPolyFace

Description
    Polyhedral faces for DG.

SourceFiles
    dgPolyFace.C

Author
    Feng Yongquan (yqfeng0418@163.com)

\*---------------------------------------------------------------------------*/

#ifndef dgPolyFace_H
#define dgPolyFace_H

#include "point.H"
#include "labelList.H"
#include "dgPolyCell.H"
#include "dgTreeUnit.H"
#include <memory>

using std::shared_ptr;
using std::make_shared;

namespace Foam
{

class dgPolyCell;

/*---------------------------------------------------------------------------*\
                          Class dgPolyFace Declaration
\*---------------------------------------------------------------------------*/

class dgPolyFace
{
private:
    //the dimension of the cell of the face
    //label meshDim_;

    //the number of point in this face
    label nPoints_;

    //the coordinate information list of points 
    List<point> points_; 

    //the neighobour cell point
    shared_ptr<dgTreeUnit<dgPolyCell>> neighbour_;

    //the index in the neighbour cell
    label neighbourIndex_;

    //the owner cell point
    shared_ptr<dgTreeUnit<dgPolyCell>> owner_;

    //the index in the owner cell
    label ownerIndex_;

    //the father neighbour face point, if the neighbour face is bigger than this
    shared_ptr<dgTreeUnit<dgPolyFace>> fatherNeighbour_;


public:
    ClassName("dgPolyFace");

    //Constructors
    dgPolyFace();

    dgPolyFace(dgPolyFace&);

    dgPolyFace(const dgPolyFace&);

    virtual ~dgPolyFace(){}

    //member function
    //Access the variable

    inline const label nPoints() const
    {
        return nPoints_;
    }

    inline const List<point>& points() const
    {
        return points_;
    }

    inline shared_ptr<dgTreeUnit<dgPolyCell>> neighbour() const
    {
        return neighbour_;
    }

    inline label neighbourIndex() const
    {
        return neighbourIndex_;
    }

    inline shared_ptr<dgTreeUnit<dgPolyCell>> owner() const
    {
        return owner_;
    }

    inline label ownerIndex() const
    {
        return ownerIndex_;
    }

    inline shared_ptr<dgTreeUnit<dgPolyFace>> fatherNeighbour() const
    {
        return fatherNeighbour_;
    }

    //Set the value for the variable

    void setnPoints(const label);

    void setPoints(const List<point>&);

    void setNeighbour(shared_ptr<dgTreeUnit<dgPolyCell>>);

    void setNeighbourIndex(const label);

    void setOwner(shared_ptr<dgTreeUnit<dgPolyCell>>);

    void setOwnerIndex(const label);

    void setFatherNeighbour(shared_ptr<dgTreeUnit<dgPolyFace>>);

    //refine the face
    const List<dgPolyFace>& refine() const;

};


}


#endif
