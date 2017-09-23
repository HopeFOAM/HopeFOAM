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
#include "dgScalarMatrix.H"
#include "zeroGradientDgPatchFields.H"
#include "petscdraw.h"
#include "petscpc.h"

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //


template<>
void Foam::dgMatrix<Foam::scalar>::setComponentReference
(
    const label patchi,
    const label facei,
    const direction,
    const scalar value
)
{

}
/*
template<>
Foam::SolverPerformance<scalar> Foam::dgMatrix<Foam::scalar>::dgSolver::solve
(
    scalarField& psi,
    const scalarField& source,
    const direction cmpt
)
{
    SolverPerformance<scalar> solverPerdgec
    (
        "dgMatrix<scalar>::solveSegregated",
        "psi.name()"
    );

    return solverPerdgec;
}*/


template<>
Foam::SolverPerformance<Foam::scalar> Foam::dgMatrix<Foam::scalar>::solveSegregated
(
    KSP ksp
)
{
    if (debug)
    {
        Info << "dgMatrix<scalar>::solveSegregated"
               "(const dictionary& solverControls) : "
               "solving dgMatrix<scalar>"
            << endl;
    }

    GeometricDofField<scalar, dgPatchField, dgGeoMesh>& psi =
       const_cast<GeometricDofField<scalar, dgPatchField, dgGeoMesh>&>(psi_);

    Mat C;
    KSPGetOperators(ksp,&C,NULL);

    PetscErrorCode ierr;
    Vec            u,b, Br;
    PetscReal      norm;
    label globalShift = psi_.mesh().localRange().first();

    // added by Howe@2017.2.17
    MPI_Comm comm = MPI_COMM_WORLD;
    Vec     x;
    PC pc;
    KSPType kspType;
    PCType  pcType;

    ierr = KSPGetType(ksp, &kspType);
    ierr = KSPGetPC(ksp, &pc);
    ierr = PCGetType(pc, &pcType);

    //create vector
    if(operatorOrder() == 0){
        ierr = VecCreateSeq(PETSC_COMM_SELF, b_.size(), &u);
        globalShift = 0;
    }
    else{
        if (Pstream::parRun())
            ierr = VecCreateMPI(MPI_COMM_WORLD, b_.size(), PETSC_DECIDE, &u);
        else{
            ierr = VecCreateSeq(MPI_COMM_WORLD, b_.size(), &u);
        }
    }
    ierr = VecDuplicate(u,&b);
    ierr = VecDuplicate(u,&Br);

    //vec set values
    PetscInt       *indice, nindice;
    PetscScalar    *vals;
    nindice = b_.size();
    ierr = PetscMalloc1(nindice, &vals);
    ierr = PetscMalloc1(nindice, &indice);
    for(int i=0; i<nindice; i++){
        vals[i] = b_[i];
        indice[i] = i + globalShift;
    }
    //set source 
    ierr = VecSetValues(b, nindice, indice, vals, INSERT_VALUES);
    for(int i=0; i<nindice; i++){
        vals[i] = psi.internalField()[i];
    }
    //set u
    ierr = VecSetValues(u, nindice, indice, vals, INSERT_VALUES);
    ierr = VecAssemblyBegin(b);
    ierr = VecAssemblyEnd(b);
    ierr = VecAssemblyBegin(u);
    ierr = VecAssemblyEnd(u);
    ierr = PetscFree(vals);
    ierr = PetscFree(indice);

    // Calc the initial residual
    ierr = VecDuplicate(u,&x);
    ierr = MatMult(C, u, x);
    ierr = VecAXPY(x, -1.0, b);
    ierr = VecNorm(x,NORM_2,&norm);

    solverPerformance solverPerf
    (
         (word)pcType + "-" + (word)kspType,
         psi.name()
    );
    solverPerf.initialResidual() = static_cast<double>(norm);
    ierr = KSPSolve(ksp,b,u);

    //ierr = PetscLogStagePop();
    
    // Get final residual and iteration number
    PetscInt iter_num;
    ierr = KSPGetIterationNumber(ksp, &iter_num);
    solverPerf.nIterations() = iter_num;
    ierr = KSPGetResidualNorm(ksp, &solverPerf.finalResidual());
    /*
    MatMult(C,u,Br);
    VecAXPY(Br,-1.0,b);
    VecNorm(Br,NORM_2,&solverPerf.finalResidual());
    //KSPBuildResidual(ksp, PETSC_NULL, PETSC_NULL, &Br);
    //VecStrideNorm(Br,0,NORM_2,&solverPerf.finalResidual());
    */
    solverPerf.print(Info);
    solverPerf.nIterations() = 0;

    const scalar* data;
    VecGetArrayRead(u, &data);
    for(int i=0;i<psi.size();i++)
        psi.primitiveFieldRef()[i]= data[i];
    VecRestoreArrayRead(u, &data);
    if(data) delete data;

    VecDestroy(&u);
    VecDestroy(&Br);
    VecDestroy(&b);
    VecDestroy(&x);

    psi.correctBoundaryConditions();
    psi.mesh().setSolverPerformance(psi.name(), solverPerf);

    return solverPerf;
}


template<>
Foam::tmp<Foam::scalarField> Foam::dgMatrix<Foam::scalar>::residual() const
{
    tmp<scalarField > tres(new scalarField(b_.size()));
    scalarField& res = tres.ref();

    return tres;
}
