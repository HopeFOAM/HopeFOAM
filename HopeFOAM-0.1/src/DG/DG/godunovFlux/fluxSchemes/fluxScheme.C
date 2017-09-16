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

Description
    Abstract base class for flux calculation schemes.

\*---------------------------------------------------------------------------*/

#include "fluxScheme.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * Selectors * * * * * * * * * * * * * * * //

// Return weighting factors for scheme given by name in dictionary

Foam::tmp<Foam::dg::fluxSchemes > Foam::dg::fluxSchemes::New
(
    const dgMesh& mesh,
    Istream& schemeData
)
{
    if (schemeData.eof())
    {
        FatalIOErrorIn
        (
            "fluxCalcScheme<Type>::New(const dgMesh&, Istream&)",
            schemeData
        )   << "Discretisation scheme not specified"
            << endl << endl
            << "Valid schemes are :" << endl
            << IstreamConstructorTablePtr_->sortedToc()
            << exit(FatalIOError);
    }

    const word schemeName(schemeData);

    typename IstreamConstructorTable::iterator constructorIter =
        IstreamConstructorTablePtr_->find(schemeName);

    if (constructorIter == IstreamConstructorTablePtr_->end())
    {
        FatalIOErrorIn
        (
            "fluxScheme::New(const dgMesh&, Istream&)",
            schemeData
        )   << "Unknown discretisation scheme "
            << schemeName << nl << nl
            << "Valid schemes are :" << endl
            << IstreamConstructorTablePtr_->sortedToc()
            << exit(FatalIOError);
    }

    return constructorIter()(mesh, schemeData);
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //


Foam::dg::fluxSchemes::~fluxSchemes()
{}


// ************************************************************************* //
