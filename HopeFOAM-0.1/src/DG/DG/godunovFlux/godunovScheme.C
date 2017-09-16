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

\*---------------------------------------------------------------------------*/

#include "godunovScheme.H"
#include "dgFields.H"
//#include "surfaceFields.H"
//#include "coupledDgPatchField.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * * * Selectors * * * * * * * * * * * * * * * //
namespace Foam{
namespace dg{

	//constructor
godunovScheme:: godunovScheme
(
	const dgMesh& mesh,
    const dgGaussVectorField& gaussU,
    const dgGaussScalarField& gaussP,
    const scalar gamma,
    const dgGaussScalarField& gaussRho,
    const dgGaussVectorField& gaussRhoU,
    const dgGaussScalarField& gaussEner
)
:
    mesh_(mesh),
	fluxFieldsize_(gaussP.ownerFaceField().size()),
	fluxRho_(fluxFieldsize_,0),
	fluxRhoU_(fluxFieldsize_,vector::zero),
	fluxEner_(fluxFieldsize_,0),
	fluxSchemes_(NULL),
	limiteSchemes_(NULL)
{
        
    if(mesh_.schemesDict().found("godunovScheme")){
        ITstream&  fluxScheme_ = mesh_.schemesDict().subDict("godunovScheme").lookup("fluxScheme");
		ITstream&   limiteScheme_ = mesh_.schemesDict().subDict("godunovScheme").lookup("limiteScheme");
		fluxSchemes_=fluxSchemes::New(mesh_,fluxScheme_);
		limiteSchemes_=limiteSchemes::New(mesh_,limiteScheme_);
                   	   
    }
	else{
  		Info<<"Godunov flux type doesn't specification"<<nl
        	<< exit(FatalIOError);
	}
}


void godunovScheme::update
(
    const dgGaussVectorField& gaussU,
    const dgGaussScalarField& gaussP,
    const scalar gamma,
    const dgGaussScalarField& gaussRho,
    const dgGaussVectorField& gaussRhoU,
    const dgGaussScalarField& gaussEner
)
{
  	fluxSchemes_().evaluateFlux(gaussU ,gaussP, gamma, gaussRho, fluxRho_, gaussRhoU, fluxRhoU_, gaussEner, fluxEner_);
}

void godunovScheme::limite
(
	dgScalarField& rho,
	dgVectorField& rhoU,
	dgScalarField& Ener,
	dgScalarField& troubledCell
)
{
		
    limiteSchemes_().limite(rho, rhoU, Ener, rho.gaussField(), rhoU.gaussField(), Ener.gaussField(), troubledCell);

}


void godunovScheme::limite
(
	dgScalarField& rho,
	dgVectorField& rhoU,
	dgScalarField& Ener
)
{

	limiteSchemes_().limite(rho,rhoU,Ener,rho.gaussField(), rhoU.gaussField(), Ener.gaussField());
}

}// End namespace dg

}// Emd namespace Foam



// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //


Foam::dg::godunovScheme::~godunovScheme()
{}


// ************************************************************************* //
