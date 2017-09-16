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

#include "processorDgPatch.H"
#include "addToRunTimeSelectionTable.H"
#include "transformField.H"
#include "physicalFaceElement.H"
#include "dgTree.H"
#include "dgMesh.H"
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

defineTypeNameAndDebug(processorDgPatch, 0);
addToRunTimeSelectionTable(dgPatch, processorDgPatch, polyPatch);


// * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * * //

//- pre delcare of inter-cell transport data
    //- 1 label -- cell shape
    //- 1 label -- order
    //- 1 label -- faceLocalId
    //- 1 label -- global shift
    //- 1 label -- faceDn row size
    //- 1 label -- faceDn col size
    //- 1 vector -- face rotate
    //- (ndof x nGauss) scalar -- faceDn
    //- 1 scalar -- fscale
void processorDgPatch::updateNeighborData(){
    if(!Pstream::parRun()){
        return ;
    }
    label activeFaceNum = 0; // number of active faces
    label scalarSize = 0; // number to count size of (1 + faceDn);
    const dgMesh& mesh = this->boundaryMesh().mesh();
    label globalShift = mesh.localRange().first();
    const labelList& dgFaceIndex = this->dgFaceIndex();
    const dgTree<physicalFaceElement>&  dgFaceTree = mesh.faceElementsTree();

    //- set size
    if(dgFaceIndex.size()>0){
        for(dgTree<physicalFaceElement>::leafIterator iter = dgFaceTree.leafBegin(dgFaceIndex); iter != dgFaceTree.end(); ++iter){
            activeFaceNum ++;
            scalarSize += (1 + iter()->value().gaussOwnerFaceDn_.actualSize());
        }
    }
    this->neighborData_.setSize(activeFaceNum);

    /* TODO:
    * remenber to change the labelBuff and scalarBuff to support hp adaption.
    */
    //- label part data
    List<label> labelBuff(activeFaceNum * 6);
    if(dgFaceIndex.size()>0){
        int index = 0;
        for(dgTree<physicalFaceElement>::leafIterator iter = dgFaceTree.leafBegin(dgFaceIndex); iter != dgFaceTree.end(); ++iter){
            const physicalFaceElement& faceEle = iter()->value();
            labelBuff[index++] = stringToCellShape(faceEle.ownerEle_->value().baseFunction().cellShape());
            labelBuff[index++] = faceEle.gaussOrder_;
            labelBuff[index++] = faceEle.faceOwnerLocalID_;
            labelBuff[index++] = globalShift + faceEle.ownerEle_->value().dofStart();
            labelBuff[index++] = faceEle.gaussOwnerFaceDn_.rowSize();
            labelBuff[index++] = faceEle.gaussOwnerFaceDn_.colSize();
        }
    }
    this->compressedSend(Pstream::blocking, labelBuff);
    this->compressedReceive(Pstream::blocking, labelBuff);

    //- scalar part data
    for(int i=0, index=4; i<activeFaceNum; i++, index+=6){
        scalarSize += (3 + labelBuff[index]*labelBuff[index+1]);
        scalarSize++; // Insert fscale
    }
    List<scalar> scalarBuff(scalarSize);
    if(dgFaceIndex.size()>0){
        int index = 0;
        for(dgTree<physicalFaceElement>::leafIterator iter = dgFaceTree.leafBegin(dgFaceIndex); iter != dgFaceTree.end(); ++iter){
            const physicalFaceElement& faceEle = iter()->value();
            vector vecLoc = faceEle.ownerEle_->value().dofLocation()[const_cast<physicalFaceElement&>(faceEle).ownerDofMapping()[0]];
            scalarBuff[index++] = vecLoc.x();
            scalarBuff[index++] = vecLoc.y();
            scalarBuff[index++] = vecLoc.z();
            label dnSize = faceEle.gaussOwnerFaceDn_.actualSize();
            for(int j=0; j<dnSize; j++){
                scalarBuff[index++] = faceEle.gaussOwnerFaceDn_[j];
            }
            scalarBuff[index++] = faceEle.fscale_;
        }
    }
    this->compressedSend(Pstream::blocking, scalarBuff);
    this->compressedReceive(Pstream::blocking, scalarBuff);

    //- set the neighbor part of faceEle
    if(dgFaceIndex.size()>0){
        int labelIndex = 0, scalarIndex = 0;
        for(dgTree<physicalFaceElement>::leafIterator iter = dgFaceTree.leafBegin(dgFaceIndex); iter != dgFaceTree.end(); ++iter){
            physicalFaceElement& faceEle = iter()->value();
            //- set neighborLocalID_;
            faceEle.faceNeighborLocalID_ = labelBuff[labelIndex + 2];
            //- new coupled physicalCellData class
            shared_ptr<physicalCellElement> tmpCellEle = make_shared<physicalCellElement>();
            tmpCellEle->setBaseFunction(const_cast<dgMesh&>(mesh).elementSets().getElement(labelBuff[labelIndex+1], cellShapeToString(labelBuff[labelIndex])));
            tmpCellEle->updateDofStart(labelBuff[labelIndex+3] - globalShift);
            faceEle.neighborEle_ = make_shared<dgTreeUnit<physicalCellElement>>(tmpCellEle);
            //- set faceRotate_
            /* TODU:
            *  use base function to determin faceRotate_;
            */
            if(mag(vector(scalarBuff[scalarIndex++], scalarBuff[scalarIndex++], scalarBuff[scalarIndex++])
                 - faceEle.ownerEle_->value().dofLocation()[const_cast<physicalFaceElement&>(faceEle).ownerDofMapping()[0]]) < SMALL){
                faceEle.faceRotate_ = 0;
            }
            else{
                faceEle.faceRotate_ = 1;
            }
            //Pout << "face "<<i<<"with rotate = "<<faceEle.faceRotate_<<endl;

            //- set faceDn
            label tmpScalarSize = labelBuff[labelIndex+4]*labelBuff[labelIndex+5];
            faceEle.gaussNeighborFaceDnRotate_.setSize(labelBuff[labelIndex+4], labelBuff[labelIndex+5]);
            const labelList& faceRotate = faceEle.neighborEle_->value().baseFunction().gaussFaceRotateIndex()[faceEle.faceRotate_];
            for(int i=0, ptr=0; i<labelBuff[labelIndex+4]; i++){
                for(int j=0, ptr1=labelBuff[labelIndex+5]*faceRotate[i]+scalarIndex; j<labelBuff[labelIndex+5]; j++, ptr++, ptr1++){
                    faceEle.gaussNeighborFaceDnRotate_[ptr] = scalarBuff[ptr1];
                }
            }
            scalarIndex += tmpScalarSize;

            //
            faceEle.fscale_ = max(faceEle.fscale_, scalarBuff[scalarIndex++]);

            //- index increase
            labelIndex += 6;
        }
    }
}

