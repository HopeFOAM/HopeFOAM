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

#include "slipDgPatchField.H"

#include "transformField.H"
#include "transform.H"
#include "mathematicalConstants.H"



#include "symmTransform.H"
#include "symmTransformField.H"




//#include "primitiveFields.H"
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class Type>
slipDgPatchField<Type>::slipDgPatchField
(
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF
)
:
    dgPatchField<Type>(p, iF)
{}


template<class Type>
slipDgPatchField<Type>::slipDgPatchField
(
    const slipDgPatchField<Type>& ptf,
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF,
    const dgPatchFieldMapper& mapper
)
:
    dgPatchField<Type>(ptf, p, iF, mapper)
{}


template<class Type>
slipDgPatchField<Type>::slipDgPatchField
(
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF,
    const dictionary& dict
)
:
    dgPatchField<Type>(p, iF, dict, false)
{}


template<class Type>
slipDgPatchField<Type>::slipDgPatchField
(
    const slipDgPatchField<Type>& ptf
)
:
    dgPatchField<Type>(ptf)
{}


template<class Type>
slipDgPatchField<Type>::slipDgPatchField
(
    const slipDgPatchField<Type>& ptf,
    const DimensionedField<Type, dgGeoMesh>& iF
)
:
    dgPatchField<Type>(ptf, iF)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class Type>
void slipDgPatchField<Type>::evaluate(const Pstream::commsTypes)
{
    if (!this->updated())
    {
        this->updateCoeffs();
    }

   const dgMesh & mesh = this->patch().boundaryMesh().mesh();

   label nf_size = this->totalDofNum();

   const dgPatch& tPatch = this->patch();

   const labelList& dgFaceIndex = tPatch.dgFaceIndex();
    
   vectorField nHat(nf_size,vector::zero) ;
	 const List<dgFace>& dgFaceList = mesh.dgFaceList();
	 	
   const List<List<vector>>& faceNx = const_cast<dgMesh&>(mesh).faceNormal();
		label index=0;
		label faceLabel=0;
		label pointNum =0;
		forAll(dgFaceIndex , faceI)
		{
				 faceLabel =  dgFaceIndex[faceI];
				const dgFace& dgFaceI = dgFaceList[faceLabel];
		
				 pointNum = dgFaceI.nOwnerDof_;
				for(int pointI=0;pointI<pointNum;pointI++)
					{
						nHat[index] = faceNx[faceLabel][pointI];
						index++;
					}
		}
   
    const Field<Type> iF(this->patchInternalField());


    Field<Type>::operator=
(
    // transform is to get vector iF symmetry about nHat,scalar iF will not change
	(iF + transform(I - 2.0*sqr(nHat), iF))/2.0
);
    dgPatchField<Type>::evaluate();
}

template<class Type>
tmp<Field<Type> > slipDgPatchField<Type>::valueInternalCoeffs
(
    const tmp<scalarField>&
) const
{
    return tmp<Field<Type> >
    (
        new Field<Type>(this->size(), pTraits<Type>::zero)
    );
}


template<class Type>
tmp<Field<Type> > slipDgPatchField<Type>::valueBoundaryCoeffs
(
    const tmp<scalarField>&
) const
{
    return *this;
}


template<class Type>
tmp<Field<Type> > slipDgPatchField<Type>::gradientInternalCoeffs() const
{
    return -pTraits<Type>::one*this->patch().deltaCoeffs();
}


template<class Type>
tmp<Field<Type> > slipDgPatchField<Type>::gradientBoundaryCoeffs() const
{
    return this->patch().deltaCoeffs()*(*this);
}

template<class Type>
void Foam::slipDgPatchField<Type>::write(Ostream& os) const
{
    dgPatchField<Type>::write(os);
    this->writeEntry("value", os);
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //
