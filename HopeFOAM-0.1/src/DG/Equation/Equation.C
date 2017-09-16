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

\*---------------------------------------------------------------------------*/
#include "Equation.H"
#include <memory>
#include "physicalElementData.H"

using std::shared_ptr;
using std::make_shared;

namespace Foam
{

namespace dg
{

template <class Type>
void Equation<Type>::evaluate()
{
    const GeometricDofField<Type, dgPatchField, dgGeoMesh> *psiptr = this->getPSI();

    if (matrix == NULL)
    {
        dimensionSet ds = getDimensionSet();
        label ml = getMatrixLabel();
        matrix = make_shared<dgMatrix<Type>>(*psiptr, ds, ml);
    }

    lduMatrixPtr = subTree->getLduMatrixPtr();

    const dgMesh &mesh = psiptr->mesh();
    label count = 0;
    label globalShift = mesh.localRange().first();
    dgTree<physicalCellElement>::iteratorEnd end =
        mesh.cellElementsTree().end();
    for (dgTree<physicalCellElement>::leafIterator iter = 
            mesh.cellElementsTree().leafBegin(); iter != end; ++iter, ++count)
    {
        calculateCell(iter);

        matrix->assembleMatrix(*getLduMatrixPtr(), iter()->value(), count, globalShift);
    }

    if (lduMatrixPtr->operatorOrder() == 0)
    {
        scalar coeff = 1.0 / (lduMatrixPtr->diagCoeff());
        matrix->b() *= coeff;
        matrix->solve(const_cast<dgMesh &>(mesh).massMatrixSolver());
    }
    else
    {
        matrix->finalAssemble();
        matrix->solve();
    }
}

}

}
