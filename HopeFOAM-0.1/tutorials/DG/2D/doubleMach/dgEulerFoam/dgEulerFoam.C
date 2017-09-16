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

Application
    dgEulerFoam

Description
    solver for compressible, inviscid flow of fluids.

\*---------------------------------------------------------------------------*/

#include "dgCFD.H"
#include "setBoundaryValues.H"
#include "godunovScheme.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

scalar rhoUref=295.8672801;
scalar rhoVref=6.45618091;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{

    #include "setRootCase.H"

    #include "createTime.H"
    #include "createMesh.H"
    #include "createFields.H"
    #include "createTimeControls.H"


    //- set initial conditions;
    #include "setNonUniformInlet.H" 
    dgScalarField rho1("rho1", rho);
    dgVectorField rhoU1("rhoU1", rhoU);
    dgScalarField Ener1("Ener1", Ener);    
    //- end set initial condition

    while(runTime.run()){
        runTime++;

        //2-order SSP Runge-kutta

        //SSP RK stage 1.
        rho1 = rho;
        rhoU1 = rhoU;
        Ener1 = Ener;
        setBoundaryValues(rho1, rhoU1, Ener1, gamma, runTime - runTime.deltaT());
        rho1.storeOldTime();
        rhoU1.storeOldTime();
        Ener1.storeOldTime();
        rho1.updateGaussField();
        rhoU1.updateGaussField();
        Ener1.updateGaussField();

        gther_U = rhoU1.gaussField()/rho1.gaussField();
        gther_p = (gamma - dimensionedScalar("one",gamma.dimensions(),1.0))*(Ener1.gaussField() - 0.5*(rho1.gaussField()*magSqr(gther_U)));

		Godunov.update(gther_U, gther_p, gamma.value(), rho1.gaussField(), rhoU1.gaussField(), Ener1.gaussField());

        dg::solveEquation(dgm::ddt(rho1) + dgc::div(gther_U, rho1, Godunov.fluxRho()));

        dg::solveEquation(dgm::ddt(rhoU1) + dgc::div(gther_U, rhoU1, Godunov.fluxRhoU()) + dgc::grad(gther_p));

        dg::solveEquation(dgm::ddt(Ener1) + dgc::div(gther_U, Ener1, Godunov.fluxEner())+ dgc::div(gther_U, gther_p));

	Godunov.limite(rho1,rhoU1,Ener1);

        rho1.storeOldTime();
        rhoU1.storeOldTime();
        Ener1.storeOldTime();
		
        //SSP RK Stage 2.
        rho1.updateGaussField();
        rhoU1.updateGaussField();
        Ener1.updateGaussField();


        gther_U = rhoU1.gaussField()/rho1.gaussField();
        gther_p = (gamma - dimensionedScalar("one",gamma.dimensions(),1.0))*(Ener1.gaussField() - 0.5*rho1.gaussField()*magSqr(gther_U));

	    Godunov.update(gther_U, gther_p, gamma.value(), rho1.gaussField(), rhoU1.gaussField(), Ener1.gaussField());

        dg::solveEquation(dgm::ddt(rho1) + dgc::div(gther_U, rho1, Godunov.fluxRho()));

        dg::solveEquation(dgm::ddt(rhoU1) + dgc::div(gther_U, rhoU1, Godunov.fluxRhoU()) + dgc::grad(gther_p));

        dg::solveEquation(dgm::ddt(Ener1) + dgc::div(gther_U, Ener1, Godunov.fluxEner())+ dgc::div(gther_U, gther_p));

         rho = 0.5*rho + 0.5*rho1;
         rhoU = 0.5*rhoU + 0.5*rhoU1;
         Ener = 0.5*Ener + 0.5*Ener1;

        Godunov.limite(rho,rhoU,Ener);
        
		rho.correctBoundaryConditions();
		rhoU.correctBoundaryConditions();
		Ener.correctBoundaryConditions();

        runTime.write();

        #include "doubleerror.H"

        Info<< "Time = " << runTime.timeName() << nl << endl;
        Info<< "ExecutionTime = " << runTime.elapsedCpuTime() << " s"
            << "  ClockTime = " << runTime.elapsedClockTime() << " s"
            << nl << endl;
    }

    Info<< runTime.value() << endl;

    return 0;
}
// ************************************************************************* //
