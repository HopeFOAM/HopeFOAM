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

#include "calculatedDgPatchFields.H"
#include "zeroGradientDgPatchFields.H"
//#include "coupleddgPatchFields.H"
#include "Pair.H"
// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

template<class Type>
Foam::dgMatrix<Type>::dgMatrix
(
    const GeometricDofField<Type, dgPatchField, dgGeoMesh>& psi,
    const dimensionSet& ds,
    label order
)
:
    psi_(psi),
    operatorOrder_(order),
    dimensions_(ds),
    ksp_(NULL),
    mat_(NULL)
{
    if (debug)
    {
        Info<< "dgMatrix<Type>(GeometricDofField<Type, dgPatchField, dgGeoMesh>&,"
               " const dimensionSet&) : "
               "constructing dgMatrix<Type> for field " << psi_.name()
            << endl;
    }

    // Initialise
    petscPreAllocate();

    // Update the boundary coefficients of psi without changing its event No.
    GeometricDofField<Type, dgPatchField, dgGeoMesh>& psiRef =
       const_cast<GeometricDofField<Type, dgPatchField, dgGeoMesh>&>(psi_);

    label currentStatePsi = psiRef.eventNo();
    psiRef.boundaryFieldRef().updateCoeffs();
    psiRef.eventNo() = currentStatePsi;
}


template<class Type>
Foam::dgMatrix<Type>::dgMatrix(const dgMatrix<Type>& dgm)
:
    refCount(),
    psi_(dgm.psi_),
    operatorOrder_(dgm.operatorOrder_),
    dimensions_(dgm.dimensions_),
    ksp_(NULL),
    mat_(NULL)
{
    if (debug)
    {
        Info<< "dgMatrix<Type>::dgMatrix(const dgMatrix<Type>&) : "
            << "copying dgMatrix<Type> for field " << psi_.name()
            << endl;
    }
}


#ifdef ConstructFromTmp
template<class Type>
Foam::dgMatrix<Type>::dgMatrix(const tmp<dgMatrix<Type> >& tdgm)
:
    refCount(),
    psi_(tdgm().psi_),
    operatorOrder_(tdgm().operatorOrder_),
    dimensions_(tdgm().dimensions_),
    ksp_(NULL),
    mat_(NULL)
{
    if (debug)
    {
        Info<< "dgMatrix<Type>::dgMatrix(const tmp<dgMatrix<Type> >&) : "
            << "copying dgMatrix<Type> for field " << psi_.name()
            << endl;
    }

    tdgm.clear();
}
#endif


template<class Type>
Foam::dgMatrix<Type>::dgMatrix
(
    const GeometricDofField<Type, dgPatchField, dgGeoMesh>& psi,
    Istream& is,
    label order
)
:
    psi_(psi),
    operatorOrder_(order),
    dimensions_(is),
    ksp_(NULL),
    mat_(NULL)
{
    if (debug)
    {
        Info<< "dgMatrix<Type>"
               "(GeometricDofField<Type, dgPatchField, dgGeoMesh>&, Istream&) : "
               "constructing dgMatrix<Type> for field " << psi_.name()
            << endl;
    }

    // Initialise

}


template<class Type>
Foam::dgMatrix<Type>::~dgMatrix()
{
    if (debug)
    {
        Info<< "dgMatrix<Type>::~dgMatrix<Type>() : "
            << "destroying dgMatrix<Type> for field " << psi_.name()
            << endl;
    }
    if(ksp_){
        KSPDestroy(&ksp_);
        ksp_ = NULL;
    }
    if(mat_){
        MatDestroy(&mat_);
    }
}

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

template<class Type>
KSP Foam::dgMatrix<Type>::kspSolver()
{
    const dictionary& solverControls(
        psi_.mesh().solverDict
        (
            psi_.select
            (
                psi_.mesh().data::template lookupOrDefault<bool>
                ("finalIteration", false)
            )
        )
    );

    PetscBool ifInit;
    PetscInitialized(&ifInit);
    if(!ifInit){
        PetscInitializeNoArguments();
    }

    if(ksp_){
        KSPDestroy(&ksp_);
        ksp_ = NULL;
    }
    PC             pc;
    PetscErrorCode ierr;
    /* Added by Howe: @2017.3.23*/
    KSPType kspType;
    PCType  pcType;
    MPI_Comm comm = MPI_COMM_WORLD;
    /* * * * * * * * * * * * * * * * * * * * * * * * * * * * */

    // Set solver
    ierr = KSPCreate(MPI_COMM_WORLD,&ksp_);
    ierr = KSPSetOperators(ksp_,mat_,mat_);

    /* Added by Howe@2017.2.17, Set the solver */
    word kspName(solverControls.lookupOrDefault<word>("kspSolver", "gmres"));
    kspType = kspName.c_str();
    ierr = KSPSetType(ksp_,kspType);
    
    if (kspName != "preonly")
    {
        ierr = KSPSetInitialGuessNonzero(ksp_,PETSC_TRUE); // Crucial to the performance
        //ierr = KSPSetDiagonalScale(ksp_,PETSC_TRUE);
        //ierr = KSPSetInitialGuessKnoll(ksp_,PETSC_TRUE);
    }
    
    ierr = KSPSetFromOptions(ksp_);
    ierr = KSPSetTolerances(ksp_,
                            solverControls.lookupOrDefault<scalar>("relTol", PETSC_DEFAULT), 
                            solverControls.lookupOrDefault<scalar>("tolerance", PETSC_DEFAULT),
                            PETSC_DEFAULT,
                            solverControls.lookupOrDefault<int>("maxIter", PETSC_DEFAULT)
                            );
    ierr = KSPSetUp(ksp_);    
    /* * * * * * * * * * * * * * * * * * * * * * * * * * * * */

    /* Added by Howe@2017.2.17, Set the preconditioner */
    word pcName(solverControls.lookupOrDefault<word>("kspPC", "jacobi"));
    word pkgName(solverControls.lookupOrDefault<word>("kspPackage", ""));
    if (pcName.empty()) // Default PC, ilu
    {
        KSPGetPC(ksp_, &pc);
        PCGetType(pc, &pcType);
        pcName = pcType;
    }
    // Hypre PC from LLNL: hypre-pilut,hypre-parasails,hypre-boomeramg,hypre-ams,hypre-ads      
    else if (pcName == "ilu")
    {
        KSPGetPC(ksp_, &pc);
        PCSetType(pc,"ilu");
        //PCFactorSetReuseOrdering(pc, PETSC_TRUE); 
        //PCFactorSetReuseFill_ILU(pc, PETSC_TRUE);
        //PCFactorReorderForNonzeroDiagonal_ILU(pc, PETSC_DECIDE);
        //PCFactorSetFill(pc,8);
        //PCFactorSetLevels(pc, 1);
        //PCFactorSetAllowDiagonalFill(pc);
    }
    else if (pcName(5) == "hypre") // hypre PC
    {
        pcType = pcName.replace("hypre-","").data();
        KSPGetPC(ksp_, &pc);
        PCSetType(pc,"hypre");
        PCHYPRESetType(pc, pcType);
        //PetscErrorCode ier = PetscStackCallStandard(HYPRE_BoomerAMGSetTol,((HYPRE_Solver)pc,1e-8));
        //HYPRE_BoomerAMGSetTol((HYPRE_Solver)pc,1e-8);
        //HYPRE_BoomerAMGSetCoarsenType((HYPRE_Solver)pc, 100);

    }
    else if (pcName == "gamg")
    {
        KSPGetPC(ksp_, &pc);
        PCSetType(pc,"gamg");
        PCMGSetLevels(pc, 1, &comm);
        PCMGSetType(pc, PC_MG_MULTIPLICATIVE);
        PCMGSetCycleType(pc, PC_MG_CYCLE_V);
        PCMGSetNumberSmoothUp(pc, 1);
        PCMGSetNumberSmoothDown(pc, 1);
    }
    else// User defined PC
    {
        pcType = pcName.data();
        KSPGetPC(ksp_, &pc);
        PCSetType(pc,pcType);

        if (pcName == "lu")
        {
            //ierr = PCFactorSetFill(pc,5.67908);
        }

        // Check whether use external packages
        if(!pkgName.empty())
        {
            const char * pkgType = pkgName.c_str();
            PCFactorSetMatSolverPackage(pc, pkgType); // Set the software used to perform factorization
        }
    }
    /* * * * * * * * * * * * * * * * * * * *  * * * *  * * * * * * * * * * * */  

    return ksp_;
}

template<class Type>
void Foam::dgMatrix<Type>::setReference
(
    const label celli,
    const Type& value,
    const bool forceReference
)
{}

// * * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * //
template<class Type>
void Foam::dgMatrix<Type>::petscPreAllocate()
{
    const dgMesh& mesh_ = psi_.mesh();
    //- set rowSize_ and colSize_
    rowSize_.setSize(mesh_.nCells(), 0);
    colSize_.setSize(mesh_.nCells(), 0);
    onnSize_.setSize(mesh_.nCells(), 0);

    int cellJ = 0;
    for(dgTree<physicalCellElement>::leafIterator iter = mesh_.cellElementsTree().leafBegin(); iter != mesh_.cellElementsTree().end(); ++iter, ++cellJ){
        rowSize_[cellJ] = colSize_[cellJ] = iter()->value().nDof();
    }

    dofRows_.setSize(mesh_.maxDofPerCell());
    dofCols_.setSize(mesh_.maxDofPerCell()*(1+mesh_.maxNFacesPerCell()));
    label totalDof = mesh_.totalDof();
    //create vector
    b_.setSize(totalDof, pTraits<Type>::zero);

    if(operatorOrder_ == 0) return;

    if(operatorOrder_ == 2){
        int cellI = 0;
        for(dgTree<physicalCellElement>::leafIterator iter = mesh_.cellElementsTree().leafBegin(); iter != mesh_.cellElementsTree().end(); ++iter, ++cellI){
            List<shared_ptr<physicalFaceElement>>& faceEleInCell = iter()->value().faceElementList();

            forAll(faceEleInCell, faceI){
                if(faceEleInCell[faceI]->ownerEle_ == iter()){
                    if(!faceEleInCell[faceI]->isPatch_){
                        colSize_[cellI] += faceEleInCell[faceI]->neighborEle_->value().baseFunction().nDofPerFace();
                        continue;
                    }
                    if(faceEleInCell[faceI]->neighborEle_)
                        onnSize_[cellI] += faceEleInCell[faceI]->neighborEle_->value().baseFunction().nDofPerFace();
                }
                else{
                    colSize_[cellI] += (faceEleInCell[faceI]->ownerEle_->value().baseFunction().nDofPerFace());
                }
            }
        }
    }
    else if(operatorOrder_ == 3){
        int cellI = 0;
        for(dgTree<physicalCellElement>::leafIterator iter = mesh_.cellElementsTree().leafBegin(); iter != mesh_.cellElementsTree().end(); ++iter, ++cellI){
            List<shared_ptr<physicalFaceElement>>& faceEleInCell = iter()->value().faceElementList();

            forAll(faceEleInCell, faceI){
                if(faceEleInCell[faceI]->ownerEle_ == iter()){
                    if(!faceEleInCell[faceI]->isPatch_){
                        colSize_[cellI] += faceEleInCell[faceI]->neighborEle_->value().baseFunction().nDofPerCell();
                        continue;
                    }
                    if(faceEleInCell[faceI]->neighborEle_)
                        onnSize_[cellI] += faceEleInCell[faceI]->neighborEle_->value().baseFunction().nDofPerCell();
                }
                else{
                    colSize_[cellI] += (faceEleInCell[faceI]->ownerEle_->value().baseFunction().nDofPerCell());
                }
            }
        }
    }

    label ptr = 0;
    
    d_nnz_.setSize(totalDof);
    o_nnz_.setSize(totalDof);
    forAll(colSize_, cellI){
        for(int i=0; i<rowSize_[cellI]; i++, ptr++){
            d_nnz_[ptr] = min(colSize_[cellI],totalDof);
            o_nnz_[ptr] = onnSize_[cellI];
        }
    }

    //create petsc Matrix
    if (Pstream::parRun()){
        MatCreate(MPI_COMM_WORLD,&mat_);
        MatSetSizes(mat_, totalDof, totalDof, PETSC_DECIDE, PETSC_DECIDE);
        MatSetType(mat_, MATMPIAIJ);
        MatMPIAIJSetPreallocation(mat_, 0, d_nnz_.data(), 0, o_nnz_.data());
    }
    else
        MatCreateSeqAIJ(MPI_COMM_WORLD,totalDof,totalDof,NULL,d_nnz_.data(),&mat_);
    MatSetFromOptions(mat_);
    MatSetUp(mat_);
}

