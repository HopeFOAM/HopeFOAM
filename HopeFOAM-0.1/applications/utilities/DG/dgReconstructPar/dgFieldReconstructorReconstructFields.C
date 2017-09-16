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

#include "dgFieldReconstructor.H"
#include "Time.H"
#include "PtrList.H"
#include "dgPatchFields.H"
#include "emptyDgPatch.H"
#include "emptyDgPatchField.H"


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //



template<class Type>
Foam::tmp<Foam::GeometricDofField<Type, Foam::dgPatchField, Foam::dgGeoMesh>>
Foam::dgFieldReconstructor::reconstructDgVolumeField
(
    const IOobject& fieldIoObject,
    const PtrList<GeometricDofField<Type, dgPatchField, dgGeoMesh>>& procFields
) const
{
    // Create the internalField
    Field<Type> internalField(mesh_.totalDof());

    // Create the patch fields
    PtrList<dgPatchField<Type>> patchFields(mesh_.boundary().size());
    labelList dgPatchFieldSize(mesh_.boundary().size(),0);
    forAll(procFields,proci){
  	    forAll(boundaryProcAddressing_[proci], patchi){
            // Get patch index of the original patch
            const label curBPatch = boundaryProcAddressing_[proci][patchi];
	        if(curBPatch >=0)
                dgPatchFieldSize[curBPatch] += procFields[proci].boundaryField()[patchi].size();
  	    }
    }
	

    forAll(procFields, proci){
        const GeometricDofField<Type, dgPatchField, dgGeoMesh>& procField = procFields[proci];

    	labelList cellProcAddr(procField.primitiveField().size(),0);
    	label position = 0;

        const dgMesh& procMesh = procField.mesh();
        const dgTree<physicalCellElement>& cellEleTree = procMesh.cellElementsTree();
        label cellI=0;
        for(dgTree<physicalCellElement>::leafIterator iter=cellEleTree.leafBegin() ; iter!=cellEleTree.end() ; ++iter, ++cellI){
            label cellsize = iter()->value().nDof();
            label cellLabel = cellProcAddressing_[proci][cellI];
            for(label i=0 ; i<cellsize ; i++,position++){
                cellProcAddr[position] = cellPointStart_[cellLabel] + i;
            }
        }

        // Set the cell values in the reconstructed field
        internalField.rmap
        (
            procField.primitiveField(),
           cellProcAddr
        );
        // Set the boundary patch values in the reconstructed field
		 //!!!!NOTE:	 no mater of tri and quad, since both have the same points if they have the same order
		 //so this implement is proper for same  order in each patch face
	   label pointsPerFace=procField.mesh().faceElements()[0]->value().nOwnerDof();  
	   
        forAll(boundaryProcAddressing_[proci], patchi)
        {
            // Get patch index of the original patch
            const label curBPatch = boundaryProcAddressing_[proci][patchi];
            // Get addressing slice for this patch 
            // thus get the global face label list of patchi in proci
            const labelList::subList cp =
                procField.mesh().boundary()[patchi].patchSlice
                (
                    faceProcAddressing_[proci]
                );

            // check if the boundary patch is not a processor patch
            if (curBPatch >= 0)
            {
                // Regular patch. Fast looping
       
                if (!patchFields(curBPatch))
                {

                    patchFields.set
                    (
                        curBPatch,
                        dgPatchField<Type>::New
                        (
                            procField.boundaryField()[patchi],
                            mesh_.boundary()[curBPatch],
                            DimensionedField<Type, Foam::dgGeoMesh>::null(),
                            dgPatchFieldReconstructor
                            (
                                 dgPatchFieldSize[curBPatch]
                            )
                        )
                    );
                }
             // get the global face start label in current patch. 
                const label curPatchStart =
                    mesh_.boundaryMesh()[curBPatch].start();

                labelList reverseAddressing(cp.size()*pointsPerFace);

			forAll(cp, facei)
                {
                    // Check
                    if (cp[facei] <= 0)
                    {
                        FatalErrorInFunction
                            << "Processor " << proci
                            << " patch "
                            << procField.mesh().boundary()[patchi].name()
                            << " face " << facei
                            << " originates from reversed face since "
                            << cp[facei]
                            << exit(FatalError);
                    }

                    // Subtract one to take into account offsets for
                    // face direction.
                    for(label i=0;i<pointsPerFace;i++)
                   		 reverseAddressing[facei*pointsPerFace+i] = (cp[facei] - 1 - curPatchStart)*pointsPerFace+i;
                }	

	
                patchFields[curBPatch].rmap
                (
                    procField.boundaryField()[patchi],
                    reverseAddressing
                );

            }
            else
            {
                const Field<Type>& curProcPatch =
                    procField.boundaryField()[patchi];

                // In processor patches, there's a mix of internal faces (some
                // of them turned) and possible cyclics. Slow loop
                forAll(cp, facei)
                {
                    // Subtract one to take into account offsets for
                    // face direction.
                    label curF = cp[facei] - 1;

                    // Is the face on the boundary?
                    if (curF >= mesh_.nInternalFaces())
                    {
                        label curBPatch = mesh_.boundaryMesh().whichPatch(curF);

                        if (!patchFields(curBPatch))
                        {
                            patchFields.set
                            (
                                curBPatch,
                                dgPatchField<Type>::New
                                (
                                    mesh_.boundary()[curBPatch].type(),
                                    mesh_.boundary()[curBPatch],
                                    DimensionedField<Type, Foam::dgGeoMesh>::null()
                                )
                            );
                        }

                        // add the face
                        label curPatchFace =
                            mesh_.boundaryMesh()
                                [curBPatch].whichFace(curF);

                        patchFields[curBPatch][curPatchFace] =
                            curProcPatch[facei];
                    }
                }
            }
        }
    }

    forAll(mesh_.boundary(), patchi)
    {
        // add empty patches
        if
        (
            isType<emptyDgPatch>(mesh_.boundary()[patchi])
         && !patchFields(patchi)
        )
        {
            patchFields.set
            (
                patchi,
                dgPatchField<Type>::New
                (
                    emptyDgPatchField<Type>::typeName,
                    mesh_.boundary()[patchi],
                    DimensionedField<Type, Foam::dgGeoMesh>::null()
                )
            );
        }
    }


    // Now construct and write the field
    // setting the internalField and patchFields
    return tmp<GeometricDofField<Type, dgPatchField, dgGeoMesh>>
    (
        new GeometricDofField<Type, dgPatchField, dgGeoMesh>
        (
            fieldIoObject,
            mesh_,
            procFields[0].dimensions(),
            internalField,
            patchFields
        )
    );
}


