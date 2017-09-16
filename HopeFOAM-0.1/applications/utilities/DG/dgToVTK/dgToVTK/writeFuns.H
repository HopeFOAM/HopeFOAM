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
    Foam::writeFuns

Description
    Various functions for collecting and writing binary data.

SourceFiles
    writeFuns.C
    writeFunsTemplates.C

\*---------------------------------------------------------------------------*/

#ifndef writeFuns_H
#define writeFuns_H

#include "floatScalar.H"
#include "DynamicList.H"
#include "GeometricDofField.H"
#include "vtkMesh.H"
#include "dgPatchField.H"
#include "volPointInterpolation.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                           Class writeFuns Declaration
\*---------------------------------------------------------------------------*/

class writeFuns
{
    // Private Member Functions

        // Swap halves of word.

            static void swapWord(label& word32);
            static void swapWords(const label nWords, label* words32);


public:

    // Write ascii or binary. If binary optionally in-place swaps argument

        static void write(std::ostream&, const bool, List<floatScalar>&);
        static void write(std::ostream&, const bool, DynamicList<floatScalar>&);
        static void write(std::ostream&, const bool, labelList&);
        static void write(std::ostream&, const bool, DynamicList<label>&);


    // Write header

        static void writeHeader
        (
            std::ostream&,
            const bool isBinary,
            const std::string& title
        );
        static void writeCellDataHeader
        (
            std::ostream&,
            const label nCells,
            const label nFields
        );
        static void writeDgDataHeader
        (
            std::ostream&,
            const label nPoints,
            const label nFields
        );


    // Convert to VTK and store

        static void insert(const scalar, DynamicList<floatScalar>&);
        static void insert(const point&, DynamicList<floatScalar>&);
        static void insert(const sphericalTensor&, DynamicList<floatScalar>&);
        static void insert(const symmTensor&, DynamicList<floatScalar>&);
        static void insert(const tensor&, DynamicList<floatScalar>&);


    //- Append elements to DynamicList
    static void insert(const labelList&, DynamicList<label>&);
    template<class Type>
    static void insert(const List<Type>&, DynamicList<floatScalar>&);

    //- Write dgField on all mesh points.
    template<class Type>
    static void write
    (
        std::ostream&,
        const bool binary,
        const GeometricDofField<Type, dgPatchField, dgGeoMesh>&,
        const vtkMesh&
    );

    //- Write generic GeometricFields
    template<class Type, template<class> class PatchField, class GeoMesh>
    static void write
    (
        std::ostream&,
        const bool binary,
        const PtrList<GeometricDofField<Type, PatchField, GeoMesh>>&,
        const vtkMesh&
    );

    //- Write cellField from pointField on all dgMesh points. Interpolate to cell centre
    //  for decomposed cell centres.
    template<class Type>
    static void writeDgToVol
    (
        std::ostream&,
        const bool binary,
        const GeometricDofField<Type, dgPatchField, dgGeoMesh>&,
        const vtkMesh&
    );

    //- Write generic GeometricVolFields
    template<class Type, template<class> class PatchField, class GeoMesh>
    static void writeDgToVol
    (
        std::ostream&,
        const bool binary,
        const PtrList<GeometricDofField<Type, PatchField, GeoMesh>>&,
        const vtkMesh&
    );

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
    #include "writeFunsTemplates.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