template<class Type>
void Foam::dgMatrix<Type>::assembleMatrix(dgLduMatrix<Type>& ldu, const physicalCellElement& cellEle, label cellI, label globalShift)
{
    //- step 1, set dofRows_ and dofCols_;
    label dofRowSize = rowSize_[cellI];
    label dofColSize = colSize_[cellI];
    label dofStart = cellEle.dofStart();

    for(int i=0; i<dofRowSize; i++){
        dofRows_[i] = dofCols_[i] = dofStart + i + globalShift;
    }

    //- step 2, add source to b
    ldu.addSourceToB(cellEle);

    if(ldu.sourceSize() == dofRowSize){
        for(int i=0; i<dofRowSize; i++){
            b_[dofStart + i] = ldu.b_[i];
        }
    }

    //- operatorOrder_ == 0, no matrix need to assemble
    if(operatorOrder_ == 0){
        return;
    }
    //- step 3, add diagCoeff to diagnal Matrix
        scalar diagCoeff = ldu.diagCoeff();
        if(diagCoeff != 0){
            const denseMatrix<scalar>& massMatrix = cellEle.massMatrix();
            denseMatrix<scalar>& diag = ldu.diag();
            for(int i=0; i<dofRowSize*dofRowSize; i++){
                diag[i] += diagCoeff * massMatrix[i];
            }
        }

    //- step 4, add matrix to petsc mat
        //- add diag part
        MatSetValues(mat_, dofRowSize, dofRows_.data(), dofRowSize, dofCols_.data(), ldu.diag().data(), INSERT_VALUES);
        //- add lu part
        const List<shared_ptr<physicalFaceElement>>& faceEleInCell = cellEle.faceElementList();
        List<denseMatrix<scalar>>& lu2 = ldu.lu2();
        forAll(faceEleInCell, faceI){
            if(lu2[faceI].rowSize() ==0) continue;
            label cdof, dofStart;
            if(&(faceEleInCell[faceI]->ownerEle_->value()) == &(cellEle)){
                cdof = faceEleInCell[faceI]->neighborEle_->value().baseFunction().nDofPerCell();
                dofStart = faceEleInCell[faceI]->neighborEle_->value().dofStart();
            }
            else{
                cdof = faceEleInCell[faceI]->ownerEle_->value().baseFunction().nDofPerCell();
                dofStart = faceEleInCell[faceI]->ownerEle_->value().dofStart();
            }
            for(int i=0; i<cdof; i++){
                dofCols_[i] = dofStart + i + globalShift;
            }
            MatSetValues(mat_, dofRowSize, dofRows_.data(), cdof, dofCols_.data(), lu2[faceI].data(), INSERT_VALUES);
        }
}

template<class Type>
void Foam::dgMatrix<Type>::finalAssemble()
{
    MatAssemblyBegin(mat_,MAT_FINAL_ASSEMBLY);
    MatAssemblyEnd(mat_,MAT_FINAL_ASSEMBLY);
}
// * * * * * * * * * * * * * * * Global Functions  * * * * * * * * * * * * * //


