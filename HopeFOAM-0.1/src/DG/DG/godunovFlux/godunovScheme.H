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
    Foam::dg::godunovScheme

Description
    Generic Godunov flux class. Calculate the face flux using a Godunov
    scheme to support an exact or approximate Riemman solver. A limiter
    is needed too when solving Riemman problem with high order.

SourceFiles
    godunovScheme.C
    godunovSchemes.C
    
Author
    Ye Shuai (shuaiye09@163.com)

\*---------------------------------------------------------------------------*/

#ifndef godunovScheme_H
#define godunovScheme_H

#include "tmp.H"
#include "dgFields.H"
#include "dgGaussField.H"
#include "typeInfo.H"
#include "runTimeSelectionTables.H"

#include "limiteScheme.H"
#include "fluxScheme.H"
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

class dgMesh;

namespace dg
{


/*---------------------------------------------------------------------------*\
                 Class fluxCalcScheme Declaration
\*---------------------------------------------------------------------------*/

class godunovScheme
:
    public refCount
{
    // Private data

        //- Hold reference to mesh
    const dgMesh& mesh_;

    label  fluxFieldsize_;
    scalarField fluxRho_;
    vectorField fluxRhoU_;
	scalarField fluxEner_;

	tmp<limiteSchemes> limiteSchemes_;
	tmp<fluxSchemes> fluxSchemes_;
	
    // Private Member Functions

    //- Disallow copy construct
    godunovScheme(const godunovScheme&);

    //- Disallow default bitwise assignment
    void operator=(const godunovScheme&);


public:


    // Constructors

    //- Construct from mesh
    godunovScheme
    (
        const dgMesh& mesh,
        const dgGaussVectorField& gaussU,
        const dgGaussScalarField& gaussP,
        const scalar gamma,
        const dgGaussScalarField& gaussRho,
        const dgGaussVectorField& gaussRhoU,
        const dgGaussScalarField& gaussEner
    );

    //- Destructor
    virtual ~godunovScheme();


    // Member Functions

        //- Return mesh reference
        const dgMesh& mesh() const
        {
            return mesh_;
        }

        /***************** gauss flux calc Weak***************************/
	
    	void update
        (
            const dgGaussVectorField& gaussU,
            const dgGaussScalarField& gaussP,
            const scalar gamma,
            const dgGaussScalarField& gaussRho,
            const dgGaussVectorField& gaussRhoU,
            const dgGaussScalarField& gaussEner
        );

    	void limite
        (
            dgScalarField& rho, 
            dgVectorField& rhoU,  
            dgScalarField& Ener,
            dgScalarField& troubledCell
        );
        
        void limite
        (
            dgScalarField&,
            dgVectorField&, 
            dgScalarField&
        );
    	
    	const scalarField& fluxRho() const{ return fluxRho_;}
    	const vectorField& fluxRhoU() const{ return fluxRhoU_;}
    	const scalarField& fluxEner() const{ return fluxEner_;}
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace dg

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// Add the patch constructor functions to the hash tables


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

/*#ifdef NoRepository
#   include "godunovScheme.C"
#endif
*/
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
