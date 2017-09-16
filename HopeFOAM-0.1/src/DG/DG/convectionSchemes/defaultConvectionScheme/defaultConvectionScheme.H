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
    Foam::dg::defaultConvectionScheme

Description


SourceFiles
    defaultConvectionScheme.C
    
Author
    Xu Liyang (xucloud77@gmail.com)
\*---------------------------------------------------------------------------*/

#ifndef defaultConvectionScheme_H
#define defaultConvectionScheme_H

#include "convectionScheme.H"


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace dg
{

//- Temporary debug switch to provide warning about backward-compatibility
//  issue with setting div schemes for steady-state
//  extern int warnUnboundedGauss;

/*---------------------------------------------------------------------------*\
                       Class gaussConvectionScheme Declaration
\*---------------------------------------------------------------------------*/

template<class Type>
class defaultConvectionScheme
:
    public dg::convectionScheme<Type>
{
    // Private data

        tmp<fluxCalcScheme<Type> > tfluxScheme_;


    // Private Member Functions

        //- Disallow default bitwise copy construct
        defaultConvectionScheme(const defaultConvectionScheme&);

        //- Disallow default bitwise assignment
        void operator=(const defaultConvectionScheme&);

        //- temp data for calculation
        Field<Type> tempX, tempY, tempZ, gtempX;

public:

    //- Runtime type information
    TypeName("default");


    // Constructors

        //- Construct from flux and Istream
        defaultConvectionScheme
        (
            const dgMesh& mesh,
            Istream& is
        )
        :
            convectionScheme<Type>(mesh),
            tempX(mesh.maxGaussPerCell()),
            tempY(mesh.maxGaussPerCell()),
            tempZ(mesh.maxGaussPerCell()),
            gtempX(mesh.maxGaussPerCell()),
            tfluxScheme_
            (
                fluxCalcScheme<Type>::New(mesh, is)
            )
        {
        }

        const fluxCalcScheme<Type>& getFluxCalcScheme()const{
            return tfluxScheme_();
        }

    // Equation CalculateCell Functions

        virtual void dgcDivCalculateCell(
            dgTree<physicalCellElement>::leafIterator, 
            shared_ptr<dgLduMatrix<Type>>, const dgGaussVectorField&, 
            const GeometricDofField<Type, dgPatchField, dgGeoMesh> *,
            const Field<Type>&);

        virtual void dgcDivCalculateCell(
            dgTree<physicalCellElement>::leafIterator, 
            shared_ptr<dgLduMatrix<Type>>, const dgGaussVectorField&, 
            const dgGaussField<Type>&,
            const Field<Type>&);

        virtual void dgcDivCalculateCell(
            dgTree<physicalCellElement>::leafIterator, 
            shared_ptr<dgLduMatrix<Type>>, 
            const GeometricDofField<vector, dgPatchField, dgGeoMesh>&, 
            const GeometricDofField<Type, dgPatchField, dgGeoMesh>&,
            const Field<Type>&);

        virtual void dgmDivCalculateCell(
            dgTree<physicalCellElement>::leafIterator, 
            shared_ptr<dgLduMatrix<Type>>, const dgGaussVectorField&, 
            const GeometricDofField<Type, dgPatchField, dgGeoMesh> *);
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace dg

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
#   include "defaultConvectionScheme.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
