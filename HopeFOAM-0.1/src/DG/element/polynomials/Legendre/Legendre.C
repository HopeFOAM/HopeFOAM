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

#include "Legendre.H"
#include "constants.H"
#include "SubList.H"
#include <slepceps.h>
#include <petscksp.h>

// * * * * * * * * * * * * * * Static Legendre functions * * * * * * * * * * * * * //

const Foam::scalarListList Foam::Legendre::JacobiGQ(scalar alpha, scalar beta, int N)
{
    scalarListList ans(2, scalarList(N+1,  0.0));

    if(N == 0)
    {
      ans[0][0] = -(alpha - beta)/(alpha+beta+2);
      ans[1][0] = 2;
      return ans;
    }

	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
		Compute the operator matrix that defines the eigensystem, Ax=kx
	- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
    Mat            A;           /* problem matrix */
    EPS            eps;         /* eigenproblem solver context */
    EPSType        type;
    PetscReal      tol,re,im;
    PetscScalar    kr,ki,value;
    Vec            xr,xi;
    PetscInt       i,j,nev,maxit,its,nconv;
    PetscErrorCode ierr;

    PetscBool ifInit;
    SlepcInitialized(&ifInit);
    if(!ifInit){
        int argc = 2;
        char **argv = new char*[3];
        argv[1] = new char[20];
        sprintf(argv[1], "-no_signal_handler");
        SlepcInitialize(&argc, &argv, (char *)0, "");
    }

    ierr = MatCreate(PETSC_COMM_SELF,&A);
    MatSetSizes(A,PETSC_DECIDE,PETSC_DECIDE,N+1,N+1);
    MatSetFromOptions(A);
    MatSetUp(A);

    for(i=0;i<N;i++){
      j = i+1;
      value = 2/(2*i+alpha+beta+2)*sqrt(j*(j+alpha+beta)*(j+alpha)*(j+beta)/(2*i+alpha+beta+1)/(2*i+alpha+beta+3));
      MatSetValues(A,1,&i,1,&j,&value,INSERT_VALUES);
      MatSetValues(A,1,&j,1,&i,&value,INSERT_VALUES);
      //PetscPrintf(PETSC_COMM_SELF," setvalue %.4g \n",(double)value);
    }
    if(alpha + beta < 10*constant::mathematical::e){
        i=0; j=0; value = 0.0;
        MatSetValues(A,1,&i,1,&j,&value, INSERT_VALUES);
    }

    MatAssemblyBegin(A,MAT_FINAL_ASSEMBLY);
    MatAssemblyEnd(A,MAT_FINAL_ASSEMBLY);

    MatGetVecs(A,NULL,&xr);
    MatGetVecs(A,NULL,&xi);

    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
       Create the eigensolver and set various options
       - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
    /* Create eigensolver context*/
    EPSCreate(PETSC_COMM_SELF,&eps);
    /*Set operators. In this case, it is a standard eigenvalue problem*/
    EPSSetOperators(eps,A,NULL);
    EPSSetProblemType(eps,EPS_HEP);
    /*Set solver parameters at runtime*/
    EPSSetFromOptions(eps);
    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
                        Solve the eigensystem
       - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
    EPSSolve(eps);
    EPSGetIterationNumber(eps,&its);
    EPSGetType(eps,&type);
    EPSGetDimensions(eps,&nev,NULL,NULL);
    EPSGetTolerances(eps,&tol,&maxit);

    /*Get number of converged approximate eigenpairs*/
    EPSGetConverged(eps,&nconv);

    scalar eigComp = Foam::GREAT;

    if (nconv>0) {
      for (i=0;i<nconv;i++) {
        /*Get converged eigenpairs: i-th eigenvalue is stored in kr (real part) andki (imaginary part)*/
        EPSGetEigenpair(eps,i,&kr,&ki,xr,xi);

    #if defined(PETSC_USE_COMPLEX)
        re = PetscRealPart(kr);
        im = PetscImaginaryPart(kr);
    #else
        re = kr;
        im = ki;
    #endif
        ans[0][i] = re;
        const scalar* data;
        VecGetArrayRead(xr, &data);

        ans[1][i] = (data[0]*data[0]) * pow(2,alpha+beta+1) / (alpha+beta+1) * Legendre::gamma(alpha+1) * Legendre::gamma(beta+1)/Legendre::gamma(alpha+beta+1);
        VecRestoreArrayRead(xr, &data);
      }
    }

   
    for(int i=0; i<N; i++){
        for(int j=i+1; j<=N; j++){
            if(ans[0][i] > ans[0][j]){
                scalar tmp = ans[0][i];
                ans[0][i] = ans[0][j];
                ans[0][j] = tmp;

                tmp = ans[1][i];
                ans[1][i] = ans[1][j];
                ans[1][j] = tmp;
            }
        }
    }
    for(int i=0; i<=(N-1)/2; i++){
        scalar tmp = (ans[1][i] + ans[1][N-i])*0.5;
        ans[1][i] = ans[1][N-i] = tmp;
    }

    /*Free work space*/
    EPSDestroy(&eps);
    MatDestroy(&A);
    VecDestroy(&xr);
    VecDestroy(&xi);

    return ans;
}


const Foam::scalarList Foam::Legendre::JacobiGL(scalar alpha, scalar beta, int N)
{
    scalarList ans(N+1, 0.0);
    ans[0] = -1.0;
    ans[N] = 1.0;

    if(N<2)return ans;

    scalarList xint(JacobiGQ(alpha+1, beta+1, N-2)[0]);

    for(int i=1; i< N; i++){
        ans[i] = xint[i-1];
    }
    return ans;
}

const Foam::scalarList Foam::Legendre::JacobiP(const scalarList& xp, scalar alpha, scalar beta, int N)
{
    int dims = xp.size();
    Foam::scalarList tmp(dims, 0.0);
    Foam::scalarListList PL(N+1, tmp);
    scalar gamma0 = pow(2,alpha+beta+1)/(alpha+beta+1)*Legendre::gamma(alpha+1)*Legendre::gamma(beta+1)/Legendre::gamma(alpha+beta+1);
    scalar gamma1 = (alpha+1)*(beta+1)/(alpha+beta+3)*gamma0;

    
    for(int i=0; i< dims; i++) PL[0][i] = 1.0/sqrt(gamma0);
    if(N==0){
         
        return PL[0]; 
    }

    
    for(int i=0; i< dims; i++) PL[1][i] = ((alpha+beta+2)/2*xp[i] + (alpha-beta)/2)/sqrt(gamma1);
    if(N==1){
         
        return PL[1]; 
    }

    //Repeat value in recurrence.
    scalar aold = 2/(2+alpha+beta)*sqrt((alpha+1)*(beta+1)/(alpha+beta+3));
    //Forward recurrence using the symmetry of the recurrence.
    for(int i=1; i<N; i++){
        scalar h1 = 2.0*i + alpha + beta;
        scalar anew = 2/(h1+2)*sqrt((i+1)*(i+1+alpha+beta)*(i+1+alpha)*(i+1+beta)/(h1+1)/(h1+3));
        scalar bnew = -(alpha*alpha - beta * beta)/h1/(h1+2);
        for(int j=0; j< dims; j++)tmp[j] = (xp[j] - bnew)*PL[i][j];
        PL[i+1] = 1/anew*( (0.0-aold)*PL[i-1] + tmp);
        aold = anew;
    }

    return PL[N];
}

const Foam::scalarList Foam::Legendre::GradJacobiP(const scalarList& xp, scalar alpha, scalar beta, int N)
{
    Foam::scalarList dP(xp.size(), 0.0);

    if(N==0) return dP;
    else{
        dP = sqrt(N*(N+alpha+beta+1))*JacobiP(xp, alpha+1,beta+1, N-1);
    }
    return dP;
}

const Foam::scalarList Foam::Legendre::GradJacobiP(const vectorList& xp, scalar alpha, scalar beta, int N)
{
	Foam::scalarList xp_Cmpt0(xp.size());
	forAll(xp, pointI){
		xp_Cmpt0[pointI] = xp[pointI].x();
	}

	return GradJacobiP(xp_Cmpt0, alpha, beta, N);
}

const Foam::scalar Foam::Legendre::gamma(int x){
    scalar ans = 1.0;
    for(int i=2;i<x; i++)ans*=i;
    return ans;
}


const Foam::scalarListList Foam::Legendre::Vandermonde1D(int N, const scalarList& r)
{
    scalarListList ans(r.size(), scalarList(N+1, 0.0));
    scalarList tmp(r.size(), 0.0);

    for(int i=0; i<=N; i++){
        tmp = JacobiP(r,0,0,i);
        for(int j=0; j<r.size(); j++)
            ans[j][i] = tmp[j];
    }

    return ans;
}

const Foam::scalarListList Foam::Legendre::Vandermonde1D(int N, const vectorList& r)
{
    scalarList tmp(r.size(), 0.0);
    forAll(r, pointI){
        tmp[pointI] = r[pointI].x();
    }
    return Vandermonde1D(N, tmp);
}

const Foam::scalarListList Foam::Legendre::Vandermonde2D(int N, const scalarList& r, const scalarList& s)
{
    scalarListList V2D(r.size(), scalarList((N+1)*(N+2)/2, 0.0));
    scalarList a(r.size(), 0.0), b(s);

    //Transfer from (r,s) -> (a,b) coordinates in triangle
    for(int i=0; i<r.size(); i++){
        if(s[i] != 1.0) a[i] = 2*(1+r[i])/(1-s[i])-1;
        else a[i] = -1.0;
    }

    // build the Vandermonde matrix
    int sk=0;
    scalar sqrt2 = sqrt(2.0);
    for(int i=0; i<=N; i++){
        for(int j=0; j<=N-i; j++){
            scalarList h1(JacobiP(a,0,0,i));
            scalarList h2(JacobiP(b,2*i+1,0,j));
            for(int n=0; n<r.size(); n++){
                V2D[n][sk] = sqrt2*h1[n]*h2[n]*pow((1-b[n]),i);
            }
            sk++;
        }
    }

    return V2D;
}

const Foam::scalarListList Foam::Legendre::Vandermonde2D(int N, const vectorList& r)
{
    scalarList tmp1(r.size(), 0.0);
    scalarList tmp2(r.size(), 0.0);
    forAll(r, pointI){
        tmp1[pointI] = r[pointI].x();
        tmp2[pointI] = r[pointI].y();
    }
    return Vandermonde2D(N, tmp1, tmp2);
}

const Foam::scalarListList Foam::Legendre::Vandermonde3D(int N, const scalarList& r, const scalarList& s, const scalarList& t)
{
    scalarListList V3D(r.size(), scalarList((N+1)*(N+2)*(N+3)/6, 0.0));
    scalar tol = 1e-10;

    //Transfer to (a,b) coordinates
    scalarList a(r.size()), b(r.size()), c(r.size());
    for(int i=0; i<r.size(); i++){
        if(fabs(s[i]+t[i])>tol) a[i] = 2*(1+r[i])/(-s[i]-t[i])-1;
        else a[i] = -1;
        if(fabs(t[i]-1)>tol) b[i] = 2*(1+s[i])/(1-t[i])-1;
        else b[i] = -1;
        c[i] = t[i];
    }

    //build the Vandermonde matrix
    int sk=0;
    scalar sqrt2 = sqrt(2.0);
    for(int i=0; i<=N; i++){
        for(int j=0; j<=N-i; j++){
            for(int k=0; k<=N-i-j; k++){
                //Simplex3DP
                scalarList h1(JacobiP(a,0,0,i));
                scalarList h2(JacobiP(b,2*i+1,0,j));
                scalarList h3(JacobiP(c,2*(i+j)+2,0,k));
                for(int n=0; n<r.size(); n++){
                    V3D[n][sk] = 2*sqrt2*h1[n]*h2[n]*pow(1-b[n],i)*h3[n]*pow(1-c[n],i+j);
                }
                sk++;
            }
        }
    }
    return V3D;
}

const Foam::scalarListList Foam::Legendre::Vandermonde3D(int N, const vectorList& r)
{
    scalarList tmp1(r.size(), 0.0);
    scalarList tmp2(r.size(), 0.0);
    scalarList tmp3(r.size(), 0.0);
    forAll(r, pointI){
        tmp1[pointI] = r[pointI].x();
        tmp2[pointI] = r[pointI].y();
        tmp3[pointI] = r[pointI].z();
    }
    return Vandermonde3D(N, tmp1, tmp2, tmp3);
}

const Foam::denseMatrix<Foam::vector> Foam::Legendre::GradVandermonde1D(label N, scalarList& r)
{
    label rSize = r.size();

    Foam::scalarListList Vr(rSize, scalarList(N+1, 0.0));
    for(int i=0; i<=N; i++){
        Foam::scalarList tmp = Foam::Legendre::GradJacobiP(r, 0, 0, i);
        for(int j=0; j<rSize; j++)Vr[j][i] = tmp[j];
    }

    Foam::denseMatrix<vector> ans(rSize, N+1);
    for(int i=0, ptr=0; i<rSize; i++){
        for(int j=0; j<=N; j++, ptr++)
            ans[ptr] = vector(Vr[i][j], 0.0, 0.0);
    }
    return ans;
}

const Foam::denseMatrix<Foam::vector> Foam::Legendre::GradVandermonde1D(label N, vectorList& r)
{
    scalarList a(r.size());
    forAll(r, pointI){
        a[pointI] = r[pointI].x();
    }
    return Foam::Legendre::GradVandermonde1D(N, a);
}

const Foam::denseMatrix<Foam::vector> Foam::Legendre::GradVandermonde2D(label N, vectorList& r)
{
    label rSize = r.size();

    Foam::scalarListList Vr(rSize, scalarList((N+1)*(N+2)/2, 0.0));
    Foam::scalarListList Vs(rSize, scalarList((N+1)*(N+2)/2, 0.0));
    scalarList a(rSize), b(rSize);
    forAll(r, pointI){
        a[pointI] = r[pointI].x();
        b[pointI] = r[pointI].y();
    }

    //Transfer from (r,s) -> (a,b) coordinates in triangle
    for(int i=0; i<r.size(); i++){
        if(r[i].y() != 1.0) a[i] = 2*(1+r[i].x())/(1-r[i].y())-1;
        else a[i] = -1.0;
    }

    // build the GradVandermonde matrix
    int sk=0;
    for(int i=0; i<=N; i++){
        for(int j=0; j<=N-i; j++){
            scalarList fa(Foam::Legendre::JacobiP(a,0,0,i));
            scalarList gb(Foam::Legendre::JacobiP(b,2*i+1,0,j));
            scalarList dfa(Foam::Legendre::GradJacobiP(a,0,0,i));
            scalarList dgb(Foam::Legendre::GradJacobiP(b,2*i+1,0,j));
            scalar drCoeff = pow(2.0,i+0.5);
            // d/dr
            if(i>0){
                for(int n=0; n<r.size(); n++){
                    Vr[n][sk] = dfa[n]*gb[n]*drCoeff*pow(0.5*(1.0-b[n]),(i-1));
                }
            }
            else{
                for(int n=0; n<r.size(); n++){
                    Vr[n][sk] = dfa[n]*gb[n]*drCoeff;
                }
            }

            // d/ds
            if(i>0){
                for(int n=0; n<r.size(); n++){
                    scalar tmp = dgb[n]*pow(0.5*(1-b[n]), i) - 0.5*i*gb[n]*pow(0.5*(1-b[n]), i-1);
                    Vs[n][sk] = (dfa[n]*gb[n]*0.5*(1+a[n])*pow(0.5*(1-b[n]), i-1) + fa[n]*tmp) * pow(2.0,i+0.5);
                }
            }
            else{
                for(int n=0; n<r.size(); n++){
                    scalar tmp = dgb[n]*pow(0.5*(1-b[n]), i);
                    Vs[n][sk] = (dfa[n]*gb[n]*0.5*(1+a[n]) + fa[n]*tmp)*pow(2.0,i+0.5);
                }
            }

            sk++;
        }
    }

    Foam::denseMatrix<vector> ans(rSize, (N+1)*(N+2)/2);
    for(int i=0, ptr=0; i<rSize; i++){
        for(int j=0; j<(N+1)*(N+2)/2; j++, ptr++)
            ans[ptr] = vector(Vr[i][j], Vs[i][j], 0.0);
    }
    return ans;
}


