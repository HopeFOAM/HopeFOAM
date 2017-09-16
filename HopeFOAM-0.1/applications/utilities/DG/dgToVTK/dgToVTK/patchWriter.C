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

#include "patchWriter.H"
#include "writeFuns.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::patchWriter::patchWriter
(
    const vtkMesh& vMesh,
    const bool binary,
    const bool nearCellValue,
    const fileName& fName,
    const labelList& patchIDs
)
:
    vMesh_(vMesh),
    binary_(binary),
    nearCellValue_(nearCellValue),
    fName_(fName),
    patchIDs_(patchIDs),
    os_(fName.c_str())
{
    const dgMesh& mesh = vMesh_.mesh();
    const dgBoundaryMesh& patches = mesh.boundary();
    const dgTree<physicalFaceElement>& faceEleTree = mesh.faceElementsTree();

    // Write header
    if (patchIDs_.size() == 1)
    {
        writeFuns::writeHeader(os_, binary_, patches[patchIDs_[0]].name());
    }
    else
    {
        writeFuns::writeHeader(os_, binary_, "patches");
    }
    os_ << "DATASET POLYDATA" << std::endl;

    // Write topology
    nPoints_ = 0;
    nFaces_ = 0;
    label nFaceVerts = 0;

    forAll(patchIDs_, patchI){
        const dgPatch& patch = patches[patchIDs_[patchI]];
        const labelList& dgFaceIndex = patch.dgFaceIndex();
        if(dgFaceIndex.size()<=0) continue;

        for(dgTree<physicalFaceElement>::leafIterator iter = faceEleTree.leafBegin(dgFaceIndex) ; iter != faceEleTree.end() ; ++iter){
            physicalFaceElement& faceEle = iter()->value();
            nPoints_ += faceEle.nOwnerDof();
            nFaces_ ++;
            nFaceVerts += faceEle.nOwnerDof() + 1;
        }

    }

    os_ << "POINTS " << nPoints_ << " float" << std::endl;

    DynamicList<floatScalar> ptField(3*nPoints_);

    //const List<List<vector>> & cellDofLocation = const_cast<dgMesh&>(mesh).dofLocation();
    List<vector> facePoints;
    forAll(patchIDs_, patchI){
        const dgPatch& patch = patches[patchIDs_[patchI]];
        const labelList& dgFaceIndex = patch.dgFaceIndex();
        if(dgFaceIndex.size()<=0) continue;
        for(dgTree<physicalFaceElement>::leafIterator iter = faceEleTree.leafBegin(dgFaceIndex) ; iter != faceEleTree.end() ; ++iter){
            physicalFaceElement& faceEle = iter()->value();
            const labelList& dofMapping = faceEle.ownerDofMapping();
            const List<vector>& cellDof = faceEle.ownerEle_->value().dofLocation();
            facePoints.setSize(dofMapping.size());
            forAll(dofMapping, dofI){
                facePoints[dofI] = cellDof[dofMapping[dofI]];
            }
            writeFuns::insert(facePoints, ptField);
        }
    }

    writeFuns::write(os_, binary_, ptField);

    os_ << "POLYGONS " << nFaces_ << ' ' << nFaceVerts << std::endl;

    DynamicList<label> vertLabels(nFaceVerts);

    label offset = 0;

    forAll(patchIDs_, i)
    {
        const dgPatch& patch = patches[patchIDs_[i]];
        const labelList& dgFaceIndex = patch.dgFaceIndex();
        if(dgFaceIndex.size()<=0) continue;

        for(dgTree<physicalFaceElement>::leafIterator iter = faceEleTree.leafBegin(dgFaceIndex) ; iter != faceEleTree.end() ; ++iter){
            physicalFaceElement& faceEle = iter()->value();
            label pointNum = faceEle.nOwnerDof();
            labelList pointIndex(pointNum);
            forAll(pointIndex, i){
                pointIndex[i] = offset + i;
            }
            vertLabels.append(pointNum);
            writeFuns::insert(pointIndex, vertLabels);
            offset += pointNum;
        }
    }
    writeFuns::write(os_, binary_, vertLabels);
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void Foam::patchWriter::writePatchIDs()
{
    const dgMesh& mesh = vMesh_.mesh();
    const dgBoundaryMesh& patches = mesh.boundary();
    //const dgTree<physicalFaceElement>& faceEleTree = mesh.faceElementsTree();

    DynamicList<floatScalar> fField(nFaces_);

    os_ << "patchID 1 " << nFaces_ << " float" << std::endl;

    forAll(patchIDs_, patchI){
        const dgPatch& patch = patches[patchIDs_[patchI]];
        const labelList& dgFaceIndex = patch.dgFaceIndex();

        if (dgFaceIndex.size()>0){
            writeFuns::insert(scalarField(dgFaceIndex.size(), patchIDs_[patchI]), fField);
        }
    }
    writeFuns::write(os_, binary_, fField);
}


// ************************************************************************* //
