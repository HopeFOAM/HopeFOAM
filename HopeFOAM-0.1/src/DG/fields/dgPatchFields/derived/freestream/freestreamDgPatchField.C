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

#include "freestreamDgPatchField.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class Type>
Foam::freestreamDgPatchField<Type>::freestreamDgPatchField
(
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF
)
:
    inletOutletDgPatchField<Type>(p, iF)
{}


template<class Type>
Foam::freestreamDgPatchField<Type>::freestreamDgPatchField
(
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF,
    const dictionary& dict
)
:
    inletOutletDgPatchField<Type>(p, iF)
{
    this->phiName_ = dict.lookupOrDefault<word>("phi","phi");

    //freestreamValue() = Field<Type>("freestreamValue", dict, p.size());
	freestreamValue() = Field<Type>("freestreamValue", dict, this->totalDofNum());
	

    if (dict.found("value"))
    {
        dgPatchField<Type>::operator=
        (
           // Field<Type>("value", dict, p.size())
            Field<Type>("value", dict, this->totalDofNum())
            
        );
    }
    else
    {
        dgPatchField<Type>::operator=(freestreamValue());
    }
}


template<class Type>
Foam::freestreamDgPatchField<Type>::freestreamDgPatchField
(
    const freestreamDgPatchField<Type>& ptf,
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF,
    const dgPatchFieldMapper& mapper
)
:
    inletOutletDgPatchField<Type>(ptf, p, iF, mapper)
{}


template<class Type>
Foam::freestreamDgPatchField<Type>::freestreamDgPatchField
(
    const freestreamDgPatchField<Type>& ptf
)
:
    inletOutletDgPatchField<Type>(ptf)
{}


template<class Type>
Foam::freestreamDgPatchField<Type>::freestreamDgPatchField
(
    const freestreamDgPatchField<Type>& ptf,
    const DimensionedField<Type, dgGeoMesh>& iF
)
:
    inletOutletDgPatchField<Type>(ptf, iF)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class Type>
void Foam::freestreamDgPatchField<Type>::write(Ostream& os) const
{
    dgPatchField<Type>::write(os);
    if (this->phiName_ != "phi")
    {
        os.writeKeyword("phi")
            << this->phiName_ << token::END_STATEMENT << nl;
    }
    freestreamValue().writeEntry("freestreamValue", os);
    this->writeEntry("value", os);
}


// ************************************************************************* //
