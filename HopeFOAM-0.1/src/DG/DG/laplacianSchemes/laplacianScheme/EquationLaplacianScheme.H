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
    Foam::dg::EquationLapicianScheme.H

Description
    This is a subclass of Equation, used to restore the result of laplacian
    scheme(including explicit and implicit).

SourceFiles
    
    
Author
    Zhen Xiao (452576504@qq.com)
\*---------------------------------------------------------------------------*/

#ifndef EquationLaplacianScheme_H
#define EquationLaplacianScheme_H

#include "Equation.H"
#include <memory>
#include "tmp.H"
#include "laplacianScheme.H"
namespace Foam
{
namespace dg
{

template<class Type>
class EquationDgcLaplacianSchemeType1;
template<class Type>
class EquationDgmLaplacianSchemeType1;
template<class Type>
class EquationDgmLaplacianSchemeType2;

template<class Type>
class EquationLaplacianScheme
    : public Equation<Type>
{
public:
    enum {
        DgcLaplacianSchemeLabel=0,
        DgmLaplacianSchemeLabel=3
    };

    static shared_ptr<Equation<Type>> dgcNew(const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf, const word& name) {
        return make_shared<EquationDgcLaplacianSchemeType1<Type>>(
            dg::laplacianScheme<Type, scalar>::New(
                vf.mesh(),
                vf.mesh().laplacianScheme(name)
            ),vf, DgcLaplacianSchemeLabel
        );
    }

    // EquationLaplacianScheme(tmp<laplacianScheme<Type, scalar>> ptr, const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf, label la) {
    //     laplacianSchemePtr = ptr;
    //     this->U = U;
    //     this->lduMatrixPtr = make_shared<dgLduMatrix<Type>>(vf.mesh(), la);
    // }

    static shared_ptr<Equation<Type>> dgmNew(const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf, const word& name) {
        return make_shared<EquationDgmLaplacianSchemeType1<Type>>(
            dg::laplacianScheme<Type, scalar>::New(
                vf.mesh(),
                vf.mesh().laplacianScheme(name)
            ),vf, DgmLaplacianSchemeLabel
        );
    }

    EquationLaplacianScheme(tmp<laplacianScheme<Type, scalar>>& ptr, const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf, label la) {
        laplacianSchemePtr = ptr;
        this->vf = &vf;
        this->lduMatrixPtr = make_shared<dgLduMatrix<Type>>(vf.mesh(), la);
    }

    static shared_ptr<Equation<Type>> dgmNew(const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf, const GeometricDofField<scalar, dgPatchField, dgGeoMesh>& gamma, const word& name) {
        return make_shared<EquationDgmLaplacianSchemeType2<Type>>(
            dg::laplacianScheme<Type, scalar>::New(
                vf.mesh(),
                vf.mesh().laplacianScheme(name)
            ),vf, gamma, DgmLaplacianSchemeLabel
        );
    }

    EquationLaplacianScheme(tmp<laplacianScheme<Type, scalar>>& ptr, const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf, const GeometricDofField<scalar, dgPatchField, dgGeoMesh>& gamma, label la) {
        laplacianSchemePtr = ptr;
        this->vf = &vf;
        this->gamma = &gamma;
        this->lduMatrixPtr = make_shared<dgLduMatrix<Type>>(vf.mesh(), la);
    }


    virtual void calculateCell(dgTree<physicalCellElement>::leafIterator iter) = 0;
    virtual dimensionSet getDimensionSet()
    {
        return this->vf->dimensions()/dimLength/dimLength;
    }

    tmp<laplacianScheme<Type, scalar>> laplacianSchemePtr;
    const GeometricDofField<scalar, dgPatchField, dgGeoMesh>* gamma;
    const GeometricDofField<Type, dgPatchField, dgGeoMesh>* vf;

};


template<class Type>
class EquationDgcLaplacianSchemeBase
    : public EquationLaplacianScheme<Type>
{
public:
    EquationDgcLaplacianSchemeBase(tmp<laplacianScheme<Type, scalar>>& ptr, const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf, label la) : EquationLaplacianScheme<Type>(ptr, vf, la) {}

    virtual const GeometricDofField<Type, dgPatchField, dgGeoMesh>* getPSI() {
        return NULL;
    }
    virtual label getMatrixLabel() {
        return EquationLaplacianScheme<Type>::DgcLaplacianSchemeLabel;
    }
};

template<class Type>
class EquationDgcLaplacianSchemeType1
    : public EquationDgcLaplacianSchemeBase<Type>
{
public:
    EquationDgcLaplacianSchemeType1(tmp<laplacianScheme<Type, scalar>> ptr, const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf, label la) : EquationDgcLaplacianSchemeBase<Type>(ptr, vf, la) {}

    virtual void calculateCell(dgTree<physicalCellElement>::leafIterator iter) {
        this->laplacianSchemePtr->dgcLaplacianCalculateCell(iter, this->lduMatrixPtr, this->vf);
    }
};

template<class Type>
class EquationDgmLaplacianSchemeBase
    : public EquationLaplacianScheme<Type>
{
public:
    EquationDgmLaplacianSchemeBase(tmp<laplacianScheme<Type, scalar>>& ptr, const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf, label la) : EquationLaplacianScheme<Type>(ptr, vf, la) {}
    EquationDgmLaplacianSchemeBase(tmp<laplacianScheme<Type, scalar>>& ptr, const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf, const GeometricDofField<scalar, dgPatchField, dgGeoMesh>& gamma, label la) : EquationLaplacianScheme<Type>(ptr, vf, gamma, la) {}

    virtual const GeometricDofField<Type, dgPatchField, dgGeoMesh>* getPSI() {
        return this->vf;
    }
    virtual label getMatrixLabel() {
        return EquationLaplacianScheme<Type>::DgmLaplacianSchemeLabel;
    }
};

template<class Type>
class EquationDgmLaplacianSchemeType1
    : public EquationDgmLaplacianSchemeBase<Type>
{
public:
    EquationDgmLaplacianSchemeType1(tmp<laplacianScheme<Type, scalar>> ptr, const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf, label la) : EquationDgmLaplacianSchemeBase<Type>(ptr, vf, la) {}

    virtual void calculateCell(dgTree<physicalCellElement>::leafIterator iter) {
        this->laplacianSchemePtr->dgmLaplacianCalculateCell(iter, this->lduMatrixPtr, this->vf);
    }

};

template<class Type>
class EquationDgmLaplacianSchemeType2
    : public EquationDgmLaplacianSchemeBase<Type>
{
public:
    EquationDgmLaplacianSchemeType2(tmp<laplacianScheme<Type, scalar>> ptr, const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf, const GeometricDofField<scalar, dgPatchField, dgGeoMesh>& gamma, label la) : EquationDgmLaplacianSchemeBase<Type>(ptr, vf, gamma, la) {}

    virtual void calculateCell(dgTree<physicalCellElement>::leafIterator iter) {
        this->laplacianSchemePtr->dgmLaplacianCalculateCell(iter, this->lduMatrixPtr, this->vf, this->gamma);
    }
};

}
}
#endif //EquationLaplacianScheme_H
