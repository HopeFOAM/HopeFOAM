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

#include "coupledDgPatchField.H"

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class Type>
Foam::coupledDgPatchField<Type>::coupledDgPatchField
(
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF
)
:
    LduInterfaceField<Type>(refCast<const lduInterface>(p)),
    dgPatchField<Type>(p, iF)
{
	
}


template<class Type>
Foam::coupledDgPatchField<Type>::coupledDgPatchField
(
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF,
    const Field<Type>& f
)
:
    LduInterfaceField<Type>(refCast<const lduInterface>(p)),
    dgPatchField<Type>(p, iF, f)
{

}


template<class Type>
Foam::coupledDgPatchField<Type>::coupledDgPatchField
(
    const coupledDgPatchField<Type>& ptf,
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF,
    const dgPatchFieldMapper& mapper
)
:
    LduInterfaceField<Type>(refCast<const lduInterface>(p)),
    dgPatchField<Type>(ptf, p, iF, mapper)
{
	
}


template<class Type>
Foam::coupledDgPatchField<Type>::coupledDgPatchField
(
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF,
    const dictionary& dict
)
:
    LduInterfaceField<Type>(refCast<const lduInterface>(p)),
    dgPatchField<Type>(p, iF, dict)
{
}


template<class Type>
Foam::coupledDgPatchField<Type>::coupledDgPatchField
(
    const coupledDgPatchField<Type>& ptf
)
:
    LduInterfaceField<Type>(refCast<const lduInterface>(ptf.patch())),
    dgPatchField<Type>(ptf)
{

}


template<class Type>
Foam::coupledDgPatchField<Type>::coupledDgPatchField
(
    const coupledDgPatchField<Type>& ptf,
    const DimensionedField<Type, dgGeoMesh>& iF
)
:
    LduInterfaceField<Type>(refCast<const lduInterface>(ptf.patch())),
    dgPatchField<Type>(ptf, iF)
{

}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //





template<class Type>
Foam::tmp<Foam::Field<Type> > Foam::coupledDgPatchField<Type>::snGrad
(
    const scalarField& deltaCoeffs
) const
{
    return
        deltaCoeffs
       *(this->patchNeighbourField() - this->patchInternalField());
}


template<class Type>
void Foam::coupledDgPatchField<Type>::initEvaluate(const Pstream::commsTypes)
{
    if (!this->updated())
    {
        this->updateCoeffs();
    }
}



template<class Type>
void Foam::coupledDgPatchField<Type>::evaluate(const Pstream::commsTypes)
{
    if (!this->updated())
    {
        this->updateCoeffs();
    }

    Field<Type>::operator=
    (
        this->patchNeighbourField()
    );

    dgPatchField<Type>::evaluate();
}


template<class Type>
Foam::tmp<Foam::Field<Type> >
Foam::coupledDgPatchField<Type>::valueInternalCoeffs
(
    const tmp<scalarField>& w
) const
{
    return Type(pTraits<Type>::one)*w;
}


template<class Type>
Foam::tmp<Foam::Field<Type> >
Foam::coupledDgPatchField<Type>::valueBoundaryCoeffs
(
    const tmp<scalarField>& w
) const
{
    return Type(pTraits<Type>::one)*(1.0 - w);
}


template<class Type>
Foam::tmp<Foam::Field<Type> >
Foam::coupledDgPatchField<Type>::gradientInternalCoeffs
(
    const scalarField& deltaCoeffs
) const
{
    return -Type(pTraits<Type>::one)*deltaCoeffs;
}


template<class Type>
Foam::tmp<Foam::Field<Type> >
Foam::coupledDgPatchField<Type>::gradientInternalCoeffs() const
{
    notImplemented("coupledDgPatchField<Type>::gradientInternalCoeffs()");
    return -Type(pTraits<Type>::one)*this->patch().deltaCoeffs();
}


template<class Type>
Foam::tmp<Foam::Field<Type> >
Foam::coupledDgPatchField<Type>::gradientBoundaryCoeffs
(
    const scalarField& deltaCoeffs
) const
{
    return -this->gradientInternalCoeffs(deltaCoeffs);
}


template<class Type>
Foam::tmp<Foam::Field<Type> >
Foam::coupledDgPatchField<Type>::gradientBoundaryCoeffs() const
{
    notImplemented("coupledDgPatchField<Type>::gradientBoundaryCoeffs()");
    return -this->gradientInternalCoeffs();
}


template<class Type>
void Foam::coupledDgPatchField<Type>::write(Ostream& os) const
{
    dgPatchField<Type>::write(os);
    this->writeEntry("value", os);
}


// ************************************************************************* //