const Foam::denseMatrix<Foam::vector> Foam::Legendre::GradVandermonde3D(label N, vectorList& r)
{
    label rSize = r.size();
    label nSize = (N+1)*(N+2)*(N+3)/6;
    //Transfer to (a,b) coordinates
    Foam::scalarListList Vr(rSize, scalarList(nSize, 0.0));
    Foam::scalarListList Vs(rSize, scalarList(nSize, 0.0));
    Foam::scalarListList Vt(rSize, scalarList(nSize, 0.0));

    scalar tol=1e-10;
    scalarList a(r.size()), b(r.size()), c(r.size());
    for(int i=0; i<r.size(); i++){
        if(fabs(r[i].y()+r[i].z())>tol) a[i] = 2*(1+r[i].x())/(-r[i].y()-r[i].z())-1;
        else a[i] = -1;
        if(fabs(r[i].z()-1)>tol) b[i] = 2*(1+r[i].x())/(1-r[i].z())-1;
        else b[i] = -1;
        c[i] = r[i].z();
    }

    // build the GradVandermonde matrix
    int sk=0;
    for(int i=0; i<=N; i++){
        for(int j=0; j<=N-i; j++){
            for(int k=0; k<=N-i-j; k++){
                scalarList fa(Foam::Legendre::JacobiP(a,0,0,i));
                scalarList gb(Foam::Legendre::JacobiP(b,2*i+1,0,j));
                scalarList hc(Foam::Legendre::JacobiP(c,2*(i+j)+2,0,k));
                scalarList dfa(Foam::Legendre::GradJacobiP(a,0,0,i));
                scalarList dgb(Foam::Legendre::GradJacobiP(b,2*i+1,0,j));
                scalarList dhc(Foam::Legendre::GradJacobiP(c,2*(i+j)+2,0,k));
                
                // d/dr
                for(int n=0; n<r.size(); n++){
                    Vr[n][sk] = dfa[n]*gb[n]*hc[n];
                }
                if(i>0){
                    for(int n=0; n<r.size(); n++){
                        Vr[n][sk] *= pow(0.5*(1.0-b[n]), i-1);
                    }
                }
                if(i+j>0){
                    for(int n=0; n<r.size(); n++){
                        Vr[n][sk] *= pow(0.5*(1-c[n]), i+j-1);
                    }
                }

                // d/ds
                scalarList tmp(r.size());
                for(int n=0; n<r.size(); n++){
                    Vs[n][sk] = 0.5*(1+a[n])*Vr[n][sk];
                    tmp[n] = dgb[n]*pow(0.5*(1-b[n]), i);
                    if(i>0) tmp[n] += -0.5*i*gb[n]*pow(0.5*(1-b[n]), i-1);
                    if(i+j>0) tmp[n] *= pow(0.5*(1-c[n]), i+j-1);
                    tmp[n] *= fa[n]*hc[n];
                    Vs[n][sk] += tmp[n];
                    
                }

                // d/dt
                for(int n=0; n<r.size(); n++){
                    Vt[n][sk] = 0.5*(1+a[n])*Vr[n][sk] + 0.5*(1+b[n])*tmp[n];
                    scalar tmp1 = dhc[n] * pow(0.5*(1-c[n]), i+j);
                    if(i+j>0) tmp1 -= 0.5*(i+j)*hc[n]*pow(0.5*(1-c[n]), i+j-1);
                    tmp1 *= (fa[n]*gb[n]) * pow(0.5*(1-b[n]), i);
                    Vt[n][sk] += tmp1;
                }

                // normalize
                scalar drCoeff = pow(2.0, 2*i+j+1.5);
                for(int n=0; n<r.size(); n++){
                    Vr[n][sk] *= drCoeff;
                    Vs[n][sk] *= drCoeff;
                    Vt[n][sk] *= drCoeff;
                }
                sk++;
            }
        }
    }

    Foam::denseMatrix<vector> ans(rSize, nSize);
    for(int i=0, ptr=0; i<rSize; i++){
        for(int j=0; j<nSize; j++, ptr++)
            ans[ptr] = vector(Vr[i][j], Vs[i][j], Vt[i][j]);
    }
    return ans;
}

