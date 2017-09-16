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

#include "internalWriter.H"
#include "writeFuns.H"
#include "cellModeller.H"
#include "vtkTopo.H"
#include "polyMesh.H"
#include "cellShape.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::internalWriter::internalWriter
(
    const vtkMesh& vMesh,
    const bool binary,
    const bool quadCell,
    const fileName& fName
)
:
    vMesh_(vMesh),
    binary_(binary),
    quadCell_(quadCell),
    fName_(fName),
    os_(fName.c_str())
{
    const dgMesh& mesh = vMesh_.mesh();
    const vtkTopo& topo = vMesh_.topo();

    // Write header
    writeFuns::writeHeader(os_, binary_, mesh.time().caseName());
    os_ << "DATASET UNSTRUCTURED_GRID" << std::endl;


    //------------------------------------------------------------------
    //
    // Write topology
    //
    //------------------------------------------------------------------

    const label nTotPoints = vMesh_.topo().nDgPoint();
    DynamicList<floatScalar> ptField(3*nTotPoints);

    const dgTree<physicalCellElement>& cellEleTree = mesh.cellElementsTree();
    // Using quadratic cell in VTK files
    if(quadCell_){
        label cellI = 0;

        // Traverse all cells
        for(dgTree<physicalCellElement>::leafIterator iter = cellEleTree.leafBegin() ; iter != cellEleTree.end() ; ++iter){
            physicalCellElement& ele = iter()->value();
            const labelList& stdTypes = topo.stdTypes();
            const label cellOrd = ele.baseFunction().nOrder();
            List<vector> biPoints(cellOrd);
            denseMatrix<vector>::MatVecMult(topo.interpolates(stdTypes[cellI], cellOrd), ele.dofLocation(), biPoints);
            writeFuns::insert(biPoints, ptField);

            cellI++;
        }
    }

    else{
        // Traverse all cells
        for(dgTree<physicalCellElement>::leafIterator iter = cellEleTree.leafBegin() ; iter != cellEleTree.end() ; ++iter){
            physicalCellElement& ele = iter()->value();
            writeFuns::insert(ele.dofLocation(), ptField);
        }
    }

    os_ << "POINTS " << nTotPoints << " float" << std::endl;
    
    writeFuns::write(os_, binary_, ptField);

    //------------------------------------------------------------------
    //
    // Write cells
    //
    //------------------------------------------------------------------

    const labelListList& vtkVertLabels = topo.vertLabels();

    // Count total number of vertices referenced.
    label nFaceVerts = 0;

    forAll(vtkVertLabels, celli)
    {
        nFaceVerts += vtkVertLabels[celli].size() + 1;
    }

    os_ << "CELLS " << vtkVertLabels.size() << ' ' << nFaceVerts << std::endl;

    DynamicList<label> vertLabels(nFaceVerts);

    forAll(vtkVertLabels, celli)
    {
        const labelList& vtkVerts = vtkVertLabels[celli];
        vertLabels.append(vtkVerts.size());
        writeFuns::insert(vtkVerts, vertLabels);
    }
    writeFuns::write(os_, binary_, vertLabels);


    const labelList& vtkCellTypes = topo.cellTypes();

    os_ << "CELL_TYPES " << vtkCellTypes.size() << std::endl;

    // Make copy since writing might swap stuff.
    DynamicList<label> cellTypes(vtkCellTypes.size());

    writeFuns::insert(vtkCellTypes, cellTypes);

    writeFuns::write(os_, binary_, cellTypes);
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::internalWriter::writeCellIDs()
{
    const dgMesh& mesh = vMesh_.mesh();
    const vtkTopo& topo = vMesh_.topo();
    const labelList& vtkCellTypes = topo.cellTypes();

    // Cell ids first
    os_ << "cellID 1 " << vtkCellTypes.size() << " int" << std::endl;

    labelList cellId(vtkCellTypes.size());
    label labelI = 0;


    if (vMesh_.useSubMesh())
    { 
        FatalErrorIn("internalWriter::writeCellIDs")
            << "does not support the function of subMesh "<<endl
            << abort(FatalError);
    }
    else
    {
        for(int i=0 ; i<vtkCellTypes.size() ; i++)
        {
            cellId[i] = i;
        }
    }

    writeFuns::write(os_, binary_, cellId);
}


// ************************************************************************* //
