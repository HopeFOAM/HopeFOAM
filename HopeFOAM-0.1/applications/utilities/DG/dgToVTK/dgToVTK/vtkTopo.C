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

#include "vtkTopo.H"
#include "polyMesh.H"
#include "cellShape.H"
#include "cellModeller.H"
#include "Swap.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

bool Foam::vtkTopo::decomposePoly = true;


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::vtkTopo::vtkTopo(const dgMesh& dgmesh, const bool quadCell)
:
    mesh_(dgmesh),
    dgMesh_(dgmesh),
    vertLabels_(),
    cellTypes_(),
    quadCell_(quadCell)
{
    const word tet = cellModeller::lookup("tet")->name();
    const word pyr = cellModeller::lookup("pyr")->name();
    const word prism = cellModeller::lookup("prism")->name();
    const word wedge = cellModeller::lookup("wedge")->name();
    const word tetWedge = cellModeller::lookup("tetWedge")->name();
    const word hex = cellModeller::lookup("hex")->name();
    const word quad = word("quad");
    const word tri = word("tri");

    // the dimension of dgMesh
    label meshDim = dgMesh_.meshDim();

    // Number of cell produced by splitting the high order cell to the low order cell
    label nSubCell = 0;

    //Number of point in dgMesh
    nDgPoint_ = 0;

    stdPoints_.setSize(2);
    stdVolCenterPoints_.setSize(2);
    interpolates_.setSize(2);
    volInterpolates_.setSize(2);
    forAll(stdPoints_ , i){
        stdPoints_[i].setSize(10);
        stdVolCenterPoints_[i].setSize(10);
        interpolates_[i].setSize(10);
        volInterpolates_[i].setSize(10);    
    }

    //const List<stdElement*> dgElement = dgMesh_.elements();
    const dgTree<physicalCellElement>& cellEleTree = dgMesh_.cellElementsTree();
    stdTypes_.setSize(cellEleTree.leafSize());

    // Scan for cells which need to be decomposed and count additional points
    // and cells
    if(decomposePoly){
        
        for(dgTree<physicalCellElement>::leafIterator iter = cellEleTree.leafBegin() ; iter != cellEleTree.end() ; ++iter){
            physicalCellElement& ele = iter()->value();
            const label meshOrd = ele.baseFunction().nOrder();
            const word cellShape = ele.baseFunction().cellShape();
            if(
                cellShape != hex
             && cellShape != wedge
             && cellShape != prism
             && cellShape != pyr
             && cellShape != tet
             && cellShape != tetWedge
             && cellShape != quad
             && cellShape != tri
            )
            {
                FatalErrorIn("vtkTopo::vtkTopo()")
                    << "does not support the cell type of "<<cellShape<<endl
                    << abort(FatalError);
            }

        }
    }

    label cellI = 0; // The index of the cell in the cellEleTree
    for(dgTree<physicalCellElement>::leafIterator iter = cellEleTree.leafBegin() ; iter != cellEleTree.end() ; ++iter){
        physicalCellElement& ele = iter()->value();
        const label meshOrd = ele.baseFunction().nOrder();
        const word cellShape = ele.baseFunction().cellShape();
        if(cellShape == quad){
            nSubCell += (meshOrd*meshOrd - 1);
            stdTypes_[cellI] = getType(cellShape);
            if(!quadCell_){
                nDgPoint_ += (meshOrd + 1) * (meshOrd + 1);
            }
                        //if using second order unit
            else{
                nDgPoint_ += (2*meshOrd + 1) * (2*meshOrd + 1);
                if(stdPoints_[stdTypes_[cellI]][meshOrd].size()<=0){
                    calStdPoints(meshOrd, stdTypes_[cellI]);
                    interpolates_[stdTypes_[cellI]][meshOrd] = ele.baseFunction().cellInterpolateMatrix(stdPoints_[stdTypes_[cellI]][meshOrd]);
                }
            }
            if(stdVolCenterPoints_[stdTypes_[cellI]][meshOrd].size()<=0){
                calStdVolCenterPoints(meshOrd, stdTypes_[cellI]);
                volInterpolates_[stdTypes_[cellI]][meshOrd] = ele.baseFunction().cellInterpolateMatrix(stdVolCenterPoints_[stdTypes_[cellI]][meshOrd]);
            }

        }
        else if(cellShape == tri){
            nSubCell += (meshOrd*meshOrd - 1);
            stdTypes_[cellI] = getType(cellShape);

            if(!quadCell_){
                nDgPoint_ += (meshOrd + 1) * (meshOrd + 2) / 2;
            }
            else{
                nDgPoint_ += (meshOrd + 1) * (2*meshOrd + 1);
                if(stdPoints_[stdTypes_[cellI]][meshOrd].size() <= 0){
                    calStdPoints(meshOrd, stdTypes_[cellI]);
                    interpolates_[stdTypes_[cellI]][meshOrd] = ele.baseFunction().cellInterpolateMatrix(stdPoints_[stdTypes_[cellI]][meshOrd]);
                }
            }
            if(stdVolCenterPoints_[stdTypes_[cellI]][meshOrd].size()<=0){
                calStdVolCenterPoints(meshOrd, stdTypes_[cellI]);
                volInterpolates_[stdTypes_[cellI]][meshOrd] = ele.baseFunction().cellInterpolateMatrix(stdVolCenterPoints_[stdTypes_[cellI]][meshOrd]);                
            }
        }
        else{
            FatalErrorIn("vtkTopo::vtkTopo()")
                << "does not support the cell type of "<<cellShape<<endl
                << abort(FatalError);
        }

        cellI++;
    }

    // List of vertex labels in VTK ordering
    vertLabels_.setSize(cellEleTree.leafSize() + nSubCell);

    // Label of vtk type
    cellTypes_.setSize(cellEleTree.leafSize() + nSubCell);

    // Set counters for additional points and additional cells
    label addPointI = 0, addCellI = 0;

    // Record the number of point(cell) which has been searched
    label nSearchPoint = 0, nSearchCell = 0;

    for(dgTree<physicalCellElement>::leafIterator iter = cellEleTree.leafBegin() ; iter != cellEleTree.end() ; ++iter){
        physicalCellElement& ele = iter()->value();

        const label meshOrd = ele.baseFunction().nOrder();
        const word cellShape = ele.baseFunction().cellShape();

        if(cellShape == tri){
            if(!quadCell_){
                label pointNumber = (meshOrd + 1) * (meshOrd + 2) / 2;
                label floor = 1;
                label fOrder = 1;
                label fNumber = meshOrd + 1 - floor;
                for(int i = meshOrd+1 ;  i<pointNumber ; i++){
                    if(fOrder == fNumber){
                        labelList& subTri = vertLabels_[nSearchCell];
                        subTri.setSize(3);
                            
                        //Organize a triangle downward
                        subTri[0] = nSearchPoint + i - fNumber - 1;
                        subTri[1] = nSearchPoint + i - fNumber;
                        subTri[2] = nSearchPoint + i;
                        cellTypes_[nSearchCell] = VTK_TRIANGLE;
                        nSearchCell++;

                        floor++;
                        fOrder = 1;
                        fNumber--;
                    }
                    else{
                        labelList& subTri = vertLabels_[nSearchCell];
                        subTri.setSize(3);
                            
                            //Organize a triangle downward
                        subTri[0] = nSearchPoint + i - fNumber - 1;
                        subTri[1] = nSearchPoint + i - fNumber;
                        subTri[2] = nSearchPoint + i;
                        cellTypes_[nSearchCell] = VTK_TRIANGLE;
                        nSearchCell++;
                            //Organize a triangle towords the right
                        labelList& subTri2 = vertLabels_[nSearchCell];
                        subTri2.setSize(3);
                        subTri2[0] = nSearchPoint + i - fNumber;
                        subTri2[1] = nSearchPoint + i + 1;
                        subTri2[2] = nSearchPoint + i;
                        cellTypes_[nSearchCell] = VTK_TRIANGLE;
                        nSearchCell++;
                        fOrder++;
                    }
                }
                nSearchPoint += pointNumber;
            }
            else{       //use the bi-cell
                label pointNumber = (meshOrd + 1) * (2*meshOrd + 1);
                for(int i=2 ; i<2*meshOrd +1 ; i+=2){
                    for(int j=0 ; j<2*meshOrd -i ; j+=2){
                        labelList& subBiTri = vertLabels_[nSearchCell];
                        subBiTri.setSize(6);

                        label tp = (4*meshOrd + 3 - i) * i/2 + j;
                        subBiTri[0] = nSearchPoint + tp - (4*meshOrd + 5 - 2*i);
                        subBiTri[1] = subBiTri[0] + 2;
                        subBiTri[2] = nSearchPoint + tp;
                        subBiTri[3] = subBiTri[0] + 1;
                        subBiTri[4] = subBiTri[3] + (2*meshOrd + 3 - i);
                        subBiTri[5] = subBiTri[4] - 1;
                        cellTypes_[nSearchCell] = VTK_BITRIANGLE;
                        nSearchCell++;

                        labelList& subBiTri2 = vertLabels_[nSearchCell];
                        subBiTri2.setSize(6);

                        subBiTri2[0] = nSearchPoint + tp - (4*meshOrd + 5 - 2*i) + 2;
                        subBiTri2[1] = nSearchPoint + tp + 2;
                        subBiTri2[2] = nSearchPoint + tp;
                        subBiTri2[3] = subBiTri2[0] + (2*meshOrd + 3 - i);
                        subBiTri2[4] = subBiTri2[2] + 1;
                        subBiTri2[5] = subBiTri2[3] - 1;
                        cellTypes_[nSearchCell] = VTK_BITRIANGLE;
                        nSearchCell++;
                    }
                        
                    label tp = (4*meshOrd + 3 - i) * i/2 + 2*meshOrd - i;
                    labelList& subBiTri = vertLabels_[nSearchCell];
                    subBiTri.setSize(6);

                    subBiTri[0] = nSearchPoint + tp - (4*meshOrd + 5 - 2*i);
                    subBiTri[1] = subBiTri[0] + 2;
                    subBiTri[2] = nSearchPoint + tp;
                    subBiTri[3] = subBiTri[0] + 1;
                    subBiTri[4] = subBiTri[3] + (2*meshOrd + 3 - i);
                    subBiTri[5] = subBiTri[4] - 1;
                    cellTypes_[nSearchCell] = VTK_BITRIANGLE;
                    nSearchCell++;
                }
                nSearchPoint += pointNumber;
            }
        }
        else if(cellShape == quad){
            if(!quadCell_){
                int pointNumber = (meshOrd + 1) * (meshOrd + 1);
                for(int i=meshOrd+1 ; i<pointNumber ; i++){
                    if((i + 1) % (meshOrd + 1) > 0){
                        labelList& subQuad = vertLabels_[nSearchCell];
                        subQuad.setSize(4);
                        subQuad[0] = nSearchPoint + i - meshOrd -1;
                        subQuad[1] = nSearchPoint + i - meshOrd;
                        subQuad[2] = nSearchPoint + i + 1;
                        subQuad[3] = nSearchPoint + i;
                        cellTypes_[nSearchCell] = VTK_QUAD;
                        nSearchCell++;
                    }
                }
                //    Info<<vertLabels_[nSearchCell_].size()<<endl;
                nSearchPoint  += pointNumber;
            }
            else{
                label pointNumber = (2*meshOrd + 1) * (2*meshOrd + 1);
                for(int i=2 ; i<2*meshOrd+1 ; i+=2){
                    for(int j=0 ; j<2*meshOrd ; j+=2){
                        labelList& subBiQuad = vertLabels_[nSearchCell];
                        subBiQuad.setSize(9);

                        label tp = (2*meshOrd + 1) * i + j;
                        subBiQuad[0] = nSearchPoint + tp - (4*meshOrd + 2);
                        subBiQuad[1] = subBiQuad[0] + 2;
                        subBiQuad[2] = nSearchPoint + tp + 2;
                        subBiQuad[3] = nSearchPoint + tp;
                        subBiQuad[4] = subBiQuad[0] + 1;
                        subBiQuad[5] = subBiQuad[1] + (2*meshOrd + 1);
                        subBiQuad[6] = subBiQuad[3] + 1;
                        subBiQuad[7] = subBiQuad[0] + (2*meshOrd + 1);
                        subBiQuad[8] = subBiQuad[4] + (2*meshOrd + 1);
                        cellTypes_[nSearchCell] = VTK_BIQUAD;
                        nSearchCell++;
                    }
                }
                nSearchPoint += pointNumber;
            }
        }
    }
    if (decomposePoly)
    {
        Pout<< "    Original cells:" << mesh_.nCells()
            << " points:" << mesh_.nPoints()
            << nl << endl;
    }
}

void Foam::vtkTopo::calStdPoints(const label meshOrder, const label type){
    
    // Bi-Triangle stdPoints
    if(type == STD_TRIANGLE){
        List<vector>& vtkTriPoints = stdPoints_[STD_TRIANGLE][meshOrder];
        label nVtkTriPoints = (meshOrder+1)*(2*meshOrder+1);
        vtkTriPoints.setSize(nVtkTriPoints);

        int tp = 0;
        for(int i=0 ; i<2*meshOrder+1 ; i++){
            for(int j=0 ; j<2*meshOrder+1-i ; j++){
                scalar sCoor = (scalar)i/meshOrder-1;
                scalar rCoor = (scalar)j/meshOrder-1;
                vtkTriPoints[tp] = vector(rCoor, sCoor, 0);
                tp++;
            }
        }
    }

    else if(type == STD_QUAD){
        List<vector>& vtkQuadPoints = stdPoints_[STD_QUAD][meshOrder];
        label nVtkQuadPoints = (2*meshOrder+1)*(2*meshOrder+1);
        vtkQuadPoints.setSize(nVtkQuadPoints);

        int tp = 0;
        for(int i=0 ; i<2*meshOrder+1 ; i++){
            for(int j=0 ; j<2*meshOrder+1 ; j++){
                scalar sCoor = (scalar)i/meshOrder-1;
                scalar rCoor = (scalar)j/meshOrder-1;
                vtkQuadPoints[tp] = vector(rCoor, sCoor, 0);
                tp++;
            }
        }
    }
    else{
        FatalErrorInFunction
            << "Can't support the cell type yet!"
            << exit(FatalError);
    }
}

void Foam::vtkTopo::calStdVolCenterPoints(const label meshOrder, const label type){
    
    if(type == STD_TRIANGLE){
        List<vector>& triCenterPoints = stdVolCenterPoints_[STD_TRIANGLE][meshOrder];
        label nTriCenterPoints = meshOrder*meshOrder;
        triCenterPoints.setSize(nTriCenterPoints);

        int tp = 0;
        for(int i=0 ; i<2*meshOrder-1 ; i++){
            if(i%2 == 0){
                scalar sBase = (scalar)i/meshOrder - 1;
                scalar rBase = 0;
                for(int j=0 ; j<meshOrder-(i/2) ; j++){
                    scalar sCoor = (sBase*3 + (scalar)2/meshOrder)/3 - 1;
                    scalar rCoor = (rBase*3 + (scalar)2/meshOrder)/3 - 1;
                    rBase += 2.0/meshOrder;
                    triCenterPoints[tp] = vector(rCoor, sCoor, 0);
                    tp++;
                }
            }
            else{
                scalar sBase = (scalar)(i+1)/meshOrder - 1;
                scalar rBase = 2.0/meshOrder;
                for(int j=0 ; j<meshOrder-(i+1)/2 ; j++){
                    scalar sCoor = (sBase*3 - 2.0/meshOrder)/3 - 1;
                    scalar rCoor = (rBase*3 - 2.0/meshOrder)/3 - 1;
                    rBase += 2.0/meshOrder;
                    triCenterPoints[tp] = vector(rCoor, sCoor, 0);
                    tp++;
                }
            }
        }
    }

    else if(type == STD_QUAD){
        List<vector>& quadCenterPoints = stdVolCenterPoints_[STD_QUAD][meshOrder];
        label nQuadCenterPoints = meshOrder*meshOrder;
        quadCenterPoints.setSize(nQuadCenterPoints);

        int tp = 0;
        for(int i=0 ; i<meshOrder ; i++){
            for(int j=0 ; j<meshOrder ; j++){
                scalar sCoor = (scalar)(2*i+1)/meshOrder-1;
                scalar rCoor = (scalar)(2*j+1)/meshOrder-1;
                quadCenterPoints[tp] = vector(rCoor, sCoor, 0);
                tp++;
            }
        }
    }
    else{
        FatalErrorInFunction
            << "Can't support the cell type yet!"
            << exit(FatalError);
    }
}

//Get the standard type of the cell which is defined in vtkTopo.H.
const Foam::label Foam::vtkTopo::getType(const word cellShape){
    const word tet = cellModeller::lookup("tet")->name();
    const word pyr = cellModeller::lookup("pyr")->name();
    const word prism = cellModeller::lookup("prism")->name();
    const word wedge = cellModeller::lookup("wedge")->name();
    const word tetWedge = cellModeller::lookup("tetWedge")->name();
    const word hex = cellModeller::lookup("hex")->name();
    const word quad = word("quad");
    const word tri = word("tri");


    const label meshDim = dgMesh_.meshDim();
    if(cellShape == quad){
        if(meshDim==2){
            const label res = STD_QUAD;
            return res;
        }
        else{
            FatalErrorInFunction
                << "Can't support the cell type! "
                << exit(FatalError);
        }
    }
    else if(cellShape == tri){
        if(meshDim == 2){
            const label res = STD_TRIANGLE; 
            return res;
        }
        else{
            FatalErrorInFunction
                << "Can't support the cell type! "
                << exit(FatalError);
        }
    }
    else{
        FatalErrorInFunction
            << "Can't support the cell type! "
            << exit(FatalError);
    }
}