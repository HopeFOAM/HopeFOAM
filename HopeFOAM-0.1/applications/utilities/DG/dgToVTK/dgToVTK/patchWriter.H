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
    Foam::patchWriter

Description
    Write patch fields

SourceFiles
    patchWriter.C
    patchWriterTemplates.C

\*---------------------------------------------------------------------------*/

#ifndef patchWriter_H
#define patchWriter_H

#include "pointMesh.H"
#include "OFstream.H"
#include "volFields.H"
#include "pointFields.H"
#include "vtkMesh.H"
#include "indirectPrimitivePatch.H"
#include "GeometricDofField.H"
#include "dgPatchField.H"
#include "PrimitivePatchInterpolation.H"

using namespace Foam;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

class volPointInterpolation;

/*---------------------------------------------------------------------------*\
                           Class patchWriter Declaration
\*---------------------------------------------------------------------------*/

class patchWriter
{
    const vtkMesh& vMesh_;

    const bool binary_;

    const bool nearCellValue_;

    const fileName fName_;

    const labelList patchIDs_;

    std::ofstream os_;

    label nPoints_;

    label nFaces_;

public:

    // Constructors

        //- Construct from components
        patchWriter
        (
            const vtkMesh&,
            const bool binary,
            const bool nearCellValue,
            const fileName&,
            const labelList& patchIDs
        );


    // Member Functions

        std::ofstream& os()
        {
            return os_;
        }

        label nPoints() const
        {
            return nPoints_;
        }

        label nFaces() const
        {
            return nFaces_;
        }

        //- Write cellIDs
        void writePatchIDs();

        //- Write dgFields
        template<class Type>
        void write
        (
            const UPtrList
            <
                const GeometricDofField<Type, dgPatchField, dgGeoMesh>
            >&
        );

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
    #include "patchWriterTemplates.C"
#endif


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
