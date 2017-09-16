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
    Foam::coupledDgPatch

Description
    An abstract base class for patches that couple regions of the
    computational domain e.g. cyclic and processor-processor links.

SourceFiles
    coupledDgPatch.C

\*---------------------------------------------------------------------------*/

#ifndef coupledDgPatch_H
#define coupledDgPatch_H

#include "dgPatch.H"
#include "lduInterface.H"
#include "coupledPolyPatch.H"
#include "stdElement.H"

using std::shared_ptr;
using std::make_shared;
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                         Class coupledDgPatch Declaration
\*---------------------------------------------------------------------------*/

class coupledDgPatch
:
    public lduInterface,
    public dgPatch
{
    // Private data

        const coupledPolyPatch& coupledPolyPatch_;


protected:

    // Protected Member Functions

        //- Make patch weighting factors
        virtual void makeWeights(scalarField&) const = 0;


public:

    friend class surfaceInterpolation;


    //- definition of class neighborDataUnit
    //- A class to hold neighbor cell data
    class neighborDataUnit
    {
        friend class coupledDgPatch;

        //- pointer to base function for coupled cell
        stdElement * baseFunction_;
        //- the face rotate of coupled face to the owner face
        label faceRotate_;
        //- face dof mapping of coupled face
        labelList dofMapping_;
        //- global dof shift of coupled cell
        label globalShift_;
        //- gaussDn for laplacian
        denseMatrix<scalar> gaussNeighborFaceDnRotate_;
    };
    List<neighborDataUnit> neighborData_;

    //- Runtime type information
    TypeName(coupledPolyPatch::typeName_());


    // Constructors

        //- Construct from polyPatch
        coupledDgPatch(const polyPatch& patch, const dgBoundaryMesh& bm)
        :
            dgPatch(patch, bm),
            coupledPolyPatch_(refCast<const coupledPolyPatch>(patch)),
            neighborData_()
        {}


    //- Destructor
    virtual ~coupledDgPatch();


    // Member Functions

        //- set dgFaceList according to the patch Type
        virtual void setDgFaces() {}

        // Access

            //- Return true because this patch is coupled
            virtual bool coupled() const
            {
                return coupledPolyPatch_.coupled();
            }

            //- Are the cyclic planes parallel.
            virtual bool parallel() const = 0;

            //- Return face transformation tensor.

            virtual const tensorField& forwardT() const = 0;

            //- Return neighbour-cell transformation tensor.
            virtual const tensorField& reverseT() const = 0;

            //- Return faceCell addressing
            virtual const labelUList& faceCells() const
            {
                return dgPatch::faceCells();
            }

            virtual const List<shared_ptr<dgTreeUnit<dgPolyCell>>>& dgFaceCells() const
            {
                return dgPatch::dgFaceCells();
            }



        // Interface transfer functions

            virtual tmp<labelField> interfaceInternalField
            (
                const labelUList& internalData
            ) const = 0;

            //- Initialise neighbour field transfer
            virtual void initInternalFieldTransfer
            (
                const Pstream::commsTypes commsType,
                labelUList& iF
            ) const
            {}

            //- Return neighbour field
            virtual tmp<labelField> internalFieldTransfer
            (
                const Pstream::commsTypes commsType,
                const labelUList& iF
            ) const = 0;


};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
