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
    Foam::emptyDgPatch

Description
    A patch which will not exist in the dgMesh. Typical example is a front and
    back plane of a 2-D geometry

SourceFiles
    emptyDgPatch.C

\*---------------------------------------------------------------------------*/

#ifndef emptyDgPatch_H
#define emptyDgPatch_H

#include "dgPatch.H"
#include "emptyPolyPatch.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

using std::shared_ptr;
using std::make_shared;

namespace Foam
{

/*---------------------------------------------------------------------------*\
                           Class emptyDgPatch Declaration
\*---------------------------------------------------------------------------*/

class emptyDgPatch
:
    public dgPatch
{
    // Private data

        //- face-cell addressing
        const List<shared_ptr<dgTreeUnit<dgPolyCell>>> dgFaceCells_;
        const labelList::subList faceCells_;

public:

    //- Runtime type information
    TypeName(emptyPolyPatch::typeName_());


    // Constructors

        //- Construct from polyPatch
        emptyDgPatch(const polyPatch& patch, const dgBoundaryMesh& bm);


    // Member Functions

        // Access

            virtual label size() const
            {
                return 0;
            }

            //- Return faceCells

            virtual const labelUList& faceCells() const;
            virtual const List<shared_ptr<dgTreeUnit<dgPolyCell>>>& dgFaceCells() const;

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
