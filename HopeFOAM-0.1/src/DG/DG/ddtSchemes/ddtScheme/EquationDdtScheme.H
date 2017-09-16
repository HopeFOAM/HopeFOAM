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
    Foam::dg::EquationDdtScheme

Description
    This is a subclass of Equation, used to restore the result of ddt
    scheme(including explicit and implicit).

SourceFiles

Author
    Zhen Xiao (452576504@qq.com)
\*---------------------------------------------------------------------------*/

#ifndef EquationDdtScheme_H
#define EquationDdtScheme_H

#include "ddtScheme.H"
#include "Equation.H"
#include <memory>
#include "tmp.H"
#include "EulerDdtScheme.H"

namespace Foam
{

namespace dg
{

template<class Type>
class EquationDdtScheme;

template<class Type>
class EquationEulerDgcDdtScheme;

template<class Type>
class EquationEulerDgmDdtScheme;

template<class Type>
class EquationDdtScheme 
	: public Equation<Type>
{
public:


	EquationDdtScheme( tmp<ddtScheme<Type>> arg, 
		const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf, label la)
	{
		//ddtSchemePtr = make_shared<ddtScheme<Type>>(arg.ref());
		//ds_tmp = arg;
		ddtSchemePtr = arg;
		this->vf = &vf;
		this->lduMatrixPtr = make_shared<dgLduMatrix<Type>>(
			vf.mesh(), la);
	}

	static shared_ptr<Equation<Type>> dgmNew(
		const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf)
	{
		return dgmNew(1.0, vf);
	}

	static shared_ptr<Equation<Type>> dgmNew( const one&,
		 const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf)
	{
		return dgmNew(vf);
	}

	static shared_ptr<Equation<Type>> dgmNew(scalar alpha,
    	 const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf)
	{
		
		return make_shared<EquationEulerDgmDdtScheme<Type>>(
			ddtScheme<Type>::New
			(
				vf.mesh(),
				vf.mesh().ddtScheme("ddt(" + vf.name() + ')')
			),
			//.ref().dgmDdt(alpha, vf),
			vf,
			0
		);
	}

	static shared_ptr<Equation<Type>> dgcNew(
    	 const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf)
	{
		
		return make_shared<EquationEulerDgcDdtScheme<Type>>(
			ddtScheme<Type>::New
			(
				vf.mesh(),
				vf.mesh().ddtScheme("ddt(" + vf.name() + ')')
			),
	//		).ref().dgcDdt(vf),
			vf,
			0
		);
	}

	virtual void calculateCell(
		dgTree<physicalCellElement>::leafIterator iter) = 0;

	shared_ptr<ddtScheme<Type>> getDdtScheme()
	{
		return ddtSchemePtr;
	}

	const dgMesh& mesh() const
    {
        return ddtSchemePtr->mesh();
    }

	virtual ~EquationDdtScheme()
	{
	};

	//shared_ptr<ddtScheme<Type>> ddtSchemePtr;
	tmp<ddtScheme<Type>> ddtSchemePtr; 
	const GeometricDofField<Type, dgPatchField, dgGeoMesh>* vf;

private:

	

};

template<class Type>
class EquationEulerDgcDdtScheme
	: public EquationDdtScheme<Type>
{
public:

	EquationEulerDgcDdtScheme( tmp<ddtScheme<Type>> arg, 
		const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf, label la)
		: EquationDdtScheme<Type>(arg, vf, la)
	{}

	virtual void calculateCell(
        dgTree<physicalCellElement>::leafIterator iter)
	{
		this->ddtSchemePtr->dgcDdtCalculateCell(iter, this->lduMatrixPtr, this->vf);
	}

	virtual dimensionSet getDimensionSet()
    {
        return this->vf->dimensions()/dimTime;
    };

	virtual const GeometricDofField<Type, dgPatchField, dgGeoMesh>*
		getPSI()
    {
        return NULL;
    }

	virtual label getMatrixLabel()
    {
        return 0;
    };

};

template<class Type>
class EquationEulerDgmDdtScheme
	: public EquationDdtScheme<Type>
{
public:

	EquationEulerDgmDdtScheme( tmp<ddtScheme<Type>> arg, 
		const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf, label la)
		: EquationDdtScheme<Type>(arg, vf, la)
	{}

	virtual void calculateCell(
        dgTree<physicalCellElement>::leafIterator iter)
	{
		this->ddtSchemePtr->dgmDdtCalculateCell(iter, this->lduMatrixPtr, this->vf);
	}

	virtual dimensionSet getDimensionSet()
    {
        return this->vf->dimensions()/dimTime;
    };

	virtual const GeometricDofField<Type, dgPatchField, dgGeoMesh>* getPSI()
    {
        return this->vf;
    }

	virtual label getMatrixLabel()
    {
        return 0;
    };

};


}

}


#endif // EquationDdtScheme_H