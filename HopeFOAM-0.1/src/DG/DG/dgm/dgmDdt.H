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
    Foam::dgm

Description
    Calulate the matrix for the first temporal derivative.

SourceFiles
    dgmDdt.C
    
Author
    Xu Liyang (xucloud77@gmail.com)
\*---------------------------------------------------------------------------*/

#ifndef dgmDdt_H
#define dgmDdt_H

#include "dgFieldsFwd.H"
#include "dgMatrix.H"
#include "one.H"
#include "EquationDdtScheme.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                     Namespace dgm functions Declaration
\*---------------------------------------------------------------------------*/

namespace dgm
{

    template<class Type>
    shared_ptr<dg::Equation<Type>>
    ddt
    (
        const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf
    );

    template<class Type>
    shared_ptr<dg::Equation<Type>>
    ddt
    (
        scalar alpha,
        const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf
    );

    template<class Type>
    shared_ptr<dg::Equation<Type>>
    ddt
    (
        const one&,
        const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf
    );

    template<class Type>
    shared_ptr<dg::Equation<Type>>
    ddt
    (
        const one&,
        const one&,
        const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf
    );
    
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
#   include "dgmDdt.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
