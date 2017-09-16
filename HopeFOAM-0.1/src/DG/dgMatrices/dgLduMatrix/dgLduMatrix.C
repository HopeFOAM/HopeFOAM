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

Author
    Xu Liyang (xucloud77@gmail.com)
\*---------------------------------------------------------------------------*/

#include "IOstreams.H"
#include "Switch.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //


#define MAT_ALL_OP(f1, f2, OP)                            \
                           \
    /* loop through fields performing f1 OP f2 */         \
    forAll(f1, i){                                       \
        f1[i] OP f2[i];                        \
    }


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
template<class Type>
Foam::dgLduMatrix<Type>::dgLduMatrix(const dgMesh& mesh, label order)
:
    mesh_(mesh),
    diagCoeff_(0),
    diag_(),
    lu_(),
    lu2_(),
    sourceSize_(0)
{
	matrixPreallocation(order);
}

template<class Type>
Foam::dgLduMatrix<Type>::dgLduMatrix(const dgLduMatrix<Type>& A)
:
    mesh_(A.mesh_),
    diagCoeff_(A.diagCoeff_),
    diag_(),
    lu_(),
    lu2_(),
    sourceSize_(0)
{
    if (A.hasLu())
    {
        lu_ = A.lu_;
    }
    if (A.hasLu2())
    {
        lu2_ = A.lu2_;
    }

    if (A.hasDiag())
    {
        diag_ = A.diag_;
    }
}

template<class Type>
Foam::dgLduMatrix<Type>::dgLduMatrix(dgLduMatrix<Type>& A, bool reuse)
:
    mesh_(A.mesh_),
    diagCoeff_(A.diagCoeff_),
    diag_(),
    lu_(),
    lu2_(),
    sourceSize_(0)
{
    if (reuse)
    {
        if (A.hasLu())
        {
            lu_.transfer(A.lu_);
        }
        if (A.hasLu2())
        {
            lu2_.transfer(A.lu2_);
        }

        if (A.hasDiag())
        {
            diag_.transfer(A.diag_);
        }
    }
    else
    {
        if (A.hasLu())
        {
            lu_ = A.lu_;
        }
        if (A.hasLu2())
        {
            lu2_ = A.lu2_;
        }

        if (A.hasDiag())
        {
            diag_ = A.diag_;
        }
    }
}

template<class Type>
Foam::dgLduMatrix<Type>::~dgLduMatrix()
{
}
/*
void Foam::dgLduMatrix::generatePetscMat(Mat& C)
{
    PetscErrorCode ierr;

	//create petsc Matrix
    if (Pstream::parRun()){
        MatCreate(MPI_COMM_WORLD,&C);
        MatSetSizes(C, mesh_.totalDof(), mesh_.totalDof(), PETSC_DECIDE, PETSC_DECIDE);
        MatSetType(C, MATMPIAIJ);
        MatMPIAIJSetPreallocation(C, 0, d_nnz_.data(), 0, o_nnz_.data());
    }
    else
        ierr = MatCreateSeqAIJ(MPI_COMM_WORLD,mesh_.totalDof(),mesh_.totalDof(),NULL,d_nnz_.data(),&C);
    ierr = MatSetFromOptions(C);
    ierr = MatSetUp(C);

    //mat set value
    labelList rows(max(rowSize_)), cols(max(colSize_));

    const label nElement = mesh_.nCells();
    //const Pair<label> localRange = mesh_.getLocalRange();
    const labelPairList& cellDofIndex = mesh_.cellDofIndexPair();

    //set Matrix diag part
    label globalShift = mesh_.localRange().first();
    if(diagPtr_){
        for(label cellI=0; cellI < nElement; cellI ++){
            //set rows and cols diag
            //PetscInt start = nrow*cellI + localRange.first();
            for(int i=0; i<rowSize_[cellI]; i++){
                rows[i] = cellDofIndex[cellI][0] + i + globalShift;
                cols[i] = rows[i];
            }

            //mat set value
            ierr = MatSetValues(C, rowSize_[cellI], rows.data(), rowSize_[cellI], cols.data(), (*diagPtr_)[cellI].data(), ADD_VALUES);
        }
    }

    const List<dgFace>& dgFaceList = mesh_.dgFaceList();
    forAll(dgFaceList, faceI){
        const dgFace& dgFaceI = dgFaceList[faceI];
        if(dgFaceI.faceNeighbor_==-1)continue;
        // add upper
        if(upperPtr_ && (*upperPtr_)[faceI].size()>0){
            labelList faceIndexO(SubList<label>((dgFaceI.ownerBase_)->faceToCellIndex(), dgFaceI.nOwnerDof_, dgFaceI.nOwnerDof_*dgFaceI.faceOwnerLocalID_));
            labelList faceIndexN(SubList<label>((dgFaceI.neighborBase_)->faceToCellIndex(), dgFaceI.nNeighborDof_, dgFaceI.nNeighborDof_*dgFaceI.faceNeighborLocalID_));
            label faceNeighbor_ = dgFaceI.faceNeighbor_;
            if(dgFaceI.faceNeighbor_<-1)
                faceNeighbor_ = -2 - dgFaceI.faceNeighbor_;
            if(dgFaceI.faceOwner_ < faceNeighbor_){
                for(int i=0, ptr=cellDofIndex[dgFaceI.faceOwner_][0]; i<dgFaceI.nOwnerDof_; i++)
                    rows[i] = ptr + faceIndexO[i];
                for(int i=0, ptr=cellDofIndex[faceNeighbor_][0]; i<dgFaceI.nNeighborDof_; i++)
                    cols[i] = ptr + faceIndexN[i];
                ierr = MatSetValues(C, dgFaceI.nOwnerDof_, rows.data(), dgFaceI.nNeighborDof_, cols.data(), (*upperPtr_)[faceI].data(), ADD_VALUES);
            }
            else{
                for(int i=0, ptr=cellDofIndex[faceNeighbor_][0]; i<dgFaceI.nNeighborDof_; i++)
                    rows[i] = ptr + faceIndexN[i];
                for(int i=0, ptr=cellDofIndex[dgFaceI.faceOwner_][0]; i<dgFaceI.nOwnerDof_; i++)
                    cols[i] = ptr + faceIndexO[i];
                ierr = MatSetValues(C, dgFaceI.nNeighborDof_, rows.data(), dgFaceI.nOwnerDof_, cols.data(), (*upperPtr_)[faceI].data(), ADD_VALUES);
            }
        }

        // add upper2
        if(upperPtr2_ && (*upperPtr2_)[faceI].size()>0){
            label faceNeighbor_ = dgFaceI.faceNeighbor_;
            if(dgFaceI.faceNeighbor_<-1)
                faceNeighbor_ = -2 - dgFaceI.faceNeighbor_;
            if(dgFaceI.faceOwner_ < faceNeighbor_){
                for(int i=0, ptr=cellDofIndex[dgFaceI.faceOwner_][0]; i<cellDofIndex[dgFaceI.faceOwner_][1]; i++, ptr++)
                    rows[i] = ptr;
                for(int i=0, ptr=cellDofIndex[faceNeighbor_][0]; i<cellDofIndex[faceNeighbor_][1]; i++, ptr++)
                    cols[i] = ptr;
                ierr = MatSetValues(C, cellDofIndex[dgFaceI.faceOwner_][1], rows.data(), cellDofIndex[faceNeighbor_][1], cols.data(), (*upperPtr2_)[faceI].data(), ADD_VALUES);
            }
            else{
                for(int i=0, ptr=cellDofIndex[faceNeighbor_][0]; i<cellDofIndex[faceNeighbor_][1]; i++, ptr++)
                    rows[i] = ptr;
                for(int i=0, ptr=cellDofIndex[dgFaceI.faceOwner_][0]; i<cellDofIndex[dgFaceI.faceOwner_][1]; i++, ptr++)
                    cols[i] = ptr;
                ierr = MatSetValues(C, cellDofIndex[faceNeighbor_][1], rows.data(), cellDofIndex[dgFaceI.faceOwner_][1], cols.data(), (*upperPtr2_)[faceI].data(), ADD_VALUES);
            }
        }

        // add lower
        if(lowerPtr_ && (*lowerPtr_)[faceI].size()>0){
            labelList faceIndexO(SubList<label>((dgFaceI.ownerBase_)->faceToCellIndex(), dgFaceI.nOwnerDof_, dgFaceI.nOwnerDof_*dgFaceI.faceOwnerLocalID_));
            labelList faceIndexN(SubList<label>((dgFaceI.neighborBase_)->faceToCellIndex(), dgFaceI.nNeighborDof_, dgFaceI.nNeighborDof_*dgFaceI.faceNeighborLocalID_));
            label faceNeighbor_ = dgFaceI.faceNeighbor_;
            if(dgFaceI.faceNeighbor_<-1)
                faceNeighbor_ = -2 - dgFaceI.faceNeighbor_;
            if(dgFaceI.faceOwner_ > faceNeighbor_){
                for(int i=0, ptr=cellDofIndex[dgFaceI.faceOwner_][0]; i<dgFaceI.nOwnerDof_; i++)
                    rows[i] = ptr + faceIndexO[i];
                for(int i=0, ptr=cellDofIndex[faceNeighbor_][0]; i<dgFaceI.nNeighborDof_; i++)
                    cols[i] = ptr + faceIndexN[i];
                ierr = MatSetValues(C, dgFaceI.nOwnerDof_, rows.data(), dgFaceI.nNeighborDof_, cols.data(), (*lowerPtr_)[faceI].data(), ADD_VALUES);
            }
            else{
                for(int i=0, ptr=cellDofIndex[faceNeighbor_][0]; i<dgFaceI.nNeighborDof_; i++)
                    rows[i] = ptr + faceIndexN[i];
                for(int i=0, ptr=cellDofIndex[dgFaceI.faceOwner_][0]; i<dgFaceI.nOwnerDof_; i++)
                    cols[i] = ptr + faceIndexO[i];
                ierr = MatSetValues(C, dgFaceI.nNeighborDof_, rows.data(), dgFaceI.nOwnerDof_, cols.data(), (*lowerPtr_)[faceI].data(), ADD_VALUES);
            }
        }

        // add lower2
        if(lowerPtr2_ && (*lowerPtr2_)[faceI].size()>0){
            label faceNeighbor_ = dgFaceI.faceNeighbor_;
            if(dgFaceI.faceNeighbor_<-1)
                faceNeighbor_ = -2 - dgFaceI.faceNeighbor_;
            if(dgFaceI.faceOwner_ > faceNeighbor_){
                for(int i=0, ptr=cellDofIndex[dgFaceI.faceOwner_][0]; i<cellDofIndex[dgFaceI.faceOwner_][1]; i++, ptr++)
                    rows[i] = ptr;
                for(int i=0, ptr=cellDofIndex[faceNeighbor_][0]; i<cellDofIndex[faceNeighbor_][1]; i++, ptr++)
                    cols[i] = ptr;
                ierr = MatSetValues(C, cellDofIndex[dgFaceI.faceOwner_][1], rows.data(), cellDofIndex[faceNeighbor_][1], cols.data(), (*lowerPtr2_)[faceI].data(), ADD_VALUES);
            }
            else{
                for(int i=0, ptr=cellDofIndex[faceNeighbor_][0]; i<cellDofIndex[faceNeighbor_][1]; i++, ptr++)
                    rows[i] = ptr;
                for(int i=0, ptr=cellDofIndex[dgFaceI.faceOwner_][0]; i<cellDofIndex[dgFaceI.faceOwner_][1]; i++, ptr++)
                    cols[i] = ptr;
                ierr = MatSetValues(C, cellDofIndex[faceNeighbor_][1], rows.data(), cellDofIndex[dgFaceI.faceOwner_][1], cols.data(), (*lowerPtr2_)[faceI].data(), ADD_VALUES);
            }
        }
    }

    ierr = MatAssemblyBegin(C,MAT_FINAL_ASSEMBLY);
    ierr = MatAssemblyEnd(C,MAT_FINAL_ASSEMBLY);

    //ierr = MatView(C,PETSC_VIEWER_STDOUT_(MPI_COMM_WORLD));
}*/

