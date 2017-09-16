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
    Foam::triangleBaseFunction

Description
    standard Triangle Element

SourceFiles
    triangleBaseFunction.C

Author
    Xu Liyang (xucloud77@gmail.com)
\*---------------------------------------------------------------------------*/

#ifndef triangleBaseFunction_H
#define triangleBaseFunction_H

#include "baseFunction.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

/*---------------------------------------------------------------------------*\
                       Class triangleBaseFunction Declaration
\*---------------------------------------------------------------------------*/

class triangleBaseFunction
:
    public baseFunction
{
	// Private data

	 // Private Member Functions

        //- Disallow default bitwise copy construct
        triangleBaseFunction(const triangleBaseFunction&);

        //- Disallow default bitwise assignment
        void operator=(const triangleBaseFunction&);

        // set dofLocation
        void initDofLocation();
        scalarList warpFactor(label, scalarList);

        // set Integration Free Method Matrix
        void initDrMatrix();
        void initLift();
        void initFaceVertex();

public:

    //- Runtime type information
    TypeName("tri");

    // Constructors

        //- Construct
        triangleBaseFunction(label nOrder, word cellShape);

    // Member functions

        //- calc the cell vandermonde matrix for cell interpolation
        denseMatrix<scalar> cellVandermonde(const vectorList&)const;
        //- calc the face vandermonde matrix for face interpolation
        denseMatrix<scalar> faceVandermonde(const vectorList&)const;

        void initCentreInterpolateMatrix();

         //-
        void initFaceToCellIndex();
        //-
        void initFaceRotateIndex();

        //- calculate physical node location
        vectorList physicalNodesLoc(const vectorList&)const;
        //- calculate dx/dr
        tensorList dxdr(const vectorList&);
        //- calculate dr/dx
        tensorList drdx(const tensorList&)const;
        //- calculate Jacobian factor
        scalarList jacobian(const tensorList&)const;
        //- calculate face normal
        void faceNxFscale(const tensorList&, label, vectorList&, scalarList&)const;
        void faceFscale(const tensorList&, label, scalar&)const;
        void addFaceShiftToCell(const vectorList& shift, label faceI, vectorList& dofLoc)const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //