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
    Calculate the gradient of the given field.

SourceFiles
    dgcGrad.C

Author
    Xu Liyang (xucloud77@gmail.com)
\*---------------------------------------------------------------------------*/


#ifndef dgcGrad_H
#define dgcGrad_H

#include "dgFieldsFwd.H"
#include "dgGaussField.H"
#include "EquationGradScheme.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                     Namespace dgc functions Declaration
\*---------------------------------------------------------------------------*/

namespace dgc
{

    template<class Type>
    shared_ptr<dg::Equation<typename outerProduct<vector, Type>::type>> grad
    (
        const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf,
        const word& name
    );

    template<class Type>
    shared_ptr<dg::Equation<typename outerProduct<vector, Type>::type>> grad
    (
        const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf
    );

    template<class Type>
    shared_ptr<dg::Equation<typename outerProduct<vector, Type>::type>> grad
    (
        const dgGaussField<Type>& U,
        const word& name
    );

    template<class Type>
    shared_ptr<dg::Equation<typename outerProduct<vector, Type>::type>> grad
    (
        const dgGaussField<Type>& U
    );



}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
#   include "dgcGrad.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
