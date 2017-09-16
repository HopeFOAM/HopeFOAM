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
    Foam::dg::noneFlux

Description
    zero flux scheme class

SourceFiles
    noneFlux.C
    
Author
    Xu Liyang (xucloud77@gmail.com)

\*---------------------------------------------------------------------------*/

#ifndef noneFlux_H
#define noneFlux_H

#include "fluxCalcScheme.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

namespace dg
{
/*---------------------------------------------------------------------------*\
                           Class noneFlux Declaration
\*---------------------------------------------------------------------------*/

template<class Type>
class noneFlux
:
    public fluxCalcScheme<Type>
{
    // Private Member Functions

        //- Disallow default bitwise assignment
        void operator=(const noneFlux&);


public:

    //- Runtime type information
    TypeName("none");


    // Constructors

        //- Construct from mesh
        noneFlux(const dgMesh& mesh)
        :
            fluxCalcScheme<Type>(mesh)
        {}

        //- Construct from Istream
        noneFlux(const dgMesh& mesh, Istream&)
        :
            fluxCalcScheme<Type>(mesh)
        {}

        //- Construct from faceFlux and Istream
        noneFlux
        (
            const dgMesh& mesh,
            const dgVectorField&,
            Istream&
        )
        :
            fluxCalcScheme<Type>(mesh)
        {}


    // Member Functions

        tmp<Field<Type>>
        fluxCalculateWeak
        (
            const dgGaussField<vector>&,
            const dgGaussField<Type>&
        )const;

        tmp<Field<Type>>
        fluxCalculateWeak
        (
            const GeometricDofField<vector, dgPatchField, dgGeoMesh>& dofU,
            const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf
        )const;
	
        tmp<Field<Type>>
        fluxCalculateWeak
        (
             const dgGaussField<Type>&
        )const;
};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace dg

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

#ifdef NoRepository
#   include "noneFlux.C"
#endif
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
