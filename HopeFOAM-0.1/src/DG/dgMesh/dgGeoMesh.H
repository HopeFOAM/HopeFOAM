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

Class
    Foam::dgGeoMesh
 
Description
    Mesh data needed to do the Discontinuous Galerkin discretisation.
    Only one dimension case

\*---------------------------------------------------------------------------*/

#ifndef dgGeoMesh_H
#define dgGeoMesh_H

#include "GeoMesh.H"
#include "dgMesh.H"
#include "primitiveMesh.H"

namespace Foam{

/*---------------------------------------------------------------------------*\
                     Class dgGeoMesh Declaration
\*---------------------------------------------------------------------------*/

class dgGeoMesh
:
	public GeoMesh<dgMesh>
{

	// Demand-driven data used for one dimensional case
		
		//- Mapping from element to point

public:

	// Constructors

		//- Construct from components
		explicit dgGeoMesh(const dgMesh& mesh)
		:
			GeoMesh<dgMesh>(mesh)
		{}

	// Member Functions

		// Access

			//- Return number of points
			label size() const
			{
				return size(mesh_);
			}

			//- Return number of points
			static label size(const Mesh& mesh)
			{
				return mesh.totalDof();
			}

			//- Return one dimensial point location

};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

}// End namespace Foam

#endif

// ************************************************************************* //
