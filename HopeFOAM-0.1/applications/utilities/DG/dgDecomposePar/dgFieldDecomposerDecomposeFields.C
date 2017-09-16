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

#include "dgFieldDecomposer.H"
#include "processorDgPatchField.H"

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class Type>
Foam::tmp<Foam::GeometricDofField<Type, Foam::dgPatchField, Foam::dgGeoMesh> >
Foam::dgFieldDecomposer::decomposeField
(
    const GeometricDofField<Type, dgPatchField, dgGeoMesh>& field,
    const bool allowUnknownPatchFields
) const
{
    // 1. Create the complete field with dummy patch fields
    PtrList<dgPatchField<Type> > patchFields(boundaryAddressing_.size());

    forAll(boundaryAddressing_, patchi)
    {
        patchFields.set
        (
            patchi,
            dgPatchField<Type>::New
            (
                calculatedDgPatchField<Type>::typeName,
                procMesh_.boundary()[patchi],
                field
            )
        );
    }

    tmp<GeometricDofField<Type, dgPatchField, dgGeoMesh> > tresF
    (
        new GeometricDofField<Type, dgPatchField, dgGeoMesh>
        (
            IOobject
            (
                field.name(),
                procMesh_.time().timeName(),
                procMesh_,
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            procMesh_,
            field.dimensions(),
            Field<Type>(field.internalField(), cellFieldAddressing_),
            patchFields
        )
    );
    GeometricDofField<Type, dgPatchField, dgGeoMesh>& resF = tresF.ref();

    // 2. Change the dgPatchFields to the correct type using a mapper
    //  constructor (with reference to the now correct internal field)

    const typename GeometricField<Type, dgPatchField, dgGeoMesh>::Boundary& bf1 = resF.boundaryField();

	typename GeometricField<Type, dgPatchField, dgGeoMesh>::Boundary& bf = const_cast<typename GeometricField<Type, dgPatchField, dgGeoMesh>::Boundary&>(bf1);
	const dgTree<physicalFaceElement>& faceEleTree = completeMesh_.faceElementsTree();
    labelList faceIndex;

    forAll(bf, patchi)
    {
        if (patchFieldDecomposerPtrs_[patchi])
        {
            bf.set
            (
                patchi,
                dgPatchField<Type>::New
                (
                    field.boundaryField()[boundaryAddressing_[patchi]],
                    procMesh_.boundary()[patchi],
                    resF.internalField(),
                    *patchFieldDecomposerPtrs_[patchi]
                )
            );
			
        }
		
        else if (isA<processorDgPatch>(procMesh_.boundary()[patchi]))
        {

			
		    const labelPairList& directFaceLabel = processorVolPatchFieldDecomposerPtrs_[patchi]->directFaceLabel();
		    Field<Type> tmpField;
		    faceIndex.setSize(directFaceLabel.size());
            forAll(faceIndex, faceI){
                faceIndex[faceI] = directFaceLabel[faceI][1];
            }
            label faceI = 0;
            for(dgTree<physicalFaceElement>::leafIterator iter = faceEleTree.leafBegin(faceIndex) ; iter != faceEleTree.end() ; ++iter, ++faceI){
                physicalFaceElement& faceEle = iter()->value();
                label isOwner = directFaceLabel[faceI][0];
                if(isOwner>0){
                    for(label i=0 ; i<faceEle.nOwnerDof() ; i++){
                        label dofLabel = (faceEle.ownerEle_->value()).dofStart() + faceEle.ownerDofMapping()[i];
                        tmpField.append(field[dofLabel]);
                    }
                }
                else{
                    for(label i=0 ; i<faceEle.nNeighborDof() ; i++){
                        label dofLabel = (faceEle.neighborEle_->value()).dofStart() + faceEle.neighborDofMapping()[i];
                        tmpField.append(field[dofLabel]);
                    }
                }
            }

		
		    bf.set
            (
                patchi,
                new processorDgPatchField<Type>
                (
                    procMesh_.boundary()[patchi],
                    resF.internalField(),
                    tmpField
                )
            );
		
        }
        else
        {
            FatalErrorIn("dgFieldDecomposer::decomposeField()")
                << "Unknown type." << abort(FatalError);
        }
    }

    return tresF;
}


template<class GeoField>
void Foam::dgFieldDecomposer::decomposeFields
(
    const PtrList<GeoField>& fields
) const
{

    forAll(fields, fieldI)
    {
        decomposeField(fields[fieldI])().write();
    }
}


// ************************************************************************* //
