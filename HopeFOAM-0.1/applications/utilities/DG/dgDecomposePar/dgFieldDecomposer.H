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
    Foam::dgFieldDecomposer

Description
    Discontinuous Galerkin Method field decomposer.

SourceFiles
    dgFieldDecomposer.C
    dgFieldDecomposerDecomposeFields.C

\*---------------------------------------------------------------------------*/

#ifndef dgFieldDecomposer_H
#define dgFieldDecomposer_H

#include "dgMesh.H"
#include "dgPatchFieldMapper.H"
#include "dgFields.H"
//#include "surfaceFields.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

class IOobjectList;

/*---------------------------------------------------------------------------*\
                    Class dgFieldDecomposer Declaration
\*---------------------------------------------------------------------------*/

class dgFieldDecomposer
{
public:

        //- Patch field decomposer class
        class patchFieldDecomposer
        :
            public dgPatchFieldMapper
        {
            // Private data

                labelList directAddressing_;

        public:

            // Constructors

                //- Construct given addressing
                patchFieldDecomposer
                (
                    const labelUList& addressingSlice,
                    const label addressingOffset
                );


            // Member functions

                label size() const
                {
                    return directAddressing_.size();
                }

                bool direct() const
                {
                    return true;
                }

                //- Are there unmapped values
                bool hasUnmapped() const
                {
                    return false;
                }

                const labelUList& directAddressing() const
                {
                    return directAddressing_;
                }
        };


        //- Processor patch field decomposer class. Maps either owner or
        //  neighbour data (no interpolate anymore - processorDgPatchField
        //  holds neighbour data)
        class processorVolPatchFieldDecomposer
        :
            public dgPatchFieldMapper
        {
            // Private data
		labelPairList directFaceLabel_;
              //  labelList directAddressing_;

        public:

            //- Construct given addressing
            processorVolPatchFieldDecomposer
            (
                const dgMesh& mesh,
                const labelUList& addressingSlice
            );


            // Member functions

                label size() const
                {
                    return directFaceLabel_.size();
                }

                bool direct() const
                {
                    return true;
                }

                //- Are there unmapped values
                bool hasUnmapped() const
                {
                    return false;
                }

                const labelPairList& directFaceLabel() const
                {
                    return directFaceLabel_;
                }
        };

private:

    // Private data

        //- Reference to complete mesh
        const dgMesh& completeMesh_;

        //- Reference to processor mesh
        const dgMesh& procMesh_;

        //- Reference to face addressing
        const labelList& faceAddressing_;

        //- Reference to cell addressing
        const labelList& cellAddressing_;
	
        labelList cellFieldAddressing_;
        //- Reference to boundary addressing
        const labelList& boundaryAddressing_;
        
        labelList boundaryFieldAddressing_;

        //- List of patch field decomposers
        List<patchFieldDecomposer*> patchFieldDecomposerPtrs_;

        List<processorVolPatchFieldDecomposer*>
        
        processorVolPatchFieldDecomposerPtrs_;

    // Private Member Functions

        //- Disallow default bitwise copy construct
        dgFieldDecomposer(const dgFieldDecomposer&);

        //- Disallow default bitwise assignment
        void operator=(const dgFieldDecomposer&);


public:

    // Constructors

        //- Construct from components
        dgFieldDecomposer
        (
            const dgMesh& completeMesh,
            const dgMesh& procMesh,
            const labelList& faceAddressing,
            const labelList& cellAddressing,
            const labelList& boundaryAddressing
        );


    //- Destructor
    ~dgFieldDecomposer();


    // Member Functions

        //- Decompose volume field
        template<class Type>
        tmp<GeometricDofField<Type, dgPatchField, dgGeoMesh> >
        decomposeField
        (
            const GeometricDofField<Type, dgPatchField, dgGeoMesh>& field,
            const bool allowUnknownPatchFields = false
        ) const;

        template<class GeoField>
        void decomposeFields(const PtrList<GeoField>& fields) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
#   include "dgFieldDecomposerDecomposeFields.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
