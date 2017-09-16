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

#include "emptyDgPatch.H"
#include "dgBoundaryMesh.H"
#include "dgMesh.H"
#include "addToRunTimeSelectionTable.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

using std::shared_ptr;
using std::make_shared;

namespace Foam
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

defineTypeNameAndDebug(emptyDgPatch, 0);
addToRunTimeSelectionTable(dgPatch, emptyDgPatch, polyPatch);


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

emptyDgPatch::emptyDgPatch(const polyPatch& patch, const dgBoundaryMesh& bm)
:
    dgPatch(patch, bm),
    dgFaceCells_(NULL),
    faceCells_
    (
        labelList::subList
        (
            boundaryMesh().mesh().faceOwner(), 0, patch.start()
        )
    )
{
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

const List<shared_ptr<dgTreeUnit<dgPolyCell>>>& emptyDgPatch::dgFaceCells() const
{
    return dgFaceCells_;
}

const labelUList& emptyDgPatch::faceCells() const
{
    return faceCells_;
}
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //
