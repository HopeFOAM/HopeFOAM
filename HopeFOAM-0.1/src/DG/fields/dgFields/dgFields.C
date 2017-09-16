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

#include "dgFields.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

defineTemplate2TypeNameAndDebug(dgScalarField::Internal, 0);
defineTemplate2TypeNameAndDebug(dgVectorField::Internal, 0);
defineTemplate2TypeNameAndDebug
(
    dgSphericalTensorField::Internal,
    0
);
defineTemplate2TypeNameAndDebug
(
    dgSymmTensorField::Internal,
    0
);
defineTemplate2TypeNameAndDebug(dgTensorField::Internal, 0);

defineTemplateTypeNameAndDebug(dgScalarField, 0);
defineTemplateTypeNameAndDebug(dgVectorField, 0);
defineTemplateTypeNameAndDebug(dgSphericalTensorField, 0);
defineTemplateTypeNameAndDebug(dgSymmTensorField, 0);
defineTemplateTypeNameAndDebug(dgTensorField, 0);

defineTemplateTypeNameAndDebugWithName(tmpDgScalarField, "dgScalarField", 0);
defineTemplateTypeNameAndDebugWithName(tmpDgVectorField, "dgVectorField", 0);
defineTemplateTypeNameAndDebugWithName(tmpDgSphericalTensorField, "dgSphericalTensorField", 0);
defineTemplateTypeNameAndDebugWithName(tmpDgSymmTensorField, "dgSymmTensorField", 0);
defineTemplateTypeNameAndDebugWithName(tmpDgTensorField, "dgTensorField", 0);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

//specialization for scalar fields
template<>
tmp<GeometricField<scalar, dgPatchField, dgGeoMesh> >
GeometricField<scalar, dgPatchField, dgGeoMesh>::component
(
 const direction
) const
{
    return *this;
}


// specialization for scalar fields
template<>
void GeometricField<scalar, dgPatchField, dgGeoMesh>::replace
(
 const direction,
 const GeometricField<scalar, dgPatchField, dgGeoMesh>& gsf
 )
{
    *this == gsf;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
