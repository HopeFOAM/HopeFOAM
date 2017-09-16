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
    Foam::Legendre

Description
    Abstract base class for standard Element.

SourceFiles
    Legendre.C

Author
    Xu Liyang (xucloud77@gmail.com)

\*---------------------------------------------------------------------------*/

#ifndef Legendre_H
#define Legendre_H

#include "label.H"
#include "scalarList.H"
#include "vectorList.H"
#include "denseMatrix.H"
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
  Class Legendre Declaration
\*---------------------------------------------------------------------------*/
class Legendre
{
public:
	//- Constructors
	Legendre()
	{}

	//- Destructor
    virtual ~Legendre()
    {}

	//- static Legendre functions
	static const scalarListList JacobiGQ(scalar, scalar, int);
	static const scalarList JacobiGL(scalar, scalar, int);
	static const scalarList JacobiP(const scalarList&, scalar, scalar, int);
	static const scalarList GradJacobiP(const vectorList&, scalar, scalar, int);
	static const scalarList GradJacobiP(const scalarList&, scalar, scalar, int);
	static const scalar gamma(int x);//evaluates the gamma function for x

	static const scalarListList Vandermonde1D(int, const scalarList&);
	static const scalarListList Vandermonde1D(int, const vectorList&);
	static const scalarListList Vandermonde2D(int, const scalarList&, const scalarList&);
	static const scalarListList Vandermonde2D(int, const vectorList&);
	static const scalarListList Vandermonde3D(int, const scalarList&, const scalarList&, const scalarList&);
	static const scalarListList Vandermonde3D(int, const vectorList&);

	static const denseMatrix<vector> GradVandermonde1D(label, scalarList&);
	static const denseMatrix<vector> GradVandermonde1D(label, vectorList&);
	static const denseMatrix<vector> GradVandermonde2D(label, vectorList&);
	static const denseMatrix<vector> GradVandermonde3D(label, vectorList&);

	//- should be put on dense Matrix class, temporally put here
	static denseMatrix<scalar> matrixInv(const denseMatrix<scalar>&);//matrix inverse
};

}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //