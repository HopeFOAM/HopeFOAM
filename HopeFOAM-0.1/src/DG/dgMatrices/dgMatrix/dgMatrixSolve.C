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
#include "dgLduMatrix.H"

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class Type>
void Foam::dgMatrix<Type>::setComponentReference
(
    const label patchi,
    const label facei,
    const direction cmpt,
    const scalar value
)
{

}

template<class Type>
Foam::SolverPerformance<Type> Foam::dgMatrix<Type>::solve
(
    const dictionary& solverControls,
    KSP ksp
)
{
    if (debug)
    {
        Info.masterStream(psi_.mesh().comm())
            << "dgMatrix<Type>::solve(const dictionary& solverControls) : "
               "solving dgMatrix<Type>"
            << endl;
    }

    label maxIter = -1;
    if (solverControls.readIfPresent("maxIter", maxIter))
    {
        if (maxIter == 0)
        {
            return SolverPerformance<Type>();
        }
    }

    word type(solverControls.lookupOrDefault<word>("type", "segregated"));

    if (type == "segregated")
    {
        return solveSegregated(ksp);
    }

    else
    {
        FatalIOErrorIn
        (
            "dgMatrix<Type>::solve(const dictionary& solverControls)",
            solverControls
        )   << "Unknown type " << type
            << "; currently supported solver types are segregated and coupled"
            << exit(FatalIOError);

        return SolverPerformance<Type>();
    }
}


template<class Type>
Foam::SolverPerformance<Type> Foam::dgMatrix<Type>::solveSegregated
(
    KSP ksp
)
{
    if (debug)
    {
        Info.masterStream(psi_.mesh().comm())
            << "dgMatrix<Type>::solveSegregated"
               "(const dictionary& solverControls) : "
               "solving dgMatrix<Type>"
            << endl;
    }

    GeometricDofField<Type, dgPatchField, dgGeoMesh>& psi =
       const_cast<GeometricDofField<Type, dgPatchField, dgGeoMesh>&>(psi_);
    
    SolverPerformance<Type> solverPerfVec
    (
        "dgMatrix<Type>::solveSegregated",
        psi.name()
    );

    Mat C;
    KSPGetOperators(ksp,&C,NULL);

    PetscErrorCode ierr;
    Vec            u,b, Br;

    MPI_Comm comm = MPI_COMM_WORLD;
    PC pc;
    KSPType kspType;
    PCType  pcType;
    PetscReal      norm;
    label globalShift = psi_.mesh().localRange().first();

    ierr = KSPGetType(ksp, &kspType);
    ierr = KSPGetPC(ksp, &pc);
    ierr = PCGetType(pc, &pcType);
 
    //create vector
    if(operatorOrder() == 0){
        ierr = VecCreateSeq(PETSC_COMM_SELF, b_.size(), &u);
        globalShift = 0;
    }
    else{
        if (Pstream::parRun() && operatorOrder() != 0){
            ierr = VecCreateMPI(MPI_COMM_WORLD, b_.size(), PETSC_DECIDE, &u);
        }
        else
            ierr = VecCreateSeq(MPI_COMM_WORLD, b_.size(), &u);
    }
    ierr = VecDuplicate(u,&b);
    ierr = VecDuplicate(u,&Br);
    //vec set values
    PetscInt       *indice, nindice;
    PetscScalar    *vals;
    nindice = b_.size();
    ierr = PetscMalloc1(nindice, &vals);
    ierr = PetscMalloc1(nindice, &indice);

    for (direction cmpt=0; cmpt<Type::nComponents; cmpt++)
    {
        for(int i=0; i<nindice; i++)
        {
            vals[i] = b_[i][cmpt];
            indice[i] = i + globalShift;
        }
        //set source 
        ierr = VecSetValues(b, nindice, indice, vals, INSERT_VALUES);
        for(int i=0; i<nindice; i++){
            vals[i] = psi.internalField()[i][cmpt];
        }
        //set u
        ierr = VecSetValues(u, nindice, indice, vals, INSERT_VALUES);
        ierr = VecAssemblyBegin(b);
        ierr = VecAssemblyEnd(b);
        ierr = VecAssemblyBegin(u);
        ierr = VecAssemblyEnd(u);

        solverPerformance solverPerf
        (
            (word)pcType + "-" + (word)kspType,
            psi.name()
        );

        MatMult(C,u,Br);
        VecAXPY(Br,-1.0,b);
        VecNorm(Br,NORM_2,&solverPerf.initialResidual());

        ierr = KSPSolve(ksp,b,u);
        // Get final residual and iteration number
	PetscInt iter_num;
	ierr = KSPGetIterationNumber(ksp, &iter_num);
	solverPerf.nIterations() = iter_num;
        
        ierr = KSPGetResidualNorm(ksp, &solverPerf.finalResidual());

        solverPerfVec.replace(cmpt, solverPerf);

        solverPerf.print(Info);
        solverPerf.nIterations() = 0;

        const scalar* data;
        VecGetArrayRead(u, &data);
        for(int i=0;i<psi.size();i++)
            psi.primitiveFieldRef()[i][cmpt]= data[i];
        VecRestoreArrayRead(u, &data);
        
        if(data) delete data;
    }

    ierr = PetscFree(vals);
    ierr = PetscFree(indice);
    VecDestroy(&u);
    VecDestroy(&b);    
    VecDestroy(&Br);  

    psi.correctBoundaryConditions();
    psi.mesh().setSolverPerformance(psi.name(), solverPerfVec);

    return solverPerfVec;
}

template<class Type>
Foam::SolverPerformance<Type> Foam::dgMatrix<Type>::solve()
{
    return solve
    (
        psi_.mesh().solverDict
        (
            psi_.select
            (
                psi_.mesh().data::template lookupOrDefault<bool>
                ("finalIteration", false)
            )
        ),
        kspSolver()
    );
}

template<class Type>
Foam::SolverPerformance<Type> Foam::dgMatrix<Type>::solve(KSP ksp)
{
    return solve
    (
        psi_.mesh().solverDict
        (
            psi_.select
            (
                psi_.mesh().data::template lookupOrDefault<bool>
                ("finalIteration", false)
            )
        ),
        ksp
    );
}

template<class Type>
Foam::tmp<Foam::Field<Type> > Foam::dgMatrix<Type>::residual() const
{
    tmp<Field<Type> > tres(new Field<Type>(b_.size()));
    Field<Type>& res = tres();

    return tres;
}

// * * * * * * * * * * * * * * * dgSolver  * * * * * * * * * * * * * //
/*
template<class Type>
Foam::dgMatrix<Type>::dgSolver::dgSolver
(
    const word& fieldName,
    const dgMatrix<Type>& matrix,
    const dictionary& solverControls
)
:
	fieldName_(fieldName),
	matrix_(matrix),
	controlDict_(solverControls)
{
	readControls();
}

template<class Type>
void Foam::dgMatrix<Type>::dgSolver::readControls()
{
    maxIter_   = controlDict_.lookupOrDefault<label>("maxIter", 1000);
    minIter_   = controlDict_.lookupOrDefault<label>("minIter", 0);
    tolerance_ = controlDict_.lookupOrDefault<scalar>("tolerance", 1e-6);
    relTol_    = controlDict_.lookupOrDefault<scalar>("relTol", 0);
}


template<class Type>
Foam::SolverPerformance<Type> Foam::dgMatrix<Type>::dgSolver::solve
(
    scalarField& psi,
    const scalarField& source,
    const direction cmpt
)
{
    SolverPerformance<Type> solverPerdgec
    (
        "dgMatrix<Type>::solveSegregated",
        "test"
    );

    return solverPerdgec;
}
*/
