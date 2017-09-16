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
    Foam::processorDgPatch

Description
    Processor patch.

SourceFiles
    processorDgPatch.C

\*---------------------------------------------------------------------------*/

#ifndef processorDgPatch_H
#define processorDgPatch_H

#include "coupledDgPatch.H"
#include "processorLduInterface.H"
#include "processorPolyPatch.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                           Class processorDgPatch Declaration
\*---------------------------------------------------------------------------*/

class processorDgPatch
:
    public coupledDgPatch,
    public processorLduInterface
{
    // Private Data

        const processorPolyPatch& procPolyPatch_;


protected:

    // Protected Member functions

        //- Make patch weighting factors
        void makeWeights(scalarField&) const;


public:

    //- Runtime type information
    TypeName(processorPolyPatch::typeName_());

    enum cellShape : int{
        line,
        tri,
        quad,
        tet,
        hex
    };
    word cellShapeToString(int);
    int stringToCellShape(word);


    // Constructors

        //- Construct from components
        processorDgPatch(const polyPatch& patch, const dgBoundaryMesh& bm)
        :
            coupledDgPatch(patch, bm),
            procPolyPatch_(refCast<const processorPolyPatch>(patch))
        {}

    // Member functions
        // update coupled neighbor cell data
        virtual void updateNeighborData();

        //- Return communicator used for comms
        virtual int comm() const
        {
            return procPolyPatch_.comm();
        }

        //- Return processor number
        virtual int myProcNo() const
        {
            return procPolyPatch_.myProcNo();
        }

        //- Return neigbour processor number
        virtual int neighbProcNo() const
        {
            return procPolyPatch_.neighbProcNo();
        }

        //- Return message tag used for sending
        virtual int tag() const
        {
            return UPstream::msgType();
        }

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

        const processorPolyPatch& procPolyPatch() const
        {
            return procPolyPatch_;
        }

        //- Are the cyclic planes parallel
        virtual bool parallel() const
        {
            return procPolyPatch_.parallel();
        }

        //- Return face transformation tensor
        virtual const tensorField& forwardT() const
        {
            return procPolyPatch_.forwardT();
        }

        //- Return neighbour-cell transformation tensor.
        virtual const tensorField& reverseT() const
        {
            return procPolyPatch_.reverseT();
        }

        //- send parallel message
        virtual void sendPstream(const labelList&, PstreamBuffers& ) const;
        //- recieve parallel message
        virtual void receivePstream(labelList&, PstreamBuffers& ) const;
        //- send parallel message
        virtual void sendPstream(const scalarList&, PstreamBuffers& ) const;
        //- recieve parallel message
        virtual void receivePstream(scalarList&, PstreamBuffers& ) const;



        // Interface transfer functions

            //- Return the values of the given internal data adjacent to
            //  the interface as a field
            virtual tmp<labelField> interfaceInternalField
            (
                const labelUList& internalData
            ) const;

            //- Initialise neighbour field transfer
            virtual void initInternalFieldTransfer
            (
                const Pstream::commsTypes commsType,
                const labelUList& internalData
            ) const;

            //- Return neighbour field
            virtual tmp<labelField> internalFieldTransfer
            (
                const Pstream::commsTypes commsType,
                const labelUList& internalData
            ) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