Foam::word processorDgPatch::cellShapeToString(int cell){
    switch(cell){
        case cellShape::line: return "line"; break;
        case cellShape::tri: return "tri"; break;
        case cellShape::quad: return "quad"; break;
        case cellShape::tet: return "tet"; break;
        case cellShape::hex: return "hex"; break;
    }
    return "null";
}
int processorDgPatch::stringToCellShape(word cell)
{
    if(cell == "line")
        return cellShape::line;
    else if(cell == "tri")
        return cellShape::tri;
    else if(cell == "quad")
        return cellShape::quad;
    else if(cell == "tet")
        return cellShape::tet;
    else if(cell == "hex")
        return cellShape::hex;
    return -1;
}

void processorDgPatch::makeWeights(scalarField& w) const
{
    if (Pstream::parRun())
    {
        // The face normals point in the opposite direction on the other side
        scalarField neighbFaceCentresCn
        (
            (
                procPolyPatch_.neighbFaceAreas()
               /(mag(procPolyPatch_.neighbFaceAreas()) + VSMALL)
            )
          & (
              procPolyPatch_.neighbFaceCentres()
            - procPolyPatch_.neighbFaceCellCentres())
        );

  //      w = neighbFaceCentresCn/((nf()&dgPatch::delta()) + neighbFaceCentresCn);//del by RXG
    }
    else
    {
        w = 1.0;
    }
}

void processorDgPatch::sendPstream(const scalarList& l, PstreamBuffers& pBufs) const
{
    if (Pstream::parRun())
    {
        UOPstream toNeighbProc(neighbProcNo(), pBufs);

        toNeighbProc << l;
    }
}

void processorDgPatch::receivePstream(scalarList& l, PstreamBuffers& pBufs) const
{
    if (Pstream::parRun())
    {
        {
            UIPstream fromNeighbProc(neighbProcNo(), pBufs);

            fromNeighbProc >> l;
        }
    }
}


void processorDgPatch::sendPstream(const labelList& l, PstreamBuffers& pBufs) const
{
    if (Pstream::parRun())
    {
        UOPstream toNeighbProc(neighbProcNo(), pBufs);

        toNeighbProc << l;
    }
}

void processorDgPatch::receivePstream(labelList& l, PstreamBuffers& pBufs) const
{
    if (Pstream::parRun())
    {
        {
            UIPstream fromNeighbProc(neighbProcNo(), pBufs);

            fromNeighbProc >> l;
        }
    }
}

tmp<labelField> processorDgPatch::interfaceInternalField
(
    const labelUList& internalData
) const
{
    return patchInternalField(internalData);
}


void processorDgPatch::initInternalFieldTransfer
(
    const Pstream::commsTypes commsType,
    const labelUList& iF
) const
{
    send(commsType, patchInternalField(iF)());
}


tmp<labelField> processorDgPatch::internalFieldTransfer
(
    const Pstream::commsTypes commsType,
    const labelUList&
) const
{
    return receive<label>(commsType, this->size());
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //
