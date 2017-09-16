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

#include "denseMatrix.H"
#include "SubList.H"
#include <petscksp.h>

// * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * * //
template<class Type>
inline Foam::denseMatrix<Type>::denseMatrix()
:
	List<Type>(),
	rowSize_(0),
	colSize_(0)
{}

template<class Type>
Foam::denseMatrix<Type>::denseMatrix(const label r, const label c)
:
    List<Type>(r*c),
    rowSize_(r),
    colSize_(c)
{
    if (r < 0 || c < 0)
    {
        FatalErrorInFunction
            << "bad set size row = " << r
            << ", colume = "<< c
            << abort(FatalError);
    }
}


template<class Type>
Foam::denseMatrix<Type>::denseMatrix(const label r, const label c, const Type& a)
:
    List<Type>(r*c, a),
    rowSize_(r),
    colSize_(c)
{
    if (r < 0 || c < 0)
    {
        FatalErrorInFunction
            << "bad set size row = " << r
            << ", colume = "<< c
            << abort(FatalError);
    }
}

template<class Type>
Foam::denseMatrix<Type>::denseMatrix(const denseMatrix<Type>& a)
:
    List<Type>(a),
    rowSize_(a.rowSize()),
    colSize_(a.colSize())
{
}


template<class Type>
Foam::denseMatrix<Type>::denseMatrix(const List<List<Type>>& lst)
:
    List<Type>(lst.size()*lst[0].size()),
    rowSize_(lst.size()),
    colSize_(lst[0].size())
{
    for(int i=0; i<rowSize_; i++){
        if(lst[i].size() != colSize_){
            FatalErrorInFunction
            << "bad colume size in list[= " << i
            << "], with others' colume size = "<< colSize_
            << abort(FatalError);
        }
        SubList<Type> subA(*this, colSize_, i*colSize_);
        subA = lst[i];
    }
}



template<class Type>
void Foam::denseMatrix<Type>::setSize(const label newR, const label newC)
{
    if (newR < 0 || newC < 0)
    {
        FatalErrorInFunction
            << "bad set size row = " << newR
            << ", colume = "<< newC
            << abort(FatalError);
    }
    rowSize_ = newR;
    colSize_ = newC;

    List<Type>::setSize(newR*newC);

}


template<class Type>
void Foam::denseMatrix<Type>::setSize(const label newR, const label newC, const Type& a)
{
    if (newR < 0 || newC < 0)
    {
        FatalErrorInFunction
            << "bad set size row = " << newR
            << ", colume = "<< newC
            << abort(FatalError);
    }

    rowSize_ = newR;
    colSize_ = newC;

    List<Type>::setSize(newR*newC, a);
}


template<class Type>
inline void Foam::denseMatrix<Type>::resize(const label newR, const label newC)
{
    this->setSize(newR, newC);
}


template<class Type>
inline void Foam::denseMatrix<Type>::resize(const label newR, const label newC, const Type& a)
{
    this->setSize(newR, newC, a);
}

template<class Type>
void Foam::denseMatrix<Type>:: resetLabel(const label newR, const label newC)
{
    rowSize_ = newR;
    colSize_ = newC;
}

template<class Type>
void Foam::denseMatrix<Type>::setToUnitMatrix(const label newR)
{
    this->setSize(newR, newR, pTraits<Type>::zero);
    for(int i=0, ptr=0; i<newR; i++, ptr+=(newR+i)){
        List<Type>::operator[](ptr) = pTraits<Type>::one;
    }
}

template<class Type>
void Foam::denseMatrix<Type>::setToZeroMatrix(){
    label actualize = rowSize_ * colSize_;
    for(int i=0; i<actualize; i++){
        List<Type>::operator[](i) = pTraits<Type>::zero;
    }
}

template<class Type>
void Foam::denseMatrix<Type>::clear()
{
	List<Type>::clear();
}

template<class Type>
void Foam::denseMatrix<Type>::operator=(const denseMatrix<Type>& a)
{
    int actualize = this->size();
    rowSize_ = a.rowSize();
    colSize_ = a.colSize();

    if(actualize == 0){
        List<Type>::operator=(a);
        return;
    }

    if (actualize < (a.rowSize()*a.colSize()))
    {
        FatalErrorInFunction
            << "attempted assignment to a larger matrix, space not enough."
            << abort(FatalError);
    }

    //List<Type>::operator=(a);
    SubList<Type> subL(*this, rowSize_*colSize_);
    subL = a;
}

template<class Type>
void Foam::denseMatrix<Type>::operator=(const Type& a)
{
    SubList<Type> subL(*this, rowSize_*colSize_);
    subL = a;
}

template<class Type>
void Foam::denseMatrix<Type>::operator=(const List<List<Type>>& lst)
{
    rowSize_ = lst.size();
    colSize_ = lst[0].size();

    List<Type>::setSize(rowSize_*colSize_);

    for(int i=0; i<rowSize_; i++){
        if(lst[i].size() != colSize_){
            FatalErrorInFunction
            << "bad colume size in list[= " << i
            << "], with others' colume size = "<< colSize_
            << abort(FatalError);
        }
        SubList<Type> subA(*this, colSize_, i*colSize_);
        subA = lst[i];
    }
}


// * * * * * * * * * * * * * * * Function members  * * * * * * * * * * * * //
template<class Type>
Foam::denseMatrix<Type> Foam::denseMatrix<Type>::T()const
{
    denseMatrix<Type> transpose(this->colSize_, this->rowSize_);
    for(int i=0, ptr1=0; i<colSize_; i++){
        for(int j=0, ptr2=i; j<rowSize_; j++, ptr1++, ptr2+=colSize_)
            transpose[ptr1] = List<Type>::operator[](ptr2);
    }
    return transpose;
}

template<class Type>
Foam::denseMatrix<Type> Foam::denseMatrix<Type>::transposeMult()const
{
    denseMatrix<Type> ans(colSize_, colSize_, pTraits<Type>::zero);

    for(int k=0; k<rowSize_; k++){
        SubList<Type> subM(*this, rowSize_, rowSize_*k);
        for(int i=0, ptr=0; i<colSize_; i++){
            for(int j=0; j<colSize_; j++, ptr++){
                ans[ptr] += subM[i] * subM[j];
            }
        }
    }

    return ans;
}

template<class Type>
Foam::denseMatrix<Type> Foam::denseMatrix<Type>::multTranspose()const
{
    denseMatrix<Type> ans(rowSize_, rowSize_, pTraits<Type>::zero);

    for(int i=0, ptr=0; i<rowSize_; i++){
        for(int j=0; j<rowSize_; j++, ptr++){
            SubList<Type> subM1(*this, colSize_, colSize_*i);
            SubList<Type> subM2(*this, colSize_, colSize_*j);
            for(int k=0; k<colSize_; k++){
                ans[ptr] += subM1[k] * subM2[k];
            }
        }
    }

    return ans;
}

template<class Type>
Foam::denseMatrix<Type> Foam::denseMatrix<Type>::subColumes(const labelList& cols)const
{
    denseMatrix<Type> ans(rowSize_, cols.size());

    for(int i=0, ptr=0; i<rowSize_; i++){
        SubList<Type> subM(*this, colSize_, colSize_*i);
        for(int j=0; j<cols.size(); j++, ptr++)
            ans[ptr] = subM[cols[j]];
    }

    return ans;
}

template<class Type>
Foam::denseMatrix<Type> Foam::denseMatrix<Type>::subRows(const labelList& rows)const
{
    denseMatrix<Type> ans(rows.size(), colSize_);

    for(int i=0, ptr=0; i<rows.size(); i++){
        SubList<Type> subM(*this, colSize_, colSize_*rows[i]);
        for(int j=0; j<colSize_; j++, ptr++)
            ans[ptr] = subM[j];
    }

    return ans;
}

// * * * * * * * * * * * * * * * Operators * * * * * * * * * * * * //

