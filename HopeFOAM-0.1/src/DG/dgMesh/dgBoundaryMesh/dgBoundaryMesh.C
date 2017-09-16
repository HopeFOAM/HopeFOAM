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

#include "dgBoundaryMesh.H"
#include "dgMesh.H"


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void Foam::dgBoundaryMesh::addPatches(const polyBoundaryMesh& basicBdry)
{
    setSize(basicBdry.size());

    // Set boundary patches
    dgPatchList& Patches = *this;

    forAll(Patches, patchI)
    {
        Patches.set(patchI, dgPatch::New(basicBdry[patchI], *this));
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::dgBoundaryMesh::dgBoundaryMesh
(
    const dgMesh& m
)
:
    dgPatchList(0),
    mesh_(m),
    patchEntries_()
{
 	const dgPatch& tmppatch = operator[](0);
	
	const polyPatch& tmparc = tmppatch.patch();
	const polyBoundaryMesh& tmpBoundaryMesh = tmparc.boundaryMesh();
			
	Istream& is = const_cast< polyBoundaryMesh&>(tmpBoundaryMesh).readStream(tmpBoundaryMesh.type());

    PtrList<entry> tmpPatchEntries(is);

    patchEntries_= tmpPatchEntries;
}


Foam::dgBoundaryMesh::dgBoundaryMesh
(
    const dgMesh& m,
    const polyBoundaryMesh& basicBdry
)
:
    dgPatchList(basicBdry.size()),
    mesh_(m),
    patchEntries_()
{
   

	const polyBoundaryMesh& tmpBoundaryMesh = basicBdry;

	Istream& is = const_cast< polyBoundaryMesh&>(tmpBoundaryMesh).readStream(tmpBoundaryMesh.type());
 	PtrList<entry> tmpPatchEntries(is);
	
    patchEntries_= tmpPatchEntries;
	addPatches(basicBdry);
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

Foam::label Foam::dgBoundaryMesh::findPatchID(const word& patchName) const
{
    const dgPatchList& patches = *this;

    forAll(patches, patchI)
    {
        if (patches[patchI].name() == patchName)
        {
            return patchI;
        }
    }

    // Not found, return -1
    return -1;
}


Foam::labelList Foam::dgBoundaryMesh::findIndices
(
    const keyType& key,
    const bool usePatchGroups
) const
{
    return mesh().boundaryMesh().findIndices(key, usePatchGroups);
}

void Foam::dgBoundaryMesh::setDgFaces()
{
    dgPatchList& patches = *this;

    forAll(patches, patchI){
        patches[patchI].setDgFaces();
    }
}

void Foam::dgBoundaryMesh::movePoints()
{
    forAll(*this, patchI)
    {
        operator[](patchI).initMovePoints();
    }

    forAll(*this, patchI)
    {
        operator[](patchI).movePoints();
    }
}


Foam::lduInterfacePtrsList Foam::dgBoundaryMesh::interfaces() const
{
    lduInterfacePtrsList interfaces(size());

    forAll(interfaces, patchI)
    {
        if (isA<lduInterface>(this->operator[](patchI)))
        {
            interfaces.set
            (
                patchI,
               &refCast<const lduInterface>(this->operator[](patchI))
            );
        }
    }

    return interfaces;
}


void Foam::dgBoundaryMesh::readUpdate(const polyBoundaryMesh& basicBdry)
{
    clear();
    addPatches(basicBdry);
}

void Foam::dgBoundaryMesh::updateTotalDofNum()
{
    forAll(*this, patchI)
    {
        operator[](patchI).updateTotalDofNum();
    }
}

void Foam::dgBoundaryMesh::updateNeighborData()
{
    forAll(*this, patchI)
    {
        operator[](patchI).updateNeighborData();
    }
}

// * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * * //

const Foam::dgPatch& Foam::dgBoundaryMesh::operator[]
(
    const word& patchName
) const
{
    const label patchI = findPatchID(patchName);

    if (patchI < 0)
    {
        FatalErrorIn
        (
            "dgBoundaryMesh::operator[](const word&) const"
        )   << "Patch named " << patchName << " not found." << nl
            << abort(FatalError);
    }

    return operator[](patchI);
}


Foam::dgPatch& Foam::dgBoundaryMesh::operator[]
(
    const word& patchName
)
{
    const label patchI = findPatchID(patchName);

    if (patchI < 0)
    {
        FatalErrorIn
        (
            "dgBoundaryMesh::operator[](const word&)"
        )   << "Patch named " << patchName << " not found." << nl
            << abort(FatalError);
    }

    return operator[](patchI);
}


// ************************************************************************* //