template<class Type>
Foam::tmp<Foam::GeometricDofField<Type, Foam::dgPatchField, Foam::dgGeoMesh>>
Foam::dgFieldReconstructor::reconstructDgVolumeField
(
    const IOobject& fieldIoObject
) const
{
    // Read the field for all the processors
    PtrList<GeometricDofField<Type, dgPatchField, dgGeoMesh>> procFields
    (
        procMeshes_.size()
    );

    forAll(procMeshes_, proci)
    {
        procFields.set
        (
            proci,
            new GeometricDofField<Type, dgPatchField, dgGeoMesh>
            (
                IOobject
                (
                    fieldIoObject.name(),
                    procMeshes_[proci].time().timeName(),
                    procMeshes_[proci],
                    IOobject::MUST_READ,
                    IOobject::NO_WRITE
                ),
                procMeshes_[proci]
            )
        );
    }
	
    return reconstructDgVolumeField
    (
        IOobject
        (
            fieldIoObject.name(),
            mesh_.time().timeName(),
            mesh_,
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        procFields
    );
}






template<class Type>
void Foam::dgFieldReconstructor::reconstructDgVolumeFields
(
    const IOobjectList& objects,
    const HashSet<word>& selectedFields
)
{
    const word& fieldClassName =
        GeometricDofField<Type, dgPatchField, dgGeoMesh>::typeName;

    IOobjectList fields = objects.lookupClass(fieldClassName);

    if (fields.size())
    {
        Info<< "    Reconstructing " << fieldClassName << "s\n" << endl;

        forAllConstIter(IOobjectList, fields, fieldIter)
        {
            if
            (
                selectedFields.empty()
             || selectedFields.found(fieldIter()->name())
            )
            {
                Info<< "        " << fieldIter()->name() << endl;

                reconstructDgVolumeField<Type>(*fieldIter())().write();

                nReconstructed_++;
            }
        }
        Info<< endl;
    }
}




// ************************************************************************* //
