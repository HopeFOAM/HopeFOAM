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
    Foam::vtkMesh

Description
    Encapsulation of VTK mesh data. Holds dgMesh and
    high-order cells decomposition on it.

SourceFiles
    vtkMesh.C

\*---------------------------------------------------------------------------*/

#ifndef vtkMesh_H
#define vtkMesh_H

#include "vtkTopo.H"
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// Forward declaration of classes
class Time;

/*---------------------------------------------------------------------------*\
                           Class vtkMesh Declaration
\*---------------------------------------------------------------------------*/

class vtkMesh
{
    // Private data

        //- Reference to mesh
        dgMesh& baseMesh_;

        //- Current cellSet (or empty)
        const word setName_;
        const bool quadCell_;

        //- Current decomposition of topology
        mutable autoPtr<vtkTopo> topoPtr_;

    // Private Member Functions

        //- Disallow default bitwise copy construct
        vtkMesh(const vtkMesh&);

        //- Disallow default bitwise assignment
        void operator=(const vtkMesh&);


public:

    // Constructors
        //- Construct from components
        vtkMesh(dgMesh& baseMesh, const word& setName = "", const bool quadCell = false);

    // Member Functions
        
        // Access

            //- whole mesh
            const dgMesh& baseMesh() const
            {
                return baseMesh_;
            }

            //- if using quadratic cells 
            const bool quadCell() const
            {
                return quadCell_;
            }

            //- if using subMesh, disabled in this version
            bool useSubMesh() const
            {
                return setName_.size();
            }

            //- topology
            const vtkTopo& topo() const
            {
                if (topoPtr_.empty())
                {
                    topoPtr_.reset(new vtkTopo(mesh(), quadCell_));
                }
                return topoPtr_();
            }

            //- Access either mesh or submesh
            const dgMesh& mesh() const
            {
                if (useSubMesh())
                {
                    FatalErrorInFunction
                        << "Can't support function of subset yet!"
                        << exit(FatalError);
                }
                else
                {
                    return baseMesh_;
                }
            }

            //- Number of field cells
            label nFieldCells() const
            {
                return topo().cellTypes().size();
            }

            //- Number of field points
            label nFieldPoints() const
            {
                return topo().nDgPoint();
            }


        // Edit

            //- Read mesh
            polyMesh::readUpdateState readUpdate();


            //- Map volume field (does in fact do very little interpolation;
            //  just copied from fvMeshSubset)
            template<class GeoField>
            tmp<GeoField> interpolate(const GeoField& fld) const
            {
                if (useSubMesh())
                {
                    FatalErrorInFunction
                        << "Can't support function of subset yet!"
                        << exit(FatalError);
                }
                else
                {
                    return fld;
                }
            }
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
