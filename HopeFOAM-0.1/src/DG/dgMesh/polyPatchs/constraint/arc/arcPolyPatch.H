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
    Foam::arcPolyPatch

Description
    arc front and back plane patch.  Used for 2-D geometries.

SourceFiles
    arcPolyPatch.C

\*---------------------------------------------------------------------------*/

#ifndef arcPolyPatch_H
#define arcPolyPatch_H

#include "polyPatch.H"
#include "dynamicCodeContext.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                      Class arcPolyPatch Declaration
\*---------------------------------------------------------------------------*/
class dynamicCodeContext;

class arcPolyPatch
	:
	public polyPatch
{
	//Private data
private:
	//  point center_;
	//  scalar radius_;
	//code
	dictionary dict_;
	
	Pair<scalar> u_Range, v_Range;
	
	dynamicCodeContext dyCode;
	
	word name;
public:

	//- Runtime type information
	TypeName("arc");


	// Constructors

	//- Construct from components
	arcPolyPatch
	(
	    const word& name,
	    const label size,
	    const label start,
	    const label index,
	    const polyBoundaryMesh& bm,
	    const word& patchType
	);

	//- Construct from dictionary
	arcPolyPatch
	(
	    const word& name,
	    const dictionary& dict,
	    const label index,
	    const polyBoundaryMesh& bm,
	    const word& patchType
	);

	//- Construct as copy, resetting the boundary mesh
	arcPolyPatch(const arcPolyPatch&, const polyBoundaryMesh&);

	//- Construct given the original patch and resetting the
	//  face list and boundary mesh information
	arcPolyPatch
	(
	    const arcPolyPatch& pp,
	    const polyBoundaryMesh& bm,
	    const label index,
	    const label newSize,
	    const label newStart
	);

	//- Construct given the original patch and a map
	arcPolyPatch
	(
	    const arcPolyPatch& pp,
	    const polyBoundaryMesh& bm,
	    const label index,
	    const labelUList& mapAddressing,
	    const label newStart
	);

	//- Construct and return a clone, resetting the boundary mesh
	virtual autoPtr<polyPatch> clone(const polyBoundaryMesh& bm) const
	{
		return autoPtr<polyPatch>(new arcPolyPatch(*this, bm));
	}

	//- Construct and return a clone, resetting the face list
	//  and boundary mesh
	virtual autoPtr<polyPatch> clone
	(
	    const polyBoundaryMesh& bm,
	    const label index,
	    const label newSize,
	    const label newStart
	) const
	{
		return autoPtr<polyPatch>
		       (
		           new arcPolyPatch(*this, bm, index, newSize, newStart)
		       );
	}

	//- Construct and return a clone, resetting the face list
	//  and boundary mesh
	virtual autoPtr<polyPatch> clone
	(
	    const polyBoundaryMesh& bm,
	    const label index,
	    const labelUList& mapAddressing,
	    const label newStart
	) const
	{
		return autoPtr<polyPatch>
		       (
		           new arcPolyPatch(*this, bm, index, mapAddressing, newStart)
		       );
	}
	inline const dictionary& dict() const
	{
		return dict_;
	}

	virtual void write(Ostream&) const;

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