template<class Type>
void Foam::checkMethod
(
    const dgMatrix<Type>& dgm1,
    const dgMatrix<Type>& dgm2,
    const char* op
)
{
    if ((&dgm1.psi() != &dgm2.psi()) && (dgm1.operatorOrder() && dgm2.operatorOrder()))
    {
        FatalErrorInFunction
            << "incompatible fields for operation "
            << endl << "    "
            << "[" << dgm1.psi().name() << "] "
            << op
            << " [" << dgm2.psi().name() << "]"
            << abort(FatalError);
    }

    if (dimensionSet::debug && dgm1.dimensions() != dgm2.dimensions())
    {
        FatalErrorInFunction
            << "incompatible dimensions for operation "
            << endl << "    "
            << "[" << (dgm1.operatorOrder()?dgm1.psi().name():"matrix1(zero operatorOrder)") << dgm1.dimensions() << " ] "
            << op
            << " [" << (dgm2.operatorOrder()?dgm2.psi().name():"matrix2(zero operatorOrder)") << dgm2.dimensions() << " ]"
            << abort(FatalError);
    }
}


template<class Type>
void Foam::checkMethod
(
    const dgMatrix<Type>& dgm,
    const DimensionedField<Type, dgGeoMesh>& df,
    const char* op
)
{
    if (dimensionSet::debug && dgm.dimensions() != df.dimensions())
    {
        FatalErrorInFunction
            << endl << "    "
            << "[" << dgm.psi().name() << dgm.dimensions() << " ] "
            << op
            << " [" << df.name() << df.dimensions() << " ]"
            << abort(FatalError);
    }
}


template<class Type>
void Foam::checkMethod
(
    const dgMatrix<Type>& dgm,
    const dimensioned<Type>& dt,
    const char* op
)
{
    if (dimensionSet::debug && dgm.dimensions() != dt.dimensions())
    {
        FatalErrorInFunction
            << "incompatible dimensions for operation "
            << endl << "    "
            << "[" << dgm.psi().name() << dgm.dimensions() << " ] "
            << op
            << " [" << dt.name() << dt.dimensions() << " ]"
            << abort(FatalError);
    }
}


template<class Type>
Foam::SolverPerformance<Type> Foam::solve
(
    dgMatrix<Type>& dgm,
    const dictionary& solverControls
)
{
    return dgm.solve(solverControls, dgm.kspSolver());
}

template<class Type>
Foam::SolverPerformance<Type> Foam::solve
(
    const tmp<dgMatrix<Type> >& tdgm,
    const dictionary& solverControls
)
{
    SolverPerformance<Type> solverPerf =
        const_cast<dgMatrix<Type>&>(tdgm()).solve(solverControls,
        const_cast<dgMatrix<Type>&>(tdgm()).kspSolver());

    tdgm.clear();

    return solverPerf;
}


template<class Type>
Foam::SolverPerformance<Type> Foam::solve(dgMatrix<Type>& dgm)
{
    return dgm.solve();
}

template<class Type>
Foam::SolverPerformance<Type> Foam::solve(dgMatrix<Type>& dgm, KSP ksp)
{
    return dgm.solve(ksp);
}

template<class Type>
Foam::SolverPerformance<Type> Foam::solve(const tmp<dgMatrix<Type> >& tdgm)
{
    SolverPerformance<Type> solverPerf =
        const_cast<dgMatrix<Type>&>(tdgm.ref()).solve();

    tdgm.clear();

    return solverPerf;
}

template<class Type>
Foam::SolverPerformance<Type> Foam::solve(const tmp<dgMatrix<Type> >& tdgm, KSP ksp)
{
    SolverPerformance<Type> solverPerf =
        const_cast<dgMatrix<Type>&>(tdgm.ref()).solve(ksp);

    tdgm.clear();

    return solverPerf;
}


// * * * * * * * * * * * * * * * IOstream Operators  * * * * * * * * * * * * //

template<class Type>
Foam::Ostream& Foam::operator<<(Ostream& os, const dgMatrix<Type>& dgm)
{
    if(dgm.operatorOrder() > 0){
        os << "PETSc mat:"<<nl;
        MatView(dgm.mat_, PETSC_VIEWER_STDOUT_SELF);
    }
    os << "b" << nl << dgm.b_ << nl;
    
    os.check("Ostream& operator<<(Ostream&, dgMatrix<Type>&");

    return os;
}

// * * * * * * * * * * * * * * * * Solvers * * * * * * * * * * * * * * * * * //

#include "dgMatrixSolve.C"

// ************************************************************************* //
