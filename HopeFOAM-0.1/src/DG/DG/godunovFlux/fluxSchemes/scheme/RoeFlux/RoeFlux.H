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
    Foam::dg::RoeFlux

Description
    Roe flux scheme class

SourceFiles
    RoeFlux.C
    RoeFluxs.C
    
Author
    Ye Shuai (shuaiye09@163.com)

\*---------------------------------------------------------------------------*/

#ifndef RoeFlux_H
#define RoeFlux_H

#include "fluxScheme.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

namespace dg
{
/*---------------------------------------------------------------------------*\
                           Class averageFlux Declaration
\*---------------------------------------------------------------------------*/


class RoeFlux
:
    public fluxSchemes
{
    // Private Member Functions

        //- Disallow default bitwise assignment
        void operator=(const RoeFlux&);


public:

    //- Runtime type information
    TypeName("Roe");


    // Constructors

        //- Construct from mesh
        RoeFlux(const dgMesh& mesh)
        :
            fluxSchemes(mesh)
        {}

        //- Construct from Istream
       RoeFlux(const dgMesh& mesh, Istream&)
        :
           fluxSchemes(mesh)
        {}

        //- Construct from faceFlux and Istream
        RoeFlux
        (
            const dgMesh& mesh,
            const dgVectorField&,
            Istream&
        )
        :
           fluxSchemes(mesh)
        {}


    // Member Functions
	  void 
        evaluateFlux
        (
          const dgGaussVectorField&  ,
           const dgGaussScalarField& , 
           scalar, 
           const dgGaussScalarField& ,
           scalarField& ,
           const dgGaussVectorField& ,
           vectorField& ,
           const dgGaussScalarField& ,
           scalarField& 
        )const;

};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace dg

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
