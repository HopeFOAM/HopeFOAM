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
    Foam::dg::ddtScheme

Description
    Abstract base class for ddt schemes.

SourceFiles
    ddtScheme.C
    
Author
    Xu Liyang (xucloud77@gmail.com)
\*---------------------------------------------------------------------------*/

#ifndef ddtScheme_H
#define ddtScheme_H

#include "tmp.H"
#include "dgFields.H"
#include "typeInfo.H"
#include "runTimeSelectionTables.H"
#include "dgLduMatrix.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

template<class Type>
class dgMatrix;

class dgMesh;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace dg
{

/*---------------------------------------------------------------------------*\
                           Class ddtScheme Declaration
\*---------------------------------------------------------------------------*/

template<class Type>
class ddtScheme
:
    public refCount
{
    

protected:

    // Protected data

        const dgMesh& mesh_;


    // Private Member Functions

        //- Disallow copy construct
        ddtScheme(const ddtScheme&);

        //- Disallow default bitwise assignment
        void operator=(const ddtScheme&);


public:

    //- Runtime type information
    virtual const word& type() const = 0;


    // Declare run-time constructor selection tables

        declareRunTimeSelectionTable
        (
            tmp,
            ddtScheme,
            Istream,
            (const dgMesh& mesh, Istream& schemeData),
            (mesh, schemeData)
        );


    // Constructors

        //- Construct from mesh
        ddtScheme(const dgMesh& mesh)
        :
            mesh_(mesh)
        {}

        //- Construct from mesh and Istream
        ddtScheme(const dgMesh& mesh, Istream&)
        :
            mesh_(mesh)
        {}


    // Selectors

        //- Return a pointer to a new ddtScheme created on freestore
        static tmp<ddtScheme<Type> > New
        (
            const dgMesh& mesh,
            Istream& schemeData
        );


    //- Destructor
    virtual ~ddtScheme();


    // Member Functions

        //- Return mesh reference
        const dgMesh& mesh() const
        {
            return mesh_;
        }

        virtual tmp<GeometricDofField<Type, dgPatchField, dgGeoMesh> > dgcDdt
        (
            const GeometricDofField<Type, dgPatchField, dgGeoMesh>&
        ) = 0;

        virtual tmp<dgMatrix<Type> > dgmDdt
        (
            scalar,
            const GeometricDofField<Type, dgPatchField, dgGeoMesh>&
        ) = 0;

        typedef GeometricDofField<Type, dgPatchField, dgGeoMesh> PSI;

        virtual void dgcDdtCalculateCell(
            dgTree<physicalCellElement>::leafIterator, 
            shared_ptr<dgLduMatrix<Type>>, 
            const PSI*) = 0;

        virtual void dgmDdtCalculateCell(
            dgTree<physicalCellElement>::leafIterator, 
            shared_ptr<dgLduMatrix<Type>>, 
            const PSI*) = 0;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace dg

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// Add the patch constructor functions to the hash tables

#define makeDgDdtTypeScheme(SS, Type)                                          \
                                                                               \
defineNamedTemplateTypeNameAndDebug(Foam::dg::SS<Foam::Type>, 0);                              \
                                                                               \
    namespace Foam                                                             \
    {                                                                          \
        namespace dg                                                           \
        {                                                                      \
            ddtScheme<Type>::addIstreamConstructorToTable<SS<Type> >           \
                add##SS##Type##IstreamConstructorToTable_;                     \
        }                                                                      \
    }


#define makeDgDdtScheme(SS)                                                    \
                                                                               \
makeDgDdtTypeScheme(SS, scalar)                                                \
makeDgDdtTypeScheme(SS, vector)                                                \
makeDgDdtTypeScheme(SS, sphericalTensor)                                       \
makeDgDdtTypeScheme(SS, symmTensor)                                            \
makeDgDdtTypeScheme(SS, tensor)


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
#   include "ddtScheme.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
