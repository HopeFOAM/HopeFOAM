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
    Foam::physicalElementData

Description
    Store the data of all the physical Elements
    including stdElement for each Cell, discritisation data

SourceFiles
    physicalElementData.C
    
Author
    Xu Liyang (xucloud77@gmail.com)
\*---------------------------------------------------------------------------*/

#ifndef physicalElementData_H
#define physicalElementData_H

#include "physicalCellElement.H"
#include "physicalFaceElement.H"
#include "dgBoundaryMesh.H"
#include "stdElementSets.H"
#include "dgPolyMesh.H"
#include "dgTree.H"
#include <memory>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

using std::shared_ptr;
using std::make_shared;

namespace Foam
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

/*---------------------------------------------------------------------------*\
                       Class physicalElementData Declaration
\*---------------------------------------------------------------------------*/

class physicalElementData
{

private:
	// Private data
        //- reference to dgPolyMesh
        const dgPolyMesh & mesh_;
        //- std elements pool
        stdElementSets elementSets_;
        //- list for all the initial stdElement
        List<shared_ptr<dgTreeUnit<physicalCellElement>>> cellElements_;
        //- list for all the initial dgFace
        List<shared_ptr<dgTreeUnit<physicalFaceElement>>> faceElements_;

        //- tree for all the stdElement
        dgTree<physicalCellElement> cellElementsTree_;

        //- tree for all the dgFace
        dgTree<physicalFaceElement> faceElementsTree_;

        label totalDof_;
        label totalGaussCellDof_;
        label totalGaussFaceDof_;

        label maxDofPerCell_;
        label maxGaussPerCell_;
        label maxNFacesPerCell_;
        label maxDofPerFace_;

public:

	//- Constructors
	physicalElementData(const dgPolyMesh&);

	//- Destructor
    virtual ~physicalElementData();

        //init elementsPtr_
        void initElements(label order = 2);
        void initGaussFaceNx();
        void initGaussFaceDn();
        void updatePatchDofIndexMapping(const dgBoundaryMesh& bMesh);

        //get total number of dof
        inline label totalDof() const { return totalDof_; }
        label updateTotalDof();

        //get total number of gauss dof
        inline label totalGaussCellDof() const { return totalGaussCellDof_;}
        label updateTotalGaussCellDof();
        inline label totalGaussFaceDof() const { return totalGaussFaceDof_;}
        label uptateTotalGaussFaceDof();

        //- interfaces for dgLduMatrix
        inline label maxDofPerCell()const { return maxDofPerCell_; }
        label updateMaxDofPerCell();
        inline label maxGaussPerCell()const { return maxGaussPerCell_; }
        label updateMaxGaussPerCell();
        inline label maxNFacesPerCell()const { return maxNFacesPerCell_; }
        label updateMaxNFacesPerCell();
        inline label maxDofPerFace()const { return maxDofPerFace_; }
        label updateMaxDofPerFace();


    // Data Access
        inline const dgTree<physicalCellElement>& cellElementsTree()const {return cellElementsTree_;}
        inline dgTree<physicalCellElement>& cellElementsTree() {return cellElementsTree_;}
        inline const dgTree<physicalFaceElement>& faceElementsTree()const {return faceElementsTree_;}
        inline dgTree<physicalFaceElement>& faceElementsTree() {return faceElementsTree_;}

        inline const List<shared_ptr<dgTreeUnit<physicalFaceElement>>>& faceElements()const {return faceElements_;}
        inline stdElementSets& elementSets(){return elementSets_;}

    // member functions
        //update dof ranges in cellElements and faceElements
        void updateDofMapping();

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //