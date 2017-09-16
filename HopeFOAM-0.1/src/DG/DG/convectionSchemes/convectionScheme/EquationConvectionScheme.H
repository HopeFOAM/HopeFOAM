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
    Foam::dg::EquationConvectionScheme

Description
    This is a subclass of Equation, used to restore the result of convection
    scheme(including explicit and implicit).

SourceFiles
    
    
Author
    Zhen Xiao (452576504@qq.com)
\*---------------------------------------------------------------------------*/

#ifndef EquationConvectionScheme_H
#define EquationConvectionScheme_H

#include "Equation.H"
#include <memory>
#include "tmp.H"
#include "convectionScheme.H"
namespace Foam
{
namespace dg
{

template<class Type>
class EquationDgcConvectionSchemeType1;
template<class Type>
class EquationDgcConvectionSchemeType2;
template<class Type>
class EquationDgcConvectionSchemeType3;
template<class Type>
class EquationDgmConvectionSchemeType1;

template<class Type>
class EquationConvectionScheme
    : public Equation<Type>
{
public:
    enum {
        DgcConvectionSchemeLabel=0,
        DgmConvectionSchemeLabel=2
    };

    static shared_ptr<Equation<Type>> dgcNew(const dgGaussVectorField& U, const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf, const Field<Type>& flux, const word& name) {
        return make_shared<EquationDgcConvectionSchemeType1<Type>>(
            dg::convectionScheme<Type>::New(
                vf.mesh(),
                vf.mesh().divScheme(name)
            ),U, vf, flux, DgcConvectionSchemeLabel
        );
    }

    EquationConvectionScheme(tmp<convectionScheme<Type>>& ptr, const dgGaussVectorField& U, const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf, const Field<Type>& flux, label la) {
        convectionSchemePtr = ptr;
        this->U = & U;
        this->vf = &vf;
        this->flux = & flux;
        this->lduMatrixPtr = make_shared<dgLduMatrix<Type>>(vf.mesh(), la);
    }

    static shared_ptr<Equation<Type>> dgcNew(const dgGaussVectorField& U, const dgGaussField<Type>& vf2, const word& name) {
        return make_shared<EquationDgcConvectionSchemeType2<Type>>(
            dg::convectionScheme<Type>::New(
                vf2.mesh(),
                vf2.mesh().divScheme(name)
            ),U, vf2, DgcConvectionSchemeLabel
        );
    }

    EquationConvectionScheme(tmp<convectionScheme<Type>>& ptr, const dgGaussVectorField& U, const dgGaussField<Type>& vf2, label la) {
        convectionSchemePtr = ptr;
        this->U = & U;
        this->vf2 = & vf2;
        tmp<Field<Type>>tflux = convectionSchemePtr->getFluxCalcScheme().fluxCalculateWeak(U, vf2);
        //Info<<__FILE__<<__LINE__<<" : Flux is "<<tflux()<<endl;
        this->flux = tflux.ptr();
        //tflux.ptr() = NULL;
        this->lduMatrixPtr = make_shared<dgLduMatrix<Type>>(vf2.mesh(), la);
    }

    static shared_ptr<Equation<Type>> dgcNew(const GeometricDofField<vector, dgPatchField, dgGeoMesh>& dofU, const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf, const word& name) {
        return make_shared<EquationDgcConvectionSchemeType3<Type>>(
            dg::convectionScheme<Type>::New(
                vf.mesh(),
                vf.mesh().divScheme(name)
            ),dofU, vf, DgcConvectionSchemeLabel
        );
    }

    EquationConvectionScheme(tmp<convectionScheme<Type>>& ptr, const GeometricDofField<vector, dgPatchField, dgGeoMesh>& dofU, const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf, label la) {
        convectionSchemePtr = ptr;
        this->dofU = &dofU;
        this->vf = &vf;
        tmp<Field<Type>>tflux = convectionSchemePtr->getFluxCalcScheme().fluxCalculateWeak(dofU, vf);
        this->flux = tflux.ptr();
        this->lduMatrixPtr = make_shared<dgLduMatrix<Type>>(vf.mesh(), la);
    }

    static shared_ptr<Equation<Type>> dgmNew(const dgGaussVectorField& U, const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf, const word& name) {
        return make_shared<EquationDgmConvectionSchemeType1<Type>>(
            dg::convectionScheme<Type>::New(
                vf.mesh(),
                vf.mesh().divScheme(name)
            ),U, vf, DgmConvectionSchemeLabel
        );
    }

    EquationConvectionScheme(tmp<convectionScheme<Type>>& ptr, const dgGaussVectorField& U, const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf, label la) {
        convectionSchemePtr = ptr;
        this->U = & U;
        this->vf = &vf;
        tmp<Field<Type>>tflux = convectionSchemePtr->getFluxCalcScheme().fluxCalculateWeak(U, vf.gaussField());
        this->flux = tflux.ptr();
        //Info<<"EquationConvectionScheme.H tflux is tmp? : "<<tflux.isTmp()<<endl;

        //tflux.ptr() = 0;
        this->lduMatrixPtr = make_shared<dgLduMatrix<Type>>(vf.mesh(), la);
    }


    virtual void calculateCell(dgTree<physicalCellElement>::leafIterator iter) = 0;
    virtual dimensionSet getDimensionSet()
    {
        return this->vf->dimensions() * this->U->dimensions()/dimLength;
    }

    tmp<convectionScheme<Type>> convectionSchemePtr;
    const dgGaussVectorField* U;
    const GeometricDofField<vector, dgPatchField, dgGeoMesh>* dofU;
    const GeometricDofField<Type, dgPatchField, dgGeoMesh>* vf;
    const dgGaussField<Type>* vf2;
    const Field<Type>* flux = NULL;

};


template<class Type>
class EquationDgcConvectionSchemeBase
    : public EquationConvectionScheme<Type>
{
public:
    EquationDgcConvectionSchemeBase(tmp<convectionScheme<Type>>& ptr, const dgGaussVectorField& U, const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf, const Field<Type>& flux, label la) : EquationConvectionScheme<Type>(ptr, U, vf, flux, la) {}
    EquationDgcConvectionSchemeBase(tmp<convectionScheme<Type>>& ptr, const dgGaussVectorField& U, const dgGaussField<Type>& vf2, label la) : EquationConvectionScheme<Type>(ptr, U, vf2, la) {}
    EquationDgcConvectionSchemeBase(tmp<convectionScheme<Type>>& ptr, const GeometricDofField<vector, dgPatchField, dgGeoMesh>& dofU, const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf, label la) : EquationConvectionScheme<Type>(ptr, dofU, vf, la) {}

    virtual const GeometricDofField<Type, dgPatchField, dgGeoMesh>* getPSI() {
        return NULL;
    }
    virtual label getMatrixLabel() {
        return EquationConvectionScheme<Type>::DgcConvectionSchemeLabel;
    }
};

template<class Type>
class EquationDgcConvectionSchemeType1
    : public EquationDgcConvectionSchemeBase<Type>
{
public:
    EquationDgcConvectionSchemeType1(tmp<convectionScheme<Type>> ptr, const dgGaussVectorField& U, const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf, const Field<Type>& flux, label la) : EquationDgcConvectionSchemeBase<Type>(ptr, U, vf, flux, la) {}

    virtual void calculateCell(dgTree<physicalCellElement>::leafIterator iter) {
        this->convectionSchemePtr->dgcDivCalculateCell(iter, this->lduMatrixPtr, *(this->U), this->vf, *(this->flux));
    }
};

template<class Type>
class EquationDgcConvectionSchemeType2
    : public EquationDgcConvectionSchemeBase<Type>
{
public:
    EquationDgcConvectionSchemeType2(tmp<convectionScheme<Type>> ptr, const dgGaussVectorField& U, const dgGaussField<Type>& vf2, label la) : EquationDgcConvectionSchemeBase<Type>(ptr, U, vf2, la) {}

    virtual void calculateCell(dgTree<physicalCellElement>::leafIterator iter) {
        this->convectionSchemePtr->dgcDivCalculateCell(iter, this->lduMatrixPtr, *(this->U), *(this->vf2), *(this->flux));
    }

    virtual dimensionSet getDimensionSet()
    {
        return this->vf2->dimensions() * this->U->dimensions()/dimLength;
    }

    virtual ~EquationDgcConvectionSchemeType2(){
        if(this->flux){
            delete this->flux;
        }
    }

};

template<class Type>
class EquationDgcConvectionSchemeType3
    : public EquationDgcConvectionSchemeBase<Type>
{
public:
    EquationDgcConvectionSchemeType3(tmp<convectionScheme<Type>> ptr, const GeometricDofField<vector, dgPatchField, dgGeoMesh>& dofU, const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf, label la) : EquationDgcConvectionSchemeBase<Type>(ptr, dofU, vf, la) {}

    virtual void calculateCell(dgTree<physicalCellElement>::leafIterator iter) {
        this->convectionSchemePtr->dgcDivCalculateCell(iter, this->lduMatrixPtr, *(this->dofU), *(this->vf), *(this->flux));
    }

    virtual dimensionSet getDimensionSet()
    {
        return this->vf->dimensions() * this->dofU->dimensions()/dimLength;
    }

    virtual ~EquationDgcConvectionSchemeType3(){
        if(this->flux){
            delete this->flux;
        }
    }
};

template<class Type>
class EquationDgmConvectionSchemeBase
    : public EquationConvectionScheme<Type>
{
public:
    EquationDgmConvectionSchemeBase(tmp<convectionScheme<Type>>& ptr, const dgGaussVectorField& U, const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf, label la) : EquationConvectionScheme<Type>(ptr, U, vf, la) {}

    virtual const GeometricDofField<Type, dgPatchField, dgGeoMesh>* getPSI() {
        return this->vf;
    }
    virtual label getMatrixLabel() {
        return EquationConvectionScheme<Type>::DgmConvectionSchemeLabel;
    }
};

template<class Type>
class EquationDgmConvectionSchemeType1
    : public EquationDgmConvectionSchemeBase<Type>
{
public:
    EquationDgmConvectionSchemeType1(tmp<convectionScheme<Type>> ptr, const dgGaussVectorField& U, const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf, label la) : EquationDgmConvectionSchemeBase<Type>(ptr, U, vf, la) {}

    virtual void calculateCell(dgTree<physicalCellElement>::leafIterator iter) {
        this->convectionSchemePtr->dgmDivCalculateCell(iter, this->lduMatrixPtr, *(this->U), *(this->vf));
    }
};

}
}
#endif //EquationConvectionScheme_H
