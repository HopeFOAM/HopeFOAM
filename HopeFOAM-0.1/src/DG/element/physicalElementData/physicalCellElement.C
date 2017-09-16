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

#include "physicalCellElement.H"

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::physicalCellElement::physicalCellElement()
{
}

Foam::physicalCellElement::physicalCellElement(const physicalCellElement& cell)
:
    baseFunction_(const_cast<stdElement*>(&(cell.baseFunction()))),
    sequenceIndex_(cell.sequenceIndex()),
    dofStart_(cell.dofStart()),
    dofLocation_(cell.dofLocation()),
    gaussStart_(cell.gaussStart()),
    dxdr_(cell.dxdr()),
    cellDxdr_(cell.cellDxdr()),
    cellDrdx_(cell.cellDrdx()),
    faceElementList_(cell.faceElementList()),
    jacobianWeights_(cell.jacobianWeights()),
    massMatrix_(cell.massMatrix()),
    cellD1dx_(cell.cellD1dx())
{

}

// * * * * * * * * * * * * * * * * Destructors  * * * * * * * * * * * * * * //
Foam::physicalCellElement::~physicalCellElement()
{
	dofLocation_.clear();
	dxdr_.clear();
    cellDxdr_.clear();
    cellDrdx_.clear();
	jacobianWeights_.clear();
	cellD1dx_.clear();
	faceElementList_.clear();
}



// * * * * * * * * *  Member Functions (data access from base function, including linear integration matrix)  * * * * * * * * //
void Foam::physicalCellElement::initElement(stdElement * stdEle, const Foam::List<Foam::vector>& cellPoints)
{
	baseFunction_ = stdEle;

    dofLocation_ = baseFunction_->physicalNodesLoc(cellPoints);
    dxdr_ = baseFunction_->dxdr(dofLocation_);
    drdx_ = baseFunction_->drdx(dxdr_);
    cellDxdr_ = baseFunction_->gaussCellDxdr(dofLocation_);
    jacobianWeights_ = baseFunction_->gaussCellWJ(cellDxdr_);

    initMassMatrix();

    cellDrdx_ = baseFunction_->gaussCellDrdx(cellDxdr_);
    label nDof = baseFunction_->nDofPerCell();
    label nGaussDof = baseFunction_->gaussNDofPerCell();
    cellD1dx_.setSize(nGaussDof, nDof, vector::zero);
    const denseMatrix<vector>& Dr = baseFunction_->gaussCellDr();
    //d_dx = Dr*drdx + Ds*dsdx +...
    for(int i=0, ptr=0; i < nGaussDof; i++){
        for(int j=0; j < nDof; j++, ptr++){
            cellD1dx_[ptr][0] = Dr[ptr] & vector(cellDrdx_[i][0], cellDrdx_[i][1], cellDrdx_[i][2]);
            cellD1dx_[ptr][1] = Dr[ptr] & vector(cellDrdx_[i][3], cellDrdx_[i][4], cellDrdx_[i][5]);
            cellD1dx_[ptr][2] = Dr[ptr] & vector(cellDrdx_[i][6], cellDrdx_[i][7], cellDrdx_[i][8]);
        }
    }

    faceElementList_.setSize(baseFunction_->nFacesPerCell());
}

void Foam::physicalCellElement::initMassMatrix()
{
    label nDof = baseFunction_->nDofPerCell();
    label nGaussDof = baseFunction_->gaussNDofPerCell();
    massMatrix_.setSize(nDof, nDof, 0);

    const denseMatrix<scalar>& gaussVr = baseFunction_->gaussCellVandermonde();

    denseMatrix<scalar>::MatDiagMatMult(gaussVr, jacobianWeights_, gaussVr, massMatrix_);
}

void Foam::physicalCellElement::updateDofStart(label start)
{
    dofStart_ = start;
}

void Foam::physicalCellElement::updateGaussStart(label start)
{
    gaussStart_ = start;
}

void Foam::physicalCellElement::updateSequenceIndex(label index)
{
    sequenceIndex_ = index;
}