template<class Type>
void Foam::dgLduMatrix<Type>::matrixPreallocation(label operatorOrder)
{
    operatorOrder_ = operatorOrder;
    b_.setSize(mesh_.maxDofPerCell(), pTraits<Type>::zero);
    source_.setSize(mesh_.maxDofPerCell(), pTraits<Type>::zero);

    switch(operatorOrder){
    case 0:
        //for 0-order operator: ddt, sp.....
        break;

    case 1:
        //for more than 1-order operator: 
    	diag_.setSize(mesh_.maxDofPerCell(), mesh_.maxDofPerCell());
        break;

    case 2:
        //for 2-order operator: div, grad......
        //only need 1 neighbor point information
        diag_.setSize(mesh_.maxDofPerCell(), mesh_.maxDofPerCell());
        lu_.setSize(mesh_.maxNFacesPerCell());
        forAll(lu_, faceI){
            lu_[faceI].setSize(mesh_.maxDofPerCell(), mesh_.maxDofPerFace());
        }
        break;
    case 3:
        //for 3-order operator: laplacian......
        //need 1 level all around cells points
        diag_.setSize(mesh_.maxDofPerCell(), mesh_.maxDofPerCell());
        lu2_.setSize(mesh_.maxNFacesPerCell());
        forAll(lu2_, faceI){
            lu2_[faceI].setSize(mesh_.maxDofPerCell(), mesh_.maxDofPerCell());
        }
        break;
    default:
        FatalErrorIn
        (
            "dgMatrix::matrixPreallocation(label)"
        )   << "operator order = " << operatorOrder << " not supported." << nl
            << abort(FatalError);
    }
}

template<class Type>
void Foam::dgLduMatrix<Type>::addSourceToB(const physicalCellElement& cellEle)
{
    if(sourceSize_ > 0){
        denseMatrix<Type>::MatVecMultAdd(cellEle.massMatrix(), SubList<Type>(source_, cellEle.nDof()), SubList<Type>(b_, cellEle.nDof()));
    }
}

/*
void Foam::dgLduMatrix::refreshPreallocate(const dgLduMatrix& mat)
{
    rowSize_ = mat.rowSize();
    colSize_ = mat.colSize();
    d_nnz_ = mat.d_nnz();
    o_nnz_ = mat.o_nnz();
}*/

template<class Type>
Foam::List<Foam::denseMatrix<Foam::scalar>>& Foam::dgLduMatrix<Type>::lu()
{
    return lu_;
}

template<class Type>
Foam::List<Foam::denseMatrix<Foam::scalar>>& Foam::dgLduMatrix<Type>::lu2()
{
    return lu2_;
}

template<class Type>
Foam::denseMatrix<Foam::scalar>& Foam::dgLduMatrix<Type>::diag()
{
    return diag_;
}

template<class Type>
const Foam::List<Foam::denseMatrix<Foam::scalar>>& Foam::dgLduMatrix<Type>::lu() const
{
    return lu_;
}

template<class Type>
const Foam::List<Foam::denseMatrix<Foam::scalar>>& Foam::dgLduMatrix<Type>::lu2() const
{
    return lu2_;
}

template<class Type>
const Foam::denseMatrix<Foam::scalar>& Foam::dgLduMatrix<Type>::diag() const
{
    return diag_;
}

/*
void Foam::dgLduMatrix::relax(const scalar alpha)
{
    scalar scale = (1.0 - alpha)/alpha;
    forAll(*diagPtr_, cellI){
        denseMatrix<scalar>& matrixI = (*diagPtr_)[cellI];
        for(int i=0, ptr=0; i<rowSize_[cellI]; i++, ptr+=(rowSize_[cellI] + 1))
            matrixI[ptr] += scale*matrixI[ptr];
    }
    // forAll(*diagPtr_, cellI){
    //     denseMatrix<scalar>& matrixI = (*diagPtr_)[cellI];
    //     for(int ptr=0; ptr < matrixI.size(); ptr++)
    //         matrixI[ptr] += matrixI[ptr] * scale;
    // }
}*/
/*
Foam::tmp<Foam::scalarField>
Foam::dgLduMatrix::D()const
{
    tmp<Field<scalar>> tdiag(new scalarField(mesh_.totalDof()));
    Field<scalar> diag = tdiag.ref();
    label index = 0;
    forAll(*diagPtr_, cellI){
        denseMatrix<scalar>& matrixI = (*diagPtr_)[cellI];
        for(int i=0, ptr=0; i<rowSize_[cellI]; i++, ptr+=(rowSize_[cellI] + 1), index++)
            diag[index] = matrixI[ptr];
    }
    return tdiag;
}*/

template<class Type>
void Foam::dgLduMatrix<Type>::operator=(const dgLduMatrix<Type>& A)
{
    if (this == &A)
    {
        FatalError
            << "dgLduMatrix::operator=(const dgLduMatrix&) : "
            << "attempted assignment to self"
            << abort(FatalError);
    }
    this->diagCoeff_ = (A.diagCoeff_);

    if (A.hasLu())
    {
        lu() = A.lu();
    }
    else if (hasLu())
    {
        lu_.clear();
    }

    if (A.hasLu2())
    {
        lu2() = A.lu2();
    }
    else if (hasLu2())
    {
        lu2_.clear();
    }

    if (A.hasDiag())
    {
        diag() = A.diag();
    }
    else if(hasDiag()){
        diag_.clear();
    }
}

template<class Type>
void Foam::dgLduMatrix<Type>::negate()
{
	/*if (lowerPtr_)
    {
        forAll(*lowerPtr_, faceI){
        	forAll((*lowerPtr_)[faceI], pointI)
        		(*lowerPtr_)[faceI][pointI] = -(*lowerPtr_)[faceI][pointI];
        }
    }

    if (lowerPtr2_)
    {
        forAll(*lowerPtr2_, faceI){
            forAll((*lowerPtr2_)[faceI], pointI)
                (*lowerPtr2_)[faceI][pointI] = -(*lowerPtr2_)[faceI][pointI];
        }
    }

    if (upperPtr_)
    {
        forAll(*upperPtr_, faceI){
        	forAll((*upperPtr_)[faceI], pointI)
        		(*upperPtr_)[faceI][pointI] = -(*upperPtr_)[faceI][pointI];
        }
    }

    if (upperPtr2_)
    {
        forAll(*upperPtr2_, faceI){
            forAll((*upperPtr2_)[faceI], pointI)
                (*upperPtr2_)[faceI][pointI] = -(*upperPtr2_)[faceI][pointI];
        }
    }

    if (diagPtr_)
    {
        forAll(*diagPtr_, cellI){
        	forAll((*diagPtr_)[cellI], pointI)
        		(*diagPtr_)[cellI][pointI] = -(*diagPtr_)[cellI][pointI];
        }
    }*/
    FatalErrorIn
        (
            "Foam::dgLduMatrix::negate()"
        )   << "xuliyang->completeThis()"
            << abort(FatalError);
}

template<class Type>
void Foam::dgLduMatrix<Type>::operator+=(const dgLduMatrix& A)
{
    this->diagCoeff_ += (A.diagCoeff_);
    if (A.hasDiag())
    {
        if(hasDiag())
            diag() += A.diag();
        else
            diag() = A.diag();
    }

    if(A.hasLu()){
        if(hasLu())
            {MAT_ALL_OP(lu(), A.lu(), +=);}
        else
            lu() = A.lu();
    }
    if(A.hasLu2()){
        if(hasLu2())
            {MAT_ALL_OP(lu2(), A.lu2(), +=);}
        else
            lu2() = A.lu2();
    }

    if(A.sourceSize()>0){
        if(sourceSize_ == 0){
            sourceSize_ = A.sourceSize();
            for(int i=0; i<sourceSize_; i++){
                source_[i] = A.source_[i];
                b_[i] = A.b_[i];
            }
        }
        else if(sourceSize_ != A.sourceSize()){
            FatalErrorIn
            (
                "Foam::dgLduMatrix<Type>::operator+=(const dgLduMatrix& A)"
            )   << __FILE__<<": "<<__LINE__<<"; this->sourceSize_ = "<<sourceSize_<<"while A.sourceSize_ = "<<A.sourceSize() << nl
            << abort(FatalError);
        }
        else{
            for(int i=0; i<sourceSize_; i++){
                source_[i] += A.source_[i];
                b_[i] += A.b_[i];
            }
        }
    }
}

template<class Type>
void Foam::dgLduMatrix<Type>::operator-=(const dgLduMatrix& A)
{
    this->diagCoeff_ -= (A.diagCoeff_);
    if (A.hasDiag())
    {
        if(hasDiag())
            diag() -= A.diag();
        else{
            diag() = A.diag();
            for(int pointI = 0; pointI< diag_.actualSize(); pointI++)
                diag_[pointI] = -diag_[pointI];
        }
    }

    if(A.hasLu()){
        if(hasLu())
            {MAT_ALL_OP(lu(), A.lu(), -=);}
        else{
            lu() = A.lu();
            forAll(lu_, faceI){
                for(int pointI = 0; pointI < lu_[faceI].actualSize(); pointI++)
                    lu_[faceI][pointI] = -lu_[faceI][pointI];
            }
        }
    }
    if(A.hasLu2()){
        if(hasLu2())
            {MAT_ALL_OP(lu2(), A.lu2(), -=);}
        else{
            lu2() = A.lu2();
            forAll(lu2_, faceI){
                for(int pointI = 0; pointI < lu2_[faceI].actualSize(); pointI++)
                    lu2_[faceI][pointI] = -lu2_[faceI][pointI];
            }
        }
    }

    if(A.sourceSize()>0){
        if(sourceSize_ == 0){
            sourceSize_ = A.sourceSize();
            for(int i=0; i<sourceSize_; i++){
                source_[i] = -A.source_[i];
                b_[i] = -A.b_[i];
            }
        }
        else if(sourceSize_ != A.sourceSize()){
            FatalErrorIn
            (
                "Foam::dgLduMatrix<Type>::operator+=(const dgLduMatrix& A)"
            )   << __FILE__<<": "<<__LINE__<<"; this->sourceSize_ = "<<sourceSize_<<"while A.sourceSize_ = "<<A.sourceSize() << nl
            << abort(FatalError);
        }
        else{
            for(int i=0; i<sourceSize_; i++){
                source_[i] -= A.source_[i];
                b_[i] -= A.b_[i];
            }
        }
    }
}

template<class Type>
void Foam::dgLduMatrix<Type>::operator*=(Foam::scalar s)
{
    this->diagCoeff_ *= s;
    if (hasDiag())
    {
        diag_ *= s;
    }

    if (hasLu())
    {
        forAll(lu_, cellI){
        	lu_[cellI] *= s;
        }
    }
    if (hasLu2())
    {
        forAll(lu2_, cellI){
            lu2_[cellI] *= s;
        }
    }
    if(sourceSize_ > 0){
        for(int i=0; i<sourceSize_; i++){
            source_[i] *= s;
            b_[i] *= s;
        }
    }
}


// * * * * * * * * * * * * * * * Friend Operators  * * * * * * * * * * * * * //
template<class Type>
Foam::Ostream& Foam::operator<<(Ostream& os, const dgLduMatrix<Type>& ldum)
{
    Switch hasLu = ldum.hasLu();
    Switch hasLu2 = ldum.hasLu2();
    Switch hasDiag = ldum.hasDiag();

    os  << hasLu << token::SPACE << hasLu2 << token::SPACE << hasDiag << token::SPACE;

    if (hasLu)
    {
        os  << "lu"<<endl<<ldum.lu();
    }
    if (hasLu2)
    {
        os  << "lu2" <<endl<<ldum.lu2();
    }

    if (hasDiag)
    {
        os  << "Diag"<<endl<< ldum.diag();
    }

    os.check("Ostream& operator<<(Ostream&, const dgLduMatrix&");

    return os;
}

template<class Type>
Foam::Ostream& Foam::operator<<(Ostream& os, const InfoProxy<dgLduMatrix<Type>>& ip)
{
    const dgLduMatrix<Type>& ldum = ip.t_;

    Switch hasLu = ldum.hasLu();
    Switch hasLu2 = ldum.hasLu2();
    Switch hasDiag = ldum.hasDiag();

    os  << " Lu:" << hasLu
        << " Lu2:" << hasLu2
        << " Diag:" << hasDiag << endl;

    if (hasLu)
    {
        os  << "lu:" << ldum.lu() << endl;
    }
    if (hasLu2)
    {
        os  << "lu2:" << ldum.lu2() << endl;
    }
    if (hasDiag)
    {
        os  << "diag :" << ldum.diag() << endl;
    }

    os.check("Ostream& operator<<(Ostream&, const dgLduMatrix&");

    return os;
}


// ************************************************************************* //
