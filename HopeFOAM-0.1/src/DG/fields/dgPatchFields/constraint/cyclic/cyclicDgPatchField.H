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
    Foam::cyclicDofDgPatchField

Group
    grpCoupledBoundaryConditions

Description
    This boundary condition enforces a cyclic condition between a pair of
    boundaries.

Usage
    Example of the boundary condition specification:
    \verbatim
    <patchName>
    {
        type            cyclic;
    }
    \endverbatim

Note
    The patches must be topologically similar, i.e. if the owner patch is
    transformed to the neighbour patch, the patches should be identical (or
    very similar).

SourceFiles
    cyclicDgPatchField.C

\*---------------------------------------------------------------------------*/

#ifndef cyclicDgPatchField_H
#define cyclicDgPatchField_H

#include "coupledDgPatchField.H"
#include "cyclicLduInterfaceField.H"
#include "cyclicDgPatch.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                        Class cyclicDgPatch Declaration
\*---------------------------------------------------------------------------*/

template<class Type>
class cyclicDgPatchField
:
    virtual public cyclicLduInterfaceField,
    public coupledDgPatchField<Type>
{
    // Private data

        //- Local reference cast into the cyclic patch
        const cyclicDgPatch& cyclicPatch_;


    // Private Member Functions

        //- Return neighbour side field given internal fields
        template<class Type2>
        tmp<Field<Type2>> neighbourSideField
        (
            const Field<Type2>&
        ) const;


public:

    //- Runtime type information
    TypeName(cyclicDgPatch::typeName_());


    // Constructors

        //- Construct from patch and internal field
        cyclicDgPatchField
        (
            const dgPatch&,
            const DimensionedField<Type, dgGeoMesh>&
        );

        //- Construct from patch, internal field and dictionary
        cyclicDgPatchField
        (
            const dgPatch&,
            const DimensionedField<Type, dgGeoMesh>&,
            const dictionary&
        );

        //- Construct by mapping given cyclicDgPatchField onto a new patch
        cyclicDgPatchField
        (
            const cyclicDgPatchField<Type>&,
            const dgPatch&,
            const DimensionedField<Type, dgGeoMesh>&,
            const dgPatchFieldMapper&
        );

        //- Construct as copy
        cyclicDgPatchField
        (
            const cyclicDgPatchField<Type>&
        );

        //- Construct and return a clone
        virtual tmp<dgPatchField<Type>> clone() const
        {
            return tmp<dgPatchField<Type>>
            (
                new cyclicDgPatchField<Type>(*this)
            );
        }

        //- Construct as copy setting internal field reference
        cyclicDgPatchField
        (
            const cyclicDgPatchField<Type>&,
            const DimensionedField<Type, dgGeoMesh>&
        );

        //- Construct and return a clone setting internal field reference
        virtual tmp<dgPatchField<Type>> clone
        (
            const DimensionedField<Type, dgGeoMesh>& iF
        ) const
        {
            return tmp<dgPatchField<Type>>
            (
                new cyclicDgPatchField<Type>(*this, iF)
            );
        }


    // Member functions

        // Access

            //- Return local reference cast into the cyclic patch
            const cyclicDgPatch& cyclicPatch() const
            {
                return cyclicPatch_;
            }


        // Evaluation functions

            //- Return neighbour coupled internal cell data
            tmp<Field<Type>> patchNeighbourField() const;

            //- Return reference to neighbour patchField
            const cyclicDgPatchField<Type>& neighbourPatchField() const;

            //- Update result field based on interface functionality
            virtual void updateInterfaceMatrix
            (
                scalarField& result,
                const scalarField& psiInternal,
                const scalarField& coeffs,
                const direction cmpt,
                const Pstream::commsTypes commsType
            ) const;

            //- Update result field based on interface functionality
            virtual void updateInterfaceMatrix
            (
                Field<Type>& result,
                const Field<Type>& psiInternal,
                const scalarField& coeffs,
                const Pstream::commsTypes commsType
            ) const;


        // Cyclic coupled interface functions

            //- Does the patch field perform the transformation
            virtual bool doTransform() const
            {
                return !(cyclicPatch_.parallel() || pTraits<Type>::rank == 0);
            }

            //- Return face transformation tensor
            virtual const tensorField& forwardT() const
            {
                return cyclicPatch_.forwardT();
            }

            //- Return neighbour-cell transformation tensor
            virtual const tensorField& reverseT() const
            {
                return cyclicPatch_.reverseT();
            }

            //- Return rank of component for transform
            virtual int rank() const
            {
                return pTraits<Type>::rank;
            }


        // I-O

            //- Write
            virtual void write(Ostream& os) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
    #include "cyclicDgPatchField.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
