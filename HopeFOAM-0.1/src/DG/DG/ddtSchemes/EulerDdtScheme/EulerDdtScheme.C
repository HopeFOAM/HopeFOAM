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

#include "EulerDdtScheme.H"
#include "dgMatrices.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace dg
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

template<class Type>
tmp<dgMatrix<Type> >
EulerDdtScheme<Type>::dgmDdt
(
    scalar alpha,
    const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf
)
{
    const dgMesh& mesh = vf.mesh();

    tmp<dgMatrix<Type> > tdgm
    (
        new dgMatrix<Type>
        (
            vf,
            vf.dimensions()/dimTime,
            1
        )
    );
    dgMatrix<Type>& dgm = tdgm.ref();

    return tdgm;
}

template<class Type>
tmp<GeometricDofField<Type, dgPatchField, dgGeoMesh>>
EulerDdtScheme<Type>::dgcDdt
(
    const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf
)
{
    scalar rDeltaT = (1.0/mesh().time().deltaT().value());

    IOobject ddtIOobject
    (
        "ddt("+vf.name()+')',
        mesh().time().timeName(),
        mesh()
    );

    return tmp<GeometricDofField<Type, dgPatchField, dgGeoMesh>>
    (
        new GeometricDofField<Type, dgPatchField, dgGeoMesh>
        (
            ddtIOobject,
            rDeltaT*(vf - vf.oldTime())
        )
    );
}

template<class Type>
void EulerDdtScheme<Type>::dgcDdtCalculateCell(
    dgTree<physicalCellElement>::leafIterator iter, 
    shared_ptr<dgLduMatrix<Type>> ldu, 
    const PSI* psi)
{
    scalar rDeltaT = (1.0/mesh().time().deltaT().value());
    physicalCellElement& ele = iter()->value();
    label nDof = ele.nDof();
    label dofStart = ele.dofStart();

    const Field<Type>& vf = psi->internalField();
    const Field<Type>& vfOld = psi->oldTime().internalField();
    Field<Type>& source = ldu->source_;
    Field<Type>& b = ldu->b_;

    for(int i=0; i<nDof; ++i, ++dofStart){
        source[i] = rDeltaT*(vfOld[dofStart] - vf[dofStart]);
        b[i] = pTraits<Type>::zero;
    }
    ldu->sourceSize_ = nDof;
}

template<class Type>
void EulerDdtScheme<Type>::dgmDdtCalculateCell(
    dgTree<physicalCellElement>::leafIterator iter, 
    shared_ptr<dgLduMatrix<Type>> ldu, 
    const PSI* psi)
{
    scalar rDeltaT = (1.0/mesh().time().deltaT().value());
    physicalCellElement& ele = iter()->value();
    label nDof = ele.nDof();
    label dofStart = ele.dofStart();

    //- old implementation
    //ldu->diag_ = ele.massMatrix();
    //ldu->diag_ *= rDeltaT;

    //- new implementation
    ldu->diagCoeff() = rDeltaT;

    const Field<Type>& vfOld = psi->oldTime().internalField();
    Field<Type>& source = ldu->source_;
    Field<Type>& b = ldu->b_;

    for(int i=0; i<nDof; ++i, ++dofStart){
        source[i] = rDeltaT*(vfOld[dofStart]);
        b[i] = pTraits<Type>::zero;
    }
    ldu->sourceSize_ = nDof;
}
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace dg

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //
