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

#include "zeroGradientDgPatchField.H"
#include "dgPatchFieldMapper.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class Type>
zeroGradientDgPatchField<Type>::zeroGradientDgPatchField
(
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF
)
:
    dgPatchField<Type>(p, iF)
{}


template<class Type>
zeroGradientDgPatchField<Type>::zeroGradientDgPatchField
(
    const zeroGradientDgPatchField<Type>& ptf,
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF,
    const dgPatchFieldMapper& mapper
)
:
    dgPatchField<Type>(ptf, p, iF, mapper)
{}


template<class Type>
zeroGradientDgPatchField<Type>::zeroGradientDgPatchField
(
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF,
    const dictionary& dict
)
:
    dgPatchField<Type>(p, iF, dict)
{
    dgPatchField<Type>::operator=(this->patchInternalField());
}


template<class Type>
zeroGradientDgPatchField<Type>::zeroGradientDgPatchField
(
    const zeroGradientDgPatchField& zgpf
)
:
    dgPatchField<Type>(zgpf)
{}


template<class Type>
zeroGradientDgPatchField<Type>::zeroGradientDgPatchField
(
    const zeroGradientDgPatchField& zgpf,
    const DimensionedField<Type, dgGeoMesh>& iF
)
:
    dgPatchField<Type>(zgpf, iF)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class Type>
void zeroGradientDgPatchField<Type>::evaluate(const Pstream::commsTypes)
{
    if (!this->updated())
    {
        this->updateCoeffs();
    }

    dgPatchField<Type>::operator==(this->patchInternalField());
    dgPatchField<Type>::evaluate();
}


template<class Type>
tmp<Field<Type> > zeroGradientDgPatchField<Type>::valueInternalCoeffs
(
    const tmp<scalarField>&
) const
{
    return tmp<Field<Type> >
    (
        new Field<Type>(this->size(), pTraits<Type>::zero)
    );
}


template<class Type>
tmp<Field<Type> > zeroGradientDgPatchField<Type>::valueBoundaryCoeffs
(
    const tmp<scalarField>&
) const
{
    return tmp<Field<Type> >
    (
        new Field<Type>(this->size(), pTraits<Type>::zero)
    );
}


template<class Type>
tmp<Field<Type> > zeroGradientDgPatchField<Type>::gradientInternalCoeffs() const
{
    return tmp<Field<Type> >
    (
        new Field<Type>(this->size(), pTraits<Type>::zero)
    );
}


template<class Type>
tmp<Field<Type> > zeroGradientDgPatchField<Type>::gradientBoundaryCoeffs() const
{
    return tmp<Field<Type> >
    (
        new Field<Type>(this->size(), pTraits<Type>::zero)
    );
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //
