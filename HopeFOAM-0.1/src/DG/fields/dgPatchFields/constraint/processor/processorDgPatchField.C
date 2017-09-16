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

#include "processorDgPatchField.H"
#include "processorDgPatch.H"
#include "demandDrivenData.H"
#include "transformField.H"

// * * * * * * * * * * * * * * * * Constructors * * * * * * * * * * * * * * //

template<class Type>
Foam::processorDgPatchField<Type>::processorDgPatchField
(
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF
)
:
    coupledDgPatchField<Type>(p, iF),
    procPatch_(refCast<const processorDgPatch>(p)),
    sendBuf_(0),
    receiveBuf_(0),
    outstandingSendRequest_(-1),
    outstandingRecvRequest_(-1),
    scalarSendBuf_(0),
    scalarReceiveBuf_(0)
{
}


template<class Type>
Foam::processorDgPatchField<Type>::processorDgPatchField
(
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF,
    const Field<Type>& f
)
:
    coupledDgPatchField<Type>(p, iF, f),
    procPatch_(refCast<const processorDgPatch>(p)),
    sendBuf_(0),
    receiveBuf_(0),
    outstandingSendRequest_(-1),
    outstandingRecvRequest_(-1),
    scalarSendBuf_(0),
    scalarReceiveBuf_(0)
{
}


// Construct by mapping given processorDgPatchField<Type>
template<class Type>
Foam::processorDgPatchField<Type>::processorDgPatchField
(
    const processorDgPatchField<Type>& ptf,
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF,
    const dgPatchFieldMapper& mapper
)
:
    coupledDgPatchField<Type>(ptf, p, iF, mapper),
    procPatch_(refCast<const processorDgPatch>(p)),
    sendBuf_(0),
    receiveBuf_(0),
    outstandingSendRequest_(-1),
    outstandingRecvRequest_(-1),
    scalarSendBuf_(0),
    scalarReceiveBuf_(0)
{
    if (!isA<processorDgPatch>(this->patch()))
    {
        FatalErrorIn
        (
            "processorDgPatchField<Type>::processorDgPatchField\n"
            "(\n"
            "    const processorDgPatchField<Type>& ptf,\n"
            "    const dgPatch& p,\n"
            "    const DimensionedField<Type, dgGeoMesh>& iF,\n"
            "    const dgPatchFieldMapper& mapper\n"
            ")\n"
        )   << "\n    patch type '" << p.type()
            << "' not constraint type '" << typeName << "'"
            << "\n    for patch " << p.name()
            << " of field " << this->dimensionedInternalField().name()
            << " in file " << this->dimensionedInternalField().objectPath()
            << exit(FatalIOError);
    }
    if (debug && !ptf.ready())
    {
        FatalErrorIn("processorDgPatchField<Type>::processorDgPatchField(..)")
            << "On patch " << procPatch_.name() << " outstanding request."
            << abort(FatalError);
    }
}


template<class Type>
Foam::processorDgPatchField<Type>::processorDgPatchField
(
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF,
    const dictionary& dict
)
:
    coupledDgPatchField<Type>(p, iF, dict),
    procPatch_(refCast<const processorDgPatch>(p)),
    sendBuf_(0),
    receiveBuf_(0),
    outstandingSendRequest_(-1),
    outstandingRecvRequest_(-1),
    scalarSendBuf_(0),
    scalarReceiveBuf_(0)
{
    if (!isA<processorDgPatch>(p))
    {
        FatalIOErrorIn
        (
            "processorDgPatchField<Type>::processorDgPatchField\n"
            "(\n"
            "    const dgPatch& p,\n"
            "    const Field<Type>& field,\n"
            "    const dictionary& dict\n"
            ")\n",
            dict
        )   << "\n    patch type '" << p.type()
            << "' not constraint type '" << typeName << "'"
            << "\n    for patch " << p.name()
            << " of field " << this->dimensionedInternalField().name()
            << " in file " << this->dimensionedInternalField().objectPath()
            << exit(FatalIOError);
    }
}


template<class Type>
Foam::processorDgPatchField<Type>::processorDgPatchField
(
    const processorDgPatchField<Type>& ptf
)
:
    processorLduInterfaceField(),
    coupledDgPatchField<Type>(ptf),
    procPatch_(refCast<const processorDgPatch>(ptf.patch())),
    sendBuf_(ptf.sendBuf_.xfer()),
    receiveBuf_(ptf.receiveBuf_.xfer()),
    outstandingSendRequest_(-1),
    outstandingRecvRequest_(-1),
    scalarSendBuf_(ptf.scalarSendBuf_.xfer()),
    scalarReceiveBuf_(ptf.scalarReceiveBuf_.xfer())
{
    if (debug && !ptf.ready())
    {
        FatalErrorIn("processorDgPatchField<Type>::processorDgPatchField(..)")
            << "On patch " << procPatch_.name() << " outstanding request."
            << abort(FatalError);
    }
}


template<class Type>
Foam::processorDgPatchField<Type>::processorDgPatchField
(
    const processorDgPatchField<Type>& ptf,
    const DimensionedField<Type, dgGeoMesh>& iF
)
:
    coupledDgPatchField<Type>(ptf, iF),
    procPatch_(refCast<const processorDgPatch>(ptf.patch())),
    sendBuf_(0),
    receiveBuf_(0),
    outstandingSendRequest_(-1),
    outstandingRecvRequest_(-1),
    scalarSendBuf_(0),
    scalarReceiveBuf_(0)
{
    if (debug && !ptf.ready())
    {
        FatalErrorIn("processorDgPatchField<Type>::processorDgPatchField(..)")
            << "On patch " << procPatch_.name() << " outstanding request."
            << abort(FatalError);
    }
}


// * * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * //

template<class Type>
Foam::processorDgPatchField<Type>::~processorDgPatchField()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //


template<class Type>
Foam::tmp<Foam::Field<Type> >
Foam::processorDgPatchField<Type>::patchNeighbourField() const
{
    if (debug && !this->ready())
    {
        FatalErrorIn("processorDgPatchField<Type>::patchNeighbourField()")
            << "On patch " << procPatch_.name()
            << " outstanding request."
            << abort(FatalError);
    }
    return *this;
}


template<class Type>
void Foam::processorDgPatchField<Type>::initEvaluate
(
    const Pstream::commsTypes commsType
)
{
    if (Pstream::parRun()){    
        this->patchInternalField(sendBuf_);
        const dgPatch& dgpatch = dgPatchField<Type>::patch();
        const labelList& dgFaceIndexList = dgpatch.dgFaceIndex();
        const dgMesh& mesh=dgpatch.boundaryMesh().mesh();
        //const labelListList& ownerFaceIndex = mesh.ownerFaceDofIndex();
        const dgTree<physicalFaceElement>& faceEleTree = mesh.faceElementsTree();

        label faceI = 0;
        label pointBase = 0;
        for(dgTree<physicalFaceElement>::leafIterator iter = faceEleTree.leafBegin(dgFaceIndexList); iter != faceEleTree.end() ; ++iter){
            physicalFaceElement& ele = iter()->value();
            label size = ele.ownerDofMapping().size();
            if(ele.faceRotate_){
                Type tmp;
                for(int i=0 ; i<(size+1)/2 ; i++){
                    tmp = sendBuf_[i+pointBase];
                    sendBuf_[i+pointBase] = sendBuf_[size-1-i+pointBase];
                    sendBuf_[size-1-i+pointBase] = tmp;
                }
            }
            faceI++;
            pointBase += size;
        }
        if (commsType == Pstream::nonBlocking && !Pstream::floatTransfer){
            // Fast path. Receive into *this
            this->setSize(sendBuf_.size());
            outstandingRecvRequest_ = UPstream::nRequests();
            UIPstream::read
            (
                Pstream::nonBlocking,
                procPatch_.neighbProcNo(),
                reinterpret_cast<char*>(this->begin()),
                this->byteSize(),
                procPatch_.tag(),
                procPatch_.comm()
            );

            outstandingSendRequest_ = UPstream::nRequests();
            UOPstream::write
            (
                Pstream::nonBlocking,
                procPatch_.neighbProcNo(),
                reinterpret_cast<const char*>(sendBuf_.begin()),
                this->byteSize(),
                procPatch_.tag(),
                procPatch_.comm()
            );
        }
        else
        {
            procPatch_.compressedSend(commsType, sendBuf_);
        }
    }
}


template<class Type>
void Foam::processorDgPatchField<Type>::evaluate
(
    const Pstream::commsTypes commsType
)
{
    if (Pstream::parRun())
    {
        if (commsType == Pstream::nonBlocking && !Pstream::floatTransfer)
        {
            // Fast path. Received into *this

            if
            (
                outstandingRecvRequest_ >= 0
             && outstandingRecvRequest_ < Pstream::nRequests()
            )
            {
                UPstream::waitRequest(outstandingRecvRequest_);
            }
            outstandingSendRequest_ = -1;
            outstandingRecvRequest_ = -1;
        }
        else
        {
            procPatch_.compressedReceive<Type>(commsType, *this);
			
        }

        if (doTransform())
        {
            transform(*this, procPatch_.forwardT(), *this);
        }
    }
}


template<class Type>
Foam::tmp<Foam::Field<Type> >
Foam::processorDgPatchField<Type>::snGrad
(
    const scalarField& deltaCoeffs
) const
{
    return deltaCoeffs*(*this - this->patchInternalField());
}


template<class Type>
void Foam::processorDgPatchField<Type>::initInterfaceMatrixUpdate
(
    scalarField&,
    const scalarField& psiInternal,
    const scalarField&,
    const direction,
    const Pstream::commsTypes commsType
) const
{
}


template<class Type>
void Foam::processorDgPatchField<Type>::updateInterfaceMatrix
(
    scalarField& result,
    const scalarField&,
    const scalarField& coeffs,
    const direction cmpt,
    const Pstream::commsTypes commsType
) const
{
    if (this->updatedMatrix())
    {
        return;
    }
}


template<class Type>
void Foam::processorDgPatchField<Type>::initInterfaceMatrixUpdate
(
    Field<Type>&,
    const Field<Type>& psiInternal,
    const scalarField&,
    const Pstream::commsTypes commsType
) const
{
}


template<class Type>
void Foam::processorDgPatchField<Type>::updateInterfaceMatrix
(
    Field<Type>& result,
    const Field<Type>&,
    const scalarField& coeffs,
    const Pstream::commsTypes commsType
) const
{
}


template<class Type>
bool Foam::processorDgPatchField<Type>::ready() const
{
    if
    (
        outstandingSendRequest_ >= 0
     && outstandingSendRequest_ < Pstream::nRequests()
    )
    {
        bool finished = UPstream::finishedRequest(outstandingSendRequest_);
        if (!finished)
        {
            return false;
        }
    }
    outstandingSendRequest_ = -1;

    if
    (
        outstandingRecvRequest_ >= 0
     && outstandingRecvRequest_ < Pstream::nRequests()
    )
    {
        bool finished = UPstream::finishedRequest(outstandingRecvRequest_);
        if (!finished)
        {
            return false;
        }
    }
    outstandingRecvRequest_ = -1;

    return true;
}


// ************************************************************************* //
