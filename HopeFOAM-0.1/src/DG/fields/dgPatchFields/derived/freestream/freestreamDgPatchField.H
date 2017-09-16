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
    Foam::freestreamDgPatchField

Group
    grpInletBoundaryConditions grpOutletBoundaryConditions

Description
    This boundary condition provides a free-stream condition.  It is a 'mixed'
    condition derived from the \c inletOutlet condition, whereby the mode of
    operation switches between fixed (free stream) value and zero gradient
    based on the sign of the flux.

Usage
    \table
        Property     | Description             | Required    | Default value
        freestreamValue   | freestream velocity          | yes         |
        phi          | flux field name         | no          | phi
    \endtable

    Example of the boundary condition specification:
    \verbatim
    <patchName>
    {
        type            freestream;
        phi             phi;
    }
    \endverbatim

See also
    Foam::mixedDgPatchField
    Foam::inletOutletDgPatchField

SourceFiles
    freestreamDgPatchField.C

\*---------------------------------------------------------------------------*/

#ifndef freestreamDgPatchField_H
#define freestreamDgPatchField_H

#include "inletOutletDgPatchField.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                   Class freestreamDgPatchField Declaration
\*---------------------------------------------------------------------------*/

template<class Type>
class freestreamDgPatchField
:
    public inletOutletDgPatchField<Type>
{

public:

    //- Runtime type information
    TypeName("freestream");


    // Constructors

        //- Construct from patch and internal field
        freestreamDgPatchField
        (
            const dgPatch&,
            const DimensionedField<Type, dgGeoMesh>&
        );

        //- Construct from patch, internal field and dictionary
        freestreamDgPatchField
        (
            const dgPatch&,
            const DimensionedField<Type, dgGeoMesh>&,
            const dictionary&
        );

        //- Construct by mapping given freestreamDgPatchField onto a new patch
        freestreamDgPatchField
        (
            const freestreamDgPatchField<Type>&,
            const dgPatch&,
            const DimensionedField<Type, dgGeoMesh>&,
            const dgPatchFieldMapper&
        );

        //- Construct as copy
        freestreamDgPatchField
        (
            const freestreamDgPatchField<Type>&
        );

        //- Construct and return a clone
        virtual tmp<dgPatchField<Type>> clone() const
        {
            return tmp<dgPatchField<Type>>
            (
                new freestreamDgPatchField<Type>(*this)
            );
        }

        //- Construct as copy setting internal field reference
        freestreamDgPatchField
        (
            const freestreamDgPatchField<Type>&,
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
                new freestreamDgPatchField<Type>(*this, iF)
            );
        }


    // Member functions

        // Return defining fields

            const Field<Type>& freestreamValue() const
            {
                return this->refValue();
            }

            Field<Type>& freestreamValue()
            {
                return this->refValue();
            }


        //- Write
        virtual void write(Ostream&) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
    #include "freestreamDgPatchField.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
