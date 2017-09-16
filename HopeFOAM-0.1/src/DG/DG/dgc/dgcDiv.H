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

InNamespace
    Foam::dgc

Description
    Calculate the divergence of the given field.

SourceFiles
    dgcDiv.C

Author
    Xu Liyang (xucloud77@gmail.com)
\*---------------------------------------------------------------------------*/


#ifndef dgcDiv_H
#define dgcDiv_H

#include "dgFieldsFwd.H"
#include "dgGaussField.H"
#include "EquationConvectionScheme.H"
#include "EquationDivScheme.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                     Namespace dgc functions Declaration
\*---------------------------------------------------------------------------*/

namespace dgc
{

    // * * * * * * * * * * * * * Equation convection scheme * * * * * * * * * * * * * * * * * * * * //

    template<class Type>
    shared_ptr<dg::Equation<typename innerProduct<vector, Type>::type>> div
    (
        const GeometricDofField<Type, dgPatchField, dgGeoMesh>&,
        const word& name
    );

    template<class Type>
    shared_ptr<dg::Equation<typename innerProduct<vector, Type>::type>> div
    (
        const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf
    );

    template<class Type>
    shared_ptr<dg::Equation<typename innerProduct<vector, Type>::type>> div
    (
        const dgGaussField<Type>& dgf,
        const word& name
    );

    template<class Type>
    shared_ptr<dg::Equation<typename innerProduct<vector, Type>::type>> div
    (
        const dgGaussField<Type>& dgf
    );

    template<class Type>
    shared_ptr<dg::Equation<Type>> div
    (
        const GeometricDofField<vector, dgPatchField, dgGeoMesh>&,
        const GeometricDofField<Type, dgPatchField, dgGeoMesh>&,
        const word& name
    );

    template<class Type>
    shared_ptr<dg::Equation<Type>> div
    (
        const GeometricDofField<vector, dgPatchField, dgGeoMesh>&,
        const GeometricDofField<Type, dgPatchField, dgGeoMesh>&
    );


    template<class Type>
    shared_ptr<dg::Equation<Type>> div
    (
        const dgGaussVectorField& U,
        const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf,
        const Field<Type>& flux, const word& name
    );

    template<class Type>
    shared_ptr<dg::Equation<Type>> div
    (
        const dgGaussVectorField& U,
        const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf,
        const Field<Type>& flux
    );

    template<class Type>
    shared_ptr<dg::Equation<Type>> div
    (
        const dgGaussVectorField& U,
        const dgGaussField<Type>& vf, const word& name
    );

    template<class Type>
    shared_ptr<dg::Equation<Type>> div
    (
        const dgGaussVectorField& U,
        const dgGaussField<Type>& vf
    );

}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
#   include "dgcDiv.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
