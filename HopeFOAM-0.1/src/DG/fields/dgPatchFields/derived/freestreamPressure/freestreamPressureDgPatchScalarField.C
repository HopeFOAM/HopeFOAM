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

#include "freestreamPressureDgPatchScalarField.H"
#include "freestreamDgPatchFields.H"
#include "dgPatchFieldMapper.H"
//#include "volFields.H"
//#include "surfaceFields.H"
#include "addToRunTimeSelectionTable.H"
#include "dgPatchFields.H"
#include "dgFields.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::freestreamPressureDgPatchScalarField::
freestreamPressureDgPatchScalarField
(
    const dgPatch& p,
    const DimensionedField<scalar, dgGeoMesh>& iF
)
:
    zeroGradientDgPatchScalarField(p, iF),
    UName_("rhoU"),
    phiName_("phi"),
    rhoName_("rho")
{}


Foam::freestreamPressureDgPatchScalarField::
freestreamPressureDgPatchScalarField
(
    const dgPatch& p,
    const DimensionedField<scalar, dgGeoMesh>& iF,
    const dictionary& dict
)
:
    zeroGradientDgPatchScalarField(p, iF, dict),
    UName_(dict.lookupOrDefault<word>("rhoU", "rhoU")),
    phiName_(dict.lookupOrDefault<word>("phi", "phi")),
    rhoName_(dict.lookupOrDefault<word>("rho", "rho"))
{}


Foam::freestreamPressureDgPatchScalarField::
freestreamPressureDgPatchScalarField
(
    const freestreamPressureDgPatchScalarField& ptf,
    const dgPatch& p,
    const DimensionedField<scalar, dgGeoMesh>& iF,
    const dgPatchFieldMapper& mapper
)
:
    zeroGradientDgPatchScalarField(ptf, p, iF, mapper),
    UName_(ptf.UName_),
    phiName_(ptf.phiName_),
    rhoName_(ptf.rhoName_)
{}


Foam::freestreamPressureDgPatchScalarField::
freestreamPressureDgPatchScalarField
(
    const freestreamPressureDgPatchScalarField& wbppsf
)
:
    zeroGradientDgPatchScalarField(wbppsf),
    UName_(wbppsf.UName_),
    phiName_(wbppsf.phiName_),
    rhoName_(wbppsf.rhoName_)
{}


Foam::freestreamPressureDgPatchScalarField::
freestreamPressureDgPatchScalarField
(
    const freestreamPressureDgPatchScalarField& wbppsf,
    const DimensionedField<scalar, dgGeoMesh>& iF
)
:
    zeroGradientDgPatchScalarField(wbppsf, iF),
    UName_(wbppsf.UName_),
    phiName_(wbppsf.phiName_),
    rhoName_(wbppsf.rhoName_)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::freestreamPressureDgPatchScalarField::updateCoeffs()
{
    if (updated())
    {
        return;
    }
 /*   const freestreamDgPatchVectorField& rhoUp =
        refCast<const freestreamDgPatchVectorField>
        (
           // patch().lookupPatchField<volVectorField, vector>(UName_)
            patch().lookupPatchField<dgVectorField, vector>(UName_)
        );

    const dgScalarField& phi =
        db().lookupObject<dgScalarField>(phiName_);

    dgPatchField<scalar>& phip =
        const_cast<dgPatchField<scalar>&>
        (
            patch().patchField<dgScalarField, scalar>(phi)
        );

    if (phi.dimensions() == dimVelocity*dimArea)
    {
       // phip = patch().Sf() & Up.freestreamValue();
    }
    else if (phi.dimensions() == dimDensity*dimVelocity*dimArea)
    {
        const dgPatchField<scalar>& rhop =
            patch().lookupPatchField<dgScalarField, scalar>(rhoName_);

     //   phip = rhop*(patch().Sf() & Up.freestreamValue());
    }
    else
    {
        FatalErrorInFunction
            << "dimensions of phi are not correct"
            << "\n    on patch " << this->patch().name()
            << " of field " << this->internalField().name()
            << " in file " << this->internalField().objectPath()
            << exit(FatalError);
    }
*/

//may be ERROR!!!!!!!!!!!!!!!!!!!
    zeroGradientDgPatchScalarField::updateCoeffs();
}


void Foam::freestreamPressureDgPatchScalarField::write(Ostream& os) const
{
    dgPatchScalarField::write(os);
    writeEntryIfDifferent<word>(os, "U", "U", UName_);
    writeEntryIfDifferent<word>(os, "phi", "phi", phiName_);
    writeEntryIfDifferent<word>(os, "rho", "rho", rhoName_);
    writeEntry("value", os);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
    makePatchTypeField
    (
        dgPatchScalarField,
        freestreamPressureDgPatchScalarField
    );
}

// ************************************************************************* //
