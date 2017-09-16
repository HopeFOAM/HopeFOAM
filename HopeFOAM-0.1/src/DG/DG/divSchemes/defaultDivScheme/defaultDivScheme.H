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
    Foam::dg::defaultDivScheme

Description
    Basic second-order div using face-gradients and default' theorem.

SourceFiles
    defaultDivScheme.C

Author
    Xu Liyang (xucloud77@gmail.com)
\*---------------------------------------------------------------------------*/

#ifndef defaultDivScheme_H
#define defaultDivScheme_H

#include "divScheme.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace dg
{

/*---------------------------------------------------------------------------*\
                       Class defaultDivScheme Declaration
\*---------------------------------------------------------------------------*/

template<class Type>
class defaultDivScheme
:
    public dg::divScheme<Type>
{
    // Private Member Functions

        //- Disallow default bitwise copy construct
        defaultDivScheme(const defaultDivScheme&);

        //- Disallow default bitwise assignment
        void operator=(const defaultDivScheme&);

        tmp<fluxCalcScheme<typename outerProduct<vector, Type>::type> > tfluxScheme_;
        Field<typename outerProduct<vector, Type>::type> tempX;
        Field<Type> tempY;

public:

    //- Runtime type information
    TypeName("default");


    // Constructors

        //- Construct null
        defaultDivScheme(const dgMesh& mesh)
        :
            divScheme<Type>(mesh)
        {}

        //- Construct from Istream
        defaultDivScheme(const dgMesh& mesh, Istream& is)
        :
            divScheme<Type>(mesh, is),
            tfluxScheme_
            (
                fluxCalcScheme<typename outerProduct<vector, Type>::type>::New(mesh, is)
            ),
            tempX(mesh.maxGaussPerCell()),
            tempY(mesh.maxGaussPerCell())
        {
        }

        const fluxCalcScheme<typename outerProduct<vector, Type>::type>& getFluxCalcScheme()const{
            return tfluxScheme_();
        }


    // Member Functions

        virtual void dgcDivCalculateCell(
            dgTree<physicalCellElement>::leafIterator iter, 
            shared_ptr<dgLduMatrix<Type>> lduMatrixPtr, 
            const GeometricDofField<typename outerProduct<vector, Type>::type, dgPatchField, dgGeoMesh> * vf);

        virtual void dgcDivCalculateCell(
            dgTree<physicalCellElement>::leafIterator iter, 
            shared_ptr<dgLduMatrix<Type>> lduMatrixPtr, 
            const dgGaussField<typename outerProduct<vector, Type>::type>& dgf,
            const Field<typename outerProduct<vector, Type>::type>&);

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace dg

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
#   include "defaultDivScheme.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
