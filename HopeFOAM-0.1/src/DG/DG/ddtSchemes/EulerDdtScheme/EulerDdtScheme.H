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
    Foam::dg::EulerDdtScheme

Description
    Basic first-order Euler implicit/explicit ddt using only the current and
    previous time-step values.

SourceFiles
    EulerDdtScheme.C
    
Author
    Xu Liyang (xucloud77@gmail.com)
\*---------------------------------------------------------------------------*/

#ifndef EulerDdtScheme_H
#define EulerDdtScheme_H

#include "ddtScheme.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace dg
{

/*---------------------------------------------------------------------------*\
                       Class EulerDdtScheme Declaration
\*---------------------------------------------------------------------------*/

template<class Type>
class EulerDdtScheme
:
    public ddtScheme<Type>
{
    // Private Member Functions

        //- Disallow default bitwise copy construct
        EulerDdtScheme(const EulerDdtScheme&);

        //- Disallow default bitwise assignment
        void operator=(const EulerDdtScheme&);


public:

    //- Runtime type information
    TypeName("Euler");


    // Constructors

        //- Construct from mesh
        EulerDdtScheme(const dgMesh& mesh)
        :
            ddtScheme<Type>(mesh)
        {}

        //- Construct from mesh and Istream
        EulerDdtScheme(const dgMesh& mesh, Istream& is)
        :
            ddtScheme<Type>(mesh, is)
        {}


    // Member Functions

        //- Return mesh reference
        const dgMesh& mesh() const
        {
            return dg::ddtScheme<Type>::mesh();
        }

        tmp<GeometricDofField<Type, dgPatchField, dgGeoMesh> > dgcDdt
        (
            const GeometricDofField<Type, dgPatchField, dgGeoMesh>&
        );

        void calculateCell(dgTree<physicalCellElement>::leafIterator iter)
        {

        }

        shared_ptr<dgLduMatrix<Type>> getLduMatrixPtr()
        {
            
        } 

        tmp<dgMatrix<Type> > dgmDdt
        (
            scalar,
            const GeometricDofField<Type, dgPatchField, dgGeoMesh>&
        );

        typedef GeometricDofField<Type, dgPatchField, dgGeoMesh> PSI;

        virtual void dgcDdtCalculateCell(
            dgTree<physicalCellElement>::leafIterator, 
            shared_ptr<dgLduMatrix<Type>>, 
            const PSI*);

        virtual void dgmDdtCalculateCell(
            dgTree<physicalCellElement>::leafIterator, 
            shared_ptr<dgLduMatrix<Type>>, 
            const PSI*);

};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace dg

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
#   include "EulerDdtScheme.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
