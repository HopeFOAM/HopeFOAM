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
    Foam::dg::divScheme

Description
    Abstract base class for div schemes.

SourceFiles
    divScheme.C

Author
    Xu Liyang (xucloud77@gmail.com)
\*---------------------------------------------------------------------------*/

#ifndef divScheme_H
#define divScheme_H

#include "tmp.H"
#include "dgFields.H"
#include "dgGaussField.H"
#include "dgLduMatrix.H"
#include "typeInfo.H"
#include "runTimeSelectionTables.H"
#include "fluxCalcScheme.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

class dgMesh;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace dg
{

/*---------------------------------------------------------------------------*\
                           Class divScheme Declaration
\*---------------------------------------------------------------------------*/

template<class Type>
class divScheme
:
    public refCount
{

protected:

    // Protected data

        const dgMesh& mesh_;
        //tmp<surfaceInterpolationScheme<Type> > tinterpScheme_;


    // Private Member Functions

        //- Disallow copy construct
        divScheme(const divScheme&);

        //- Disallow default bitwise assignment
        void operator=(const divScheme&);


public:

    //- Runtime type information
    virtual const word& type() const = 0;


    // Declare run-time constructor selection tables

        declareRunTimeSelectionTable
        (
            tmp,
            divScheme,
            Istream,
            (const dgMesh& mesh, Istream& schemeData),
            (mesh, schemeData)
        );


    // Constructors

        //- Construct from mesh
        divScheme(const dgMesh& mesh)
        :
            mesh_(mesh)
        {}
        //- Construct from mesh and Istream
        divScheme(const dgMesh& mesh, Istream& is)
        :
            mesh_(mesh)
        {}

    // Selectors

        //- Return a pointer to a new divScheme created on freestore
        static tmp<divScheme<Type> > New
        (
            const dgMesh& mesh,
            Istream& schemeData
        );


    //- Destructor
    virtual ~divScheme();


    // Member Functions
        virtual const fluxCalcScheme<typename outerProduct<vector, Type>::type>& getFluxCalcScheme()const = 0;

        //- Return mesh reference
        const dgMesh& mesh() const
        {
            return mesh_;
        }

        virtual void dgcDivCalculateCell(
            dgTree<physicalCellElement>::leafIterator iter, 
            shared_ptr<dgLduMatrix<Type>> lduMatrixPtr, 
            const GeometricDofField<typename outerProduct<vector, Type>::type, dgPatchField, dgGeoMesh> * vf) = 0;

        virtual void dgcDivCalculateCell(
            dgTree<physicalCellElement>::leafIterator iter, 
            shared_ptr<dgLduMatrix<Type>> lduMatrixPtr, 
            const dgGaussField<typename outerProduct<vector, Type>::type>& dgf,
            const Field<typename outerProduct<vector, Type>::type>&) = 0;

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace dg

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// Add the patch constructor functions to the hash tables

#define makeDgDivTypeScheme(SS, Type)                                          \
                                                                               \
defineNamedTemplateTypeNameAndDebug(Foam::dg::SS<Foam::Type>, 0);              \
                                                                               \
    namespace Foam                                                             \
    {                                                                          \
        namespace dg                                                           \
        {                                                                      \
           divScheme<Type>::addIstreamConstructorToTable<SS<Type> >            \
                add##SS##Type##IstreamConstructorToTable_;                     \
        }                                                                      \
    }


#define makeDgDivScheme(SS)                                                    \
                                                                               \
makeDgDivTypeScheme(SS, vector)                                                \
makeDgDivTypeScheme(SS, scalar)


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
#   include "divScheme.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
