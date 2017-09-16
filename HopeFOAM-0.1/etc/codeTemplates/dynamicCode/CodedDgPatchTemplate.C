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

#include "CodedDgPatchTemplate.H"
#include "addToRunTimeSelectionTable.H"
#include "fvPatchFieldMapper.H"
#include "volFields.H"
#include "surfaceFields.H"
#include "unitConversion.H"
//{{{ begin codeInclude
${codeInclude}
//}}} end codeInclude


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * Local Functions * * * * * * * * * * * * * * //

//{{{ begin localCode
${localCode}
//}}} end localCode


// * * * * * * * * * * * * * * * Global Functions  * * * * * * * * * * * * * //

extern "C"
{
	// dynamicCode:
	// SHA1 = ${SHA1sum}
	//
	// unique function name that can be checked if the correct library version
	// has been loaded
	void ${typeName}_${SHA1sum}(bool load)
	{
		if (load) {
			// code that can be explicitly executed after loading
		} else {
			// code that can be explicitly executed before unloading
		}
	}
}

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //


defineTypeNameAndDebug( ${typeName}DgPatch, 0);
addRemovableToRunTimeSelectionTable(dgPatch,  ${typeName}DgPatch, polyPatch);


const char* const ${typeName}DgPatch::SHA1sum =
    "${SHA1sum}";


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

${typeName}DgPatch::
${typeName}DgPatch
(
   const polyPatch& patch, 
   const dgBoundaryMesh& bm
   )
:
dgPatch(patch, bm)
{}




// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

//Get vector value from parameter equation
vector ${typeName}DgPatch::function(scalar u,scalar v)const
{
	
	vector func(0.0, 0.0, 0.0);
	if (${verbose:-false}) {
		Info<<"updateCoeffs ${typeName} sha1: ${SHA1sum}\n";
	}
	func = vector(
//{{{ begin code
	${code}
//}}} end code
	);
	return func;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //
