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
#include "dgMesh.H"
#include "demandDrivenData.H"
#include "MeshObject.H"
#include "denseMatrix.H"

#include <petscksp.h>

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
defineTypeNameAndDebug(dgMesh, 0);
}


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void Foam::dgMesh::clearGeomNotOldVol()
{
	//deleteDemandDrivenData(SfPtr_);
}

void Foam::dgMesh::updateGeomNotOldVol()
{
	//bool haveSf = (SfPtr_ != NULL);
	clearGeomNotOldVol();
}

void Foam::dgMesh::clearGeom()
{
	clearGeomNotOldVol();

	// Mesh motion flux cannot be deleted here because the old-time flux
	// needs to be saved.
}

void Foam::dgMesh::clearOut()
{
	clearGeom();
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::dgMesh::dgMesh(const IOobject& io)
	:
	dgPolyMesh(io),
	dgSchemes(static_cast<const objectRegistry&>(*this)),
	dgSolution(static_cast<const objectRegistry&>(*this)),
	data(static_cast<const objectRegistry&>(*this)),
	physicalElementData((const dgPolyMesh&)(*this)),
	boundary_(*this, polyMesh::boundaryMesh()),
	massMatrixSolver_(NULL)
	
{
	PetscBool ifInit;
	PetscInitialized(&ifInit);
	if(!ifInit){
		int argc = 2;
		char **argv = new char*[3];
		argv[1] = new char[20];
		sprintf(argv[1], "-no_signal_handler");
		PetscInitialize(&argc, &argv, (char *)0, "");
	}

	if (debug) {
		Info<< "Constructing dgMesh from IOobject"
		    << endl;
	}

	const dictionary& dgDict = Foam::dgSolution::solutionDict().subDict("DG");
	if(!dgDict.found("meshDimension") || !dgDict.found("baseOrder")) {
		FatalErrorIn("dgMesh::dgMesh()")
		        << "DG parameters (meshDimension and baseOrder) have not been set"<<endl
		        << abort(FatalError);
	}

	label meshDim_ = meshDim();
	int order = dgDict.lookupOrDefault<int>("baseOrder", 2);

	string newStr("applyCurveBoundary");
    word newW(newStr);
	if(!io.db().foundObject<regIOobject>(newW)){
		initElements(order);
		updateLocalRange();
		boundary_.updateTotalDofNum();
		updatePatchDofIndexMapping(boundary_);
		boundary_.updateNeighborData();
	}
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::dgMesh::~dgMesh()
{
	clearOut();
}

// * * * * * * * * * * * * * * * * Member Function  * * * * * * * * * * * * * * * //


KSP Foam::dgMesh::massMatrixSolver()
{
	if(massMatrixSolver_ == NULL){
		Info << "Init KSP massMatrixSolver"<<endl;
		Mat C;
		PetscErrorCode ierr;
		label totalDof = physicalElementData::totalDof();
		labelList nnz(totalDof);
		label index = 0;
		for(dgTree<physicalCellElement>::leafIterator iter = physicalElementData::cellElementsTree().leafBegin();
			iter != physicalElementData::cellElementsTree().end(); ++iter){
            label nDof = iter()->value().nDof();
        	for(int i=0; i<nDof; i++, index++)
				nnz[index] = nDof;
        }
		ierr = MatCreateSeqAIJ(PETSC_COMM_SELF, totalDof, totalDof, NULL, nnz.data(), &C);

		labelList rows(max(nnz));
		for(dgTree<physicalCellElement>::leafIterator iter = physicalElementData::cellElementsTree().leafBegin();
			iter != physicalElementData::cellElementsTree().end(); ++iter){
			label dofStart = iter()->value().dofStart();
			label nDof = iter()->value().nDof();
			for(int i=0; i<nDof; i++, dofStart++){
                rows[i] = dofStart;
            }
            //scalarList massmatrix(cdof * cdof, 0.0);
            // A_diag = Vr' * diag(diagWJ) * Vr
            const denseMatrix<scalar>& massMatrix = iter()->value().massMatrix();

            ierr = MatSetValues(C, nDof, rows.data(), nDof, rows.data(), const_cast<denseMatrix<scalar>&>(massMatrix).data(), INSERT_VALUES);
		}
		ierr = MatAssemblyBegin(C,MAT_FINAL_ASSEMBLY);
    	ierr = MatAssemblyEnd(C,MAT_FINAL_ASSEMBLY);

		ierr = KSPCreate(PETSC_COMM_SELF, &massMatrixSolver_);
	    ierr = KSPSetOperators(massMatrixSolver_,C,C);
	    ierr = KSPSetType(massMatrixSolver_,"preonly");
	    ierr = KSPSetTolerances(massMatrixSolver_,1.e-15,1.e-15,PETSC_DEFAULT,200);
	    ierr = KSPSetFromOptions(massMatrixSolver_);
	    ierr = KSPSetUp(massMatrixSolver_);
	}

	return massMatrixSolver_;
}


bool Foam::dgMesh::writeObjects
(
    IOstream::streamFormat fmt,
    IOstream::versionNumber ver,
    IOstream::compressionType cmp
) const
{
	return writeObjects(fmt, ver, cmp);
}

//- Write mesh using IO settings from the time
bool Foam::dgMesh::write() const
{
	bool ok = false;

	return ok;
}


void Foam::dgMesh::updateLocalRange()
{
	PetscErrorCode ierr;
    label n0=0, n1;
    if(Pstream::parRun()){
        PetscBool ifInit;
        PetscInitialized(&ifInit);
		if(!ifInit){
			int argc = 2;
			char **argv = new char*[3];
			argv[1] = new char[20];
			sprintf(argv[1], "-no_signal_handler");
			PetscInitialize(&argc, &argv, (char *)0, "");
		}
        Vec vecTemplate_;
        VecCreateMPI(MPI_COMM_WORLD, this->totalDof(), PETSC_DECIDE, &vecTemplate_);
        VecAssemblyBegin(vecTemplate_);
        VecAssemblyEnd(vecTemplate_);
        VecGetOwnershipRange(vecTemplate_, &n0, &n1);
        VecDestroy(&vecTemplate_);
    }
    else
        n1 = this->totalDof();

    localRange_ = Pair<label>(n0,n1);
}


// * * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * //

bool Foam::dgMesh::operator!=(const dgMesh& bm) const
{
	return &bm != this;
}


bool Foam::dgMesh::operator==(const dgMesh& bm) const
{
	return &bm == this;
}

// ************************************************************************* //
