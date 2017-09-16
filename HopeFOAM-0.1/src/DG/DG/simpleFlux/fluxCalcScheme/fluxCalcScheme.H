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
    Foam::dg::fluxCalcScheme

Description
    Abstract base class for simple flux calculation schemes.

SourceFiles
    fluxCalcScheme.C
    
Author
    Xu Liyang (xucloud77@gmail.com)

\*---------------------------------------------------------------------------*/

#ifndef fluxCalcScheme_H
#define fluxCalcScheme_H

#include "tmp.H"
#include "dgFields.H"
#include "dgGaussField.H"
#include "typeInfo.H"
#include "runTimeSelectionTables.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

class dgMesh;

namespace dg
{

/*---------------------------------------------------------------------------*\
                 Class fluxCalcScheme Declaration
\*---------------------------------------------------------------------------*/

template<class Type>
class fluxCalcScheme
:
    public refCount
{
    // Private data

        //- Hold reference to mesh
        const dgMesh& mesh_;


    // Private Member Functions

        //- Disallow copy construct
        fluxCalcScheme(const fluxCalcScheme&);

        //- Disallow default bitwise assignment
        void operator=(const fluxCalcScheme&);


public:

    //- Runtime type information
    virtual const word& type() const = 0;


    // Declare run-time constructor selection tables

        declareRunTimeSelectionTable
        (
            tmp,
            fluxCalcScheme,
            Istream,
            (
                const dgMesh& mesh,
                Istream& schemeData
            ),
            (mesh, schemeData)
        );


    // Constructors

        //- Construct from mesh
        fluxCalcScheme(const dgMesh& mesh)
        :
            mesh_(mesh)
        {}


    // Selectors

        //- Return new tmp interpolation scheme
        static tmp<fluxCalcScheme<Type> > New
        (
            const dgMesh& mesh,
            Istream& schemeData
        );


    //- Destructor
    virtual ~fluxCalcScheme();


    // Member Functions

        //- Return mesh reference
        const dgMesh& mesh() const
        {
            return mesh_;
        }

        /***************** gauss flux calc Weak***************************/

        //- Return the flux-calculate of the given cell field
        //  with the given owner and neighbour weigting factors
        virtual tmp<Field<Type>>
        fluxCalculateWeak
        (
            const dgGaussField<vector>&,
            const dgGaussField<Type>&
        )const =0;

        virtual tmp<Field<Type>>
        fluxCalculateWeak
        (
            const GeometricDofField<vector, dgPatchField, dgGeoMesh>& dofU,
            const GeometricDofField<Type, dgPatchField, dgGeoMesh>& vf
        )const =0;

		virtual tmp<Field<Type>>
        fluxCalculateWeak
        (
            const dgGaussField<Type>&
        )const =0;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace dg

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// Add the patch constructor functions to the hash tables

#define makeFluxCalcTypeScheme(SS, Type)                                       \
                                                                               \
defineNamedTemplateTypeNameAndDebug(Foam::dg::SS<Foam::Type>, 0);              \
                                                                               \
    namespace Foam                                                             \
    {                                                                          \
        namespace dg                                                           \
        {                                                                      \
            fluxCalcScheme<Type>::addIstreamConstructorToTable<SS<Type> >      \
                add##SS##Type##IstreamConstructorToTable_;                     \
        }                                                                      \
    }

#define makeFluxCalcScheme(SS)                                                 \
                                                                               \
makeFluxCalcTypeScheme(SS, scalar)                                             \
makeFluxCalcTypeScheme(SS, vector)                                             \
makeFluxCalcTypeScheme(SS, sphericalTensor)                                    \
makeFluxCalcTypeScheme(SS, symmTensor)                                         \
makeFluxCalcTypeScheme(SS, tensor)


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
#   include "fluxCalcScheme.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
