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
    Foam::baseFunction

Description
    Abstract class for Legendre base Function.

SourceFiles
    baseFunction.C

Author
    Xu Liyang (xucloud77@gmail.com)

\*---------------------------------------------------------------------------*/

#ifndef baseFunction_H
#define baseFunction_H

#include "labelList.H"
#include "scalarList.H"
#include "vectorList.H"
#include "tensorList.H"
#include "denseMatrix.H"
#include "typeInfo.H"
#include "autoPtr.H"
#include "runTimeSelectionTables.H"
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
typedef List<List<vector>> vectorListList;
/*---------------------------------------------------------------------------*\
  Class baseFunction Declaration
\*---------------------------------------------------------------------------*/
class baseFunction
{

protected:
    //- Disallow default bitwise copy construct
    baseFunction(const baseFunction&);

public:
    // base Function Data

        // base controls
        //-
        const label nOrder_;
        //-
        const word cellShape_;

        // dof controls
        //-
        label nDofPerFace_;
        //-
        label nFacesPerCell_;
        //-
        label nDofPerCell_;
        //- location of dofs in reference cell
        vectorList dofLocation_;
        //- location of dofs in reference face
        vectorList faceDofLocation_;

        //- vertex label of each face
        labelListList faceVertex_;

        //- index from face points to cell points, 1d-localFaceID, 2d-faceRoateID, 3d-each dof in face
        List<labelListList> faceToCellIndex_;

        //- the mapping index for face to rotate n
        labelListList faceRotateIndex_;

        //- Vandermonde matrix
        denseMatrix<scalar> V_;
        denseMatrix<scalar> invV_; //inverse Vandermonde matrix

        //- inv Face Vandermonde matrix
        denseMatrix<scalar> invFaceMatrix_;

        denseMatrix<vector> drMatrix_; //differentiation matrices

        //matrix for interpolate to centre
        denseMatrix<scalar> cellCentreInterpolateMatrix_;
        denseMatrix<scalar> faceCentreInterpolateMatrix_;

        //Linear Integration Method Matrix
        denseMatrix<scalar> massMatrix_; // Mass matrix
        denseMatrix<scalar> invMassMatrix_; //inverse of Mass matrix
        denseMatrix<scalar> Lift_; //face integration part of invMassMatrix

    //- Runtime type information
    TypeName("baseFunction");

    //- Constructors
    baseFunction(label nOrder, word cellShape)
    :
        nOrder_(nOrder),
        cellShape_(cellShape)
    {}

    //- Destructor
    virtual ~baseFunction()
    {}

    // Declare run-time constructor selection tables

        declareRunTimeSelectionTable
        (
            autoPtr,
            baseFunction,
            straightBaseFunction,
            (label nOrder, word cellShape),
            (nOrder, cellShape)
        );


    // Selectors

        //- Return a pointer to a new baseFunction created on freestore
        static autoPtr<baseFunction> New
        (
            label nOrder,
            word cellShape
        );

    // Member functions
    //std Element menmber functions

        //-
        virtual void initFaceToCellIndex() =0;
        //-
        virtual void initFaceRotateIndex() =0;
        //-
        virtual void initFaceVertex() =0;

        // set Integration Free Method Matrix
        void initMassMatrix();
        void initInvMassMatrix();
        virtual void initDrMatrix() =0;
        virtual void initLift() =0;

        virtual void initCentreInterpolateMatrix() =0;

        //- calc the cell vandermonde matrix for face interpolation
        virtual denseMatrix<scalar> cellVandermonde(const vectorList&)const =0;
        //- calc the face vandermonde matrix for face interpolation
        virtual denseMatrix<scalar> faceVandermonde(const vectorList&)const =0;

        //- calculate physical node location
        virtual vectorList physicalNodesLoc(const vectorList&)const = 0;
        //- calculate dx/dr
        virtual tensorList dxdr(const vectorList&) = 0;
        //- calculate dr/dx
        virtual tensorList drdx(const tensorList&)const = 0;
        //- calculate Jacobian factor
        virtual scalarList jacobian(const tensorList&)const = 0;
        //- calculate faceNormal_
        virtual void faceNxFscale(const tensorList&, label, vectorList&, scalarList&)const =0;
        virtual void faceFscale(const tensorList&, label, scalar&)const =0;
        //- calculate Fscale on face integration
        //virtual scalarList fscale() =0;
        //- using blending factor, add curved face shift to cell dof location
        virtual void addFaceShiftToCell(const vectorList& shift, label faceI, vectorList& dofLoc)const{};

};

}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //