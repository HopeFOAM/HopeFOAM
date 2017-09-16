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

Author
    Xu Liyang (xucloud77@gmail.com)

\*---------------------------------------------------------------------------*/

#include "dgcDiv.H"
#include "dgMesh.H"
#include "divScheme.H"
#include "convectionScheme.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace dgc
{


//*************************EquationSchemes***********************************//
template<class Type>
shared_ptr<dg::Equation<typename innerProduct<vector, Type>::type>> div
(
    const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf,
    const word& name
)
{
    return dg::EquationDivScheme<typename innerProduct<vector, Type>::type>::dgcNew(vf, name);
}

template<class Type>
shared_ptr<dg::Equation<typename innerProduct<vector, Type>::type>> div
(
    const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf
)
{
    return dg::EquationDivScheme<typename innerProduct<vector, Type>::type>::dgcNew(vf, "div("+vf.name()+")");
}

template<class Type>
shared_ptr<dg::Equation<typename innerProduct<vector, Type>::type>> div
(
    const dgGaussField<Type>& dgf,
    const word& name
)
{
    return dg::EquationDivScheme<typename innerProduct<vector, Type>::type>::dgcNew(dgf, name);
}

template<class Type>
shared_ptr<dg::Equation<typename innerProduct<vector, Type>::type>> div
(
    const dgGaussField<Type>& dgf
)
{
    return dg::EquationDivScheme<typename innerProduct<vector, Type>::type>::dgcNew(dgf, "div("+dgf.name()+")");
}


template<class Type>
shared_ptr<dg::Equation<Type>> div
(
    const GeometricDofField<vector, dgPatchField, dgGeoMesh>& dofU,
    const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf,
    const word& name
)
{
    return dg::EquationConvectionScheme<Type>::dgcNew(dofU, vf, name);
}

template<class Type>
shared_ptr<dg::Equation<Type>> div
(
    const GeometricDofField<vector, dgPatchField, dgGeoMesh>& dofU,
    const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf
)
{
    return dg::EquationConvectionScheme<Type>::dgcNew(dofU, vf, "div("+dofU.name()+','+vf.name()+')');
}

template<class Type>
shared_ptr<dg::Equation<Type>> div
(
    const dgGaussVectorField& U,
    const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf,
    const Field<Type>& flux, const word& name
)
{
    return dg::EquationConvectionScheme<Type>::dgcNew(U, vf, flux, name);
}

template<class Type>
shared_ptr<dg::Equation<Type>> div
(
    const dgGaussVectorField& U,
    const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf,
    const Field<Type>& flux
)
{
    return dg::EquationConvectionScheme<Type>::dgcNew(U, vf, flux, "div("+U.name()+','+vf.name()+')');
}

template<class Type>
shared_ptr<dg::Equation<Type>> div
(
    const dgGaussVectorField& U,
    const dgGaussField<Type>& vf, const word& name
)
{
    return dg::EquationConvectionScheme<Type>::dgcNew(U, vf, name);
}

template<class Type>
shared_ptr<dg::Equation<Type>> div
(
    const dgGaussVectorField& U,
    const dgGaussField<Type>& vf
)
{
    return dg::EquationConvectionScheme<Type>::dgcNew(U, vf, "div("+U.name()+','+vf.name()+')');
}


} // End namespace dgc

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //
