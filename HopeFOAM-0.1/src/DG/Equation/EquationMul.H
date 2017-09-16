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
    Foam::dg::EquationMul

Description
    Basic equation. Basic equation. Construct an equation base on a*b.

\*---------------------------------------------------------------------------*/

#ifndef EquationMul_H
#define EquationMul_H

#include "Equation.H"
#include "Field.H"
#include <memory>
#include "scalar.H"
#include "dimensionedScalar.H"

using std::shared_ptr;
using std::make_shared;
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

namespace dg
{
/*---------------------------------------------------------------------------*\
                             Class EquationMul Declaration
\*---------------------------------------------------------------------------*/
template<class Type>
class EquationMul 
	: public Equation<Type>
{

public:

    //- Constructors
	EquationMul(shared_ptr<Equation<Type>> equ, dimensionedScalar & ds) : argDimensionedScalar(ds)
	{

		argEqu = equ;
		if (this->lduMatrixPtr == NULL)
		{
			this->lduMatrixPtr = argEqu->getLduMatrixPtr();
		}
	}

	virtual void calculateCell
		(dgTree<physicalCellElement>::leafIterator iter)
    {
		argEqu->calculateCell(iter);
		//argDimensionedScalar->calculateCell(iter);

		this->lduMatrixPtr->operator*=(argDimensionedScalar.value());
    }

	virtual label getMatrixLabel()
    {
        return argEqu->getMatrixLabel();
    };

	virtual dimensionSet getDimensionSet()
    {
        
		return argEqu->getDimensionSet() * argDimensionedScalar.dimensions();
    };

	virtual const GeometricDofField<Type, dgPatchField, dgGeoMesh>* getPSI()
    {
		if(argEqu->getPSI() != NULL) 
			return argEqu->getPSI();
		return NULL;
    }

	//- Destructor
	~EquationMul()
	{}

private:
	shared_ptr<Equation<Type>> argEqu;
	dimensionedScalar & argDimensionedScalar;
};


template<class Type>
shared_ptr<Equation<Type>> operator*
    (shared_ptr<Equation<Type>> equ,dimensionedScalar ds)
{
    shared_ptr<Equation<Type>> result = make_shared<EquationMul<Type>>(equ, ds);
    return result;
}

template<class Type>
shared_ptr<Equation<Type>> operator*
    (dimensionedScalar ds, shared_ptr<Equation<Type>> equ)
{
    shared_ptr<Equation<Type>> result = make_shared<EquationMul<Type>>(equ, ds);
    return result;
}

}

}
#endif // !EquationMul_H
