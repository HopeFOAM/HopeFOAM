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

#include "arcDgPatch.H"
#include "dgBoundaryMesh.H"
#include "dgMesh.H"
#include "addToRunTimeSelectionTable.H"
#include "dynamicCode.H"
#include "dynamicCodeContext.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //



const Foam::word Foam::arcDgPatch::codeTemplateC
    = "CodedDgPatchTemplate.C";

const Foam::word Foam::arcDgPatch::codeTemplateH
    = "CodedDgPatchTemplate.H";


namespace Foam
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

defineTypeNameAndDebug(arcDgPatch, 0);
addToRunTimeSelectionTable(dgPatch, arcDgPatch, polyPatch);



// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

arcDgPatch::arcDgPatch(const polyPatch& patch, const dgBoundaryMesh& bm)
	:
	dgPatch(patch, bm),
	codedBase(),
	arcPolyPatch_(refCast<const arcPolyPatch>(patch)),
	dict_(boundaryMesh().patchEntries()[index()].dict()),
	redirectPatchPtr_(),
	u0(0),
	v0(0),
	initPos(false),
	faceNormal_(0),
	name_
	(
	   dict_.found("redirectType")
	   ? dict_.lookup("redirectType")
	   : dict_.lookup("name")
	),
	faceCells_
	(
	   labelList::subList
	   (
	       boundaryMesh().mesh().faceOwner(), 0, patch.start()
	   )
	)
{

	updateLibrary(name_);
	

}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

const labelUList& arcDgPatch::faceCells() const
{
	return faceCells_;
}


Foam::dlLibraryTable& arcDgPatch::libs() const
{
	//return const_cast<dlLibraryTable&>(boundaryMesh().mesh().time().libs());
	return const_cast<dlLibraryTable&>(boundaryMesh().mesh().time().libs());

}

Foam::string arcDgPatch::description() const
{
	return
	    "patch "
	    + this->patch().name()
	    + " on field "
	    ;
}

void arcDgPatch::clearRedirect() const
{
	// remove instantiation of fvPatchField provided by library
	redirectPatchPtr_.clear();

}

const Foam::IOdictionary& arcDgPatch::dict() const
{
	const objectRegistry& obr = boundaryMesh().mesh();
	if (obr.foundObject<IOdictionary>("codeDict")) {
		return obr.lookupObject<IOdictionary>("codeDict");
	} else    {
		return obr.store
		       (
		           new IOdictionary
		           (
		               IOobject
		               (
		                   "codeDict",
		                   boundaryMesh().mesh().time().system(),
		                   boundaryMesh().mesh(),
		                   IOobject::MUST_READ_IF_MODIFIED,
		                   IOobject::NO_WRITE
		               )
		           )
		       );
	}
}

void arcDgPatch::prepare
(
    dynamicCode& dynCode,
    const dynamicCodeContext& context
)
const
{
// take no chances - typeName must be identical to name_
	dynCode.setFilterVariable("typeName", name_);

		
// set TemplateType and FieldType filter variables
// (for fvPatchField)
	//setFieldTemplates(dynCode);
// compile filtered C template
	dynCode.addCompileFile(codeTemplateC);
// copy filtered H template
	dynCode.addCopyFile(codeTemplateH);
// debugging: make BC verbose
//  dynCode.setFilterVariable("verbose", "true");

// define Make/options
	dynCode.setMakeOptions
	(            "EXE_INC = -g \\\n"
	             "-I$(LIB_SRC)/finiteVolume/lnInclude \\\n"
	             "-I$(LIB_SRC)/DG2.0/lnInclude \\\n"
	             + context.options()
	             + "\n\nLIB_LIBS = \\\n"
	             + "    -lOpenFOAM \\\n"
	             + "    -lfiniteVolume \\\n"
	             + "    -lDG2 \\\n"
	             + context.libs()
	);

	Info<<dynCode.libPath()<<"*******************"<<endl;
}

const Foam::dictionary& arcDgPatch::codeDict()
const
{
	// use system/codeDict or in-line

	return    (
	              dict_.found("code")
	              ? dict_
	              :dict_.subDict(name_)
	          );
}

const Foam::dgPatch& arcDgPatch::redirectPatch() const
{
	if (!redirectPatchPtr_.valid()) {
		// Construct a patch
		// Make sure to construct the patchfield with up-to-date value
		OStringStream os;
		os.writeKeyword("type") << name_ << token::END_STATEMENT
		                        << nl;
		//static_cast<const Field<Type>&>(*this).writeEntry("value", os);
		IStringStream is(os.str());
		dictionary dict(is);
	
		word& name = const_cast<word&>(patch().type());

		name = name_;
	

		redirectPatchPtr_.set
		(
		    dgPatch::New
		    (
		        patch(),
		        boundaryMesh()
		    ).ptr()
		);
	}
	return redirectPatchPtr_();
}

Foam::vector arcDgPatch::function(scalar u,scalar v) const
{
	vector func;
	 updateLibrary(name_);
	const dgPatch& fvp = redirectPatch();
	func = const_cast<dgPatch&>(fvp).function(u,v);
	return func;
}


Foam::List<vector> arcDgPatch::partialDerivative(Pair<scalar> pairI)const{
	List<vector> res;
	res.setSize(2);

	const point initialPoint = function(pairI[0], pairI[1]);
	const scalar error = 1e-6;
	scalar d0, d1;
	scalar step = 1e-2;
	point difPoint = function(pairI[0]+step, pairI[1]);
	d0 = (difPoint[0]-initialPoint[0])/step;
	step/=2;
	for(int i=0 ; i<3 ; i++){
		while(1){
			point difPoint = function(pairI[0]+step, pairI[1]);
			d1 = (difPoint[i]-initialPoint[i])/step; 
			if(fabs(d1-d0) > error){
				d0=d1;
				step/=2;
			}
			else{
				break;
			}
			
		}
	}
	difPoint = function(pairI[0], pairI[1]+step);
	d0 = (difPoint[0]-initialPoint[0])/step;
	for(int i=0 ; i<3 ; i++){
		while(1){
			point difPoint = function(pairI[0], pairI[1]+step);
			d1 = (difPoint[i]-initialPoint[i])/step; 
			if(fabs(d1-d0) > error){
				d0=d1;
				step/=2;
			}
			else{
				break;
			}
		}
	}
	difPoint = function(pairI[0]+step, pairI[1]);
	res[0][0] = (difPoint[0]-initialPoint[0])/step;
	res[0][1] = (difPoint[1]-initialPoint[1])/step;
	res[0][2] = (difPoint[2]-initialPoint[2])/step;

	difPoint = function(pairI[0], pairI[1]+step);
	res[1][0] = (difPoint[0]-initialPoint[0])/step;
	res[1][1] = (difPoint[1]-initialPoint[1])/step;
	res[1][2] = (difPoint[2]-initialPoint[2])/step;
	const label meshDim = boundaryMesh().mesh().meshDim();
	if(meshDim==2){
		res[1][0]=0;
		res[1][1]=0;
		res[1][2]=1;
	}
	

	return res;
}