template<class Type>
void Foam::denseMatrix<Type>::operator+=(const denseMatrix<Type>& a)
{
    if(a.size()==0)return;
    if(this->rowSize_ != a.rowSize() || this->colSize_ != a.colSize()){
        FatalErrorInFunction
            << "bad size for operator += "
            << abort(FatalError);
    }

    label size = this->actualSize();
    for(int i=0; i<size; i++)
        List<Type>::operator[](i) += a[i];
}

template<class Type>
void Foam::denseMatrix<Type>::operator-=(const denseMatrix<Type>& a)
{
    if(a.size()==0)return;
    if(this->rowSize_ != a.rowSize() || this->colSize_ != a.colSize()){
        FatalErrorInFunction
            << "bad size for operator += "
            << abort(FatalError);
    }

    label size = this->actualSize();
    for(int i=0; i<size; i++)
        List<Type>::operator[](i) -= a[i];
}

template<class Type>
void Foam::denseMatrix<Type>::operator*=(Foam::scalar a)
{
    label size = this->actualSize();
    for(int i=0; i<size; i++)
        List<Type>::operator[](i) *= a;
}

template<class Type>
void Foam::denseMatrix<Type>::MatDiagMatMult
(const denseMatrix<scalar>& mat_A, const UList<scalar> vec_x, 
const denseMatrix<scalar>& mat_B, denseMatrix<scalar>& mat_C)
{
    if(mat_A.rowSize() != vec_x.size() || mat_B.rowSize() != vec_x.size()){
        FatalErrorInFunction
            << "bad size for denseMatrix<Type>::AT_Diag_B "
            << "mat_A.rowSize = "<<mat_A.rowSize()
            << "; vec_x.size = "<<vec_x.size()
            << "; mat_B.rowSize = "<<mat_B.rowSize()<<nl
            << abort(FatalError);
    }
    label arow = mat_A.rowSize();
    label acol = mat_A.colSize();
    label bcol = mat_B.colSize();

    for(int i=0 ; i<acol*bcol ; i++){
        mat_C[i] = 0.0;
    }

    for(int k=0, ps1=0, ps2=0; k<arow; k++, ps1+=acol, ps2+=bcol){
        for(int i=0, ptr=0, pi=ps1; i<acol; i++, pi++){
            for(int j=0, pj=ps2; j<bcol; j++, ptr++, pj++){
                mat_C[ptr] += mat_A[pi] * mat_B[pj] * vec_x[k];
            }
        }
    }
}

template<class Type>
void Foam::denseMatrix<Type>::MatMatMult(
    const denseMatrix<Type>& mat_A, 
    const denseMatrix<scalar>& mat_B,
    denseMatrix<Type>& mat_C)
{
    label arow = mat_A.rowSize();
    label acol = mat_A.colSize();
    label bcol = mat_B.colSize();

    if(acol != mat_B.rowSize() || arow != mat_C.rowSize() || bcol != mat_C.colSize()){
        FatalErrorInFunction
            << __FILE__<<": "<<__LINE__<<endl
            << "bad size for MatMatMult with mat_C( "<<mat_C.rowSize()<<","<<mat_C.colSize()<<") = "
            << "mat_A("<<arow<<","<<acol<<") * mat_B("<<mat_B.rowSize()<<","<<bcol<<")."
            << abort(FatalError);
    }

    for(int i=0; i<arow*bcol; i++){
        mat_C[i] = pTraits<Type>::zero;
    }
    for(int i=0; i<arow; i++){
        SubList<Type> subAns(mat_C, bcol, i*bcol);
        SubList<Type> subA(mat_A, acol, i*acol);
        for(int k=0; k<acol; k++){
            SubList<scalar> subB(mat_B, bcol, k*bcol);
            for(int j=0; j<bcol; j++){
                subAns[j] += subA[k] * subB[j];
            }
        }
    }
}

