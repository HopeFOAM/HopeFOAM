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

#include "dg.H"
#include "HashTable.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace dg
{


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class Type>
convectionScheme<Type>::convectionScheme(const convectionScheme& cs)
:
    refCount(),
    mesh_(cs.mesh_)
{}


// * * * * * * * * * * * * * * * * * Selectors * * * * * * * * * * * * * * * //

template<class Type>
tmp<convectionScheme<Type> > convectionScheme<Type>::New
(
    const dgMesh& mesh,
    Istream& schemeData
)
{
    if (dg::debug)
    {
        Info<< "convectionScheme<Type>::New"
               "(const dgMesh&, Istream&) : "
               "constructing convectionScheme<Type>"
            << endl;
    }

    if (schemeData.eof())
    {
        FatalIOErrorIn
        (
            "convectionScheme<Type>::New"
            "(const dgMesh&, Istream&)",
            schemeData
        )   << "Convection scheme not specified" << endl << endl
            << "Valid convection schemes are :" << endl
            << IstreamConstructorTablePtr_->sortedToc()
            << exit(FatalIOError);
    }

    const word schemeName(schemeData);

    typename IstreamConstructorTable::iterator cstrIter =
        IstreamConstructorTablePtr_->find(schemeName);

    if (cstrIter == IstreamConstructorTablePtr_->end())
    {
        FatalIOErrorIn
        (
            "convectionScheme<Type>::New"
            "(const dgMesh&, Istream&)",
            schemeData
        )   << "Unknown convection scheme " << schemeName << nl << nl
            << "Valid convection schemes are :" << endl
            << IstreamConstructorTablePtr_->sortedToc()
            << exit(FatalIOError);
    }

    return cstrIter()(mesh, schemeData);
}

// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

template<class Type>
convectionScheme<Type>::~convectionScheme()
{}


// * * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * //

template<class Type>
void convectionScheme<Type>::operator=(const convectionScheme<Type>& cs)
{
    if (this == &cs)
    {
        FatalErrorIn
        (
            "convectionScheme<Type>::operator=(const convectionScheme<Type>&)"
        )   << "attempted assignment to self"
            << abort(FatalError);
    }
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace dg

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //
