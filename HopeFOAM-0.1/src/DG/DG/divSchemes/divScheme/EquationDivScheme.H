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
    Foam::dg::EquationDivScheme

Description
    This is a subclass of Equation, used to restore the result of div
    scheme(including explicit and implicit).

SourceFiles
    
    
Author
    Zhen Xiao (452576504@qq.com)
\*---------------------------------------------------------------------------*/

#ifndef EquationDivScheme_H
#define EquationDivScheme_H

#include "Equation.H"
#include <memory>
#include "tmp.H"
#include "divScheme.H"
namespace Foam
{
namespace dg
{

template<class Type>
class EquationDgcDivSchemeType1;
template<class Type>
class EquationDgcDivSchemeType2;

template<class Type>
class EquationDivScheme
    : public Equation<Type>
{
public:
    enum {
        DgcDivSchemeLabel=0};

    static shared_ptr<Equation<Type>> dgcNew(const GeometricDofField<typename outerProduct<vector, Type>::type, dgPatchField, dgGeoMesh>& vf, const word& name) {
        return make_shared<EquationDgcDivSchemeType1<Type>>(
            dg::divScheme<Type>::New(
                vf.mesh(),
                vf.mesh().divScheme(name)
            ),vf, DgcDivSchemeLabel
        );
    }

    EquationDivScheme(tmp<divScheme<Type>>& ptr, const GeometricDofField<typename outerProduct<vector, Type>::type, dgPatchField, dgGeoMesh>& vf, label la) {
        divSchemePtr = ptr;
        this->vf = &vf;
        this->lduMatrixPtr = make_shared<dgLduMatrix<Type>>(vf.mesh(), la);
    }

    static shared_ptr<Equation<Type>> dgcNew(const dgGaussField<typename outerProduct<vector, Type>::type>& dgf, const word& name) {
        return make_shared<EquationDgcDivSchemeType2<Type>>(
            dg::divScheme<Type>::New(
                dgf.mesh(),
                dgf.mesh().divScheme(name)
            ),dgf, DgcDivSchemeLabel
        );
    }

    EquationDivScheme(tmp<divScheme<Type>>& ptr, const dgGaussField<typename outerProduct<vector, Type>::type>& dgf, label la) {
        divSchemePtr = ptr;
        this->dgf = & dgf;
        this->lduMatrixPtr = make_shared<dgLduMatrix<Type>>(dgf.mesh(), la);
        tmp<Field<typename outerProduct<vector, Type>::type>>tflux = divSchemePtr->getFluxCalcScheme().fluxCalculateWeak(dgf);
        this->flux = tflux.ptr();
    }

    virtual ~EquationDivScheme(){
        if(flux){
            delete flux;
        }
    }

    virtual void calculateCell(dgTree<physicalCellElement>::leafIterator iter) = 0;
    virtual dimensionSet getDimensionSet()
    {
        return this->vf->dimensions()/dimLength;
    }

    tmp<divScheme<Type>> divSchemePtr;
    const GeometricDofField<typename outerProduct<vector, Type>::type, dgPatchField, dgGeoMesh>* vf;
    const dgGaussField<typename outerProduct<vector, Type>::type>* dgf;
    const Field<typename outerProduct<vector, Type>::type>* flux = NULL;

};


template<class Type>
class EquationDgcDivSchemeBase
    : public EquationDivScheme<Type>
{
public:
    EquationDgcDivSchemeBase(tmp<divScheme<Type>>& ptr, const GeometricDofField<typename outerProduct<vector, Type>::type, dgPatchField, dgGeoMesh>& vf, label la) : EquationDivScheme<Type>(ptr, vf, la) {}
    EquationDgcDivSchemeBase(tmp<divScheme<Type>>& ptr, const dgGaussField<typename outerProduct<vector, Type>::type>& dgf, label la) : EquationDivScheme<Type>(ptr, dgf, la) {}

    virtual const GeometricDofField<Type, dgPatchField, dgGeoMesh>* getPSI() {
        return NULL;
    }
    virtual label getMatrixLabel() {
        return EquationDivScheme<Type>::DgcDivSchemeLabel;
    }
};

template<class Type>
class EquationDgcDivSchemeType1
    : public EquationDgcDivSchemeBase<Type>
{
public:
    EquationDgcDivSchemeType1(tmp<divScheme<Type>> ptr, const GeometricDofField<typename outerProduct<vector, Type>::type, dgPatchField, dgGeoMesh>& vf, label la) : EquationDgcDivSchemeBase<Type>(ptr, vf, la) {}

    virtual void calculateCell(dgTree<physicalCellElement>::leafIterator iter) {
        this->divSchemePtr->dgcDivCalculateCell(iter, this->lduMatrixPtr, this->vf);
    }

    virtual dimensionSet getDimensionSet()
    {
        return this->vf->dimensions()/dimLength;
    }
};

template<class Type>
class EquationDgcDivSchemeType2
    : public EquationDgcDivSchemeBase<Type>
{
public:
    EquationDgcDivSchemeType2(tmp<divScheme<Type>> ptr, const dgGaussField<typename outerProduct<vector, Type>::type>& dgf, label la) : EquationDgcDivSchemeBase<Type>(ptr, dgf, la) {}

    virtual void calculateCell(dgTree<physicalCellElement>::leafIterator iter) {
        this->divSchemePtr->dgcDivCalculateCell(iter, this->lduMatrixPtr, *(this->dgf), *(this->flux));
    }

    virtual dimensionSet getDimensionSet()
    {
        return this->dgf->dimensions()/dimLength;
    }
};

}
}
#endif //EquationDivScheme_H
