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
    Foam::dg::convectionScheme

Description
    Abstract base class for convection schemes.

SourceFiles
    convectionScheme.C
    convectionSchemes.C
    
Author
    Xu Liyang (xucloud77@gmail.com)
\*---------------------------------------------------------------------------*/

#ifndef convectionScheme_H
#define convectionScheme_H

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

template<class Type>
class dgMatrix;

class dgMesh;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace dg
{

/*---------------------------------------------------------------------------*\
                           Class convectionScheme Declaration
\*---------------------------------------------------------------------------*/

template<class Type>
class convectionScheme
:
    public refCount
{
    // Private data

        const dgMesh& mesh_;

public:

    //- Runtime type information
    virtual const word& type() const = 0;


    // Declare run-time constructor selection tables

        declareRunTimeSelectionTable
        (
            tmp,
            convectionScheme,
            Istream,
            (
                const dgMesh& mesh,
                Istream& schemeData
            ),
            (mesh, schemeData)
        );


    // Constructors

        //- Copy construct
        convectionScheme(const convectionScheme&);

        //- Construct from mesh, flux and Istream
        convectionScheme
        (
            const dgMesh& mesh
        )
        :
            mesh_(mesh)
        {}


    // Selectors

        //- Return a pointer to a new convectionScheme created on freestore
        static tmp<convectionScheme<Type> > New
        (
            const dgMesh& mesh,
            Istream& schemeData
        );


    //- Destructor
    virtual ~convectionScheme();


    // Member Functions

        //- Return mesh reference
        const dgMesh& mesh() const
        {
            return mesh_;
        }

        virtual const fluxCalcScheme<Type>& getFluxCalcScheme()const = 0;

    // Equation CalculateCell Functions

        virtual void dgcDivCalculateCell(
            dgTree<physicalCellElement>::leafIterator, 
            shared_ptr<dgLduMatrix<Type>>, const dgGaussVectorField&, 
            const GeometricDofField<Type, dgPatchField, dgGeoMesh> *,
            const Field<Type>&) = 0;

        virtual void dgcDivCalculateCell(
            dgTree<physicalCellElement>::leafIterator, 
            shared_ptr<dgLduMatrix<Type>>, const dgGaussVectorField&, 
            const dgGaussField<Type>&,
            const Field<Type>&) = 0;

        virtual void dgcDivCalculateCell(
            dgTree<physicalCellElement>::leafIterator, 
            shared_ptr<dgLduMatrix<Type>>, 
            const GeometricDofField<vector, dgPatchField, dgGeoMesh>&, 
            const GeometricDofField<Type, dgPatchField, dgGeoMesh>&,
            const Field<Type>&) = 0;

        virtual void dgmDivCalculateCell(
            dgTree<physicalCellElement>::leafIterator, 
            shared_ptr<dgLduMatrix<Type>>, const dgGaussVectorField&, 
            const GeometricDofField<Type, dgPatchField, dgGeoMesh> *) = 0;

    // Member operators

        void operator=(const convectionScheme<Type>&);
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace dg

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// Add the patch constructor functions to the hash tables

#define makeDgConvectionTypeScheme(SS, Type)                                   \
                                                                               \
defineNamedTemplateTypeNameAndDebug(Foam::dg::SS<Foam::Type>, 0);              \
                                                                               \
    namespace Foam                                                             \
    {                                                                          \
        namespace dg                                                           \
        {                                                                      \
            convectionScheme<Type>::addIstreamConstructorToTable<SS<Type> >    \
                add##SS##Type##IstreamConstructorToTable_;                     \
        }                                                                      \
    }


#define makeDgConvectionScheme(SS)                                             \
                                                                               \
makeDgConvectionTypeScheme(SS, scalar)                                         \
makeDgConvectionTypeScheme(SS, vector)                                         \
makeDgConvectionTypeScheme(SS, sphericalTensor)                                \
makeDgConvectionTypeScheme(SS, symmTensor)                                     \
makeDgConvectionTypeScheme(SS, tensor)


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
#   include "convectionScheme.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
