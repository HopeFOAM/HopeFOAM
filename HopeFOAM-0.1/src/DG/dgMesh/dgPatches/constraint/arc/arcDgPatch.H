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
    Foam::emptyDgPatch

Description
    A patch which will not exist in the dgMesh. Typical example is a front and
    back plane of a 2-D geometry

SourceFiles
    emptyDgPatch.C

\*---------------------------------------------------------------------------*/

#ifndef arcDgPatch_H
#define arcDgPatch_H

#include "Pair.H"
#include "dgPatch.H"
#include "arcPolyPatch.H"
#include "codedBase.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
class dynamicCode;
class dynamicCodeContext;
class IOdictionary;
/*---------------------------------------------------------------------------*\
                      Class emptyDgPatch Declaration
\*---------------------------------------------------------------------------*/

class arcDgPatch
	:
	public dgPatch,
	public codedBase
{
	// Private data
	const arcPolyPatch& arcPolyPatch_;
	

	const dictionary  dict_;

	mutable autoPtr<dgPatch> redirectPatchPtr_;

	const word name_;

	//- face-cell addressing
	const labelList::subList faceCells_;

	List<vector> faceNormal_;

	scalar u0, v0;
	bool initPos;

	

	virtual void prepare(dynamicCode&, const dynamicCodeContext&) const;

	virtual const dictionary& codeDict() const;

	const IOdictionary& dict() const;

	virtual dlLibraryTable& libs() const;

	virtual string description() const;

	virtual void clearRedirect() const;

public:

	//- Runtime type information
	TypeName(arcPolyPatch::typeName_());

	static const word codeTemplateC;

	static const word codeTemplateH;
	// Constructors

	//- Construct from polyPatch
	arcDgPatch(const polyPatch& patch, const dgBoundaryMesh& bm);


	// Member Functions

	// Access

	virtual label size() const
	{
		return arcPolyPatch_.size();
	}

	//- Return faceCells
	virtual const labelUList& faceCells() const;



	const arcPolyPatch& arcPatch()
	{
		return arcPolyPatch_;
	}

	inline const List<vector>& faceNormal() const
	{
		return faceNormal_;
	}


	vector function(scalar u, scalar v) const;

        List<vector> partialDerivative(Pair<scalar> pairI) const;

	const bool getShortestPoint(point srPoint, point& resPoint);

	const point position(point pointI)const;

	const scalar distance(point pointI, point pointJ)const;

	const void initialPosition(point pointI);

	const labelList sortPoint(List<point> points)const;

	const List<point> positions(List<point> points, List<bool> isEnd)const;

	void updateFaceNormal(List<point> oriPoints, List<point> moveRes);

	virtual bool curvedPatch() const
		{
			return true;
		}

	const dgPatch& redirectPatch() const;

};



// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
