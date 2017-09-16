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

Class
    Foam::processorDgPatchField

Group
    grpCoupledBoundaryConditions

Description
    This boundary condition enables processor communication across patches.

    \heading Patch usage

    Example of the boundary condition specification:
    \verbatim
    myPatch
    {
        type            processor;
    }
    \endverbatim

SourceFiles
    processorDgPatchField.C

\*---------------------------------------------------------------------------*/

#ifndef processorDgPatchField_H
#define processorDgPatchField_H

#include "coupledDgPatchField.H"
#include "processorLduInterfaceField.H"
#include "processorDgPatch.H"


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                    Class processorDgPatchField Declaration
\*---------------------------------------------------------------------------*/

template<class Type>
class processorDgPatchField
:
    public processorLduInterfaceField,
    public coupledDgPatchField<Type>
{
    // Private data

        //- Local reference cast into the processor patch
        const processorDgPatch& procPatch_;

        // Sending and receiving

            //- Send buffer.
            mutable Field<Type> sendBuf_;

            //- Receive buffer.
            mutable Field<Type> receiveBuf_;

            //- Outstanding request
            mutable label outstandingSendRequest_;

            //- Outstanding request
            mutable label outstandingRecvRequest_;

            //- Scalar send buffer
            mutable Field<scalar> scalarSendBuf_;

            //- Scalar receive buffer
            mutable Field<scalar> scalarReceiveBuf_;

public:

    //- Runtime type information
    TypeName(processorDgPatch::typeName_());


    // Constructors

        //- Construct from patch and internal field
        processorDgPatchField
        (
            const dgPatch&,
            const DimensionedField<Type, dgGeoMesh>&
        );

        //- Construct from patch and internal field and patch field
        processorDgPatchField
        (
            const dgPatch&,
            const DimensionedField<Type, dgGeoMesh>&,
            const Field<Type>&
        );

        //- Construct from patch, internal field and dictionary
        processorDgPatchField
        (
            const dgPatch&,
            const DimensionedField<Type, dgGeoMesh>&,
            const dictionary&
        );

        //- Construct by mapping given processorDgPatchField onto a new patch
        processorDgPatchField
        (
            const processorDgPatchField<Type>&,
            const dgPatch&,
            const DimensionedField<Type, dgGeoMesh>&,
            const dgPatchFieldMapper&
        );

        //- Construct as copy
        processorDgPatchField(const processorDgPatchField<Type>&);

        //- Construct and return a clone
        virtual tmp<dgPatchField<Type> > clone() const
        {
            return tmp<dgPatchField<Type> >
            (
                new processorDgPatchField<Type>(*this)
            );
        }

        //- Construct as copy setting internal field reference
        processorDgPatchField
        (
            const processorDgPatchField<Type>&,
            const DimensionedField<Type, dgGeoMesh>&
        );

        //- Construct and return a clone setting internal field reference
        virtual tmp<dgPatchField<Type> > clone
        (
            const DimensionedField<Type, dgGeoMesh>& iF
        ) const
        {
            return tmp<dgPatchField<Type> >
            (
                new processorDgPatchField<Type>(*this, iF)
            );
        }


    //- Destructor
    ~processorDgPatchField();


    // Member functions

        // Access

            //- Return true if running parallel
            virtual bool coupled() const
            {
                if (Pstream::parRun())
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }
		
            //- Return true if this patch field is distributed on processors
            virtual bool processored() const
            {
                if (Pstream::parRun())
                {
                    return true;
                }
                else
                {
                    return false;
                }
            }

            //- Return neighbour field given internal field
            virtual tmp<Field<Type> > patchNeighbourField() const;


        // Evaluation functions

            //- Initialise the evaluation of the patch field
            virtual void initEvaluate(const Pstream::commsTypes commsType);

            //- Evaluate the patch field
            virtual void evaluate(const Pstream::commsTypes commsType);

            //- Return patch-normal gradient
            virtual tmp<Field<Type> > snGrad
            (
                const scalarField& deltaCoeffs
            ) const;

            //- Is all data available
            virtual bool ready() const;

            //- Initialise neighbour matrix update
            virtual void initInterfaceMatrixUpdate
            (
                scalarField& result,
                const scalarField& psiInternal,
                const scalarField& coeffs,
                const direction cmpt,
                const Pstream::commsTypes commsType
            ) const;

            //- Update result field based on interface functionality
            virtual void updateInterfaceMatrix
            (
                scalarField& result,
                const scalarField& psiInternal,
                const scalarField& coeffs,
                const direction cmpt,
                const Pstream::commsTypes commsType
            ) const;

            //- Initialise neighbour matrix update
            virtual void initInterfaceMatrixUpdate
            (
                Field<Type>& result,
                const Field<Type>& psiInternal,
                const scalarField& coeffs,
                const Pstream::commsTypes commsType
            ) const;

            //- Update result field based on interface functionality
            virtual void updateInterfaceMatrix
            (
                Field<Type>& result,
                const Field<Type>& psiInternal,
                const scalarField& coeffs,
                const Pstream::commsTypes commsType
            ) const;


        //- Processor coupled interface functions

            //- Return communicator used for comms
            virtual int comm() const
            {
                return procPatch_.comm();
            }

            //- Return processor number
            virtual int myProcNo() const
            {
                return procPatch_.myProcNo();
            }

            //- Return neigbour processor number
            virtual int neighbProcNo() const
            {
                return procPatch_.neighbProcNo();
            }

            //- Does the patch field perform the transfromation
            virtual bool doTransform() const
            {
                return !(procPatch_.parallel() || pTraits<Type>::rank == 0);
            }

            //- Return face transformation tensor
            virtual const tensorField& forwardT() const
            {
                return procPatch_.forwardT();
            }

            //- Return rank of component for transform
            virtual int rank() const
            {
                return pTraits<Type>::rank;
            }

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
#   include "processorDgPatchField.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