Foam::denseMatrix<Foam::scalar> Foam::Legendre::matrixInv(const Foam::denseMatrix<Foam::scalar>& arcs)
{
	/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		Compute the matrix inv with PETSc, using MatMatSolve()
	- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
    KSP            ksp;             
    Mat            A,B,F,X;
    PetscInt       M,N,i,j,ptr;
    PetscScalar    val;
    PetscErrorCode ierr;
    PC             pc;

    PetscBool ifInit;
    PetscInitialized(&ifInit);
    if(!ifInit){
        PetscInitializeNoArguments();
        PetscPopSignalHandler();
    }

    //- judge if is square matrix
    M = arcs.rowSize();
    N = arcs.colSize();
    if(M != N){
        FatalErrorInFunction
            << "Inv operation on non-square matrix, with row size = " << M
            << ", colume size = "<< N
            << abort(FatalError);
    }
    //create petsc Matrix A
    
    ierr = MatCreate(PETSC_COMM_SELF,&A);
    ierr = MatSetSizes(A,M,N,M,N);
    ierr = MatSetType(A,MATSEQDENSE);
    ierr = MatSeqDenseSetPreallocation(A,PETSC_NULL);
    for (i=0, ptr=0; i<M; i++){
        for(j=0; j<N; j++, ptr++){
            val = arcs[ptr];
            ierr = MatSetValues(A,1,&i,1,&j,&val,INSERT_VALUES);
        }
    }
    ierr = MatAssemblyBegin(A,MAT_FINAL_ASSEMBLY);
    ierr = MatAssemblyEnd(A,MAT_FINAL_ASSEMBLY);


    /* Create dense matric B and X. Set B as an identity matrix */
    ierr = MatDuplicate(A,MAT_DO_NOT_COPY_VALUES,&B);
    val = 1.0;
    for (i=0; i<M; i++){
        ierr = MatSetValues(B,1,&i,1,&i,&val,INSERT_VALUES);
    }
    ierr = MatAssemblyBegin(B,MAT_FINAL_ASSEMBLY);
    ierr = MatAssemblyEnd(B,MAT_FINAL_ASSEMBLY);

    ierr = MatDuplicate(B,MAT_DO_NOT_COPY_VALUES,&X);

    /* Compute X=inv(A) by MatMatSolve() */
    ierr = KSPCreate(PETSC_COMM_SELF,&ksp);
    ierr = KSPSetOperators(ksp,A,A);
    ierr = KSPGetPC(ksp,&pc);
    ierr = PCSetType(pc,PCLU);
    ierr = KSPSetUp(ksp);
    ierr = PCFactorGetMatrix(pc,&F);
    ierr = MatMatSolve(F,B,X);

    Foam::denseMatrix<scalar> inv(M,N);
    for (i=0, ptr=0; i<M; i++){
        for(j=0; j<N; j++, ptr++){
            ierr = MatGetValues(X,1,&i,1,&j,&val);
            inv[ptr] = val;
        }
    }

    // Free work space.
    ierr = MatDestroy(&B);
    ierr = MatDestroy(&X);
    ierr = MatDestroy(&A);
    ierr = KSPDestroy(&ksp);

    return inv;
}