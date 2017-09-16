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
    Foam::noSlipDgPatchVectorField

Group
    grpWallBoundaryConditions

Description
    This boundary condition fixes the velocity to zero at walls.

Usage
    Example of the boundary condition specification:
    \verbatim
    <patchName>
    {
        type            noSlip;
    }
    \endverbatim

See also
    Foam::fixedValueDgPatchVectorField

SourceFiles
    noSlipDgPatchVectorField.C

\*---------------------------------------------------------------------------*/

#ifndef noSlipDgPatchVectorField_H
#define noSlipDgPatchVectorField_H

#include "fixedValueDgPatchFields.H"
#include "dgPatchFields.H"
#include "dgFields.H"
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
             Class noSlipDgPatchVectorField Declaration
\*---------------------------------------------------------------------------*/

class noSlipDgPatchVectorField
:
    public fixedValueDgPatchVectorField
{

public:

    //- Runtime type information
    TypeName("noSlip");


    // Constructors

        //- Construct from patch and internal field
        noSlipDgPatchVectorField
        (
            const dgPatch&,
            const DimensionedField<vector, dgGeoMesh>&
        );

        //- Construct from patch, internal field and dictionary
        noSlipDgPatchVectorField
        (
            const dgPatch&,
            const DimensionedField<vector, dgGeoMesh>&,
            const dictionary&
        );

        //- Construct by mapping given noSlipDgPatchVectorField
        //  onto a new patch
        noSlipDgPatchVectorField
        (
            const noSlipDgPatchVectorField&,
            const dgPatch&,
            const DimensionedField<vector, dgGeoMesh>&,
            const dgPatchFieldMapper&
        );

        //- Construct as copy
        noSlipDgPatchVectorField
        (
            const noSlipDgPatchVectorField&
        );

        //- Construct and return a clone
        virtual tmp<dgPatchVectorField> clone() const
        {
            return tmp<dgPatchVectorField>
            (
                new noSlipDgPatchVectorField(*this)
            );
        }

        //- Construct as copy setting internal field reference
        noSlipDgPatchVectorField
        (
            const noSlipDgPatchVectorField&,
            const DimensionedField<vector, dgGeoMesh>&
        );

        //- Construct and return a clone setting internal field reference
        virtual tmp<dgPatchVectorField> clone
        (
            const DimensionedField<vector, dgGeoMesh>& iF
        ) const
        {
            return tmp<dgPatchVectorField>
            (
                new noSlipDgPatchVectorField(*this, iF)
            );
        }


    // Member functions

        //- Write
        virtual void write(Ostream&) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