const bool arcDgPatch::getShortestPoint(point srPoint, point& resPoint){

	List<scalar> B;
	B.setSize(3);
	List<List<scalar>> A;
	A.setSize(3);
	A[0].setSize(2); A[1].setSize(2); A[2].setSize(2);
	const scalar error = 1e-12;
	List<scalar> z;
	z.setSize(2);

	const scalar t = 0.7;  // The Convergence Factor;
	const Pair<scalar> uRange(boundaryMesh().patchEntries()[index()].dict().lookup("u_Range"));
    const Pair<scalar> vRange(boundaryMesh().patchEntries()[index()].dict().lookup("v_Range"));

    label numStep = dgFaceIndex().size();
	scalar uStep = (uRange[1]-uRange[0])/numStep;
    scalar vStep = (vRange[1]-vRange[0])/numStep;

	const label meshDim = boundaryMesh().mesh().meshDim();
	scalar h=0; 
    if(meshDim<3){
		h = uStep/2; // Differential step size. It will become smaller with the Convergence Facetor.
    }
    else if(meshDim==3){
    	h = min(uStep, vStep)/2;
    }

	int nCycle=0;

	while(1){
		if(nCycle>500){
			FatalErrorIn("arcDgPatch::position()")
		        << "can`t convergence "
		        <<endl
		        << abort(FatalError);
		}
		nCycle++;
		Pair<scalar> initialPoint(u0, v0);
		List<vector> partDerivative = partialDerivative(initialPoint);

		point iniPoint = function(u0, v0);


		B[0] = (iniPoint[1]-srPoint[1])*(partDerivative[1][0]*partDerivative[0][1]-partDerivative[1][1]*partDerivative[0][0])
			  -(iniPoint[2]-srPoint[2])*(partDerivative[1][2]*partDerivative[0][0]-partDerivative[1][0]*partDerivative[0][2]);
		B[1] = (iniPoint[2]-srPoint[2])*(partDerivative[1][1]*partDerivative[0][2]-partDerivative[1][2]*partDerivative[0][1])
			  -(iniPoint[0]-srPoint[0])*(partDerivative[1][0]*partDerivative[0][1]-partDerivative[1][1]*partDerivative[0][0]);
		B[2] = (iniPoint[0]-srPoint[0])*(partDerivative[1][2]*partDerivative[0][0]-partDerivative[1][0]*partDerivative[0][2])
			  -(iniPoint[1]-srPoint[1])*(partDerivative[1][1]*partDerivative[0][2]-partDerivative[1][2]*partDerivative[0][1]);
		if((fabs(B[0])<error) && (fabs(B[1])<error) && (fabs(B[2])<error)){
			resPoint = iniPoint;

			return true;
		}
		Pair<scalar> nPair(u0+h, v0);
		partDerivative = partialDerivative(nPair);
		point nPoint = function(u0+h, v0);
		

		A[0][0] = (nPoint[1]-srPoint[1])*(partDerivative[1][0]*partDerivative[0][1]-partDerivative[1][1]*partDerivative[0][0])
			     -(nPoint[2]-srPoint[2])*(partDerivative[1][2]*partDerivative[0][0]-partDerivative[1][0]*partDerivative[0][2]);
		A[1][0] = (nPoint[2]-srPoint[2])*(partDerivative[1][1]*partDerivative[0][2]-partDerivative[1][2]*partDerivative[0][1])
			     -(nPoint[0]-srPoint[0])*(partDerivative[1][0]*partDerivative[0][1]-partDerivative[1][1]*partDerivative[0][0]);
		A[2][0] = (nPoint[0]-srPoint[0])*(partDerivative[1][2]*partDerivative[0][0]-partDerivative[1][0]*partDerivative[0][2])
                 -(nPoint[1]-srPoint[1])*(partDerivative[1][1]*partDerivative[0][2]-partDerivative[1][2]*partDerivative[0][1]);

		Pair<scalar> nPair2(u0, v0+h);
		partDerivative = partialDerivative(nPair2);
		nPoint = function(u0, v0+h);
		
		A[0][1] = (nPoint[1]-srPoint[1])*(partDerivative[1][0]*partDerivative[0][1]-partDerivative[1][1]*partDerivative[0][0])
			     -(nPoint[2]-srPoint[2])*(partDerivative[1][2]*partDerivative[0][0]-partDerivative[1][0]*partDerivative[0][2]);
		A[1][1] = (nPoint[2]-srPoint[2])*(partDerivative[1][1]*partDerivative[0][2]-partDerivative[1][2]*partDerivative[0][1])
			     -(nPoint[0]-srPoint[0])*(partDerivative[1][0]*partDerivative[0][1]-partDerivative[1][1]*partDerivative[0][0]);
		A[2][1] = (nPoint[0]-srPoint[0])*(partDerivative[1][2]*partDerivative[0][0]-partDerivative[1][0]*partDerivative[0][2])
                 -(nPoint[1]-srPoint[1])*(partDerivative[1][1]*partDerivative[0][2]-partDerivative[1][2]*partDerivative[0][1]);


        if((A[1][1]*A[2][0] - A[2][1]*A[1][0])==0){
        	z[1]/=2;
        }
        else{
	    	z[1] = (A[2][0]*B[1] - A[1][0]*B[2]) / (A[1][1]*A[2][0] - A[2][1]*A[1][0]);
		}

		if((A[2][1]*A[1][0] - A[2][0]*A[1][1])==0){
			z[0]/=2;
		}
		else{
			z[0] = (A[2][1]*B[1] - A[1][1]*B[2]) / (A[2][1]*A[1][0] - A[2][0]*A[1][1]);
		}

		scalar Beta = 1-z[1]-z[0];
		u0 -= (h*z[0]/Beta);
		v0 -= (h*z[1]/Beta);
		h *= t;
	}
}

const Foam::scalar arcDgPatch::distance(point pointI, point pointJ)const
{
	return sqrt((pointI[0]-pointJ[0])*(pointI[0]-pointJ[0])+(pointI[1]-pointJ[1])*(pointI[1]-pointJ[1])+(pointI[2]-pointJ[2])*(pointI[2]-pointJ[2]));
}

Foam::point const arcDgPatch::position(point pointI)const
{
	scalar t1=0,t2=0;
	point resPoint;

	const label meshDim = boundaryMesh().mesh().meshDim();

	bool flag = false;
	if(meshDim == 1){
		resPoint = pointI;
		flag = true;
	}
	else if(meshDim == 2){
		flag = const_cast<arcDgPatch&>(*this).getShortestPoint(pointI, resPoint);
	}
	else if(meshDim == 3){
		flag = const_cast<arcDgPatch&>(*this).getShortestPoint(pointI, resPoint);		
	}
	else{
		FatalErrorIn("arcDgPatch::position()")
                        << "The dimension of the mesh is wrong(not be 1/2/3) "
                        <<endl
                        << abort(FatalError);
	}

	if(!flag){
		FatalErrorIn("arcDgPatch::position()")
		        << "find the shortes distance point failed "
		        <<endl
		        << abort(FatalError);
	}

	return resPoint;
}


const void arcDgPatch::initialPosition(point pointI){
	const Pair<scalar> uRange(boundaryMesh().patchEntries()[index()].dict().lookup("u_Range"));
    const Pair<scalar> vRange(boundaryMesh().patchEntries()[index()].dict().lookup("v_Range"));
	const label meshDim = boundaryMesh().mesh().meshDim();

	scalar uStep = (uRange[1]-uRange[0])/10;
    scalar vStep = (vRange[1]-vRange[0])/10;

	if(meshDim == 1){
		initPos = true;
		return;
	}
	else if(meshDim == 2){
		for(int i=0 ; i<10 ; i++){
			u0 = uRange[0]+uStep*i;
			v0 = 0;
			point resPoint;
			
			bool flag = getShortestPoint(pointI, resPoint);
			if(flag){
				initPos = true;
				return;
			}
		}
	}
	else if(meshDim == 3){
		for(int i=0 ; i<10 ; i++){
			u0 = uRange[0]+uStep*i;
			for(int j=0 ; j<10 ; j++){
				v0 = vRange[0]+vStep*j;
				point resPoint;
				bool flag = getShortestPoint(pointI, resPoint);
				if(flag){
					initPos = true;
					return;
				}
			}			
		}
	}
	else{
		FatalErrorIn("arcDgPatch::position()")
                        << "The dimension of the mesh is wrong(not be 1/2/3) "
                        <<endl
                        << abort(FatalError);
	}

	FatalErrorIn("arcDgPatch::position()")
                    << "find the shortes distance point failed "
                    <<endl
                    << abort(FatalError);

}

const Foam::labelList arcDgPatch::sortPoint(List<point> points) const{
	List<bool> flag(points.size(), false);
	labelList pindex(points.size());
	point cp;
	label cindex;
	for(int i=0 ; i<points.size() ; i++){
		bool tp=false;
		forAll(points, pointI){
			if(!flag[pointI]){
				if(!tp){
					cp = points[pointI];
					cindex = pointI;
					tp = true;
				}
				else{
					if(cp[0]>points[pointI][0]){
						cp = points[pointI];
						cindex = pointI;
					}
					else if(cp[0] == points[pointI][0]){
						if(cp[1] > points[pointI][1]){
							cp = points[pointI];
							cindex = pointI;
						}
					}
				}
			}
		}
		pindex[i] = cindex;
		flag[cindex] = true;
	}
	return pindex;
}

const Foam::List<Foam::point>  arcDgPatch::positions(List<point> points, List<bool> isEnd)const{
	labelList pointIndex;
	if(!initPos){
		pointIndex = sortPoint(points);
		const_cast<arcDgPatch&>(*this).initialPosition(points[pointIndex[0]]);
	}

		List<point> tmpPoints(points.size());
		forAll(points,pointI){
			if(!isEnd[pointIndex[pointI]]){
				tmpPoints[pointIndex[pointI]] = position(points[pointIndex[pointI]]);
				
			}
			else tmpPoints[pointIndex[pointI]] = points[pointIndex[pointI]];
		}

	return tmpPoints;
}

void arcDgPatch::updateFaceNormal(List<point> oriPoints, List<point> moveRes){
	FatalErrorIn("arcDgPatch::updateFaceNormal()")
                    << " have not implemented!"
                    <<endl
                    << abort(FatalError);
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //
