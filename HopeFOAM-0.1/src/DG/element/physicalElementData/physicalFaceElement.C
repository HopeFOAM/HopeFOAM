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

Author
    Xu Liyang (xucloud77@gmail.com)
\*---------------------------------------------------------------------------*/

#include "physicalFaceElement.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::physicalFaceElement::physicalFaceElement()
{
}

Foam::physicalFaceElement::physicalFaceElement(const physicalFaceElement& face)
{
    faceOwnerLocalID_ = face.faceOwnerLocalID_;
    faceNeighborLocalID_ = face.faceNeighborLocalID_;
    ownerEle_ = face.ownerEle_;
    neighborEle_ = face.neighborEle_;
    faceRotate_ = face.faceRotate_;
    nGaussPoints_ = face.nGaussPoints_;
    ownerFaceStart_ = face.ownerFaceStart_;
    neighbourFaceStart_ = face.neighbourFaceStart_;
    gaussOrder_ = face.gaussOrder_;
    faceNx_ = face.faceNx_;
    faceWJ_ = face.faceWJ_;
    sequenceIndex_ = face.sequenceIndex();
    fscale_ = face.fscale_;
}

// * * * * * * * * * * * * * * * * Destructors  * * * * * * * * * * * * * * //
Foam::physicalFaceElement::~physicalFaceElement()
{
}

// * * * * * * * * * * * * *  Member Functions * *  * * * * * ** * * * * * //

void Foam::physicalFaceElement::updateSequenceIndex(Pair<label> index){
    sequenceIndex_ = index;
}

const Foam::label Foam::physicalFaceElement::nOwnerDof()const{
    return ownerEle_->value().baseFunction().nDofPerFace();
}

const Foam::label Foam::physicalFaceElement::nNeighborDof()const{
    if(neighborEle_ == NULL){
        FatalErrorIn("physicalFaceElement::nNeighborDof()")
                << "neighborEle_ is NULL "<<endl
                << abort(FatalError);
    }
    return neighborEle_->value().baseFunction().nDofPerFace();
}

const Foam::labelList& Foam::physicalFaceElement::ownerDofMapping()
{
    return ownerEle_->value().baseFunction().faceToCellIndex()[faceOwnerLocalID_][0];
}

const Foam::labelList& Foam::physicalFaceElement::neighborDofMapping()
{
    return neighborEle_->value().baseFunction().faceToCellIndex()[faceNeighborLocalID_][faceRotate_];
}

const Foam::labelList& Foam::physicalFaceElement::neighborNoRotateDofMapping()
{
    return neighborEle_->value().baseFunction().faceToCellIndex()[faceNeighborLocalID_][0];
}