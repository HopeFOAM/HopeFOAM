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
    Foam::gaussIntegration

Description
    Abstract class for gaussIntegration.

SourceFiles
    gaussIntegration.C

Author
    Xu Liyang (xucloud77@gmail.com)
\*---------------------------------------------------------------------------*/

#ifndef gaussIntegration_H
#define gaussIntegration_H

#include "baseFunction.H"
#include "autoPtr.H"
#include "runTimeSelectionTables.H"
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                   Class gaussIntegration Declaration
\*---------------------------------------------------------------------------*/
class gaussIntegration
{

protected:
    //- Disallow default bitwise copy construct
    gaussIntegration(const gaussIntegration&);

public:
    // base Function Data

        // base function
        const baseFunction & baseFunction_;

        // cell integration data
        //-
        label volIntOrder_;
        //-
        label nDofPerCell_;
        //- integration points in standard reference cell
        vectorList cellIntLocation_;
        //- integration weights
        scalarList cellIntWeights_;
        //-generalized Vandermonde of Lagrange interpolant functions at cubature nodes
        denseMatrix<scalar> cellVandermonde_;
        //-local derivatives of Lagrange interpolants at cubature nodes
        denseMatrix<vector> cellDr_;

        // face integration data
        //-
        label faceIntOrder_;
        //-
        label nDofPerFace_;
        //- the mapping index for face to rotate n
        labelListList faceRotateIndex_;
        //- integration points in face
        vectorList faceIntLocation_;
        //- integration weights
        scalarList faceIntWeights_;
        //- interpolate matrix from node base points to gauss face points
        denseMatrix<scalar> faceInterp_;


    //- Runtime type information
    TypeName("gaussIntegration");

    //- Constructors
    gaussIntegration(const baseFunction& base);

    //- Destructor
    virtual ~gaussIntegration()
    {}

    // Declare run-time constructor selection tables

        declareRunTimeSelectionTable
        (
            autoPtr,
            gaussIntegration,
            gaussIntegration,
            (const baseFunction& base),
            (base)
        );


    // Selectors

        //- Return a pointer to a new gaussIntegration created on freestore
        static autoPtr<gaussIntegration> New
        (
            const baseFunction& base
        );

    // Member functions
        //-
        virtual void initFaceRotateIndex() =0;
        virtual vectorList gaussCellNodesLoc(const vectorList&)const = 0;
        //- calculate cell dx/dr
        virtual tensorList cellDxdr(const vectorList&)const;
        //- calculate cell dr/dx
        virtual tensorList cellDrdx(const tensorList&)const = 0;
        //- calculate cell cell integration factors
        virtual scalarList cellWJ(const tensorList&)const = 0;
        //- (face normal) * Weight * Jacobian
        virtual void faceNxWJ(const tensorList&, label, vectorList&, scalarList&)const = 0;

};

}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //