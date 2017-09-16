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
    Ye Shuai (shuaiye09@163.com)

\*---------------------------------------------------------------------------*/

#include "RoeFlux.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
namespace Foam
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace dg
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //


void RoeFlux::evaluateFlux
(
    const dgGaussVectorField& gther_U ,
    const dgGaussScalarField& gther_p, 
    scalar gamma1, 
    const dgGaussScalarField& grho,
    scalarField& fluxRho,
    const dgGaussVectorField& grhoU,
    vectorField& fluxRhoU,
    const dgGaussScalarField& gEner,
    scalarField& fluxEner
)const
{
	scalar ther_gamma = gamma1;
    const dgMesh& mesh = (gther_U).mesh();
    
	label size = fluxRho.size();

	const Field<vector>& gUf_O = gther_U.ownerFaceField();
	const Field<vector>& gUf_N = gther_U.neighborFaceField();

	const Field<scalar>& gther_pO = gther_p.ownerFaceField();
	const Field<scalar>& gther_pN = gther_p.neighborFaceField();
	
	const Field<scalar>& grho_O = grho.ownerFaceField();
	const Field<scalar>& grho_N = grho.neighborFaceField();

	const Field<vector>& grhoU_O = grhoU.ownerFaceField();
	const Field<vector>& grhoU_N = grhoU.neighborFaceField();

	const Field<scalar>& gEner_O = gEner.ownerFaceField();
	const Field<scalar>& gEner_N = gEner.neighborFaceField();


      //1. rotate "-"  
      //Reference : Hesthaven J S, Warburton T. Nodal discontinuous 
      //Galerkin methods: algorithms, analysis, and applications[M].
      //Springer Science & Business Media, 2007.
      //Page 220
	tmp<Field<scalar>> tmpQP2(new Field<scalar> (size, 0));
	tmp<Field<scalar>> tmpQP3(new Field<scalar> (size, 0));
	tmp<Field<scalar>> tmpQM2(new Field<scalar> (size, 0));
	tmp<Field<scalar>> tmpQM3(new Field<scalar> (size, 0));
	Field<scalar> &QP2 = tmpQP2.ref();
	Field<scalar> &QP3 = tmpQP3.ref();
	Field<scalar> &QM2 = tmpQM2.ref();
    Field<scalar> &QM3 = tmpQM3.ref();

	tmp<Field<scalar> > DW1(new Field<scalar> (size, 0));
    tmp<Field<scalar> > DW2(new Field<scalar> (size, 0));
    tmp<Field<scalar> > DW3(new Field<scalar> (size, 0));
    tmp<Field<scalar> > DW4(new Field<scalar> (size, 0));
    Field<scalar> &dw1 = DW1.ref();
    Field<scalar> &dw2 = DW2.ref();
    Field<scalar> &dw3 = DW3.ref();
    Field<scalar> &dw4 = DW4.ref();

    scalar rhoM=0,rhoP=0,uM=0,uP=0,pM=0,pP=0,vM=0,vP=0,HM=0,HP=0;
    scalar rhob=0,u=0,v=0,H=0,rhoMs=0,rhoPs=0,c2 = 0,c=0;
	scalar e=0;

	const dgTree<physicalCellElement>& cellPhysicalEleTree = mesh.cellElementsTree();

	for(dgTree<physicalCellElement>::leafIterator iter = cellPhysicalEleTree.leafBegin(); iter != cellPhysicalEleTree.end(); ++iter){
		List<shared_ptr<physicalFaceElement>>& faceEleInCell = iter()->value().faceElementList();

		forAll(faceEleInCell, faceI){
			if(faceEleInCell[faceI]->ownerEle_ != iter()) continue;
			label subSize = faceEleInCell[faceI]->nGaussPoints_;
			label subPos = faceEleInCell[faceI]->ownerFaceStart_;
			SubList<scalar> subfluxRho(fluxRho, subSize, subPos);
			SubList<scalar> subQP2(QP2, subSize, subPos);
        	SubList<scalar> subQM2(QM2, subSize, subPos);
			SubList<scalar> subQP3(QP3, subSize, subPos);
			SubList<scalar> subQM3(QM3, subSize, subPos);
			SubList<vector> subfluxRhoU(fluxRhoU, subSize, subPos);
       		SubList<scalar> subfluxEner(fluxEner, subSize, subPos);	
			SubList<scalar> subdw1(dw1, subSize, subPos);
       		SubList<scalar> subdw2(dw2, subSize, subPos);
    		SubList<scalar> subdw3(dw3, subSize, subPos);				
	    	SubList<scalar> subdw4(dw4, subSize, subPos);

	    	vectorList& faceNx = faceEleInCell[faceI]->faceNx_;
	    	for(int pointI=0 ; pointI<subSize ; pointI++){
	    		label posInField = subPos + pointI;
	    		subQP2[pointI] =  faceNx[pointI].x() * grhoU_N[posInField].x()
                                  + faceNx[pointI].y() * grhoU_N[posInField].y();
				subQM2[pointI] =  faceNx[pointI].x() * grhoU_O[posInField].x()
                                  + faceNx[pointI].y() * grhoU_O[posInField].y();
				subQP3[pointI] =  faceNx[pointI].x() * grhoU_N[posInField].y()
                                  - faceNx[pointI].y() * grhoU_N[posInField].x();
				subQM3[pointI] =  faceNx[pointI].x() * grhoU_O[posInField].y()
                                  - faceNx[pointI].y() * grhoU_O[posInField].x();


                rhoP = grho_N[posInField];
				rhoM = grho_O[posInField];
				uM = subQM2[pointI]/rhoM;
				uP = subQP2[pointI]/rhoP;
				vM = subQM3[pointI]/rhoM;
				vP = subQP3[pointI]/rhoP;
				pM =  (ther_gamma-1)*(gEner_O[posInField]-0.5*(subQM2[pointI]*uM+subQM3[pointI]*vM));
				pP =  (ther_gamma-1)*(gEner_N[posInField]-0.5*(subQP2[pointI]*uP+subQP3[pointI]*vP));
	            HM = (gEner_O[posInField]+pM)/rhoM;
	            HP = (gEner_N[posInField]+pP)/rhoP;

	            subfluxRho[pointI] = (subQM2[pointI]+subQP2[pointI])/2;
				subfluxRhoU[pointI].x() = (subQM2[pointI]*uM+pM+subQP2[pointI]*uP+pP)/2;
				subfluxRhoU[pointI].y() = (subQM3[pointI]*uM+subQP3[pointI]*uP)/2;	
				subfluxEner[pointI] = (uM*(gEner_O[posInField]+pM)+uP*(gEner_N[posInField]+pP))/2;
				rhoMs=Foam::sqrt(rhoM); 
				rhoPs=Foam::sqrt(rhoP);		
				rhob = rhoMs*rhoPs; 
				u = (rhoMs*uM + rhoPs*uP)/(rhoMs+rhoPs);
				v = (rhoMs*vM + rhoPs*vP)/(rhoMs+rhoPs);
				H = (rhoMs*HM + rhoPs*HP)/(rhoMs+rhoPs);
				c2 = (ther_gamma-1)*(H-0.5*(sqr(u)+sqr(v)));
				c = Foam::sqrt(fabs(c2)+e);
				subdw1[pointI] = (-0.5*rhob*(uP-uM)/c+0.5*(pP-pM)/c2)*fabs(u-c);
				subdw2[pointI] = ((rhoP-rhoM)-(pP-pM)/c2)*fabs(u);
				subdw3[pointI] =( rhob*(vP-vM))*fabs(u);
				subdw4[pointI] = (0.5*rhob*(uP-uM)/c+0.5*(pP-pM)/c2)*fabs(u+c);
				subfluxRho[pointI] -= (subdw1[pointI]+subdw2[pointI]+subdw4[pointI])/2;
				subfluxRhoU[pointI].x() -= (subdw1[pointI]*(u-c)+subdw2[pointI]*u+subdw4[pointI]*(u+c))/2;
				subfluxRhoU[pointI].y() -= (subdw1[pointI]*(v)+subdw2[pointI]*v+subdw3[pointI]+subdw4[pointI]*(v))/2;
				subfluxEner[pointI]  -= (subdw1[pointI]*(H-u*c)+subdw2[pointI]*(sqr(u)+sqr(v))/2+subdw3[pointI]*(v)+subdw4[pointI]*(H+u*c))/2;
				scalar tmp1 = faceNx[pointI].x()*subfluxRhoU[pointI].x() - faceNx[pointI].y()*subfluxRhoU[pointI].y() ;
				scalar tmp2 = faceNx[pointI].y()*subfluxRhoU[pointI].x() + faceNx[pointI].x()*subfluxRhoU[pointI].y() ;
				
				
				subfluxRhoU[pointI].x() =  tmp1;
				subfluxRhoU[pointI].y() =  tmp2;
	    	}
		}
		
	}
	tmpQP2.clear();
	tmpQM2.clear();
	tmpQP3.clear();		
	tmpQM3.clear();	
	DW1.clear();
	DW2.clear();
	DW3.clear();
	DW4.clear();

} // End funtion of evaluateFlux



} // End namespace dg

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //
