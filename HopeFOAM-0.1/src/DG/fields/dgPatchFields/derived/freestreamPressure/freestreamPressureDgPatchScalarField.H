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
    Foam::freestreamPressureDgPatchScalarField

Group
    grpInletBoundaryConditions grpOutletBoundaryConditions

Description
    This boundary condition provides a free-stream condition for pressure.
    It is a zero-gradient condition that constrains the flux across the patch
    based on the free-stream velocity.

Usage
    \table
        Property     | Description             | Required    | Default value
        U            | velocity field name     | no          | U
        phi          | flux field name         | no          | phi
        rho          | density field name      | no          | none
    \endtable

    Example of the boundary condition specification:
    \verbatim
    <patchName>
    {
        type            freestreamPressure;
    }
    \endverbatim

Note
    This condition is designed to operate with a freestream velocity condition

See also
    Foam::zeroGradientDgPatchField
    Foam::freestreamDgPatchField

SourceFiles
    freestreamPressureDgPatchScalarField.C

\*---------------------------------------------------------------------------*/

#ifndef freestreamPressureDgPatchScalarFields_H
#define freestreamPressureDgPatchScalarFields_H

#include "dgPatchFields.H"
#include "zeroGradientDgPatchFields.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
             Class freestreamPressureDgPatchScalarField Declaration
\*---------------------------------------------------------------------------*/

class freestreamPressureDgPatchScalarField
:
    public zeroGradientDgPatchScalarField
{
    // Private data

        //- Name of the velocity field
        word UName_;

        //- Name of the flux transporting the field
        word phiName_;

        //- Name of the density field used to normalise the mass flux
        //  if neccessary
        word rhoName_;


public:

    //- Runtime type information
    TypeName("freestreamPressure");


    // Constructors

        //- Construct from patch and internal field
        freestreamPressureDgPatchScalarField
        (
            const dgPatch&,
            const DimensionedField<scalar, dgGeoMesh>&
        );

        //- Construct from patch, internal field and dictionary
        freestreamPressureDgPatchScalarField
        (
            const dgPatch&,
            const DimensionedField<scalar, dgGeoMesh>&,
            const dictionary&
        );

        //- Construct by mapping given freestreamPressureDgPatchScalarField onto
        //  a new patch
        freestreamPressureDgPatchScalarField
        (
            const freestreamPressureDgPatchScalarField&,
            const dgPatch&,
            const DimensionedField<scalar, dgGeoMesh>&,
            const dgPatchFieldMapper&
        );

        //- Construct as copy
        freestreamPressureDgPatchScalarField
        (
            const freestreamPressureDgPatchScalarField&
        );

        //- Construct and return a clone
        virtual tmp<dgPatchScalarField> clone() const
        {
            return tmp<dgPatchScalarField>
            (
                new freestreamPressureDgPatchScalarField(*this)
            );
        }

        //- Construct as copy setting internal field reference
        freestreamPressureDgPatchScalarField
        (
            const freestreamPressureDgPatchScalarField&,
            const DimensionedField<scalar, dgGeoMesh>&
        );

        //- Construct and return a clone setting internal field reference
        virtual tmp<dgPatchScalarField> clone
        (
            const DimensionedField<scalar, dgGeoMesh>& iF
        ) const
        {
            return tmp<dgPatchScalarField>
            (
                new freestreamPressureDgPatchScalarField(*this, iF)
            );
        }


    // Member functions

        // Evaluation functions

            //- Update the coefficients associated with the patch field
            virtual void updateCoeffs();


        //- Write
        virtual void write(Ostream&) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
