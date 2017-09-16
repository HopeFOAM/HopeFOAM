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
    Foam::processorDgMeshes

Description
    Container for processor dgMesh addressing.

SourceFiles
    processorDgMeshes.C

\*---------------------------------------------------------------------------*/

#ifndef processorDgMeshes_H
#define processorDgMeshes_H

#include "PtrList.H"
#include "dgMesh.H"
#include "IOobjectList.H"
#include "labelIOList.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{


/*---------------------------------------------------------------------------*\
                     Class processorMeshes Declaration
\*---------------------------------------------------------------------------*/

class processorDgMeshes
{
    // Private data

        const word meshName_;

        //- Processor databases
        PtrList<Time>& databases_;

        //- List of processor meshes
        PtrList<dgMesh> meshes_;

        //- List of processor point addressing lists
        PtrList<labelIOList> pointProcAddressing_;

        //- List of processor face addressing lists
        PtrList<labelIOList> faceProcAddressing_;

        //- List of processor cell addressing lists
        PtrList<labelIOList> cellProcAddressing_;

        //- List of processor boundary addressing lists
        PtrList<labelIOList> boundaryProcAddressing_;


    // Private Member Functions

        //- Read all meshes
        void read();

        //- Disallow default bitwise copy construct
        processorDgMeshes(const processorDgMeshes&);

        //- Disallow default bitwise assignment
        void operator=(const processorDgMeshes&);


public:

    // Constructors

        //- Construct from components
        processorDgMeshes(PtrList<Time>& databases, const word& meshName);


    // Member Functions

        //- Update the meshes based on the mesh files saved in time directories
        dgMesh::readUpdateState readUpdate();

        //- Reconstruct point position after motion in parallel
        void reconstructPoints(dgMesh&);

        PtrList<dgMesh>& meshes()
        {
            return meshes_;
        }

        const PtrList<labelIOList>& pointProcAddressing() const
        {
            return pointProcAddressing_;
        }

        PtrList<labelIOList>& faceProcAddressing()
        {
            return faceProcAddressing_;
        }

        const PtrList<labelIOList>& cellProcAddressing() const
        {
            return cellProcAddressing_;
        }

        const PtrList<labelIOList>& boundaryProcAddressing() const
        {
            return boundaryProcAddressing_;
        }


};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
