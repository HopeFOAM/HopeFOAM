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

Description
    Template for use with dynamic code generation of a
    fixedValueDgPatchField.

    - without state

SourceFiles
    fixedValueDgPatchFieldTemplate.C

\*---------------------------------------------------------------------------*/

#ifndef ${typeName}DgPatch_H
#define ${typeName}DgPatch_H

#include "dgPatch.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                         A templated FixedValueFvPatch
\*---------------------------------------------------------------------------*/

class ${typeName}DgPatch
:
public dgPatch 
	{
public:

	//- Information about the SHA1 of the code itself
	static const char* const SHA1sum;

	//- Runtime type information
	TypeName("${typeName}");


	// Constructors

	//- Construct from patch and internal field
	${typeName}DgPatch
	(
	   const polyPatch& patch,
	   const dgBoundaryMesh& bm
	);
	

	//- Destructor
	virtual ~${typeName}DgPatch(){}


	// Member functions

	//- Update the coefficients associated with the patch field
	virtual vector function(scalar u, scalar v) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
