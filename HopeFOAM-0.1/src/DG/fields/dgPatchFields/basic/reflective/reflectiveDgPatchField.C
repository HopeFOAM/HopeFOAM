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

#include "reflectiveDgPatchField.H"

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
reflectiveDgPatchField<Type>::reflectiveDgPatchField
(
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF
)
:
    dgPatchField<Type>(p, iF)
{}


template<class Type>
reflectiveDgPatchField<Type>::reflectiveDgPatchField
(
    const reflectiveDgPatchField<Type>& ptf,
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF,
    const dgPatchFieldMapper& mapper
)
:
    dgPatchField<Type>(ptf, p, iF, mapper)
{}


template<class Type>
reflectiveDgPatchField<Type>::reflectiveDgPatchField
(
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF,
    const dictionary& dict
)
:
    dgPatchField<Type>(p, iF, dict, false)
{}


template<class Type>
reflectiveDgPatchField<Type>::reflectiveDgPatchField
(
    const reflectiveDgPatchField<Type>& ptf
)
:
    dgPatchField<Type>(ptf)
{}


template<class Type>
reflectiveDgPatchField<Type>::reflectiveDgPatchField
(
    const reflectiveDgPatchField<Type>& ptf,
    const DimensionedField<Type, dgGeoMesh>& iF
)
:
    dgPatchField<Type>(ptf, iF)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class Type>
void reflectiveDgPatchField<Type>::evaluate(const Pstream::commsTypes)
{
    if (!this->updated())
    {
        this->updateCoeffs();
    }

    const dgMesh & mesh = this->patch().boundaryMesh().mesh();

    const dgTree<physicalFaceElement>& faceEleTree = mesh.faceElementsTree();
    label nf_size = this->totalDofNum();

    const labelList& dgFaceIndex = this->patch().dgFaceIndex();
    if(dgFaceIndex.size()<=0) return;
    vectorField nHat(nf_size,vector::zero) ;
    
  label index=0;
  label pointNum =0;

    for(dgTree<physicalFaceElement>::leafIterator iter = faceEleTree.leafBegin(dgFaceIndex); iter != faceEleTree.end() ; ++iter){
        const physicalFaceElement& ele = iter()->value();
        pointNum = ele.nOwnerDof();
        for(int i=0 ; i<pointNum ; i++){
            nHat[index] = ele.faceNx_[i];
            index++;
        }
    }
   /* for(label faceI=0;faceI<tPatch.subEleListSize();faceI++){
          const subElement& subEle = mesh.boundarySubElement()[faceI + tPatch.subEleListStart()];
            for(label i=0;i<pointsPerFace;i++)
               nHat[faceI*pointsPerFace+i] = subEle.normal_;*/

   // Pout<<"nHat:"<<nHat<<endl;
    const Field<Type> iF(this->patchInternalField());
  //  Info<<"if:==="<<iF<<endl;
  //  Info<<"transform:"<<   transform(I - 2.0*sqr(nHat),iF)<<endl;
  //  Info<<"size1:"<<nHat.size()<<"size2:"<<iF.size()<<endl;
    Field<Type>::operator=
    (
    // transform is to get vector iF symmetry about nHat,scalar iF will not change
        transform(I - 2.0*sqr(nHat), iF)
    );
    dgPatchField<Type>::evaluate();
}

template<class Type>
tmp<Field<Type> > reflectiveDgPatchField<Type>::valueInternalCoeffs
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
tmp<Field<Type> > reflectiveDgPatchField<Type>::valueBoundaryCoeffs
(
    const tmp<scalarField>&
) const
{
    return *this;
}


template<class Type>
tmp<Field<Type> > reflectiveDgPatchField<Type>::gradientInternalCoeffs() const
{
    return -pTraits<Type>::one*this->patch().deltaCoeffs();
}


template<class Type>
tmp<Field<Type> > reflectiveDgPatchField<Type>::gradientBoundaryCoeffs() const
{
    return this->patch().deltaCoeffs()*(*this);
}

template<class Type>
void Foam::reflectiveDgPatchField<Type>::write(Ostream& os) const
{
    dgPatchField<Type>::write(os);
    this->writeEntry("value", os);
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //
