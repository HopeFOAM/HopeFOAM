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
    Foam::dg::defaultLaplacianScheme

Description
    Basic second-order laplacian using face-gradients and default' theorem.

SourceFiles
    defaultLaplacianScheme.C

Author
    Xu Liyang (xucloud77@gmail.com)

\*---------------------------------------------------------------------------*/

#ifndef defaultLaplacianScheme_H
#define defaultLaplacianScheme_H

#include "laplacianScheme.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace dg
{

/*---------------------------------------------------------------------------*\
                       Class defaultLaplacianScheme Declaration
\*---------------------------------------------------------------------------*/


template<class Type, class GType>
class defaultLaplacianScheme
:
    public dg::laplacianScheme<Type, GType>
{
    // Private Member Functions

        //- Disallow default bitwise copy construct
        defaultLaplacianScheme(const defaultLaplacianScheme&);

        //- Disallow default bitwise assignment
        void operator=(const defaultLaplacianScheme&);

        //- Internal Penalty
        scalar tau_;
        denseMatrix<scalar> tmpMatrix_;
        List<Type> tmpData;

public:

    //- Runtime type information
    TypeName("default");


    // Constructors

        //- Construct null
        defaultLaplacianScheme(const dgMesh& mesh)
        :
            laplacianScheme<Type, GType>(mesh)
        {}

        //- Construct from Istream
        defaultLaplacianScheme(const dgMesh& mesh, Istream& is)
        :
            laplacianScheme<Type, GType>(mesh, is),
            tau_(readScalar(is)), 
            tmpMatrix_(mesh.maxDofPerCell(), mesh.maxDofPerCell()),
            tmpData(mesh.maxDofPerCell())
        {
            if (tau_ < 1)
            {
                FatalIOErrorInFunction(is)
                    << "coefficient = " << tau_
                    << " should be >= 1"
                    << exit(FatalIOError);
            }
        }


    //- Destructor
    virtual ~defaultLaplacianScheme()
    {}


    // Member Functions
    /*
        tmp<dgMatrix<Type> > dgmLaplacian
        (
            const GeometricDofField<GType, dgPatchField, dgGeoMesh>&,
            const GeometricDofField<Type, dgPatchField, dgGeoMesh>&
        );

        tmp<dgMatrix<Type> > dgmLaplacian
        (
            const GeometricDofField<Type, dgPatchField, dgGeoMesh>&
        );*/

        void laplacianFunc
        (
            dgTree<physicalCellElement>::leafIterator, 
            shared_ptr<dgLduMatrix<Type>>,
            const GeometricDofField<Type, dgPatchField, dgGeoMesh> *,
            const GeometricDofField<scalar, dgPatchField, dgGeoMesh>&
        );

        void laplacianFunc
        (
            dgTree<physicalCellElement>::leafIterator, 
            shared_ptr<dgLduMatrix<Type>>,
            const GeometricDofField<Type, dgPatchField, dgGeoMesh> *
        );

       /*  tmp<GeometricDofField<Type, dgPatchField, dgGeoMesh> > dgcLaplacian
        (
            const GeometricDofField<Type, dgPatchField, dgGeoMesh>&
        );

       tmp<GeometricDofField<Type, dgPatchField, dgGeoMesh> > dgcLaplacian
        (
            const GeometricDofField<GType, dgPatchField, dgGeoMesh>&,
            const GeometricDofField<Type, dgPatchField, dgGeoMesh>&
        );
     
      tmp<GeometricDofField<Type, dgPatchField, dgGeoMesh> >dgcLaplacian
        (
           const dgGaussField<Type>& 
        );
//for tensor /sysmtensor and sphericalTensor , temp return itself,!!!!!!!!!!!!!!!!! unimplement

	   tmp<GeometricDofField<Type, dgPatchField, dgGeoMesh> > laplacianFuncDgc
        (
           const dgGaussField<Type>& 
        );
*/

        virtual void dgcLaplacianCalculateCell(
            dgTree<physicalCellElement>::leafIterator, 
            shared_ptr<dgLduMatrix<Type>>, 
            const GeometricDofField<Type, dgPatchField, dgGeoMesh>&);

        virtual void dgmLaplacianCalculateCell(
            dgTree<physicalCellElement>::leafIterator, 
            shared_ptr<dgLduMatrix<Type>>,
            const GeometricDofField<Type, dgPatchField, dgGeoMesh> *);

        virtual void dgmLaplacianCalculateCell(
            dgTree<physicalCellElement>::leafIterator, 
            shared_ptr<dgLduMatrix<Type>>,
            const GeometricDofField<Type, dgPatchField, dgGeoMesh> *,
            const GeometricDofField<scalar, dgPatchField, dgGeoMesh>&);

        //virtual void applyBoundaryConditions(shared_ptr<dgMatrix<Type>> source, const GeometricDofField<Type, dgPatchField, dgGeoMesh> *psi);

};

/*
template<>                                                                  
tmp<GeometricDofField<scalar, dgPatchField, dgGeoMesh> > defaultLaplacianScheme<scalar, scalar>:: dgcLaplacian 
        (                                                                                                 
           const dgGaussField<scalar>&   vf 
        ){
          return laplacianFuncDgc(vf); 
}                             
       template<>                                                                  
tmp<GeometricDofField<vector, dgPatchField, dgGeoMesh> > defaultLaplacianScheme<vector, scalar>:: dgcLaplacian
        (                                                                                                 
           const dgGaussField<vector>&  vf   
        ){
 return laplacianFuncDgc(vf); 
		}
*/


// Use macros to emulate partial-specialisation of the the Laplacian functions
// for scalar diffusivity gamma
#define defineDgmLaplacianScalarGamma(Type)                                 \
                                                                            \
template<>                                                                  \
void defaultLaplacianScheme<Type, scalar>::dgmLaplacianCalculateCell        \
(                                                                           \
    dgTree<physicalCellElement>::leafIterator,                              \
    shared_ptr<dgLduMatrix<Type>>,                                          \
    const GeometricDofField<Type, dgPatchField, dgGeoMesh> *,               \
    const GeometricDofField<scalar, dgPatchField, dgGeoMesh>&               \
);                                                                          \
template<>                                                                  \
void defaultLaplacianScheme<Type, scalar>::dgmLaplacianCalculateCell        \
(                                                                           \
    dgTree<physicalCellElement>::leafIterator,                              \
    shared_ptr<dgLduMatrix<Type>>,                                          \
    const GeometricDofField<Type, dgPatchField, dgGeoMesh> *                \
);                                                        


defineDgmLaplacianScalarGamma(scalar);
defineDgmLaplacianScalarGamma(vector);
defineDgmLaplacianScalarGamma(sphericalTensor);
defineDgmLaplacianScalarGamma(symmTensor);
defineDgmLaplacianScalarGamma(tensor);
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace dg

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
#   include "defaultLaplacianScheme.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
