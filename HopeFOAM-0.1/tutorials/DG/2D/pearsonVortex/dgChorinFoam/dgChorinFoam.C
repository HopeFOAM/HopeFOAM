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
    dgChorinFoam

Description
    Transient solver for 2D, incompressible, laminar flow of Newtonian fluids.

\*---------------------------------------------------------------------------*/
#include "dgCFD.H"
#include "pCorrectEquation.H"
#include "bCorrectEquation.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{
    #include "setRootCase.H"
    #include "createTime.H"
    #include "createMesh.H"
    #include "createFields.H"

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
    Info<< "\nStarting time loop\n" << endl;
    label outFlag;
    label inFlag;
    scalar pi = constant::mathematical::pi;

    #include "setNonUniformInlet.H"

    scalar paraT1 = 1;
    scalar paraT2 = 1;
    scalar paraT = 1;
    scalar paraT3 = 1;
    scalar paraT4 = 1;
    
    #include "setparaT.H"
	
    while(runTime.run())
    {

        runTime++;
        Info << "Time = " << runTime.timeName() << nl << endl;
        dimensionedScalar rDeltaT = 1.0/runTime.deltaT();

        //- step 1: velocity prediction

        convecUold = convecU;

        dg::solveEquation(dgm::Sp(convecU) == dgc::div(U, U) );

        UT = (a0*U.oldTime() + a1*U.oldTime().oldTime() - (b0*convecU + b1*convecUold) * runTime.deltaT()) / g0;

        UT.correctBoundaryConditions();

        //- step 2: pressure correction

        #include "pEqnCorrect.H"
        shared_ptr<dg::Equation<scalar>> result1 = make_shared<bCorrectEquation<scalar>>(dgm::laplacian(p), paraT3);

        (dg::solveEquation( result1 == (pEqnCorrect + dgc::div(UT) * (rDeltaT * g0 )) ));

        //- step 3: velocity correction

        dg::solveEquation(dgm::Sp(UT) + (dgc::grad(p) * (runTime.deltaT() / g0)) == UT );

        //- step 4: viscosity contribution
        dimensionedScalar tmp( g0/(runTime.deltaT() * nu) );

        shared_ptr<dg::Equation<vector>> result2 = make_shared<bCorrectEquation<vector>>(dgm::laplacian(U), paraT4);

        (dg::solveEquation( dgm::Sp(U) *tmp == result2 + UT*tmp));

        runTime.write();
		
        #include "setBoundaryValues.H"

        if(a0<2.0){
            g0 = 1.5, a0 = 2.0, a1 = -0.5, b0 = 2.0, b1 = -1.0;
        }

        Info<< "ExecutionTime = " << runTime.elapsedCpuTime() << " s"
    	<< "  ClockTime = " << runTime.elapsedClockTime() << " s"
                << nl << endl;
    }

    Info<< "End\n" << endl;
	
        #include "chorinerror.H"
	   
    return 0;
}


// ************************************************************************* //
