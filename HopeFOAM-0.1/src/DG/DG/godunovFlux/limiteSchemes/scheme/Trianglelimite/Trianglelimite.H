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
    Foam::dg::TriangleLimite

Description
    Note this limite is only support triangle mesh only.

    Reference to  a Slope Limiting Procedure in Discontinuous Galerkin Finite
    Element Method for Gasdynamics Applications.

SourceFiles
    Trianglelimite.C
    
Author
    Ye Shuai (shuaiye09@163.com)

\*---------------------------------------------------------------------------*/

#ifndef Trianglelimite_H
#define Trianglelimite_H

#include "limiteScheme.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

namespace dg
{
/*---------------------------------------------------------------------------*\
                           Class averageFlux Declaration
\*---------------------------------------------------------------------------*/


class Trianglelimite
:
    public limiteSchemes
{
    // Private Member Functions

        //- Disallow default bitwise assignment
        void operator=(const Trianglelimite&);


public:

    //- Runtime type information
    TypeName("Triangle");

    // Constructors

        //- Construct from mesh
        Trianglelimite(const dgMesh& mesh)
        :
            limiteSchemes(mesh)
        {}

        //- Construct from Istream
       Trianglelimite(const dgMesh& mesh, Istream&)
        :
           limiteSchemes(mesh)
        {}

        //- Construct from faceFlux and Istream
        Trianglelimite
        (
            const dgMesh& mesh,
            const dgVectorField&,
            Istream&
        )
        :
           limiteSchemes(mesh)
        {}


    // Member Functions
    	virtual void limite
        ( 
	  	    dgScalarField& ,
	  	    dgVectorField& ,
	  	    dgScalarField& ,
	  	    const dgGaussScalarField&,
	  	    const dgGaussVectorField&,
	  	    const dgGaussScalarField&,
	  	    dgScalarField&
        )const;

	    virtual void limite
        ( 
	  	    dgScalarField& ,
	  	    dgVectorField& ,
	  	    dgScalarField& ,
	  	    const dgGaussScalarField&,
	  	    const dgGaussVectorField&,
	  	    const dgGaussScalarField&
        )const;

};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace dg

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

/*#ifdef NoRepository
#   include "RoeFlux.C"
#endif*/
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
