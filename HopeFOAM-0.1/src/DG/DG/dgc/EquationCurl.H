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
    Foam::dg::EquationCurl

Description
    This is a subclass of Equation, used to restore the result of curl
    scheme.

SourceFiles
    
    
Author
    Zhen Xiao (452576504@qq.com)
\*---------------------------------------------------------------------------*/

#ifndef EquationCurl_H
#define EquationCurl_H

#include "Equation.H"
#include <memory>
#include "tmp.H"

namespace Foam
{
namespace dg
{

template<class Type>
class EquationCurl
    : public Equation<Type>
{
public:
    enum {
        DgcCurlLabel=0};

    static shared_ptr<Equation<Type>> dgcNew(const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf, const word& name) {
        return make_shared<EquationCurl<Type>>(vf, DgcCurlLabel);
    }

    EquationCurl(const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf, label la) {
        this->vf = &vf;
        this->lduMatrixPtr = make_shared<dgLduMatrix<Type>>(vf.mesh(), la);
    }


    virtual void calculateCell(dgTree<physicalCellElement>::leafIterator iter)
    {
        FatalErrorIn
        (
            "Foam::dg::EquationCurl::calculateCell()"
        )   << "xuliyang->completeThis()"
            << abort(FatalError);
    }

    virtual dimensionSet getDimensionSet()
    {
        return this->vf->dimensions()/dimTime;
    }

    virtual const GeometricDofField<Type, dgPatchField, dgGeoMesh>* getPSI() {
        return NULL;
    }
    virtual label getMatrixLabel() {
        return EquationCurl<Type>::DgcCurlLabel;
    }

    const GeometricDofField<Type, dgPatchField, dgGeoMesh>* vf;

};


}
}
#endif //EquationCurl_H
