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
    Foam::dgPolyCell

Description
    Polyhedral cells for DG.

SourceFiles
    dgPolyCell.C

Author
    Feng Yongquan (yqfeng0418@163.com)

\*---------------------------------------------------------------------------*/

#ifndef dgPolyCell_H
#define dgPolyCell_H


#include "point.H"
#include "labelList.H"
#include "dgPolyFace.H"
#include "dgTreeUnit.H"

using std::shared_ptr;
using std::make_shared;

//  * * * * * * * * * * * * * * * * * *  * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

class dgPolyFace;
/*---------------------------------------------------------------------------*\
                          Class dgPolyCell Declaration
\*---------------------------------------------------------------------------*/

class dgPolyCell
{
private:
    //shape of cell 
    word cellShape_;

    // the dimension of the cell, permanent data
    label meshDim_;

    //the number of point in this cell
    label nPoints_;

    //the coordinate information list of points   
    List<point> points_;                          

    //the number of face in this cell
    label nFaces_;

    //the face of this cell
    List<shared_ptr<dgPolyFace>> dgFacesPtr_;

    //the edge of this cell
    //List<edge> edges_;

    //label of point address in the neighbor faces, thus rotate the nbumber
    labelList firstPointLabel_;

    //neighbour cell point of face point by face
    List<shared_ptr<dgTreeUnit<dgPolyCell>>> dgFaceNeighbourPtr_;

    //owner cell point of face point by face
    List<shared_ptr<dgTreeUnit<dgPolyCell>>> dgFaceOwnerPtr_;

    //the label of the face in the neighbor cell , edge when 2D
    labelList neighbourFaceLabel_;

public:
    ClassName("dgPolyCell");

    //Constructors
    dgPolyCell();

    dgPolyCell(const dgPolyCell&);
    //Construct from cell shape
    //dgPolyCell(const word cellShape);

    virtual ~dgPolyCell(){}

    //member function
    //Initial the value of the variable
    void setCellShape(const word);
    
    void setMeshDim(const label);

    void setnPoints(const label);

    void setPoints(const List<point>&);

    void setnFaces(const label);

    void setDgFaces(const label, shared_ptr<dgPolyFace>);

    // setEdges(const labelList&);

    void setFirstPointLabel(const label, label);

    void setDgFaceNeighbourPtr(const label, shared_ptr<dgTreeUnit<dgPolyCell>>);

    void setDgFaceOwnerPtr(const label, shared_ptr<dgTreeUnit<dgPolyCell>>);

    void setNeighbourFaceLabel(const label, label);

    //refine the cell
    const List<dgPolyCell>& refine() const;


    inline const word cellShape() const
    {
        return cellShape_;
    }

    inline const label meshDim() const
    {
        return meshDim_;
    }

    inline const label nPoints() const
    {
        return nPoints_;
    }

    inline const List<point>& points() const
    {
        return points_;
    }

    inline const label nFaces() const
    {
        return nFaces_;
    }

    inline const List<shared_ptr<dgPolyFace>>& faces() const
    {
        return dgFacesPtr_;
    }

    inline const labelList& firstPointLabel() const
    {
        return firstPointLabel_;
    }

    inline const List<shared_ptr<dgTreeUnit<dgPolyCell>>>& dgFaceNeighbourPtr() const
    {
        return dgFaceNeighbourPtr_;
    }

    inline const List<shared_ptr<dgTreeUnit<dgPolyCell>>>& dgFaceOwnerPtr() const
    {
        return dgFaceOwnerPtr_;
    }

    inline const labelList& neighbourFaceLabel() const
    {
        return neighbourFaceLabel_;
    }
};


}


#endif
