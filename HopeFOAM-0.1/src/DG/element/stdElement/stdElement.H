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
    Foam::stdElement

Description
    standard Element, inclulde baseFunction and gauss integration Method.

SourceFiles
    stdElement.C

Author
    Xu Liyang (xucloud77@gmail.com)
\*---------------------------------------------------------------------------*/

#ifndef stdElement_H
#define stdElement_H

#include "baseFunction.H"
#include "gaussIntegration.H"
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                      Class stdElement Declaration
\*---------------------------------------------------------------------------*/
class stdElement
{
	//- Disallow default bitwise copy construct
    stdElement(const stdElement&);

    // std Element Data

    	//- base function ptr
    	autoPtr<baseFunction> baseFunction_;
    	//- gauss Integration ptr
    	autoPtr<gaussIntegration> gaussIntegration_;

        //- interpolation matrix from this baseElement face data to critical order baseElement face data, list for different order
        List<denseMatrix<scalar>> faceInterpolateMatrix_;
        //- interpolation matrix from this baseElement face data to critical order gaussElement face data, list for different order
        List<List<denseMatrix<scalar>>> gaussFaceInterpolateMatrix_;

public:

    // Declare name of the class and its debug switch
    ClassName("stdElement");

    //- Constructors
    stdElement(label nOrder, word cellShape);

    //- Destructor
    virtual ~stdElement();

    //- Data Access

    	//- data from baseFunction and linear Integration matrix
    	inline label nOrder()const{return baseFunction_->nOrder_;}
    	inline const word cellShape()const{return baseFunction_->cellShape_;}
    	inline label nDofPerCell()const{return baseFunction_->nDofPerCell_;}
    	inline label nFacesPerCell()const{return baseFunction_->nFacesPerCell_;}
    	inline label nDofPerFace()const{return baseFunction_->nDofPerFace_;}
        inline const List<labelListList>& faceToCellIndex()const{return baseFunction_->faceToCellIndex_;}
        inline const labelListList& faceRotateIndex()const{return baseFunction_->faceRotateIndex_;}
        inline const labelListList& faceVertex()const{return baseFunction_->faceVertex_;}
    	inline const denseMatrix<scalar>& Vandermonde()const{return baseFunction_->V_;}
    	inline const denseMatrix<scalar>& invVandermonde()const{return baseFunction_->invV_;}
        inline const denseMatrix<scalar>& invFaceMatrix()const{return baseFunction_->invFaceMatrix_;}
        inline denseMatrix<scalar> cellVandermonde(const vectorList& loc)const{return baseFunction_->cellVandermonde(loc);}
        inline denseMatrix<scalar> faceVandermonde(const vectorList& loc)const{return baseFunction_->faceVandermonde(loc);}
        inline const vectorList& dofLocation()const{return baseFunction_->dofLocation_;}
        inline const vectorList& faceDofLocation()const{return baseFunction_->faceDofLocation_;}
        inline const vectorList physicalNodesLoc(const vectorList& nodes)const{return baseFunction_->physicalNodesLoc(nodes);}
        inline const tensorList dxdr(const vectorList& physicalLocs){return baseFunction_->dxdr(physicalLocs);}
        inline const tensorList drdx(const tensorList& xr)const{return baseFunction_->drdx(xr);}
        inline const scalarList jacobian(const tensorList& xr)const{return baseFunction_->jacobian(xr);}
        inline void faceNxFscale(const tensorList& xr, label faceI, vectorList& nx, scalarList& fscale)const{return baseFunction_->faceNxFscale(xr, faceI, nx, fscale);}
        inline void faceFscale(const tensorList& xr, label faceI, scalar& fscale)const{return baseFunction_->faceFscale(xr, faceI, fscale);}

        const denseMatrix<scalar> cellInterpolateMatrix(const vectorList& loc)const;
        const denseMatrix<scalar> faceInterpolateMatrix(const vectorList& loc)const;
        const denseMatrix<scalar>& faceInterpolateMatrix(label neighborOrder);

        inline const denseMatrix<scalar>& massMatrix()
        {
			if(baseFunction_->massMatrix_.size() == 0){
				baseFunction_->initMassMatrix();
			}
			return baseFunction_->massMatrix_;
		}

        inline const denseMatrix<scalar>& invMassMatrix()
        {
			if(baseFunction_->invMassMatrix_.size() == 0){
				baseFunction_->initInvMassMatrix();
			}
			return baseFunction_->invMassMatrix_;
		}

        inline const denseMatrix<vector>& drMatrix()
        {
			if(baseFunction_->drMatrix_.size() == 0){
				baseFunction_->initDrMatrix();
			}
			return baseFunction_->drMatrix_;
		}

        inline const denseMatrix<scalar>& Lift()
        {
			if(baseFunction_->Lift_.size() == 0){
				baseFunction_->initLift();
			}
			return baseFunction_->Lift_;
		}

        inline const denseMatrix<scalar>& cellCentreInterpolateMatrix()
        {
            if(baseFunction_->cellCentreInterpolateMatrix_.size() == 0){
                baseFunction_->initCentreInterpolateMatrix();
            }
            return baseFunction_->cellCentreInterpolateMatrix_;
        }

        inline const denseMatrix<scalar>& faceCentreInterpolateMatrix()
        {
            if(baseFunction_->faceCentreInterpolateMatrix_.size() == 0){
                baseFunction_->initCentreInterpolateMatrix();
            }
            return baseFunction_->faceCentreInterpolateMatrix_;
        }

        void addFaceShiftToCell(const vectorList& shift, label faceI, vectorList& dofLoc)
        {
            return baseFunction_->addFaceShiftToCell(shift, faceI, dofLoc);
        };

        //- data from gauss integration method
        // cell integration data
        //-
        inline label gaussVolIntOrder()const{return gaussIntegration_->volIntOrder_;}
        inline label gaussNDofPerCell()const{return gaussIntegration_->nDofPerCell_;}
        //- integration points in standard reference cell
        inline const vectorList& gaussCellIntLocation()const{return gaussIntegration_->cellIntLocation_;}
        //- integration weights
        inline const scalarList& gaussCellIntWeights()const{return gaussIntegration_->cellIntWeights_;}
        //-generalized Vandermonde of Lagrange interpolant functions at cubature nodes
        inline const denseMatrix<scalar>& gaussCellVandermonde()const{return gaussIntegration_->cellVandermonde_;}
        //-local derivatives of Lagrange interpolants at cubature nodes
        inline const denseMatrix<vector>& gaussCellDr()const{return gaussIntegration_->cellDr_;}
        //-physical nodes location of gauss cell points for certain cell nodes;
        inline const vectorList gaussCellNodesLoc(const vectorList& nodes)const{return gaussIntegration_->gaussCellNodesLoc(nodes);}
        //- gauss cell points dxdr
        inline const tensorList gaussCellDxdr(const vectorList& physicalLocs){return gaussIntegration_->cellDxdr(physicalLocs);}
        //- gauss cell points drdx
        inline const tensorList gaussCellDrdx(const tensorList& xr){return gaussIntegration_->cellDrdx(xr);}
        //- gauss cell integration factor, jacobian * weight
        inline const scalarList gaussCellWJ(const tensorList& xr)const{return gaussIntegration_->cellWJ(xr);}

        // face integration data
        //-
        inline label gaussFaceIntOrder()const{return gaussIntegration_->faceIntOrder_;}
        //-
        inline label gaussNDofPerFace()const{return gaussIntegration_->nDofPerFace_;}
        //-
        inline const labelListList& gaussFaceRotateIndex()const{return gaussIntegration_->faceRotateIndex_;}
        //- integration points in face
        inline const vectorList& gaussFaceIntLocation()const{return gaussIntegration_->faceIntLocation_;}
        //- integration weights
        inline const scalarList& gaussFaceIntWeights()const{return gaussIntegration_->faceIntWeights_;}
        //- interpolate matrix from node base points to gauss face points
        inline const denseMatrix<scalar>& gaussFaceInterp()const{return gaussIntegration_->faceInterp_;}

        inline const denseMatrix<scalar>& gaussFaceInterpolateMatrix(label neighborOrder, label rotate) {
            if(neighborOrder > 9){
                FatalErrorInFunction
                    << "the order of element cannot larger than " << neighborOrder
                    << "in Foam::denseMatrix<scalar>& Foam::stdElement::gaussFaceInterpolateMatrix(label neighborOrder)"
                    << abort(FatalError);
            }
            if(gaussFaceInterpolateMatrix_[neighborOrder][0].size()==0){
                //new neighborOrder gaussbase element
                autoPtr<baseFunction> neighborElement = baseFunction::New(neighborOrder, baseFunction_->cellShape_);
                autoPtr<gaussIntegration> neighborGaussElement = gaussIntegration::New(neighborElement());

                // get face points location of neighborOrder base element
                const vectorList& faceDofLoc = neighborGaussElement->faceIntLocation_;
                // calc vandermonde matrix (the value of base function at the gauss face points)
                denseMatrix<scalar> faceVandermondeMatrix = baseFunction_->faceVandermonde(faceDofLoc);

                // multiply the face vandermonde matrix with invMatrix
                //gaussFaceInterpolateMatrix_[neighborOrder] = faceVandermondeMatrix * (baseFunction_->invFaceMatrix_);
                gaussFaceInterpolateMatrix_[neighborOrder][0].setSize(faceVandermondeMatrix.rowSize(), (baseFunction_->invFaceMatrix_).colSize());
                denseMatrix<scalar>::MatMatMult(faceVandermondeMatrix, (baseFunction_->invFaceMatrix_), gaussFaceInterpolateMatrix_[neighborOrder][0]);

                neighborGaussElement.clear();
                neighborElement.clear();
            }
            if(gaussFaceInterpolateMatrix_[neighborOrder][rotate].size()==0){
                gaussFaceInterpolateMatrix_[neighborOrder][rotate] = gaussFaceInterpolateMatrix_[neighborOrder][0].subColumes(this->faceRotateIndex()[rotate]);
            }
            return gaussFaceInterpolateMatrix_[neighborOrder][rotate];
        }
        //- (face normal) * Weight * Jacobian
        inline void gaussFaceNxWJ(const tensorList& xr, label faceI, vectorList& nx, scalarList& wj)const
        {
            gaussIntegration_->faceNxWJ(xr, faceI, nx, wj);
        }

};

}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //