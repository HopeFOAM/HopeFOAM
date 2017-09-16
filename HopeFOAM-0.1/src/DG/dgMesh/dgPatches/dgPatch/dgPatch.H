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
    Foam::dgPatch

Description
    A DG patch using a polyPatch and a dgBoundaryMesh

SourceFiles
    dgPatch.C
    dgPatchNew.C

\*---------------------------------------------------------------------------*/

#ifndef dgPatch_H
#define dgPatch_H

#include "polyPatch.H"
#include "labelList.H"
#include "SubList.H"
#include "typeInfo.H"
#include "tmp.H"
#include "primitiveFields.H"
#include "SubField.H"

#include "autoPtr.H"
#include "runTimeSelectionTables.H"
#include "dgPolyFace.H"
#include "dgPolyCell.H"
#include "dgTreeUnit.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

using std::shared_ptr;
using std::make_shared;

namespace Foam
{

class dgBoundaryMesh;
class dgMesh;
//class surfaceInterpolation;

/*---------------------------------------------------------------------------*\
                           Class dgPatch Declaration
\*---------------------------------------------------------------------------*/

class dgPatch
{
    // Private data

    //- Reference to the underlying polyPatch
    const polyPatch& polyPatch_;

    //- Reference to boundary mesh
    const dgBoundaryMesh& boundaryMesh_;

    //only valid in 2D, each face
    List<List<point>> facesPoints_;

    //The initial face index in the baseLst of the faceTree
    labelList dgFaceIndex_;

    //- the number of face
    label size_;

    //- the number of dofs
    label totalDofNum_;


    //only in 2D
    //labelList edgeLabel_;

    //- Disallow construct as copy
    dgPatch(const dgPatch&);

    //- Disallow assignment
    void operator=(const dgPatch&);


protected:

    // Protected Member Functions

    //- Make patch weighting factors
    virtual void makeWeights(scalarField&) const;

    //- Initialise the patches for moving points
    virtual void initMovePoints();

    //- Correct patches after moving points
    virtual void movePoints();


public:

    typedef dgBoundaryMesh BoundaryMesh;

    friend class dgBoundaryMesh;
    //friend class surfaceInterpolation;

    //- Runtime type information
    TypeName(polyPatch::typeName_());


    // Declare run-time constructor selection tables

    declareRunTimeSelectionTable
    (
        autoPtr,
        dgPatch,
        polyPatch,
        (const polyPatch& patch, const dgBoundaryMesh& bm),
        (patch, bm)
    );


    // Constructors

    //- Construct from polyPatch and dgBoundaryMesh
    dgPatch(const polyPatch&, const dgBoundaryMesh&);


    // Selectors

        //- Return a pointer to a new patch created on freestore from polyPatch
    static autoPtr<dgPatch> New
    (
        const polyPatch&,
        const dgBoundaryMesh&
    );


    //- Destructor
    virtual ~dgPatch();


    // Member Functions

        //- set dgFaceList according to the patch Type
    virtual void setDgFaces() {}

    // Access

    //- Return the polyPatch
    const polyPatch& patch() const
    {
        return polyPatch_;
    }

	inline const labelList& dgFaceIndex() const
	{
		return dgFaceIndex_;
	}


    //- Return name
    const word& name() const
    {
        return polyPatch_.name();
    }

    //- Return start label of this patch in the polyMesh face list
    label start() const
    {
        return polyPatch_.start();
    }

    //- Return size
    virtual label size() const
    {
        return size_;
    }

    label totalDofNum() const{
        return totalDofNum_;
    }

    //- Return true if this patch is coupled
    virtual bool coupled() const
    {
        return polyPatch_.coupled();
    }
		

    //- Return true if this patch field is distributed on processors
    virtual bool processored() const
    {
        return false;
    }

	virtual bool curvedPatch() const
	{
		return false;
	}


    //- Return the index of this patch in the dgBoundaryMesh
    label index() const
    {
        return polyPatch_.index();
    }

    //- Return boundaryMesh reference
    const dgBoundaryMesh& boundaryMesh() const
    {
        return boundaryMesh_;
    }

    //- Slice list to patch
    template<class T>
    const typename List<T>::subList patchSlice(const List<T>& l) const
    {
        return typename List<T>::subList(l, size(), start());
    }

    //- Return faceCells
    virtual const labelUList& faceCells() const;

    virtual const List<shared_ptr<dgTreeUnit<dgPolyCell>>>& dgFaceCells() const;

    //- send parallel message
    virtual void sendPstream(const labelList& l, PstreamBuffers& pBufs) const
    {
    }

    //- receive parallel message
    virtual void receivePstream(labelList& l, PstreamBuffers& pBufs) const
    {
        l.clear();//return empty list
    }
	
    //- send parallel message
    virtual void sendPstream(const scalarList& l, PstreamBuffers& pBufs) const
    {
    }
            
    //- receive parallel message
    virtual void receivePstream(scalarList& l, PstreamBuffers& pBufs) const
    {
        l.clear();//return empty list
    }

	//add by yeshuai   for arc boundary point process
    virtual vector function(scalar u, scalar v) const{
        return vector(0.0, 0.0, 0.0);
    }

	virtual const point position(point pointI)const
    {
        return point(0.0, 0.0, 0.0);
    }

	virtual const List<point> positions(List<point> points, List<bool> isEnd)const
    {
        return List<point>(0);
    }

	const List<List<point>>& facesPoints() const
    {
        return facesPoints_;
    }


    //- Member functions to update information

        // update coupled neighbor cell data
        virtual void updateNeighborData(){}

        //- update the number of patch coresponding dofs
        void updateTotalDofNum();


    // Access functions for demand driven data

        //- Return patch weighting factors
    const scalarField& weights() const;

            //- Return the face - cell distance coeffient
            //  except for coupled patches for which the cell-centre
            //  to coupled-cell-centre distance coeffient is returned
    const scalarField& deltaCoeffs() const;

        // Evaluation functions
            //- Return given internal field next to patch as patch field
            template<class Type>
            tmp<Field<Type> > patchInternalField(const UList<Type>&) const;

            //- Return given internal field next to patch as patch field
            template<class Type>
            void patchInternalField(const UList<Type>&, Field<Type>&) const;
            
            //- Return the corresponding patchField of the named field
            template<class GeometricField, class Type>
            const typename GeometricField::Patch& patchField
            (
                const GeometricField&
            ) const;

            //- Lookup and return the patchField of the named field from the
            //  local objectRegistry.
            //  N.B.  The dummy pointer arguments are used if this function is
            //  instantiated within a templated function to avoid a bug in gcc.
            //  See inletOutletDgPatchField.C and outletInletDgPatchField.C
           // template<class GeometricField, class Type>

    template<class GeometricDofField, class Type>//modify by RXG
    const typename GeometricDofField::Patch& lookupPatchField
    //const typename GeometricDofField::PatchFieldType& lookupPatchField
    (
        const word& name,
        const GeometricDofField* = NULL,
        const Type* = NULL
    ) const;

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
#   include "dgPatchTemplates.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //