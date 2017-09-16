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
    Foam::dg::EquationGradScheme

Description
    This is a subclass of Equation, used to restore the result of grad
    scheme(including explicit and implicit).

SourceFiles
    
    
Author
    Zhen Xiao (452576504@qq.com)
\*---------------------------------------------------------------------------*/

#ifndef EquationGradScheme_H
#define EquationGradScheme_H

#include "Equation.H"
#include <memory>
#include "tmp.H"
#include "gradScheme.H"
namespace Foam
{
namespace dg
{

template<class Type>
class EquationDgcGradSchemeType1;
template<class Type>
class EquationDgcGradSchemeType2;
template<class Type>
class EquationDgmGradSchemeType1;

template<class Type>
class EquationGradScheme
    : public Equation<Type>
{
public:
    enum {
        DgcGradSchemeLabel=0,
        DgmGradSchemeLabel=2
    };

    static shared_ptr<Equation<Type>> dgcNew(const dgGaussField<typename innerProduct<vector, Type>::type>& U, const word& name) {
        return make_shared<EquationDgcGradSchemeType1<Type>>(
            dg::gradScheme<Type>::New(
                U.mesh(),
                U.mesh().gradScheme(name)
            ),U, DgcGradSchemeLabel
        );
    }

    EquationGradScheme(tmp<gradScheme<Type>>& ptr, const dgGaussField<typename innerProduct<vector, Type>::type>& U, label la) {
        gradSchemePtr = ptr;
        this->U = &U;
        this->lduMatrixPtr = make_shared<dgLduMatrix<Type>>(U.mesh(), la);
        tmp<Field<typename innerProduct<vector, Type>::type>>tflux = gradSchemePtr->getFluxCalcScheme().fluxCalculateWeak(U);
        this->flux = tflux.ptr();
    }

    static shared_ptr<Equation<Type>> dgcNew(const GeometricDofField<typename innerProduct<vector, Type>::type, dgPatchField, dgGeoMesh>& vf, const word& name) {
        return make_shared<EquationDgcGradSchemeType2<Type>>(
            dg::gradScheme<Type>::New(
                vf.mesh(),
                vf.mesh().gradScheme(name)
            ),vf, DgcGradSchemeLabel
        );
    }

    EquationGradScheme(tmp<gradScheme<Type>>& ptr, const GeometricDofField<typename innerProduct<vector, Type>::type, dgPatchField, dgGeoMesh>& vf, label la) {
        gradSchemePtr = ptr;
        this->vf = &vf;
        this->lduMatrixPtr = make_shared<dgLduMatrix<Type>>(vf.mesh(), la);
    }

    static shared_ptr<Equation<Type>> dgmNew(const dgGaussField<typename innerProduct<vector, Type>::type>& U, const word& name) {
        return make_shared<EquationDgmGradSchemeType1<Type>>(
            dg::gradScheme<Type>::New(
                U.mesh(),
                U.mesh().gradScheme(name)
            ),U, DgmGradSchemeLabel
        );
    }

    virtual ~EquationGradScheme(){
        if(flux){
            delete flux;
        }
    }

    virtual void calculateCell(dgTree<physicalCellElement>::leafIterator iter) = 0;
    virtual dimensionSet getDimensionSet()
    {
        return this->vf->dimensions()/dimLength;
    }

    tmp<gradScheme<Type>> gradSchemePtr;
    const dgGaussField<typename innerProduct<vector, Type>::type>* U;
    const GeometricDofField<typename innerProduct<vector, Type>::type, dgPatchField, dgGeoMesh>* vf;
    const Field<typename innerProduct<vector, Type>::type>* flux = NULL;
};


template<class Type>
class EquationDgcGradSchemeBase
    : public EquationGradScheme<Type>
{
public:
    EquationDgcGradSchemeBase(tmp<gradScheme<Type>>& ptr, const dgGaussField<typename innerProduct<vector, Type>::type>& U, label la) : EquationGradScheme<Type>(ptr, U, la) {}
    EquationDgcGradSchemeBase(tmp<gradScheme<Type>>& ptr, const GeometricDofField<typename innerProduct<vector, Type>::type, dgPatchField, dgGeoMesh>& vf, label la) : EquationGradScheme<Type>(ptr, vf, la) {}

    virtual const GeometricDofField<Type, dgPatchField, dgGeoMesh>* getPSI() {
        return NULL;
    }
    virtual label getMatrixLabel() {
        return EquationGradScheme<Type>::DgcGradSchemeLabel;
    }
};

template<class Type>
class EquationDgcGradSchemeType1
    : public EquationDgcGradSchemeBase<Type>
{
public:
    EquationDgcGradSchemeType1(tmp<gradScheme<Type>> ptr, const dgGaussField<typename innerProduct<vector, Type>::type>& U, label la) : EquationDgcGradSchemeBase<Type>(ptr, U, la) {}

    virtual void calculateCell(dgTree<physicalCellElement>::leafIterator iter) {
        this->gradSchemePtr->dgcGradCalculateCell(iter, this->lduMatrixPtr, *(this->U), *(this->flux));
    }
    virtual dimensionSet getDimensionSet()
    {
        return this->U->dimensions()/dimLength;
    }
};

template<class Type>
class EquationDgcGradSchemeType2
    : public EquationDgcGradSchemeBase<Type>
{
public:
    EquationDgcGradSchemeType2(tmp<gradScheme<Type>> ptr, const GeometricDofField<typename innerProduct<vector, Type>::type, dgPatchField, dgGeoMesh>& vf, label la) : EquationDgcGradSchemeBase<Type>(ptr, vf, la) {}

    virtual void calculateCell(dgTree<physicalCellElement>::leafIterator iter) {
        this->gradSchemePtr->dgcGradCalculateCell(iter, this->lduMatrixPtr, this->vf);
    }

    virtual dimensionSet getDimensionSet()
    {
        return this->vf->dimensions()/dimLength;
    }
};

template<class Type>
class EquationDgmGradSchemeBase
    : public EquationGradScheme<Type>
{
public:
    EquationDgmGradSchemeBase(tmp<gradScheme<Type>>& ptr, const dgGaussField<typename innerProduct<vector, Type>::type>& U, label la) : EquationGradScheme<Type>(ptr, U, la) {}

    virtual const GeometricDofField<typename innerProduct<vector, Type>::type, dgPatchField, dgGeoMesh>* getPSI() {
        return this->vf;
    }
    virtual label getMatrixLabel() {
        return EquationGradScheme<Type>::DgmGradSchemeLabel;
    }
};

template<class Type>
class EquationDgmGradSchemeType1
    : public EquationDgmGradSchemeBase<Type>
{
public:
    EquationDgmGradSchemeType1(tmp<gradScheme<Type>> ptr, const dgGaussField<typename innerProduct<vector, Type>::type>& U, label la) : EquationDgmGradSchemeBase<Type>(ptr, U, la) {}

    virtual void calculateCell(dgTree<physicalCellElement>::leafIterator iter) {
        this->gradSchemePtr->dgmGradCalculateCell(iter, this->lduMatrixPtr, *(this->U));
    }

    virtual dimensionSet getDimensionSet()
    {
        return this->U->dimensions()/dimLength;
    }
};

}
}
#endif //EquationGradScheme_H
