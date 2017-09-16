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
    Foam::dgFieldReconstructor

Description
    Discontinuous Galerkin Method fields reconstruct.

SourceFiles
    dgFieldReconstructor.C
    dgFieldReconstructorReconstructFields.C

\*---------------------------------------------------------------------------*/

#ifndef dgFieldReconstructor_H
#define dgFieldReconstructor_H

#include "dgFields.H"
#include "PtrList.H"
#include "IOobjectList.H"
#include "dgPatchFieldMapper.H"
#include "labelIOList.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{


/*---------------------------------------------------------------------------*\
                    Class dgFieldReconstructor Declaration
\*---------------------------------------------------------------------------*/

class dgFieldReconstructor
{
    // Private data

        //- Reconstructed mesh reference
        dgMesh& mesh_;

        //- List of processor meshes
        const PtrList<dgMesh>& procMeshes_;

        //- List of processor face addressing lists
        const PtrList<labelIOList>& faceProcAddressing_;

        //- List of processor cell addressing lists
        const PtrList<labelIOList>& cellProcAddressing_;

        //- List of processor boundary addressing lists
        const PtrList<labelIOList>& boundaryProcAddressing_;

        //- Number of fields reconstructed
        label nReconstructed_;

        labelList cellPointStart_;

    // Private Member Functions

        //- Disallow default bitwise copy construct
        dgFieldReconstructor(const dgFieldReconstructor&);

        //- Disallow default bitwise assignment
        void operator=(const dgFieldReconstructor&);


public:

        //- Mapper for sizing only - does not do any actual mapping.
        class dgPatchFieldReconstructor
        :
            public dgPatchFieldMapper
        {
            label size_;

        public:

            // Constructors

                //- Construct given size
                dgPatchFieldReconstructor(const label size)
                :
                    size_(size)
                {}


            // Member functions

                label size() const
                {
                    return size_;
                }

                bool direct() const
                {
                    return true;
                }

                bool hasUnmapped() const
                {
                    return false;
                }

                const labelUList& directAddressing() const
                {
                    return labelUList::null();
                }
        };


    // Constructors

        //- Construct from components
        dgFieldReconstructor
        (
            dgMesh& mesh,
            const PtrList<dgMesh>& procMeshes,
            const PtrList<labelIOList>& faceProcAddressing,
            const PtrList<labelIOList>& cellProcAddressing,
            const PtrList<labelIOList>& boundaryProcAddressing
        );


    // Member Functions

        //- Return number of fields reconstructed
        label nReconstructed() const
        {
            return nReconstructed_;
        }

       //- Reconstruct volume field
        template<class Type>
        tmp<GeometricDofField<Type, dgPatchField, dgGeoMesh>>
        reconstructDgVolumeField
        (
            const IOobject& fieldIoObject,
            const PtrList<GeometricDofField<Type, dgPatchField, dgGeoMesh>>&
        ) const;

        //- Read and reconstruct volume field
        template<class Type>
        tmp<GeometricDofField<Type, dgPatchField, dgGeoMesh>>
        reconstructDgVolumeField(const IOobject& fieldIoObject) const;


      

        //- Read, reconstruct and write all/selected volume fields
        template<class Type>
        void reconstructDgVolumeFields
        (
            const IOobjectList& objects,
            const HashSet<word>& selectedFields
        );

     
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
    #include "dgFieldReconstructorReconstructFields.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
