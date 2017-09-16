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

#include "emptyDgPatchField.H"
#include "dgPatchFieldMapper.H"
#include "dgGeoMesh.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class Type>
emptyDgPatchField<Type>::emptyDgPatchField
(
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF
)
:
    dgPatchField<Type>(p, iF, Field<Type>(0))
{}


template<class Type>
emptyDgPatchField<Type>::emptyDgPatchField
(
    const emptyDgPatchField<Type>&,
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF,
    const dgPatchFieldMapper&
)
:
    dgPatchField<Type>(p, iF, Field<Type>(0))
{
    if (!isType<emptyDgPatch>(this->patch()))
    {
        FatalErrorIn
        (
            "emptyDgPatchField<Type>::emptyDgPatchField\n"
            "(\n"
            "    const emptyDgPatchField<Type>&,\n"
            "    const dgPatch& p,\n"
            "    const DimensionedField<Type, dgGeoMesh>& iF,\n"
            "    const dgPatchFieldMapper& mapper\n"
            ")\n"
        )   << "Field type does not correspond to patch type for patch "
            << this->patch().index() << "." << endl
            << "Field type: " << typeName << endl
            << "Patch type: " << this->patch().type()
            << exit(FatalError);
    }
}


template<class Type>
emptyDgPatchField<Type>::emptyDgPatchField
(
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF,
    const dictionary& dict
)
:
    dgPatchField<Type>(p, iF, Field<Type>(0))
{
    if (!isType<emptyDgPatch>(p))
    {
        FatalIOErrorIn
        (
            "emptyDgPatchField<Type>::emptyDgPatchField\n"
            "(\n"
            "    const dgPatch& p,\n"
            "    const Field<Type>& field,\n"
            "    const dictionary& dict\n"
            ")\n",
            dict
        )   << "patch " << this->patch().index() << " not empty type. "
            << "Patch type = " << p.type()
            << exit(FatalIOError);
    }
}


template<class Type>
emptyDgPatchField<Type>::emptyDgPatchField
(
    const emptyDgPatchField<Type>& ptf
)
:
    dgPatchField<Type>
    (
        ptf.patch(),
        ptf.dimensionedInternalField(),
        Field<Type>(0)
    )
{}


template<class Type>
emptyDgPatchField<Type>::emptyDgPatchField
(
    const emptyDgPatchField<Type>& ptf,
    const DimensionedField<Type, dgGeoMesh>& iF
)
:
    dgPatchField<Type>(ptf.patch(), iF, Field<Type>(0))
{}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //
