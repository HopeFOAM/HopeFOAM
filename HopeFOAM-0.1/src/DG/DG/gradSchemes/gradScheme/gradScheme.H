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
    Foam::dg::gradScheme

Description
    Abstract base class for gradient schemes.

SourceFiles
    gradScheme.C

Author
    Xu Liyang (xucloud77@gmail.com)
\*---------------------------------------------------------------------------*/

#ifndef gradScheme_H
#define gradScheme_H

#include "tmp.H"
#include "dgFields.H"
#include "dgGaussField.H"
#include "dgMatrix.H"
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
                           Class gradScheme Declaration
\*---------------------------------------------------------------------------*/

template<class Type>
class gradScheme
:
    public refCount
{
    // Private data

        const dgMesh& mesh_;


    // Private Member Functions

        //- Disallow copy construct
        gradScheme(const gradScheme&);

        //- Disallow default bitwise assignment
        void operator=(const gradScheme&);


public:

    //- Runtime type information
    virtual const word& type() const = 0;


    // Declare run-time constructor selection tables

        declareRunTimeSelectionTable
        (
            tmp,
            gradScheme,
            Istream,
            (const dgMesh& mesh, Istream& schemeData),
            (mesh, schemeData)
        );


    // Constructors

        //- Construct from mesh
        gradScheme(const dgMesh& mesh)
        :
            mesh_(mesh)
        {}


    // Selectors

        //- Return a pointer to a new gradScheme created on freestore
        static tmp<gradScheme<Type> > New
        (
            const dgMesh& mesh,
            Istream& schemeData
        );


    //- Destructor
    virtual ~gradScheme();


    // Member Functions
        virtual const fluxCalcScheme<typename innerProduct<vector, Type>::type>& getFluxCalcScheme()const = 0;

        //- Return mesh reference
        const dgMesh& mesh() const
        {
            return mesh_;
        }

        //- Calculate and return the grad of the given field.
        //  Used by grad either to recalculate the cached gradient when it is
        //  out of date with respect to the field or when it is not cached.

        virtual void dgcGradCalculateCell(
            dgTree<physicalCellElement>::leafIterator, 
            shared_ptr<dgLduMatrix<Type>>, 
            const GeometricDofField<typename innerProduct<vector, Type>::type, 
                dgPatchField, dgGeoMesh> *) = 0;

        virtual void dgcGradCalculateCell(
            dgTree<physicalCellElement>::leafIterator, 
            shared_ptr<dgLduMatrix<Type>>, 
            const dgGaussField<typename innerProduct<vector, Type>::type>&,
            const Field<typename innerProduct<vector, Type>::type>& flux) = 0;

        virtual void dgmGradCalculateCell(
            dgTree<physicalCellElement>::leafIterator, 
            shared_ptr<dgLduMatrix<Type>>, 
            const GeometricDofField<typename innerProduct<vector, Type>::type,
                dgPatchField, dgGeoMesh> *) = 0;
    

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace fv

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// Add the patch constructor functions to the hash tables

#define makeDgGradTypeScheme(SS, Type)                                         \
    defineNamedTemplateTypeNameAndDebug(Foam::dg::SS<Foam::Type>, 0);          \
                                                                               \
    namespace Foam                                                             \
    {                                                                          \
        namespace dg                                                           \
        {                                                                      \
            gradScheme<Type>::addIstreamConstructorToTable<SS<Type>>           \
                add##SS##Type##IstreamConstructorToTable_;                     \
        }                                                                      \
    }


#define makeDgGradScheme(SS)                                                   \
                                                                               \
makeDgGradTypeScheme(SS, vector)                                               \
makeDgGradTypeScheme(SS, tensor)

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
#   include "gradScheme.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