template<class Type>
void Foam::denseMatrix<Type>::MatScalarMatMult(
    const denseMatrix<scalar>& mat_A, 
    const denseMatrix<Type>& mat_B,
    denseMatrix<Type>& mat_C)
{
    label arow = mat_A.rowSize();
    label acol = mat_A.colSize();
    label bcol = mat_B.colSize();

    if(acol != mat_B.rowSize() || arow != mat_C.rowSize() || bcol != mat_C.colSize()){
        FatalErrorInFunction
            << __FILE__<<": "<<__LINE__<<endl
            << "bad size for MatMatMult with mat_C( "<<mat_C.rowSize()<<","<<mat_C.colSize()<<") = "
            << "mat_A("<<arow<<","<<acol<<") * mat_B("<<mat_B.rowSize()<<","<<bcol<<")."
            << abort(FatalError);
    }

    for(int i=0; i<arow*bcol; i++){
        mat_C[i] = pTraits<Type>::zero;
    }
    for(int i=0; i<arow; i++){
        SubList<Type> subAns(mat_C, bcol, i*bcol);
        SubList<scalar> subA(mat_A, acol, i*acol);
        for(int k=0; k<acol; k++){
            SubList<Type> subB(mat_B, bcol, k*bcol);
            for(int j=0; j<bcol; j++){
                subAns[j] += subA[k] * subB[j];
            }
        }
    }
}

template<class Type>
void Foam::denseMatrix<Type>::MatVecMult(
    const denseMatrix<scalar>& mat_A, 
    const UList<Type> vec_x,
    UList<Type> vec_y)
{
    label arow = mat_A.rowSize();
    label acol = mat_A.colSize();

    if(acol != vec_x.size() || arow != vec_y.size()){
        FatalErrorInFunction
            << __FILE__<<": "<<__LINE__<<endl
            << "bad size for MatVecMult with vec_y("<<vec_y.size()<<") = "
            << "mat_A("<<arow<<","<<acol<<") * vec_x("<<vec_x.size()<<")"
            << abort(FatalError);
    }

    for(int i=0, ptr=0; i<arow; i++){
        vec_y[i] = pTraits<Type>::zero;
        for(int k=0; k<acol; k++, ptr++){
            vec_y[i] += mat_A[ptr] * vec_x[k];
        }
    }
}

template<class Type>
void Foam::denseMatrix<Type>::MatTVecMult(
    const denseMatrix<scalar>& mat_A, 
    const UList<Type> vec_x,
    UList<Type> vec_y)
{
    label arow = mat_A.rowSize();
    label acol = mat_A.colSize();

    if(acol != vec_y.size() || arow != vec_x.size()){
        FatalErrorInFunction
            << __FILE__<<": "<<__LINE__<<endl
            << "bad size for MatVecMult with vec_y("<<vec_y.size()<<") = "
            << "mat_A("<<arow<<","<<acol<<").T() * vec_x("<<vec_x.size()<<")"
            << abort(FatalError);
    }

    for(int i=0; i<acol; i++){
        vec_y[i] = pTraits<Type>::zero;
    }
    for(int i=0, ptr=0; i<arow; i++){
        for(int k=0; k<acol; k++, ptr++){
            vec_y[k] += mat_A[ptr] * vec_x[i];
        }
    }
}

template<class Type>
void Foam::denseMatrix<Type>::MatVecMultAdd(
    const denseMatrix<scalar>& mat_A, 
    const UList<Type> vec_x,
    UList<Type> vec_y)
{
    label arow = mat_A.rowSize();
    label acol = mat_A.colSize();

    if(acol != vec_x.size() || arow != vec_y.size()){
        FatalErrorInFunction
            << __FILE__<<": "<<__LINE__<<endl
            << "bad size for MatVecMultAdd with vec_y("<<vec_y.size()<<") = "
            << "mat_A("<<arow<<","<<acol<<") * vec_x("<<vec_x.size()<<")"
            << abort(FatalError);
    }

    for(int i=0, ptr=0; i<arow; i++){
        for(int k=0; k<acol; k++, ptr++){
            vec_y[i] += mat_A[ptr] * vec_x[k];
        }
    }
}

template<class Type>
void Foam::denseMatrix<Type>::MatVecOuterProduct(
    const denseMatrix<vector>& mat_A, 
    const UList<scalar> vec_x,
    UList<vector> vec_y)
{
    label arow = mat_A.rowSize();
    label acol = mat_A.colSize();

    if(acol != vec_x.size() || arow != vec_y.size()){
        FatalErrorInFunction
            << __FILE__<<": "<<__LINE__<<endl
            << "bad size for MatVecOuterProduct with vec_y("<<vec_y.size()<<") = "
            << "mat_A("<<arow<<","<<acol<<") * vec_x("<<vec_x.size()<<")"
            << abort(FatalError);
    }

    for(int i=0; i<arow; i++){
        vec_y[i] = vector::zero;
        SubList<vector> subA(mat_A, acol, i*acol);
        for(int k=0; k<acol; k++){
            vec_y[i] += subA[k] * vec_x[k];
        }
    }
}

template<class Type>
void Foam::denseMatrix<Type>::MatVecOuterProduct(
    const denseMatrix<vector>& mat_A, 
    const UList<vector> vec_x,
    UList<tensor> vec_y)
{
    label arow = mat_A.rowSize();
    label acol = mat_A.colSize();

    if(acol != vec_x.size() || arow != vec_y.size()){
        FatalErrorInFunction
            << __FILE__<<": "<<__LINE__<<endl
            << "bad size for MatVecOuterProduct with vec_y("<<vec_y.size()<<") = "
            << "mat_A("<<arow<<","<<acol<<") * vec_x("<<vec_x.size()<<")"
            << abort(FatalError);
    }

    for(int i=0; i<arow; i++){
        vec_y[i] = tensor::zero;
        SubList<vector> subA(mat_A, acol, i*acol);
        for(int k=0; k<acol; k++){
            vec_y[i] += subA[k] * vec_x[k];
        }
    }
}

template<class Type>
void Foam::denseMatrix<Type>::MatVecInnerProductProduct(
    const denseMatrix<vector>& mat_A, 
    const UList<vector> vec_x,
    UList<scalar> vec_y)
{
    label arow = mat_A.rowSize();
    label acol = mat_A.colSize();

    if(acol != vec_x.size() || arow != vec_y.size()){
        FatalErrorInFunction
            << __FILE__<<": "<<__LINE__<<endl
            << "bad size for MatVecInnerProductProduct with vec_y("<<vec_y.size()<<") = "
            << "mat_A("<<arow<<","<<acol<<") * vec_x("<<vec_x.size()<<")"
            << abort(FatalError);
    }

    for(int i=0; i<arow; i++){
        vec_y[i] = 0;
        SubList<vector> subA(mat_A, acol, i*acol);
        for(int k=0; k<acol; k++){
            vec_y[i] += subA[k] & vec_x[k];
        }
    }
}

template<class Type>
void Foam::denseMatrix<Type>::MatVecInnerProductProduct(
    const denseMatrix<vector>& mat_A, 
    const UList<tensor> vec_x,
    UList<vector> vec_y)
{
    label arow = mat_A.rowSize();
    label acol = mat_A.colSize();

    if(acol != vec_x.size() || arow != vec_y.size()){
        FatalErrorInFunction
            << __FILE__<<": "<<__LINE__<<endl
            << "bad size for MatVecInnerProductProduct with vec_y("<<vec_y.size()<<") = "
            << "mat_A("<<arow<<","<<acol<<") * vec_x("<<vec_x.size()<<")"
            << abort(FatalError);
    }

    for(int i=0; i<arow; i++){
        vec_y[i] = vector::zero;
        SubList<vector> subA(mat_A, acol, i*acol);
        for(int k=0; k<acol; k++){
            vec_y[i] += subA[k] & vec_x[k];
        }
    }
}

template<class Type>
Foam::List<Type> Foam::solve(const denseMatrix<scalar>& a, const List<Type>& source)
{
    label rowSize = a.rowSize();
    if(rowSize != a.colSize()){
        FatalErrorInFunction
        << "not square matrix, cannot be used to solved "
        << abort(FatalError);
    }
    if(rowSize != source.size()){
        FatalErrorInFunction
        << "square matrix size = "<<rowSize
        <<", not compatiable with source size ="<<source.size()
        << abort(FatalError);
    }
    List<vector> ans(rowSize);
    PetscBool ifInit;
    PetscInitialized(&ifInit);
    if(!ifInit){
        int argc = 2;
        char **argv = new char*[3];
        argv[1] = new char[20];
        sprintf(argv[1], "-no_signal_handler");
        PetscInitialize(&argc, &argv, (char *)0, "");
    }
    Mat            A;
    PetscErrorCode ierr;
    Vec            u,b;
    PetscScalar    val;
    KSP            ksp;
    PC             pc;
    PetscReal      norm;

    //create petsc Matrix

    ierr = MatCreate(PETSC_COMM_SELF,&A);
    ierr = MatSetSizes(A,rowSize,rowSize,rowSize,rowSize);
    ierr = MatSetType(A,MATSEQDENSE);
    ierr = MatSeqDenseSetPreallocation(A,PETSC_NULL);
    for(int i=0, ptr=0; i<rowSize; i++){
        for(int j=0; j<rowSize; j++, ptr++){
            val = a[ptr];
            ierr = MatSetValues(A,1,&i,1,&j,&val,INSERT_VALUES);
        }
    }
    ierr = MatAssemblyBegin(A,MAT_FINAL_ASSEMBLY);
    ierr = MatAssemblyEnd(A,MAT_FINAL_ASSEMBLY);

    for (direction cmpt=0; cmpt<vector::nComponents; cmpt++){
        //create vector
        ierr = VecCreateSeq(PETSC_COMM_SELF, rowSize, &u);
        ierr = VecDuplicate(u,&b);

        //set source 
        
        for(int i=0; i<rowSize; i++){
            val = source[i][cmpt];
            ierr = VecSetValue(b, i, val, INSERT_VALUES);
        }
        //set u
        ierr = VecAssemblyBegin(b);
        ierr = VecAssemblyEnd(b);
        ierr = VecAssemblyBegin(u);
        ierr = VecAssemblyEnd(u);

        /* Set solver */
        ierr = KSPCreate(MPI_COMM_WORLD,&ksp);
        ierr = KSPSetOperators(ksp,A,A);
        ierr = KSPSetType(ksp,KSPCG);


        /* Solve linear system; */
        ierr = KSPSetTolerances(ksp,1.e-10,1.e-50,PETSC_DEFAULT,200);
        ierr = KSPSetFromOptions(ksp);
        ierr = KSPSetUp(ksp);
        ierr = KSPSolve(ksp,b,u);

        // Get final residual and iteration number
        int interation;
        KSPGetIterationNumber(ksp, &interation);
        KSPGetResidualNorm(ksp, &val);
        Info << "Solving for curved shift, component["<<cmpt<<"], ";
        Info << "nIterations = " <<interation;
        Info << ", ResidualNorm = " <<val<<endl;

        const scalar* data;
        VecGetArrayRead(u, &data);
        for(int i=0;i<rowSize;i++)
            ans[i][cmpt]= data[i];
        VecRestoreArrayRead(u, &data);
        
        if(data) delete data;
        KSPDestroy(&ksp);
        VecDestroy(&u);
        VecDestroy(&b);
    }

    MatDestroy(&A);
    return ans;
}
// * * * * * * * * * * * * * * * IOstream Operators  * * * * * * * * * * * * //

template<class Type>
Foam::Ostream& Foam::operator<<
(
    Ostream& os,
    const denseMatrix<Type>& gf
)
{
    os << "[ "<<gf.rowSize()<<" x "<<gf.colSize()<<" ]" << nl;

    for(int i=0, ptr=0; i<gf.rowSize(); i++){
        os<<i<<" { ";
        for(int j=0; j<gf.colSize(); j++, ptr++){
            os<<gf[ptr]<<" ";
        }
        os<<"};"<<nl;
    }
    os<<nl;

    return (os);
}
