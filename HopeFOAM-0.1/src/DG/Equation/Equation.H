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
    Foam::dg::Equation

Description
    class Equation. We use composite design pattern to construct an 
    equation. All the single operators and binary operators are base on the 
    class Equation.

SourceFiles
    Equation.C

Author
    Zhen Xiao (452576504@qq.com)

\*---------------------------------------------------------------------------*/

#ifndef Equation_H
#define Equation_H

#include "Field.H"
#include <memory>
#include "dgLduMatrix.H"
#include "GeometricDofField.H"
#include "dgPatchField.H"
#include "dgGeoMesh.H"
#include "dgMatrix.H"

using std::shared_ptr;
using std::make_shared;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

namespace dg
{

/*---------------------------------------------------------------------------*\
                             Class Equation Declaration
\*---------------------------------------------------------------------------*/
template <class Type>
class Equation : public std::enable_shared_from_this<Equation<Type>>
{

  public:

     //- Constructors
    Equation(){};

    Equation(shared_ptr<Equation<Type>> subTree)
    {
        this->subTree = subTree;
    };

    //- calculate the equation and save the result on member matrix
    void evaluate();

    //- calculate one element and update it in member matrix
    virtual void calculateCell(dgTree<physicalCellElement>::leafIterator iter)
    {
        subTree->calculateCell(iter);
    }

    //- Destructor
    virtual ~Equation(){};

    virtual label getMatrixLabel()
    {
        return subTree->getMatrixLabel();
    };

    virtual dimensionSet getDimensionSet()
    {
        return subTree->getDimensionSet();
    };

    virtual const GeometricDofField<Type, dgPatchField, dgGeoMesh> * getPSI()
    {
        return subTree->getPSI();
    }

    shared_ptr<dgLduMatrix<Type>> getLduMatrixPtr()
    {
        return lduMatrixPtr;
    }

    shared_ptr<dgMatrix<Type>> matrix;

    shared_ptr<dgLduMatrix<Type>> lduMatrixPtr;

  private:
    shared_ptr<Equation<Type>> subTree;
};

//- wrap function. Construct an equation and return it's result matrix pointer
template <class Type>
shared_ptr<dgMatrix<Type>> solveEquation(shared_ptr<Equation<Type>> subTree)
{
    shared_ptr<Equation<Type>> equ = make_shared<Equation<Type>>(subTree);
    equ->evaluate();
    return equ->matrix;
}

} // End namespace dg

} // End namespace Foam

#ifdef NoRepository
#include "Equation.C"
#endif

#endif // !Equation_H
