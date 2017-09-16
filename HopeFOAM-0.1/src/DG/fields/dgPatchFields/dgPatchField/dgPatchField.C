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

#include "IOobject.H"
#include "dictionary.H"
#include "dgMesh.H"
#include "dgPatchFieldMapper.H"
#include "dgGeoMesh.H"
#include "physicalFaceElement.H"
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class Type>
Foam::dgPatchField<Type>::dgPatchField
(
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF
)
	:
	Field<Type>(p.totalDofNum()),
	patch_(p),
	internalField_(iF),
	updated_(false),
	patchType_(word::null)
{
}


template<class Type>
Foam::dgPatchField<Type>::dgPatchField
(
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF,
    const Type& value
)
:
    Field<Type>(p.totalDofNum(), value),
    patch_(p),
    internalField_(iF),
    updated_(false),
    patchType_(word::null)
{
}

template<class Type>
Foam::dgPatchField<Type>::dgPatchField
(
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF,
    const word& patchType
)
	:
	Field<Type>(p.totalDofNum()),
	patch_(p),
	internalField_(iF),
	updated_(false),
	patchType_(patchType)
{
}


template<class Type>
Foam::dgPatchField<Type>::dgPatchField
(
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF,
    const Field<Type>& f
)
	:
	Field<Type>(f),
	patch_(p),
	internalField_(iF),
	updated_(false),
	patchType_(word::null)
{
}


template<class Type>
Foam::dgPatchField<Type>::dgPatchField
(
    const dgPatchField<Type>& ptf,
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF,
    const dgPatchFieldMapper& mapper
)
	:
	Field<Type>(p.totalDofNum()),
	patch_(p),
	internalField_(iF),
	updated_(false),
	patchType_(ptf.patchType_)
{
	// For unmapped faces set to internal field value (zero-gradient)
	if (notNull(iF) && mapper.hasUnmapped()) {
		dgPatchField<Type>::operator=(this->patchInternalField());
	}

	this->map(ptf, mapper);
}


template<class Type>
Foam::dgPatchField<Type>::dgPatchField
(
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF,
    const dictionary& dict,
    const bool valueRequired
)
	:
	Field<Type>(p.totalDofNum()),
	patch_(p),
	internalField_(iF),
	updated_(false),
	patchType_(dict.lookupOrDefault<word>("patchType", word::null))
{

	if (dict.found("value")) {
		dgPatchField<Type>::operator=
		(
		    Field<Type>("value", dict, p.totalDofNum())
		);
	} else if (!valueRequired) {
		dgPatchField<Type>::operator=(pTraits<Type>::zero);
	} else {
		FatalIOErrorIn
		(
		    "dgPatchField<Type>::dgPatchField"
		    "("
		    "const dgPatch& p,"
		    "const DimensionedField<Type, dgGeoMesh>& iF,"
		    "const dictionary& dict,"
		    "const bool valueRequired"
		    ")",
		    dict
		)   << "Essential entry 'value' missing"
		    << exit(FatalIOError);
	}
}


template<class Type>
Foam::dgPatchField<Type>::dgPatchField
(
    const dgPatchField<Type>& ptf
)
	:
	Field<Type>(ptf),
	patch_(ptf.patch_),
	internalField_(ptf.internalField_),
	updated_(false),
	patchType_(ptf.patchType_)
{
}


template<class Type>
Foam::dgPatchField<Type>::dgPatchField
(
    const dgPatchField<Type>& ptf,
    const DimensionedField<Type, dgGeoMesh>& iF
)
	:
	Field<Type>(ptf),
	patch_(ptf.patch_),
	internalField_(iF),
	updated_(false),
	patchType_(ptf.patchType_)
{
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class Type>
const objectRegistry& dgPatchField<Type>::db() const
{
	return patch_.boundaryMesh().mesh();
}


template<class Type>
void dgPatchField<Type>::check(const dgPatchField<Type>& ptf) const
{
	if (&patch_ != &(ptf.patch_)) {
		FatalErrorIn("PatchField<Type>::check(const dgPatchField<Type>&)")
		        << "different patches for dgPatchField<Type>s"
		        << abort(FatalError);
	}
}

template<class Type>
Foam::tmp<Foam::Field<Type> > Foam::dgPatchField<Type>::snGrad() const
{
	const dgMesh& mesh=patch_.boundaryMesh().mesh();
	const labelList& dgFaceIndex = patch_.dgFaceIndex();
	
	tmp<Field<Type> > tpGrad(new Field<Type>(patch_.totalDofNum()));
    Field<Type>& pGrad = tpGrad.ref();


    return tpGrad;
}

template<class Type>
Foam::tmp<Foam::Field<Type> >
Foam::dgPatchField<Type>::patchInternalField() const
{

	//return tpif;
	const dgMesh& mesh=patch_.boundaryMesh().mesh();

    tmp<Field<Type> > tpif(new Field<Type>(patch_.totalDofNum()));
    Field<Type>& pif = tpif.ref();
    if(this->totalDofNum() == 0) return tpif;

    const labelList& dgFaceIndex = patch_.dgFaceIndex();

    const dgTree<physicalFaceElement>& faceEleTree = mesh.faceElementsTree();

    label pointIndex = 0;
    for(dgTree<physicalFaceElement>::leafIterator iter = faceEleTree.leafBegin(dgFaceIndex); iter != faceEleTree.end() ; ++iter){
    	physicalCellElement cellI = iter()->value().ownerEle_->value();
    	label start = cellI.dofStart();
    	const labelList& faceDofMapping = iter()->value().ownerDofMapping();
    	forAll(faceDofMapping, pointI){
    		pif[pointIndex] = internalField_[start+faceDofMapping[pointI]];

    		pointIndex++;
    	}
    }

    return tpif;
	//return patch_.patchInternalField(internalField_);
}

template<class Type>
void Foam::dgPatchField<Type>::patchInternalField(Field<Type>& pif) const
{

	//patch_.patchInternalField(internalField_, pif);
	const dgMesh& mesh=patch_.boundaryMesh().mesh();

    const labelList& dgFaceIndex = patch_.dgFaceIndex();
    if(this->totalDofNum() == 0) return;
    else pif.setSize(this->totalDofNum());

    const dgTree<physicalFaceElement>& faceEleTree = mesh.faceElementsTree();

    label pointIndex = 0;
    for(dgTree<physicalFaceElement>::leafIterator iter = faceEleTree.leafBegin(dgFaceIndex); iter != faceEleTree.end() ; ++iter){
    	physicalCellElement cellI = iter()->value().ownerEle_->value();
    	label start = cellI.dofStart();
    	const labelList& faceDofMapping = iter()->value().ownerDofMapping();
    	forAll(faceDofMapping, pointI){
    		pif[pointIndex] = internalField_[start+faceDofMapping[pointI]];

    		pointIndex++;
    	}
    }
}

template<class Type>
void Foam::dgPatchField<Type>::setPatchToInternalField(Field<Type>& pif)
{
}

// Map from self
template<class Type>
void dgPatchField<Type>::autoMap
(
    const dgPatchFieldMapper& m
)
{
	Field<Type>::autoMap(m);
}


// Reverse-map the given dgPatchField onto this dgPatchField
template<class Type>
void dgPatchField<Type>::rmap
(
    const dgPatchField<Type>& ptf,
    const labelList& addr
)
{
	Field<Type>::rmap(ptf, addr);
}


template<class Type>
void Foam::dgPatchField<Type>::updateCoeffs()
{
	updated_ = true;
}


template<class Type>
void Foam::dgPatchField<Type>::updateCoeffs(const scalarField& weights)
{
	if (!updated_) {
		updateCoeffs();

		Field<Type>& fld = *this;
		fld *= weights;

		updated_ = true;
	}
}

template<class Type>
void Foam::dgPatchField<Type>::evaluate(const Pstream::commsTypes)
{
	if (!updated_) {
		updateCoeffs();
	}

	updated_ = false;
	//manipulatedMatrix_ = false;
}

// Write
template<class Type>
void dgPatchField<Type>::write(Ostream& os) const
{
	os.writeKeyword("type") << type() << token::END_STATEMENT << nl;

	if (patchType_.size()) {
		os.writeKeyword("patchType") << patchType_
		                             << token::END_STATEMENT << nl;
	}
}

template<class Type>
template<class EntryType>
void Foam::dgPatchField<Type>::writeEntryIfDifferent
(
    Ostream& os,
    const word& entryName,
    const EntryType& value1,
    const EntryType& value2
) const
{
    if (value1 != value2)
    {
        os.writeKeyword(entryName) << value2 << token::END_STATEMENT << nl;
    }
}

// * * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * //

template<class Type>
void dgPatchField<Type>::operator=
(
    const UList<Type>& ul
)
{
	Field<Type>::operator=(ul);
}


template<class Type>
void dgPatchField<Type>::operator=
(
    const dgPatchField<Type>& ptf
)
{
	check(ptf);
	Field<Type>::operator=(ptf);
}


template<class Type>
void dgPatchField<Type>::operator+=
(
    const dgPatchField<Type>& ptf
)
{
	check(ptf);
	Field<Type>::operator+=(ptf);
}


template<class Type>
void dgPatchField<Type>::operator-=
(
    const dgPatchField<Type>& ptf
)
{
	check(ptf);
	Field<Type>::operator-=(ptf);
}


template<class Type>
void dgPatchField<Type>::operator*=
(
    const dgPatchField<scalar>& ptf
)
{
	if (&patch_ != &ptf.patch()) {
		FatalErrorIn
		(
		    "PatchField<Type>::operator*=(const dgPatchField<scalar>& ptf)"
		)   << "incompatible patches for patch fields"
		    << abort(FatalError);
	}

	Field<Type>::operator*=(ptf);
}


template<class Type>
void dgPatchField<Type>::operator/=
(
    const dgPatchField<scalar>& ptf
)
{
	if (&patch_ != &ptf.patch()) {
		FatalErrorIn
		(
		    "PatchField<Type>::operator/=(const dgPatchField<scalar>& ptf)"
		)   << "    incompatible patches for patch fields"
		    << abort(FatalError);
	}

	Field<Type>::operator/=(ptf);
}


template<class Type>
void dgPatchField<Type>::operator+=
(
    const Field<Type>& tf
)
{
	Field<Type>::operator+=(tf);
}


template<class Type>
void dgPatchField<Type>::operator-=
(
    const Field<Type>& tf
)
{
	Field<Type>::operator-=(tf);
}


template<class Type>
void dgPatchField<Type>::operator*=
(
    const scalarField& tf
)
{
	Field<Type>::operator*=(tf);
}


template<class Type>
void dgPatchField<Type>::operator/=
(
    const scalarField& tf
)
{
	Field<Type>::operator/=(tf);
}


template<class Type>
void dgPatchField<Type>::operator=
(
    const Type& t
)
{
	Field<Type>::operator=(t);
}


template<class Type>
void dgPatchField<Type>::operator+=
(
    const Type& t
)
{
	Field<Type>::operator+=(t);
}


template<class Type>
void dgPatchField<Type>::operator-=
(
    const Type& t
)
{
	Field<Type>::operator-=(t);
}


template<class Type>
void dgPatchField<Type>::operator*=
(
    const scalar s
)
{
	Field<Type>::operator*=(s);
}


template<class Type>
void dgPatchField<Type>::operator/=
(
    const scalar s
)
{
	Field<Type>::operator/=(s);
}


// Force an assignment, overriding fixedValue status
template<class Type>
void dgPatchField<Type>::operator==
(
    const dgPatchField<Type>& ptf
)
{
	Field<Type>::operator=(ptf);
}


template<class Type>
void dgPatchField<Type>::operator==
(
    const Field<Type>& tf
)
{
	Field<Type>::operator=(tf);
}


template<class Type>
void dgPatchField<Type>::operator==
(
    const Type& t
)
{
	Field<Type>::operator=(t);
}


// * * * * * * * * * * * * * * * IOstream Operators  * * * * * * * * * * * * //

template<class Type>
Ostream& operator<<(Ostream& os, const dgPatchField<Type>& ptf)
{
	ptf.write(os);

	os.check("Ostream& operator<<(Ostream&, const dgPatchField<Type>&");

	return os;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#   include "dgPatchFieldNew.C"

// ************************************************************************* //
