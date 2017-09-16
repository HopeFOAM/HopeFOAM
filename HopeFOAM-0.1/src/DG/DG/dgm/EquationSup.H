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
    Foam::dg::EquationSup

Description
    This is a subclass of Equation, used to restore the result of Sp
    scheme.

SourceFiles
    
    
Author
    Zhen Xiao (452576504@qq.com)
\*---------------------------------------------------------------------------*/

#ifndef EquationSup_H
#define EquationSup_H

#include "Equation.H"
#include <memory>
#include "tmp.H"

namespace Foam
{
namespace dg
{

template<class Type>
class EquationDgmSupType1;
template<class Type>
class EquationDgmSupType2;

template<class Type>
class EquationSup
    : public Equation<Type>
{
public:
    enum {
        DgmSupLabel=0};

    static shared_ptr<Equation<Type>> dgmNew(const DimensionedField<scalar, dgGeoMesh>& sp, const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf, const word& name) {
        return make_shared<EquationDgmSupType1<Type>>(
            sp, vf, DgmSupLabel
        );
    }

    EquationSup(const DimensionedField<scalar, dgGeoMesh>& sp, const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf, label la) {
        this->sp = & sp;
        this->vf = &vf;
        this->lduMatrixPtr = make_shared<dgLduMatrix<Type>>(vf.mesh(), la);
    }

    static shared_ptr<Equation<Type>> dgmNew(const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf, const word& name) {
        return make_shared<EquationDgmSupType2<Type>>(
            vf, DgmSupLabel
        );
    }

    EquationSup(const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf, label la) {
        this->vf = &vf;
        this->lduMatrixPtr = make_shared<dgLduMatrix<Type>>(vf.mesh(), la);
    }


    virtual void calculateCell(dgTree<physicalCellElement>::leafIterator iter) = 0;
    virtual dimensionSet getDimensionSet()
    {
        return this->vf->dimensions();
    }

    const GeometricDofField<Type, dgPatchField, dgGeoMesh>* vf;
    const DimensionedField<scalar, dgGeoMesh>* sp;

};


template<class Type>
class EquationDgmSupBase
    : public EquationSup<Type>
{
public:
    EquationDgmSupBase(const DimensionedField<scalar, dgGeoMesh>& sp, const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf, label la) : EquationSup<Type>(sp, vf, la) {}
    EquationDgmSupBase(const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf, label la) : EquationSup<Type>(vf, la) {}

    virtual const GeometricDofField<Type, dgPatchField, dgGeoMesh>* getPSI() {
        return this->vf;
    }
    virtual label getMatrixLabel() {
        return EquationSup<Type>::DgmSupLabel;
    }
};

template<class Type>
class EquationDgmSupType1
    : public EquationDgmSupBase<Type>
{
public:
    EquationDgmSupType1(const DimensionedField<scalar, dgGeoMesh>& sp, const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf, label la) : EquationDgmSupBase<Type>(sp, vf, la) {}

    virtual void calculateCell(dgTree<physicalCellElement>::leafIterator iter) {
        //dgmDivCalculateCell(iter, this->lduMatrixPtr, *(this->sp), *(this->vf));
        FatalErrorIn
        (
            "Foam::dg::EquationDgmSupType1::calculateCell()"
        )   << "xuliyang->completeThis()"
            << abort(FatalError);
    }
};

template<class Type>
class EquationDgmSupType2
    : public EquationDgmSupBase<Type>
{
public:
    EquationDgmSupType2(const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf, label la) : EquationDgmSupBase<Type>(vf, la) {}

    virtual void calculateCell(dgTree<physicalCellElement>::leafIterator iter) {
        //dgmDivCalculateCell(iter, this->lduMatrixPtr, *(this->ds), *(this->vf));
        this->lduMatrixPtr->diagCoeff() = 1;
    }
};

}
}
#endif //EquationSup_H
