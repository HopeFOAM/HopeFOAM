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
    Foam::dgBoundaryMesh

Description
    Foam::dgBoundaryMesh

SourceFiles
    dgBoundaryMesh.C

\*---------------------------------------------------------------------------*/

#ifndef dgBoundaryMesh_H
#define dgBoundaryMesh_H

#include "dgPatchList.H"
#include "lduInterfacePtrsList.H"
#include "PtrList.H"
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

class dgMesh;
class polyBoundaryMesh;

/*---------------------------------------------------------------------------*\
                       Class dgBoundaryMesh Declaration
\*---------------------------------------------------------------------------*/

class dgBoundaryMesh
:
    public dgPatchList
{
    // Private data

    //- Reference to mesh
    const dgMesh& mesh_;

	PtrList<entry>   patchEntries_;	

    // Private Member Functions

    //- Disable default copy construct
    dgBoundaryMesh(const dgBoundaryMesh&);

    //- Disallow assignment
    void operator=(const dgBoundaryMesh&);

    //- Add dgPatches corresponding to the given polyBoundaryMesh
    void addPatches(const polyBoundaryMesh&);

protected:

    //- Update boundary based on new polyBoundaryMesh
    void readUpdate(const polyBoundaryMesh&);

public:

    friend class dgMesh;


    // Constructors

    //- Construct with zero size
    dgBoundaryMesh(const dgMesh&);

    //- Construct from polyBoundaryMesh
    dgBoundaryMesh(const dgMesh&, const polyBoundaryMesh&);

    // Member Functions

        // Access
        //- Return the mesh reference
        const dgMesh& mesh() const
        {
            return mesh_;
        }

        dgMesh& mesh()
        {
            return const_cast<dgMesh&>(mesh_);
        }

        lduInterfacePtrsList interfaces() const;

        //- Find patch index given a name
        label findPatchID(const word& patchName) const;

        //- Find patch indices given a name
        labelList findIndices(const keyType&, const bool useGroups) const;

        //- set dgFaceList according to the patch Type
        void setDgFaces();

        //- Correct patches after moving points
        void movePoints();

        //- update patch corresponding dof number
        void updateTotalDofNum();

        //- update neighbor element data, especially for coupled patches
        void updateNeighborData();


	   const PtrList<entry>  patchEntries() const{return patchEntries_;}

    // Member Operators

        //- Return const and non-const reference to fvPatch by index.
        using dgPatchList::operator[];

        //- Return const reference to dgPatch by name.
        const dgPatch& operator[](const word&) const;

        dgPatch& operator[](const word&);
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
