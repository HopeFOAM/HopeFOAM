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

\*---------------------------------------------------------------------------*/

#include "dgFields.H"
#include "dgMatrix.H"
#include "laplacianScheme.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace dgm
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

template<class Type>
shared_ptr<dg::Equation<Type>>
laplacian
(
    const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf
)
{
    return dgm::laplacian
    (
        vf,
        "laplacian(" + vf.name() + ')'
    );
}


template<class Type>
shared_ptr<dg::Equation<Type>>
laplacian
(
    const one&,
    const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf,
    const word& name
)
{
    return dgm::laplacian(vf, name);
}


template<class Type>
shared_ptr<dg::Equation<Type>>
laplacian
(
    const one&,
    const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf
)
{
    return dgm::laplacian(vf);
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

template<class Type>
shared_ptr<dg::Equation<Type>>
laplacian
(
    const tmp<GeometricDofField<scalar, dgPatchField, dgGeoMesh> >& tgamma,
    const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf,
    const word& name
)
{
    tmp<dgMatrix<Type> > Laplacian(dgm::laplacian(tgamma(), vf, name));
    tgamma.clear();
    return Laplacian;
}


template<class Type>
shared_ptr<dg::Equation<Type>>
laplacian
(
    const GeometricDofField<scalar, dgPatchField, dgGeoMesh>& gamma,
    const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf
)
{
    return dgm::laplacian
    (
        gamma,
        vf,
        "laplacian(" + gamma.name() + ',' + vf.name() + ')'
    );
}


template<class Type>
shared_ptr<dg::Equation<Type>>
laplacian
(
    const tmp<GeometricDofField<scalar, dgPatchField, dgGeoMesh> >& tgamma,
    const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf
)
{
    shared_ptr<dg::Equation<Type>> Laplacian(dgm::laplacian(tgamma(), vf));
    tgamma.clear();
    return Laplacian;
}


template<class Type>
shared_ptr<dg::Equation<Type>>
laplacian
(
    const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf,
    const word& name
)
{
    return dg::EquationLaplacianScheme<Type>::dgmNew(vf, name);
}

template<class Type>
shared_ptr<dg::Equation<Type>>
laplacian
(
    const GeometricDofField<scalar, dgPatchField, dgGeoMesh>& gamma,
    const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf,
    const word& name
)
{
    return dg::EquationLaplacianScheme<Type>::dgmNew(vf, gamma, name);
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace dgm

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //
