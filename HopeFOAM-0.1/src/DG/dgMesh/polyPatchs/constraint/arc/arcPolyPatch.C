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

#include "arcPolyPatch.H"
#include "addToRunTimeSelectionTable.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(arcPolyPatch, 0);

    addToRunTimeSelectionTable(polyPatch, arcPolyPatch, word);
    addToRunTimeSelectionTable(polyPatch, arcPolyPatch, dictionary);
}

// * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * * * * * //

Foam::arcPolyPatch::arcPolyPatch
(
    const word& name,
    const label size,
    const label start,
    const label index,
    const polyBoundaryMesh& bm,
    const word& patchType
)
:
    polyPatch(name, size, start, index, bm, patchType),
    dict_(),
    name(),
    u_Range(0, 0),
    v_Range(0,0),
    dyCode(dict_)
{}


Foam::arcPolyPatch::arcPolyPatch
(
    const word& name,
    const dictionary& dict,
    const label index,
    const polyBoundaryMesh& bm,
    const word& patchType
)
:
    polyPatch(name, dict, index, bm, patchType),
    dict_(dict),
    name(dict.lookup("name")),
    u_Range(dict.lookup("u_Range")),
    v_Range(dict.lookup("v_Range")),
    dyCode(dict)

{}


Foam::arcPolyPatch::arcPolyPatch
(
    const arcPolyPatch& pp,
    const polyBoundaryMesh& bm
)
:
    polyPatch(pp, bm),
    dict_(pp.dict()),
    name(),
    u_Range(0, 0),
    v_Range(0,0),
    dyCode(dict_)
{}


Foam::arcPolyPatch::arcPolyPatch
(
    const arcPolyPatch& pp,
    const polyBoundaryMesh& bm,
    const label index,
    const label newSize,
    const label newStart
)
:
    polyPatch(pp, bm, index, newSize, newStart),
    dict_(pp.dict()),
    name(),
    u_Range(0, 0),
    v_Range(0,0),
    dyCode(dict_)
{
    dict_.add("nFaces", newSize, true);
    dict_.add("startFace", newStart, true);
}


Foam::arcPolyPatch::arcPolyPatch
(
    const arcPolyPatch& pp,
    const polyBoundaryMesh& bm,
    const label index,
    const labelUList& mapAddressing,
    const label newStart
)
:
    polyPatch(pp, bm, index, mapAddressing, newStart),
    dict_(pp.dict()),
    name(),
    u_Range(0, 0),
    v_Range(0,0),
    dyCode(dict_)
{
    dict_.add("nFaces", mapAddressing.size(), true);
    dict_.add("startFace", newStart, true);
}

void Foam::arcPolyPatch::write(Ostream& os) const
{
    dict_.write(os,false);
}
		


// ************************************************************************* //
