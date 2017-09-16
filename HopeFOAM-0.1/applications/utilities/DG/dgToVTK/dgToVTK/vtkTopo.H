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
    Foam::vtkTopo

Description
    Decomposition of high-order DG cells to low-order cells and
    generation of quadratic cells interpolation matrix and cellField
    interpolation matrix according to basefunction and the number of order.

SourceFiles
    vtkTopo.C

\*---------------------------------------------------------------------------*/

#ifndef vtkTopo_H
#define vtkTopo_H

#include "labelList.H"
#include "dgMesh.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// Forward declaration of classes
class polyMesh;

/*---------------------------------------------------------------------------*\
                           Class vtkTopo Declaration
\*---------------------------------------------------------------------------*/

class vtkTopo
{
    // Private data

        const polyMesh& mesh_;

        const dgMesh& dgMesh_;

        //- If true, then the cell will be expressed by quadratic cell
        const bool quadCell_;

        //- Vertices per cell (including added cells) in VTK ordering
        labelListList vertLabels_;

        //- Cell types (including added cells) in VTK numbering
        labelList cellTypes_;

        //- Coordinates of standard cells' points after DG cell mapping
        List<List<List<vector>>> stdPoints_;

        //- Coordinates of standard cells' center points, used for cellField interpolation
        List<List<List<vector>>> stdVolCenterPoints_;

        //- Interpolation matrix for caculation field value of quadratic cells
        List<List<denseMatrix<scalar>>> interpolates_;

        //- Interpolation matrix for caculation cellField
        List<List<denseMatrix<scalar>>> volInterpolates_;

        //- The number of dg points in dgMesh
        label nDgPoint_;

        //- stdType label for each cell
        labelList stdTypes_;


    // Private Member Functions

        //- Disallow default bitwise copy construct
        vtkTopo(const vtkTopo&);

        //- Disallow default bitwise assignment
        void operator=(const vtkTopo&);


public:

   // Public static data

        //- equivalent to enumeration in "vtkCellType.h"
        enum vtkTypes
        {
            VTK_TRIANGLE   = 5,
            VTK_POLYGON    = 7,
            VTK_QUAD       = 9,

            VTK_TETRA      = 10,
            VTK_HEXAHEDRON = 12,
            VTK_WEDGE      = 13,
            VTK_PYRAMID    = 14,
            VTK_BITRIANGLE = 22,
            VTK_BIQUAD     = 28,
            VTK_POLYHEDRON = 42
        };

        //- Assign a number to each type of cell, index for stdPoints_,
        //  stdVolCenterPoints_, interpolates_ and volInterpolates_
        enum stdTypes
        {
            STD_TRIANGLE   = 0,
            STD_QUAD    = 1
        };
    //- Enable/disable polyhedron decomposition. Default = true
    static bool decomposePoly;


    // Constructors

        //- Construct from components
        vtkTopo(const dgMesh&, const bool);

    // Member Functions

        //- Calculate the coordinates of standard cells' points
        void calStdPoints(const label meshOrd, const label type);

        //- Calculate the coordinates of standard cells' center
        void calStdVolCenterPoints(const label meshOrd, const label type);
        
        // Access
            //- Get the stdTypes according to cell type name
            const label getType(const word);
         
            inline const label nDgPoint() const
            {
                return nDgPoint_;
            }

            inline const labelList& stdTypes() const
            {
                return stdTypes_;
            }

            inline const labelListList& vertLabels() const
            {
                return vertLabels_;
            }

            inline const labelList& cellTypes() const
            {
                return cellTypes_;
            }

            inline const List<vector> stdPoints(const label vtkType, const label meshOrd) const
            {
                return stdPoints_[vtkType][meshOrd];
            }

            inline const List<vector> stdVolCenterPoints(const label vtkType, const label meshOrd) const
            {
                return stdVolCenterPoints_[vtkType][meshOrd];
            }

            inline const denseMatrix<scalar>& interpolates(const label vtkType, const label meshOrd) const
            {
                return interpolates_[vtkType][meshOrd];
            }

            inline const denseMatrix<scalar>& volInterpolates(const label vtkType, const label meshOrd) const
            {
                return volInterpolates_[vtkType][meshOrd];
            }
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
