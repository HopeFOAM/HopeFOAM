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
    Foam::physicalCellElement

Description
    Contains the information for critical cell discretisation, including integration
    Jacobian coefficient, pointer to reference standard element.

SourceFiles
    physicalCellElement.C
    
Author
    Xu Liyang (xucloud77@gmail.com)
\*---------------------------------------------------------------------------*/
#ifndef physicalCellElement_H
#define physicalCellElement_H

#include "stdElement.H"
#include "physicalFaceElement.H"
#include "dgTreeUnit.H"
#include <memory>

using std::shared_ptr;
using std::make_shared;

namespace Foam
{
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
class physicalFaceElement;
/*---------------------------------------------------------------------------*\
                       Class physicalCellElement Declaration
\*---------------------------------------------------------------------------*/

class physicalCellElement
{

    stdElement * baseFunction_; //pointer to the reference standard base function

    //the index number in tree structure
    label sequenceIndex_;

    label dofStart_; //
    //- dg points in mesh
    List<vector> dofLocation_;
    //- dx/dr
    List<tensor> dxdr_;
    //- dr/dx
    List<tensor> drdx_;

    //- the start position in gauss field
    label gaussStart_;

    //- gauss dx/dr
    List<tensor> cellDxdr_;
    //- gauss dr/dx
    List<tensor> cellDrdx_;

    //- gauss quadrature weights in this cell
    scalarList jacobianWeights_;

    //- integration results of /phi_i/cdot/phi_j in this physical cell
    denseMatrix<scalar> massMatrix_;

    //- (phi_x, phi_y, phi_z) in every quadrature points
    denseMatrix<vector> cellD1dx_;

    //- ptr to face elements
    List<shared_ptr<physicalFaceElement>> faceElementList_;

    /************ private member functions **************/
    void initMassMatrix();


public:
    physicalCellElement();

    physicalCellElement(const physicalCellElement&);

    ~physicalCellElement();

    void initElement(stdElement*, const List<vector>&);

    void setFaceElement(label faceI, shared_ptr<physicalFaceElement> ele){
        faceElementList_[faceI] = ele;
    }

    void updateDofStart(label);

    void updateGaussStart(label);

    void updateSequenceIndex(label);

    inline const stdElement& baseFunction()const{
        return *baseFunction_;
    }

    void setBaseFunction(stdElement* newBaseFunc){
        baseFunction_ = newBaseFunc;
    }

    //- total dof in this element
    //- not implemented for hp, when the dofs of subcells should also be considered
    inline const label nDof()const{
        return baseFunction_->nDofPerCell();
    }

    inline const label sequenceIndex()const{
        return sequenceIndex_;
    }

    inline const label dofStart()const{
        return dofStart_;
    }

    inline const List<vector>& dofLocation()const{
        return dofLocation_;
    }

    inline const label gaussStart()const{
        return gaussStart_;
    }

    inline const List<tensor>& dxdr()const{
        return dxdr_;
    }

    inline const List<tensor>& drdx()const{
        return drdx_;
    }

    inline const List<tensor>& cellDxdr()const{
        return cellDxdr_;
    }

    inline const List<tensor>& cellDrdx()const{
        return cellDrdx_;
    }

    inline const List<shared_ptr<physicalFaceElement>>& faceElementList()const{
        return faceElementList_;
    }

    inline List<shared_ptr<physicalFaceElement>>& faceElementList(){
        return faceElementList_;
    }

    //- get the jacobian weights at gauss quadrature points
    inline const scalarList& jacobianWeights()const{
        return jacobianWeights_;
    }

    //- get the vandermonde values at gauss quadrature points
    inline const denseMatrix<scalar>& massMatrix()const{
        return massMatrix_;
    }

    //- get the vandermonde values at gauss quadrature points
    inline const denseMatrix<scalar>& cellVandermonde()const{
        return baseFunction_->gaussCellVandermonde();
    }

    //- get the 1-st derivative at gauss quadrature points
    inline const denseMatrix<vector>& cellD1dx()const{
        return cellD1dx_;
    }

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //