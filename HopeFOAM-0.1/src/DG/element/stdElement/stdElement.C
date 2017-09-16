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

#include "stdElement.H"


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::stdElement::stdElement(label nOrder, word cellShape)
{
	baseFunction_ = baseFunction::New(nOrder, cellShape);
	gaussIntegration_ = gaussIntegration::New(baseFunction_());
	faceInterpolateMatrix_.setSize(10);
	gaussFaceInterpolateMatrix_.setSize(10, List<denseMatrix<scalar>>(baseFunction_->faceRotateIndex_.size()));
}


// * * * * * * * * * * * * * * * * Destructors  * * * * * * * * * * * * * * //
Foam::stdElement::~stdElement()
{
	baseFunction_.clear();
}

// * * * * * * * * *  Member Functions (data access from base function, including linear integration matrix)  * * * * * * * * //
const Foam::denseMatrix<Foam::scalar>& Foam::stdElement::faceInterpolateMatrix(label neighborOrder)
{
	if(neighborOrder > 9){
		FatalErrorInFunction
            << "the order of element cannot larger than " << neighborOrder
            << "in Foam::denseMatrix<scalar>& Foam::stdElement::faceInterpolateMatrix(label neighborOrder)"
            << abort(FatalError);
    }
	if(faceInterpolateMatrix_[neighborOrder].size()==0){
		if((baseFunction_->nOrder_) == neighborOrder){
			faceInterpolateMatrix_[neighborOrder].setToUnitMatrix(baseFunction_->nDofPerFace_);
		}

		//new neighborOrder base element
		autoPtr<baseFunction> neighborElement = baseFunction::New(neighborOrder, baseFunction_->cellShape_);
		// get face points location of neighborOrder base element
		const vectorList& faceDofLoc = neighborElement->faceDofLocation_;
		// calc vandermonde matrix (the value of neighbor cell's base function at the face points)
		denseMatrix<scalar> faceVandermondeMatrix = baseFunction_->faceVandermonde(faceDofLoc);

		// multiply the face vandermonde matrix with neighbor face's invMatrix
		//faceInterpolateMatrix_[neighborOrder] = faceVandermondeMatrix * (baseFunction_->invFaceMatrix_);
		denseMatrix<scalar>::MatMatMult(faceVandermondeMatrix, (baseFunction_->invFaceMatrix_), faceInterpolateMatrix_[neighborOrder]);

		neighborElement.clear();
	}
	return faceInterpolateMatrix_[neighborOrder];
}

const Foam::denseMatrix<Foam::scalar> Foam::stdElement::cellInterpolateMatrix(const vectorList& loc)const
{
	denseMatrix<scalar> cellV(baseFunction_->cellVandermonde(loc));
	denseMatrix<scalar> ans(cellV.rowSize(), baseFunction_->invV_.colSize());
	denseMatrix<scalar>::MatMatMult(cellV, baseFunction_->invV_, ans);
	return ans;
}

const Foam::denseMatrix<Foam::scalar> Foam::stdElement::faceInterpolateMatrix(const vectorList& loc)const
{
	denseMatrix<scalar> faceV(baseFunction_->faceVandermonde(loc));
	denseMatrix<scalar> ans(faceV.rowSize(), baseFunction_->invFaceMatrix_.colSize());
	denseMatrix<scalar>::MatMatMult(faceV, baseFunction_->invFaceMatrix_, ans);
	return ans;
}
// ************************************************************************* //