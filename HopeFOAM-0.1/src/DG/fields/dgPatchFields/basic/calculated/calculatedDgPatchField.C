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

#include "calculatedDgPatchField.H"
#include "dgPatchFieldMapper.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

template<class Type>
const word& dgPatchField<Type>::calculatedType()
{
    return calculatedDgPatchField<Type>::typeName;
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class Type>
calculatedDgPatchField<Type>::calculatedDgPatchField
(
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF
)
:
    dgPatchField<Type>(p, iF)//modified by RXG  
{
}


template<class Type>
calculatedDgPatchField<Type>::calculatedDgPatchField
(
    const calculatedDgPatchField<Type>& ptf,
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF,
    const dgPatchFieldMapper& mapper
)
:
    dgPatchField<Type>(ptf, p, iF, mapper)
{}


template<class Type>
calculatedDgPatchField<Type>::calculatedDgPatchField
(
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF,
    const dictionary& dict
)
:
    dgPatchField<Type>(p, iF, dict, true)
{}


template<class Type>
calculatedDgPatchField<Type>::calculatedDgPatchField
(
    const calculatedDgPatchField<Type>& ptf
)
:
    dgPatchField<Type>(ptf)
{}


template<class Type>
calculatedDgPatchField<Type>::calculatedDgPatchField
(
    const calculatedDgPatchField<Type>& ptf,
    const DimensionedField<Type, dgGeoMesh>& iF
)
:
    dgPatchField<Type>(ptf, iF)
{}


template<class Type>
tmp<dgPatchField<Type> > dgPatchField<Type>::NewCalculatedType
(
    const dgPatch& p
)
{
    typename patchConstructorTable::iterator patchTypeCstrIter =
        patchConstructorTablePtr_->find(p.type());

    if (patchTypeCstrIter != patchConstructorTablePtr_->end())
    {
        return patchTypeCstrIter()
        (
            p,
            DimensionedField<Type, dgGeoMesh>::null()
        );
    }
    else
    {
        return tmp<dgPatchField<Type> >
        (
            new calculatedDgPatchField<Type>
            (
                p,
                DimensionedField<Type, dgGeoMesh>::null()
            )
        );
    }
}


template<class Type>
template<class Type2>
tmp<dgPatchField<Type> > dgPatchField<Type>::NewCalculatedType
(
    const dgPatchField<Type2>& pf
)
{
    return NewCalculatedType(pf.patch());
}

template<class Type>
void Foam::calculatedDgPatchField<Type>::write(Ostream& os) const
{
    dgPatchField<Type>::write(os);
    this->writeEntry("value", os);
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //
