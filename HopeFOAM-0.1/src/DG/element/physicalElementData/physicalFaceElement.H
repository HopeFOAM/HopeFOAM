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
    Foam::physicalFaceElement

Description
    all face data, used for face integration and flux calculation

SourceFiles
    physicalFaceElement.C
    
Author
    Xu Liyang (xucloud77@gmail.com)
\*---------------------------------------------------------------------------*/
#ifndef physicalFaceElement_H
#define physicalFaceElement_H

#include "stdElement.H"
#include "physicalCellElement.H"
#include "dgTreeUnit.H"
#include "Pair.H"
#include <memory>

using std::shared_ptr;
using std::make_shared;

namespace Foam
{
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
class physicalCellElement;
/*---------------------------------------------------------------------------*\
                       Class physicalFaceElement Declaration
\*---------------------------------------------------------------------------*/

class physicalFaceElement
{
public:
	// for boundary face, true;
    bool isPatch_; 
	// this face is the i-th face in owner or neighbor cell, according to the stardard cell orientation
    label faceOwnerLocalID_, faceNeighborLocalID_; 

    shared_ptr<dgTreeUnit<physicalCellElement>> ownerEle_;
    shared_ptr<dgTreeUnit<physicalCellElement>> neighborEle_; 

    //- dof information
    //- the face rotate of neighbor to owner face
    label faceRotate_; 
	
	//- the sequence index in the boundary face dof, if not boundary face, it does not work
	//- first() the patch index, second() the dofstart in corresponding patchField
    Pair<label> sequenceIndex_; 

    //- quadrature information
    
    label nGaussPoints_; //- number of gauss interpolation points in one face
    label ownerFaceStart_, neighbourFaceStart_; //- index in dgGaussField
    label gaussOrder_; //- gauss quadrature order

    vectorList faceNx_;
    scalarList faceWJ_;//- quadrature weights for this face, index consistent with ownercell
    denseMatrix<scalar> gaussOwnerFaceDn_;
    denseMatrix<scalar> gaussNeighborFaceDn_;
    denseMatrix<scalar> gaussOwnerFaceDnRotate_;
    denseMatrix<scalar> gaussNeighborFaceDnRotate_;
    scalar fscale_;

    physicalFaceElement();
    physicalFaceElement(const physicalFaceElement&);
    ~physicalFaceElement();

    //void updateDofMapping();

    void updateSequenceIndex(Pair<label>);

    //- the dofs for the face, critically for patch field.
    //- in hp case, the dof of subFaces should be considered.
    //

    const label nOwnerDof()const;

    const label nNeighborDof()const;

    const labelList& ownerDofMapping();

    const labelList& neighborDofMapping();

    const labelList& neighborNoRotateDofMapping();

    inline const Pair<label> sequenceIndex()const{
        return sequenceIndex_;
    }

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
