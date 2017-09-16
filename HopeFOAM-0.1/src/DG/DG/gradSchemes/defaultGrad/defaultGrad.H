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
    Foam::dg::defaultGrad

Description


SourceFiles
    defaultGrad.C

Author
    Xu Liyang (xucloud77@gmail.com)
\*---------------------------------------------------------------------------*/

#ifndef defaultGrad_H
#define defaultGrad_H

#include "gradScheme.H"


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace dg
{

/*---------------------------------------------------------------------------*\
                       Class defaultGrad Declaration
\*---------------------------------------------------------------------------*/

template<class Type>
class defaultGrad
:
    public dg::gradScheme<Type>
{
    // Private data

        tmp<fluxCalcScheme<typename innerProduct<vector, Type>::type> > tfluxScheme_;

        Field<typename innerProduct<vector, Type>::type> tempX;
        Field<Type> tempY;


    // Private Member Functions

        //- Disallow default bitwise copy construct
        defaultGrad(const defaultGrad&);

        //- Disallow default bitwise assignment
        void operator=(const defaultGrad&);


public:

    //- Runtime type information
    TypeName("default");


    // Constructors

        //- Construct from mesh


        //- Construct from mesh and Istream
        defaultGrad(const dgMesh& mesh, Istream& is)
        :
            gradScheme<Type>(mesh),
            tfluxScheme_
            (
                fluxCalcScheme<typename innerProduct<vector, Type>::type>::New(mesh, is)
            ),
            tempX(mesh.maxGaussPerCell()),
            tempY(mesh.maxGaussPerCell())
        {
        }

        const fluxCalcScheme<typename innerProduct<vector, Type>::type>& getFluxCalcScheme()const{
            return tfluxScheme_();
        }

       virtual void dgcGradCalculateCell(
            dgTree<physicalCellElement>::leafIterator, 
            shared_ptr<dgLduMatrix<Type>>, 
            const GeometricDofField<typename innerProduct<vector, Type>::type, 
                dgPatchField, dgGeoMesh> *) ;

        virtual void dgcGradCalculateCell(
            dgTree<physicalCellElement>::leafIterator, 
            shared_ptr<dgLduMatrix<Type>>, 
            const dgGaussField<typename innerProduct<vector, Type>::type>&,
            const Field<typename innerProduct<vector, Type>::type>& flux);

        virtual void dgmGradCalculateCell(
            dgTree<physicalCellElement>::leafIterator, 
            shared_ptr<dgLduMatrix<Type>>, 
            const GeometricDofField<typename innerProduct<vector, Type>::type,
                dgPatchField, dgGeoMesh> *) ;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace dg

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
#   include "defaultGrad.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
