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

#include "inletOutletDgPatchField.H"
#include "dgPatchFields.H"
#include "dgFields.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class Type>
Foam::inletOutletDgPatchField<Type>::inletOutletDgPatchField
(
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF
)
:
    mixedDgPatchField<Type>(p, iF),
    phiName_("phi")
{
    this->refValue() = Zero;
    this->refGrad() = Zero;
    this->valueFraction() = 0.0;
	//Info<<"this->size()1   "<<this->size()<<endl;
}


template<class Type>
Foam::inletOutletDgPatchField<Type>::inletOutletDgPatchField
(
    const inletOutletDgPatchField<Type>& ptf,
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF,
    const dgPatchFieldMapper& mapper
)
:
    mixedDgPatchField<Type>(ptf, p, iF, mapper),
    phiName_(ptf.phiName_)
{
	//Info<<"this->size()2   "<<this->size()<<endl;
}


template<class Type>
Foam::inletOutletDgPatchField<Type>::inletOutletDgPatchField
(
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF,
    const dictionary& dict
)
:
    mixedDgPatchField<Type>(p, iF),
    phiName_(dict.lookupOrDefault<word>("phi", "phi"))
{
    //this->refValue() = Field<Type>("inletValue", dict, p.size());
	this->refValue() = Field<Type>("inletValue", dict, this->totalDofNum());
	

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
        dgPatchField<Type>::operator=(this->refValue());
    }

    this->refGrad() = Zero;
    this->valueFraction() = 0.0;
	//Info<<"this->size()3   "<<this->size()<<endl;
}


template<class Type>
Foam::inletOutletDgPatchField<Type>::inletOutletDgPatchField
(
    const inletOutletDgPatchField<Type>& ptf
)
:
    mixedDgPatchField<Type>(ptf),
    phiName_(ptf.phiName_)
{
//Info<<"this->size()4   "<<this->size()<<endl;
}


template<class Type>
Foam::inletOutletDgPatchField<Type>::inletOutletDgPatchField
(
    const inletOutletDgPatchField<Type>& ptf,
    const DimensionedField<Type, dgGeoMesh>& iF
)
:
    mixedDgPatchField<Type>(ptf, iF),
    phiName_(ptf.phiName_)
{
//Info<<"this->size()5   "<<this->size()<<endl;
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class Type>
void Foam::inletOutletDgPatchField<Type>::updateCoeffs()
{
    if (this->updated())
    {
        return;
    }

   const Field<scalar>& phip =
    this->patch().template lookupPatchField<dgScalarField, scalar>
        (
            phiName_
        );

	//Info<<"from inletoulet phip:"<<phip<<endl;
	   this->valueFraction() = 1.0 - pos(phip);
	
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!may be ERROR!!!!!!!!!!!!!!!!!!!!!!!
// by RXG

    mixedDgPatchField<Type>::updateCoeffs();
}


template<class Type>
void Foam::inletOutletDgPatchField<Type>::write(Ostream& os) const
{
    dgPatchField<Type>::write(os);
    if (phiName_ != "phi")
    {
        os.writeKeyword("phi") << phiName_ << token::END_STATEMENT << nl;
    }
    this->refValue().writeEntry("inletValue", os);
    this->writeEntry("value", os);
}


// * * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * //

template<class Type>
void Foam::inletOutletDgPatchField<Type>::operator=
(
    const dgPatchField<Type>& ptf
)
{
    dgPatchField<Type>::operator=
    (
        this->valueFraction()*this->refValue()
        + (1 - this->valueFraction())*ptf
    );
}


// ************************************************************************* //
