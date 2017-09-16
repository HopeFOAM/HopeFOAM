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

InClass
    Foam::dgFields

Description

SourceFiles
    dgFields.C

\*---------------------------------------------------------------------------*/

#ifndef dgFieldsFwd_H
#define dgFieldsFwd_H

#include "fieldTypes.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

class dgGeoMesh;

template<class Type>
class dgPatchField;

template<class Type, template<class> class PatchField, class GeoMesh>
class GeometricDofField;
template<class Type, template<class> class PatchField, class GeoMesh>
class GeometricField;

typedef GeometricDofField<scalar, dgPatchField, dgGeoMesh> dgScalarField;
typedef GeometricDofField<vector, dgPatchField, dgGeoMesh> dgVectorField;
typedef GeometricDofField<sphericalTensor, dgPatchField, dgGeoMesh>
    dgSphericalTensorField;
typedef GeometricDofField<symmTensor, dgPatchField, dgGeoMesh> dgSymmTensorField;
typedef GeometricDofField<tensor, dgPatchField, dgGeoMesh> dgTensorField;

typedef GeometricField<scalar, dgPatchField, dgGeoMesh> tmpDgScalarField;
typedef GeometricField<vector, dgPatchField, dgGeoMesh> tmpDgVectorField;
typedef GeometricField<sphericalTensor, dgPatchField, dgGeoMesh>
    tmpDgSphericalTensorField;
typedef GeometricField<symmTensor, dgPatchField, dgGeoMesh> tmpDgSymmTensorField;
typedef GeometricField<tensor, dgPatchField, dgGeoMesh> tmpDgTensorField;

typedef List<UPtrList<scalar>> dofScalarField;
typedef List<UPtrList<vector>> dofVectorField;
typedef List<UPtrList<sphericalTensor>> dofSphericalTensorField;
typedef List<UPtrList<symmTensor>> dofSymmTensorField;
typedef List<UPtrList<tensor>> dofTensorField;
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //

