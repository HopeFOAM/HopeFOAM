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
    Foam::dg::laplacianScheme

Description
    Abstract base class for laplacian schemes.

SourceFiles
    laplacianScheme.C
    
Author
    Xu Liyang (xucloud77@gmail.com)

\*---------------------------------------------------------------------------*/
#ifndef laplacianScheme_H
#define laplacianScheme_H

#include "tmp.H"
#include "dgFields.H"
#include "dgMatrix.H"
#include "dgGaussField.H"
#include "typeInfo.H"
#include "runTimeSelectionTables.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

template<class Type>
class dgLduMatrix;

template<class Type>
class dgMatrix;

class dgMesh;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace dg
{

/*---------------------------------------------------------------------------*\
                           Class laplacianScheme Declaration
\*---------------------------------------------------------------------------*/

template<class Type, class GType>
class laplacianScheme
:
    public refCount
{

protected:

    // Protected data

        const dgMesh& mesh_;
        //tmp<surfaceInterpolationScheme<GType> > tinterpGammaScheme_;
        //tmp<snGradScheme<Type> > tsnGradScheme_;


    // Private Member Functions

        //- Disallow copy construct
        laplacianScheme(const laplacianScheme&);

        //- Disallow default bitwise assignment
        void operator=(const laplacianScheme&);


public:

    //- Runtime type information
    virtual const word& type() const = 0;


    // Declare run-time constructor selection tables

        declareRunTimeSelectionTable
        (
            tmp,
            laplacianScheme,
            Istream,
            (const dgMesh& mesh, Istream& schemeData),
            (mesh, schemeData)
        );


    // Constructors

        //- Construct from mesh
        laplacianScheme(const dgMesh& mesh)
        :
            mesh_(mesh)
        {}

        //- Construct from mesh and Istream
        laplacianScheme(const dgMesh& mesh, Istream& is)
        :
            mesh_(mesh)
        {
        }

    // Selectors

        //- Return a pointer to a new laplacianScheme created on freestore
        static tmp<laplacianScheme<Type, GType> > New
        (
            const dgMesh& mesh,
            Istream& schemeData
        );


    //- Destructor
    virtual ~laplacianScheme();


    // Member Functions

        //- Return mesh reference
        const dgMesh& mesh() const
        {
            return mesh_;
        }
/*
        virtual tmp<dgMatrix<Type> > dgmLaplacian
        (
            const GeometricDofField<GType, dgPatchField, dgGeoMesh>&,
            const GeometricDofField<Type, dgPatchField, dgGeoMesh>&
        ) = 0;

        virtual tmp<dgMatrix<Type> > dgmLaplacian
        (
            const GeometricDofField<Type, dgPatchField, dgGeoMesh>&
        ) = 0;
  virtual tmp<GeometricDofField<Type, dgPatchField, dgGeoMesh> > dgcLaplacian
        (
            const dgGaussField<Type>&
        ) = 0;*/
        //virtual tmp<GeometricDofField<Type, dgPatchField, dgGeoMesh> > dgcLaplacian
        //(
        //    const GeometricDofField<Type, dgPatchField, dgGeoMesh>&
        //) = 0;

        //virtual tmp<GeometricDofField<Type, dgPatchField, dgGeoMesh> > dgcLaplacian
        //(
        //    const GeometricDofField<GType, dgPatchField, dgGeoMesh>&,
        //    const GeometricDofField<Type, dgPatchField, dgGeoMesh>&
        //);


        virtual void dgcLaplacianCalculateCell(
            dgTree<physicalCellElement>::leafIterator, 
            shared_ptr<dgLduMatrix<Type>>, 
            const GeometricDofField<Type, dgPatchField, dgGeoMesh>&)=0;

        virtual void dgmLaplacianCalculateCell(
            dgTree<physicalCellElement>::leafIterator, 
            shared_ptr<dgLduMatrix<Type>>,
            const GeometricDofField<Type, dgPatchField, dgGeoMesh> *)=0;

        virtual void dgmLaplacianCalculateCell(
            dgTree<physicalCellElement>::leafIterator, 
            shared_ptr<dgLduMatrix<Type>>,
            const GeometricDofField<Type, dgPatchField, dgGeoMesh> *,
            const GeometricDofField<scalar, dgPatchField, dgGeoMesh>&)=0;

        //virtual void applyBoundaryConditions(shared_ptr<dgMatrix<Type>> source, const GeometricDofField<Type, dgPatchField, dgGeoMesh> *psi)=0;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace dg

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// Add the patch constructor functions to the hash tables

#define makeDgLaplacianTypeScheme(SS, GType, Type)                             \
    typedef Foam::dg::SS<Foam::Type, Foam::GType> SS##Type##GType;             \
    defineNamedTemplateTypeNameAndDebug(SS##Type##GType, 0);                   \
                                                                               \
    namespace Foam                                                             \
    {                                                                          \
        namespace dg                                                           \
        {                                                                      \
            typedef SS<Type, GType> SS##Type##GType;                           \
                                                                               \
            laplacianScheme<Type, GType>::                                     \
                addIstreamConstructorToTable<SS<Type, GType> >                 \
                add##SS##Type##GType##IstreamConstructorToTable_;              \
        }                                                                      \
    }


#define makeDgLaplacianScheme(SS)                                              \
                                                                               \
makeDgLaplacianTypeScheme(SS, scalar, scalar)                                  \
makeDgLaplacianTypeScheme(SS, scalar, vector)                                  \
makeDgLaplacianTypeScheme(SS, scalar, sphericalTensor)                         \
makeDgLaplacianTypeScheme(SS, scalar, symmTensor)                              \
makeDgLaplacianTypeScheme(SS, scalar, tensor)                                  \

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
#   include "laplacianScheme.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
