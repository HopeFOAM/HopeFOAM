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

#ifndef dgFields_H
#define dgFields_H

#include "GeometricFields.H"
#include "GeometricDofFields.H"
#include "dgGeoMesh.H"
#include "dgMesh.H"
#include "dgPatchField.H"
#include "dgFieldsFwd.H"
#include "calculatedDgPatchFields.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

    template<>
	tmp<GeometricField<scalar, dgPatchField, dgGeoMesh> >
	GeometricField<scalar, dgPatchField, dgGeoMesh>::component
	(
	 const direction
	) const;

    template<>
	void GeometricField<scalar, dgPatchField, dgGeoMesh>::replace
	(
	 const direction,
	 const GeometricField<scalar, dgPatchField, dgGeoMesh>& sf
	);

	
} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //

