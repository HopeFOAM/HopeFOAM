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
    Foam::dg::EquationAdd

Description
    Basic equation. Construct an equation base on a+b.

\*---------------------------------------------------------------------------*/

#ifndef EquationAdd_H
#define EquationAdd_H

#include "Equation.H"
#include "Field.H"
#include <memory>

using std::shared_ptr;
using std::make_shared;
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

namespace dg
{
template <class Type>
class Equation;

/*---------------------------------------------------------------------------*\
                             Class EquationAdd Declaration
\*---------------------------------------------------------------------------*/
template <class Type>
class EquationAdd
	: public Equation<Type>
{

  public:
  	
  	//- Constructors
	EquationAdd(shared_ptr<Equation<Type>> a, shared_ptr<Equation<Type>> b)
	{
		arg1 = a;
		arg2 = b;
		if (this->lduMatrixPtr == NULL)
		{
			if (arg1->getMatrixLabel() > arg2->getMatrixLabel())
			{
				this->lduMatrixPtr = arg1->getLduMatrixPtr();
				this->otherLduMatrixPtr = arg2->getLduMatrixPtr();
			}
			else
			{
				this->lduMatrixPtr = arg2->getLduMatrixPtr();
				this->otherLduMatrixPtr = arg1->getLduMatrixPtr();
			}
		}
	}

	virtual void calculateCell(dgTree<physicalCellElement>::leafIterator iter)
	{
		arg1->calculateCell(iter);
		arg2->calculateCell(iter);

		this->lduMatrixPtr->operator+=(*(this->otherLduMatrixPtr));
	}

	virtual label getMatrixLabel()
	{
		return max(arg1->getMatrixLabel(), arg2->getMatrixLabel());
	};

	virtual dimensionSet getDimensionSet()
	{
		if (arg1->getDimensionSet() != arg2->getDimensionSet())
			FatalErrorInFunction
				<< "incompatible dimensions for operation "
				<< endl
				<< "    "
				<< "[" << (arg1->getMatrixLabel() ? arg1->getPSI()->name() : "matrix1(zero operatorOrder)") << arg1->getDimensionSet() << " ] "
				<< "EquationAdd"
				<< " [" << (arg2->getMatrixLabel() ? arg2->getPSI()->name() : "matrix2(zero operatorOrder)") << arg2->getDimensionSet() << " ]"
				<< abort(FatalError);
		else
		{
			return arg1->getDimensionSet();
		}
	};

	virtual const GeometricDofField<Type, dgPatchField, dgGeoMesh> *getPSI()
	{
		if (arg1->getPSI() != NULL &&
			arg2->getPSI() != NULL &&
			(arg1->getPSI() != arg2->getPSI()))
		{
			FatalErrorInFunction
				<< "incompatible fields for operation "
				<< endl
				<< "    "
				<< "[" << arg1->getPSI()->name() << "] "
				<< "EquationAdd"
				<< " [" << arg2->getPSI()->name() << "]"
				<< abort(FatalError);
		}
		else
		{
			if (arg1->getPSI() != NULL)
				return arg1->getPSI();
			return arg2->getPSI();
		}
	}

	//- Destructor
	~EquationAdd()
	{}

  private:
	shared_ptr<dgLduMatrix<Type>> otherLduMatrixPtr;
	shared_ptr<Equation<Type>> arg1;
	shared_ptr<Equation<Type>> arg2;
};

template <class Type>
shared_ptr<Equation<Type>> operator+(shared_ptr<Equation<Type>> a, shared_ptr<Equation<Type>> b)
{
	shared_ptr<Equation<Type>> result =
		make_shared<EquationAdd<Type>>(a, b);
	return result;
}

template <class Type>
shared_ptr<Equation<Type>> operator+(shared_ptr<Equation<Type>> equ, GeometricDofField<Type, dgPatchField, dgGeoMesh> &vf)
{
	return make_shared<EquationAdd<Type>>(equ, make_shared<EquationGeometricDofField<Type>>(vf));
}

template <class Type>
shared_ptr<Equation<Type>> operator+(shared_ptr<Equation<Type>> equ, tmp<GeometricDofField<Type, dgPatchField, dgGeoMesh>> vf)
{
	return make_shared<EquationAdd<Type>>(equ, make_shared<EquationGeometricDofField<Type>>(vf));
}
}
}
#endif // !EquationAdd_H
