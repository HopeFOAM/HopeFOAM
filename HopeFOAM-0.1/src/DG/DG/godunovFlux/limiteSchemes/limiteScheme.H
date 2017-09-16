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
    Foam::dg::limiteScheme

Description
    Abstract base class for limiter.

SourceFiles
    limiteScheme.C
    
Author
    Ye Shuai (shuaiye09@163.com)

\*---------------------------------------------------------------------------*/

#ifndef limiteScheme_H
#define limiteScheme_H

#include "tmp.H"
#include "dgFields.H"
#include "dgGaussField.H"
//#include "surfaceFieldsFwd.H"
#include "typeInfo.H"
#include "runTimeSelectionTables.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

class dgMesh;

namespace dg
{

/*---------------------------------------------------------------------------*\
                 Class fluxCalcScheme Declaration
\*---------------------------------------------------------------------------*/


class limiteSchemes
:
    public refCount
{
    // Private data

        //- Hold reference to mesh
        const dgMesh& mesh_;


    // Private Member Functions

        //- Disallow copy construct
        limiteSchemes(const limiteSchemes&);

        //- Disallow default bitwise assignment
        void operator=(const limiteSchemes&);


public:

    //- Runtime type information
    virtual const word& type() const = 0;


    // Declare run-time constructor selection tables

        declareRunTimeSelectionTable
        (
            tmp,
            limiteSchemes,
            Istream,
            (
                const dgMesh& mesh,
                Istream& schemeData
            ),
            (mesh, schemeData)
        );


    // Constructors

        //- Construct from mesh
        limiteSchemes(const dgMesh& mesh)
        :
            mesh_(mesh)
        {}


    // Selectors

        //- Return new tmp interpolation scheme
        static tmp<limiteSchemes > New
        (
            const dgMesh& mesh,
            Istream& schemeData
        );


    //- Destructor
    virtual ~limiteSchemes();


    // Member Functions

        //- Return mesh reference
        const dgMesh& mesh() const
        {
            return mesh_;
        }

        /***************** gauss flux calc Weak***************************/

        virtual void limite
        (
            dgScalarField& ,
            dgVectorField&  ,
            dgScalarField& ,
            const dgGaussScalarField&,
            const dgGaussVectorField&,
            const dgGaussScalarField&,
            dgScalarField&
        )const=0;
		
  
        virtual void limite
        ( 
	  	    dgScalarField& ,
	  	    dgVectorField& ,
	  	    dgScalarField& ,
	  	    const dgGaussScalarField&,
	  	    const dgGaussVectorField&,
	  	    const dgGaussScalarField&
        )const=0;
      
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace dg

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// Add the patch constructor functions to the hash tables

#define makelimiteScheme(SS)                                       \
                                                                               \
defineTypeName(Foam::dg::SS);              \
                                                                               \
namespace Foam                                                             \
{                                                                          \
    namespace dg                                                           \
    {                                                                      \
        limiteSchemes::addIstreamConstructorToTable<SS >      \
        add##SS##IstreamConstructorToTable_;                     \
    }                                                                      \
}


                                                                               



// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //



// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
