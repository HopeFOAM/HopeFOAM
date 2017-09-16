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

#include "cyclicDgPatchField.H"
#include "transformField.H"
#include "GeometricFields.H"
// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class Type>
Foam::cyclicDgPatchField<Type>::cyclicDgPatchField
(
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF
)
:
    coupledDgPatchField<Type>(p, iF),
    cyclicPatch_(refCast<const cyclicDgPatch>(p))
{}


template<class Type>
Foam::cyclicDgPatchField<Type>::cyclicDgPatchField
(
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF,
    const dictionary& dict
)
:
    coupledDgPatchField<Type>(p, iF, dict),
    cyclicPatch_(refCast<const cyclicDgPatch>(p))
{
    if (!isA<cyclicDgPatch>(p))
    {
        FatalIOErrorInFunction
        (
            dict
        )   << "    patch type '" << p.type()
            << "' not constraint type '" << typeName << "'"
            << "\n    for patch " << p.name()
            << " of field " << this->dimensionedInternalField().name()
            << " in file " << this->dimensionedInternalField().objectPath()
            << exit(FatalIOError);
    }

    this->evaluate(Pstream::blocking);
}


template<class Type>
Foam::cyclicDgPatchField<Type>::cyclicDgPatchField
(
    const cyclicDgPatchField<Type>& ptf,
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF,
    const dgPatchFieldMapper& mapper
)
:
    coupledDgPatchField<Type>(ptf, p, iF, mapper),
    cyclicPatch_(refCast<const cyclicDgPatch>(p))
{
    if (!isA<cyclicDgPatch>(this->patch()))
    {
        FatalErrorInFunction
            << "' not constraint type '" << typeName << "'"
            << "\n    for patch " << p.name()
            << " of field " << this->dimensionedInternalField().name()
            << " in file " << this->dimensionedInternalField().objectPath()
            << exit(FatalIOError);
    }
}


template<class Type>
Foam::cyclicDgPatchField<Type>::cyclicDgPatchField
(
    const cyclicDgPatchField<Type>& ptf
)
:
    cyclicLduInterfaceField(),
    coupledDgPatchField<Type>(ptf),
    cyclicPatch_(ptf.cyclicPatch_)
{}


template<class Type>
Foam::cyclicDgPatchField<Type>::cyclicDgPatchField
(
    const cyclicDgPatchField<Type>& ptf,
    const DimensionedField<Type, dgGeoMesh>& iF
)
:
    coupledDgPatchField<Type>(ptf, iF),
    cyclicPatch_(ptf.cyclicPatch_)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class Type>
Foam::tmp<Foam::Field<Type>>
Foam::cyclicDgPatchField<Type>::patchNeighbourField() const
{
    tmp<Field<Type>> tpnf(new Field<Type>(this->size()));
    Field<Type>& pnf = tpnf.ref();

    const dgMesh& mesh=cyclicPatch_.boundaryMesh().mesh();
    const labelListList& cellDofIndex = mesh.cellDofIndex();

    const labelList bdFaceIndex = cyclicPatch_.dgFaceIndex();
    const List<dgFace>& dgFaceList = mesh.dgFaceList();
    const Field<Type>& internalField = this->internalField();
    label index = 0;
    forAll(bdFaceIndex, faceI){
        label bdFaceI = bdFaceIndex[faceI];
        const dgFace& dgFaceI = dgFaceList[bdFaceI];

        const labelList& faceIndexN = (dgFaceI.neighborBase_)->faceToCellIndex();
        labelList tmpList(dgFaceI.nNeighborDof_);
        for(int i=0, j=dgFaceI.nNeighborDof_*dgFaceI.faceNeighborLocalID_; i<dgFaceI.nNeighborDof_; i++, j++){
            tmpList[i] = cellDofIndex[-2-dgFaceI.faceNeighbor_][faceIndexN[j]];
        }
        labelList faceRotate = ((dgFaceI.neighborBase_)->faceRotateIndex())[mesh.firstPointIndex()[-2-dgFaceI.faceNeighbor_][dgFaceI.faceNeighborLocalID_]];
        
        for(int i=0,indexS = index; i<dgFaceI.nNeighborDof_; i++){
            pnf[index] = internalField[tmpList[faceRotate[i]]];
            index ++;
        }
    }


    /*
    if (doTransform())
    {
        forAll(pnf, facei)
        {
            pnf[facei] = transform
            (
                forwardT()[0], this->patchInternalField()
            );
        }
    }
    else
    {
        forAll(pnf, facei)
        {
            pnf[facei] = iField[nbrFaceCells[facei]];
        }
    }*/

    return tpnf;
}


template<class Type>
const Foam::cyclicDgPatchField<Type>&
Foam::cyclicDgPatchField<Type>::neighbourPatchField() const
{
    const GeometricField<Type, dgPatchField, dgGeoMesh>& fld =
    static_cast<const GeometricField<Type, dgPatchField, dgGeoMesh>&>
    (
        this->internalField()
    );

    return refCast<const cyclicDgPatchField<Type>>
    (
        fld.boundaryField()[this->cyclicPatch().neighbPatchID()]
    );
}


template<class Type>
void Foam::cyclicDgPatchField<Type>::updateInterfaceMatrix
(
    scalarField& result,
    const scalarField& psiInternal,
    const scalarField& coeffs,
    const direction cmpt,
    const Pstream::commsTypes
) const
{
    const labelUList& nbrFaceCells =
        cyclicPatch().cyclicPatch().neighbPatch().faceCells();

    scalarField pnf(psiInternal, nbrFaceCells);

    // Transform according to the transformation tensors
    transformCoupleField(pnf, cmpt);

    // Multiply the field by coefficients and add into the result
    const labelUList& faceCells = cyclicPatch_.faceCells();

    forAll(faceCells, elemI)
    {
        result[faceCells[elemI]] -= coeffs[elemI]*pnf[elemI];
    }
}


template<class Type>
void Foam::cyclicDgPatchField<Type>::updateInterfaceMatrix
(
    Field<Type>& result,
    const Field<Type>& psiInternal,
    const scalarField& coeffs,
    const Pstream::commsTypes
) const
{
    const labelUList& nbrFaceCells =
        cyclicPatch().cyclicPatch().neighbPatch().faceCells();

    Field<Type> pnf(psiInternal, nbrFaceCells);

    // Transform according to the transformation tensors
    transformCoupleField(pnf);

    // Multiply the field by coefficients and add into the result
    const labelUList& faceCells = cyclicPatch_.faceCells();

    forAll(faceCells, elemI)
    {
        result[faceCells[elemI]] -= coeffs[elemI]*pnf[elemI];
    }
}


template<class Type>
void Foam::cyclicDgPatchField<Type>::write(Ostream& os) const
{
    dgPatchField<Type>::write(os);
}


// ************************************************************************* //
