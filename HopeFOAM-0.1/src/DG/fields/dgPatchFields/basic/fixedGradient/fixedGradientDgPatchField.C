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

#include "fixedGradientDgPatchField.H"
#include "dictionary.H"

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class Type>
Foam::fixedGradientDgPatchField<Type>::fixedGradientDgPatchField
(
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF
)
:
    dgPatchField<Type>(p, iF),
    gradient_(this->size(), Zero)
{

}


template<class Type>
Foam::fixedGradientDgPatchField<Type>::fixedGradientDgPatchField
(
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF,
    const dictionary& dict
)
:
    dgPatchField<Type>(p, iF, dict),
    gradient_("gradient", dict, this->size())
{
    evaluate();
}


template<class Type>
Foam::fixedGradientDgPatchField<Type>::fixedGradientDgPatchField
(
    const fixedGradientDgPatchField<Type>& ptf,
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF,
    const dgPatchFieldMapper& mapper
)
:
    dgPatchField<Type>(ptf, p, iF, mapper),
    gradient_(ptf.gradient_, mapper)
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
Foam::fixedGradientDgPatchField<Type>::fixedGradientDgPatchField
(
    const fixedGradientDgPatchField<Type>& ptf
)
:
    dgPatchField<Type>(ptf),
    gradient_(ptf.gradient_)
{}


template<class Type>
Foam::fixedGradientDgPatchField<Type>::fixedGradientDgPatchField
(
    const fixedGradientDgPatchField<Type>& ptf,
    const DimensionedField<Type, dgGeoMesh>& iF
)
:
    dgPatchField<Type>(ptf, iF),
    gradient_(ptf.gradient_)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class Type>
void Foam::fixedGradientDgPatchField<Type>::autoMap
(
    const dgPatchFieldMapper& m
)
{
    dgPatchField<Type>::autoMap(m);
    gradient_.autoMap(m);
}


template<class Type>
void Foam::fixedGradientDgPatchField<Type>::rmap
(
    const dgPatchField<Type>& ptf,
    const labelList& addr
)
{
    dgPatchField<Type>::rmap(ptf, addr);

    const fixedGradientDgPatchField<Type>& fgptf =
        refCast<const fixedGradientDgPatchField<Type>>(ptf);

    gradient_.rmap(fgptf.gradient_, addr);
}


template<class Type>
void Foam::fixedGradientDgPatchField<Type>::evaluate(const Pstream::commsTypes)
{
    if (!this->updated())
    {
        this->updateCoeffs();
    }

    dgPatchField<Type>::operator==(this->patchInternalField());
    dgPatchField<Type>::evaluate();
}

/*
template<class Type>
Foam::tmp<Foam::Field<Type>>
Foam::fixedGradientDgPatchField<Type>::valueInternalCoeffs
(
    const tmp<scalarField>&
) const
{
    return tmp<Field<Type>>(new Field<Type>(this->size(), pTraits<Type>::one));
}


template<class Type>
Foam::tmp<Foam::Field<Type>>
Foam::fixedGradientDgPatchField<Type>::valueBoundaryCoeffs
(
    const tmp<scalarField>&
) const
{
    return gradient()/this->patch().deltaCoeffs();
}


template<class Type>
Foam::tmp<Foam::Field<Type>>
Foam::fixedGradientDgPatchField<Type>::gradientInternalCoeffs() const
{
    return tmp<Field<Type>>
    (
        new Field<Type>(this->size(), Zero)
    );
}


template<class Type>
Foam::tmp<Foam::Field<Type>>
Foam::fixedGradientDgPatchField<Type>::gradientBoundaryCoeffs() const
{
    return gradient();
}
*/

template<class Type>
void Foam::fixedGradientDgPatchField<Type>::write(Ostream& os) const
{
    dgPatchField<Type>::write(os);
    gradient_.writeEntry("gradient", os);
}


// ************************************************************************* //
