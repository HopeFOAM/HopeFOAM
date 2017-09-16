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
    Foam::internalWriter

Description
    Write fields (internal).

SourceFiles
    internalWriter.C
    internalWriterTemplates.C

\*---------------------------------------------------------------------------*/

#ifndef internalWriter_H
#define internalWriter_H

#include "OFstream.H"
#include "volFields.H"
#include "pointFields.H"
#include "vtkMesh.H"
#include "GeometricDofField.H"

using namespace Foam;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

class volPointInterpolation;

/*---------------------------------------------------------------------------*\
                           Class internalWriter Declaration
\*---------------------------------------------------------------------------*/

class internalWriter
{
    const vtkMesh& vMesh_;

    const bool binary_;

    const bool quadCell_;

    const fileName fName_;

    List<List<List<vector>>> stdPoint_;
    List<List<denseMatrix<scalar>>> interpolate_;

    std::ofstream os_;

public:

        enum stdTypes
        {
            STD_TRIANGLE   = 0,
            STD_QUAD    = 1
        };
    // Constructors

        //- Construct from components
        internalWriter
        (
            const vtkMesh&,
            const bool binary,
            const bool quadCell,
            const fileName&
        );


    // Member Functions

        std::ofstream& os()
        {
            return os_;
        }

        //- Write cellIDs
        void writeCellIDs();

        //- Write generic GeometricFields
        template<class Type, template<class> class PatchField, class GeoMesh>
        void write
        (
            const UPtrList<const GeometricDofField<Type, PatchField, GeoMesh>>&
        );

        //- Write cellField generated from interpolation of dgField
        template<class Type, template<class> class PatchField, class GeoMesh>
        void writeDgToVol
        (
            const UPtrList<const GeometricDofField<Type, PatchField, GeoMesh>>&
        );
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
    #include "internalWriterTemplates.C"
#endif


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
