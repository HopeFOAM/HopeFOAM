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

#include "dgFieldReconstructor.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::dgFieldReconstructor::dgFieldReconstructor
(
    dgMesh& mesh,
    const PtrList<dgMesh>& procMeshes,
    const PtrList<labelIOList>& faceProcAddressing,
    const PtrList<labelIOList>& cellProcAddressing,
    const PtrList<labelIOList>& boundaryProcAddressing
)
:
    mesh_(mesh),
    procMeshes_(procMeshes),
    faceProcAddressing_(faceProcAddressing),
    cellProcAddressing_(cellProcAddressing),
    boundaryProcAddressing_(boundaryProcAddressing),
    nReconstructed_(0),
    cellPointStart_(mesh.nCells(),0)
{
    label cellI = 0;
    dgTree<physicalCellElement>& cellEleTree = mesh.cellElementsTree();
    for(dgTree<physicalCellElement>::leafIterator iter = cellEleTree.leafBegin() ; iter != cellEleTree.end() ; ++iter, ++cellI){
        physicalCellElement& cellEle = iter()->value();
        cellPointStart_[cellI] = cellEle.dofStart();
    }



    forAll(procMeshes_, proci)
    {
        const dgMesh& procMesh = procMeshes_[proci];
        if
        (
            faceProcAddressing[proci].size() != procMesh.nFaces()
         || cellProcAddressing[proci].size() != procMesh.nCells()
         || boundaryProcAddressing[proci].size() != procMesh.boundary().size()
        )
        {
            FatalErrorInFunction
                << "Size of maps does not correspond to size of mesh"
                << " for processor " << proci << endl
                << "faceProcAddressing : " << faceProcAddressing[proci].size()
                << " nFaces : " << procMesh.nFaces() << endl
                << "cellProcAddressing : " << cellProcAddressing[proci].size()
                << " nCell : " << procMesh.nCells() << endl
                << "boundaryProcAddressing : "
                << boundaryProcAddressing[proci].size()
                << " nFaces : " << procMesh.boundary().size()
                << exit(FatalError);
        }
    }
}


// ************************************************************************* //
