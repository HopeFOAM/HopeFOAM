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
    Foam::dofAddressing

Description
    Abstract class indicating the addressing rule for dof.

SourceFiles
    dofAddressing.C
    
Author
    Xu Liyang (xucloud77@gmail.com)
\*---------------------------------------------------------------------------*/

#ifndef dofAddressing_H
#define dofAddressing_H

#include "labelList.H"
#include "labelPair.H"
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

/*---------------------------------------------------------------------------*\
                       Class dofAddressing Declaration
\*---------------------------------------------------------------------------*/

class dofAddressing
{
protected:
    //- index of all dof in every cell
    labelListList cellDofIndex_;
    labelPairList cellDofIndexPair_;
    //- index of all dofs in owner faces
    labelListList ownerFaceDofIndex_;
    //- index of all dofs in neighbor faces, the dofs are arranged in the view of owner face
    labelListList neighborFaceDofIndex_;
    //- the dof start of each face in every patch
    labelListList patchDofStart_;

    //- index of all dof in every cell, pair[0]-the start point, pair[1]-the size of the list
    labelPairList gaussCellDofIndex_;
    //- index of all dof in faces of each cell
    labelPairList gaussFaceDofIndex_;

    label totalDOFs_;
    //- index of dofs in correspoding neighbor cell

public:
    //- Constructors
    dofAddressing();

    //- Destructor
    virtual ~dofAddressing();

    // Member Functions
    inline const labelListList& cellDofIndex()const{return cellDofIndex_;}
    inline const label& totalDOFs()const{return totalDOFs_;}//add by RXG
    inline const labelPairList& cellDofIndexPair()const{return cellDofIndexPair_;}
    inline const labelListList& ownerFaceDofIndex()const{return ownerFaceDofIndex_;}
    inline const labelListList& neighborFaceDofIndex()const{return neighborFaceDofIndex_;}

    inline const labelPairList& gaussCellDofIndex()const{return gaussCellDofIndex_;}
    inline const labelPairList& gaussFaceDofIndex()const{return gaussFaceDofIndex_;}
    //inline const List<labelListList>& gaussNeighborFaceDofIndex()const{return gaussNeighborFaceDofIndex_;}
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //