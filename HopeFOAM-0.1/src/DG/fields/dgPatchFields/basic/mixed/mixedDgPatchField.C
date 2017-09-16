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

#include "mixedDgPatchField.H"

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //
namespace Foam
{

template<class Type>
Foam::mixedDgPatchField<Type>::mixedDgPatchField
(
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF
)
:
    dgPatchField<Type>(p, iF),
    refValue_(this->totalDofNum() ),
    refGrad_(this->totalDofNum() ),
    valueFraction_(this->totalDofNum() )
{}


template<class Type>
Foam::mixedDgPatchField<Type>::mixedDgPatchField
(
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF,
    const dictionary& dict
)
:
    dgPatchField<Type>(p, iF, dict),
    refValue_("refValue", dict, this->totalDofNum() ),
    refGrad_("refGradient", dict, this->totalDofNum() ),
    valueFraction_("valueFraction", dict, this->totalDofNum() )
{
    evaluate();
}


template<class Type>
Foam::mixedDgPatchField<Type>::mixedDgPatchField
(
    const mixedDgPatchField<Type>& ptf,
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF,
    const dgPatchFieldMapper& mapper
)
:
    dgPatchField<Type>(ptf, p, iF, mapper),
    refValue_(ptf.refValue_, mapper),
    refGrad_(ptf.refGrad_, mapper),
    valueFraction_(ptf.valueFraction_, mapper)
{
    if (notNull(iF) && mapper.hasUnmapped())
    {
        WarningInFunction
            << "On field " << iF.name() << " patch " << p.name()
            << " patchField " << this->type()
            << " : mapper does not map all values." << nl
            << "    To avoid this warning fully specify the mapping in derived"
            << " patch fields." << endl;
    }
}


template<class Type>
Foam::mixedDgPatchField<Type>::mixedDgPatchField
(
    const mixedDgPatchField<Type>& ptf
)
:
    dgPatchField<Type>(ptf),
    refValue_(ptf.refValue_),
    refGrad_(ptf.refGrad_),
    valueFraction_(ptf.valueFraction_)
{}


template<class Type>
Foam::mixedDgPatchField<Type>::mixedDgPatchField
(
    const mixedDgPatchField<Type>& ptf,
    const DimensionedField<Type, dgGeoMesh>& iF
)
:
    dgPatchField<Type>(ptf, iF),
    refValue_(ptf.refValue_),
    refGrad_(ptf.refGrad_),
    valueFraction_(ptf.valueFraction_)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class Type>
void Foam::mixedDgPatchField<Type>::autoMap
(
    const dgPatchFieldMapper& m
)
{
    dgPatchField<Type>::autoMap(m);
    refValue_.autoMap(m);
    refGrad_.autoMap(m);
    valueFraction_.autoMap(m);
}


template<class Type>
void Foam::mixedDgPatchField<Type>::rmap
(
    const dgPatchField<Type>& ptf,
    const labelList& addr
)
{
    dgPatchField<Type>::rmap(ptf, addr);

    const mixedDgPatchField<Type>& mptf =
        refCast<const mixedDgPatchField<Type>>(ptf);

    refValue_.rmap(mptf.refValue_, addr);
    refGrad_.rmap(mptf.refGrad_, addr);
    valueFraction_.rmap(mptf.valueFraction_, addr);
}


template<class Type>
void Foam::mixedDgPatchField<Type>::evaluate(const Pstream::commsTypes)
{
    if (!this->updated())
    {
        this->updateCoeffs();
    }

	//Info<<"deltaCoeffs().size:  "<<this->patch().deltaCoeffs().size()<<endl;
	//Info<<"patchInternalField().size: "<<this->patchInternalField()().size()<<endl;
	//Info<<"this->.size: "<<this->size()<<endl;
	//Info<<"XGR0"<<endl;
	//Info<<"refValue_.size "<<refValue_.size()<<endl;

   Field<Type>::operator=
    (
        valueFraction_*refValue_
      +
        (1.0 - valueFraction_)*
        (
            this->patchInternalField()
        )
    );
   //only fixedvalue or zerogradient

	/*    Field<Type>::operator=
    (
        valueFraction_*refValue_
      +
        (1.0 - valueFraction_)*
        (
            this->patchInternalField()
        )
    );*/
	//Info<<"XGR"<<endl;

    dgPatchField<Type>::evaluate();
}


template<class Type>
Foam::tmp<Foam::Field<Type>>
Foam::mixedDgPatchField<Type>::snGrad() const
{
    return
        valueFraction_
       *(refValue_ - this->patchInternalField())
       *this->patch().deltaCoeffs()
      + (1.0 - valueFraction_)*refGrad_;
}


template<class Type>
Foam::tmp<Foam::Field<Type>>
Foam::mixedDgPatchField<Type>::valueInternalCoeffs
(
    const tmp<scalarField>&
) const
{
    return Type(pTraits<Type>::one)*(1.0 - valueFraction_);

}


template<class Type>
Foam::tmp<Foam::Field<Type>>
Foam::mixedDgPatchField<Type>::valueBoundaryCoeffs
(
    const tmp<scalarField>&
) const
{
    return
         valueFraction_*refValue_
       + (1.0 - valueFraction_)*refGrad_/this->patch().deltaCoeffs();
}


template<class Type>
Foam::tmp<Foam::Field<Type>>
Foam::mixedDgPatchField<Type>::gradientInternalCoeffs() const
{
    return -Type(pTraits<Type>::one)*valueFraction_*this->patch().deltaCoeffs();
}


template<class Type>
Foam::tmp<Foam::Field<Type>>
Foam::mixedDgPatchField<Type>::gradientBoundaryCoeffs() const
{
    return
        valueFraction_*this->patch().deltaCoeffs()*refValue_
      + (1.0 - valueFraction_)*refGrad_;
}


template<class Type>
void Foam::mixedDgPatchField<Type>::write(Ostream& os) const
{
    dgPatchField<Type>::write(os);
    refValue_.writeEntry("refValue", os);
    refGrad_.writeEntry("refGradient", os);
    valueFraction_.writeEntry("valueFraction", os);
    this->writeEntry("value", os);
}

}

// ************************************************************************* //
