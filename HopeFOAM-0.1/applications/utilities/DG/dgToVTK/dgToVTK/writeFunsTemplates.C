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

#include "writeFuns.H"
#include "interpolatePointToCell.H"
#include "GeometricDofField.H"
#include "dgPatchField.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// Store List in dest
template<class Type>
void Foam::writeFuns::insert
(
    const List<Type>& source,
    DynamicList<floatScalar>& dest
)
{
    forAll(source, i)
    {
        insert(source[i], dest);
    }
}


template<class Type>
void Foam::writeFuns::write
(
    std::ostream& os,
    const bool binary,
    const GeometricDofField<Type, dgPatchField, dgGeoMesh>& pvf,
    const vtkMesh& vMesh
)
{
    const dgMesh& mesh = vMesh.mesh();
    const vtkTopo& topo = vMesh.topo();
    const labelList& stdTypes = topo.stdTypes();

    const label nTotPoints = topo.nDgPoint();

    os  << pvf.name() << ' ' << pTraits<Type>::nComponents << ' '
        << nTotPoints << " float" << std::endl;

    DynamicList<floatScalar> fField(pTraits<Type>::nComponents*nTotPoints);

    const dgTree<physicalCellElement>& cellEleTree = mesh.cellElementsTree();
    bool quadCell = vMesh.quadCell();
    label cellI = 0;

    for(dgTree<physicalCellElement>::leafIterator iter = cellEleTree.leafBegin() ; iter != cellEleTree.end() ; ++iter){
        physicalCellElement& ele = iter()->value();
        const label size = ele.dofLocation().size();
        const label start = ele.dofStart();
        const stdElement& baseFunction = ele.baseFunction();

        List<Type> cellData(size);
        forAll(cellData, pointI){
            cellData[pointI] = pvf[start+pointI];
        }

        if(quadCell){
            List<Type> biField = topo.interpolates(stdTypes[cellI], baseFunction.nOrder())*cellData;
            writeFuns::insert(biField, fField);
        }

        else{
            writeFuns::insert(cellData, fField);
        }
        cellI++;
    }
    write(os, binary, fField);
}


template<class Type>
void Foam::writeFuns::writeDgToVol
(
    std::ostream& os,
    const bool binary,
    const GeometricDofField<Type, dgPatchField, dgGeoMesh>& pvf,
    const vtkMesh& vMesh
)
{
    const dgMesh& mesh = vMesh.mesh();
    const vtkTopo& topo = vMesh.topo();
    const labelList& stdTypes = topo.stdTypes();

    const label nTotPoints = topo.cellTypes().size();

    os  << pvf.name() << ' ' << pTraits<Type>::nComponents << ' '
        << nTotPoints << " float" << std::endl;

    DynamicList<floatScalar> fField(pTraits<Type>::nComponents*nTotPoints);

    const dgTree<physicalCellElement>& cellEleTree = mesh.cellElementsTree();
    bool quadCell = vMesh.quadCell();
    label cellI = 0;

    for(dgTree<physicalCellElement>::leafIterator iter = cellEleTree.leafBegin() ; iter != cellEleTree.end() ; ++iter){
        physicalCellElement& ele = iter()->value();
        const label size = ele.dofLocation().size();
        const label start = ele.dofStart();
        const stdElement& baseFunction = ele.baseFunction();

        List<Type> cellData(size);
        forAll(cellData, pointI){
            cellData[pointI] = pvf[start+pointI];
        }

        List<Type> volField(baseFunction.nOrder() * baseFunction.nOrder());
        denseMatrix<Type>::MatVecMult(topo.volInterpolates(stdTypes[cellI], baseFunction.nOrder()), cellData, volField);
        writeFuns::insert(volField, fField);
        cellI++;
    }

    write(os, binary, fField);
}


template<class Type, template<class> class PatchField, class GeoMesh>
void Foam::writeFuns::write
(
    std::ostream& os,
    const bool binary,
    const PtrList<GeometricDofField<Type, PatchField, GeoMesh>>& flds,
    const vtkMesh& vMesh
)
{
    forAll(flds, i)
    {
        write(os, binary, flds[i], vMesh);
    }
}

template<class Type, template<class> class PatchField, class GeoMesh>
void Foam::writeFuns::writeDgToVol
(
    std::ostream& os,
    const bool binary,
    const PtrList<GeometricDofField<Type, PatchField, GeoMesh>>& flds,
    const vtkMesh& vMesh
)
{
    forAll(flds, i)
    {
        writeDgToVol(os, binary, flds[i], vMesh);
    }
}

// ************************************************************************* //
