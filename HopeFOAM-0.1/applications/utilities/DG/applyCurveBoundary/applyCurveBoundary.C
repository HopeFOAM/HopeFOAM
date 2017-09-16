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
    applyCurveBoundary

Description
    Correct the points' location of the curve boundary.

Usage
    applyCurveBoundary

Author
    Feng Yongquan(yqfeng0418@163.com)

\*---------------------------------------------------------------------------*/

#include "argList.H"
#include "dgCFD.H"
#include "IOdictionary.H"
#include "OSspecific.H"
#include "OFstream.H"
#include "dynamicCode.H"
#include "dynamicCodeContext.H"
#include "timeSelector.H"
#include "Time.H"
#include "polyMesh.H"
#include "correctRegIOobject.H"

using namespace Foam;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

int main(int argc, char *argv[])
{

    #include "setRootCase.H"
    #include "createTime.H"

    Foam::word regionName;

    if (args.optionReadIfPresent("region", regionName))
    { 
    	Foam::Info
            << "Create mesh " << regionName << " for time = "
            << runTime.timeName() << Foam::nl << Foam::endl;
    }
    else
    {
    	regionName = Foam::polyMesh::defaultRegion;
    	Foam::Info
            << "Create mesh for time = "
            << runTime.timeName() << Foam::nl << Foam::endl;
    }

    Foam::IOobject io
    (
        regionName,
        runTime.constant(),
        runTime,
        Foam::IOobject::MUST_READ
    );

    //- Add a string into regIOobject to avoid unnecessary initialization procedures.
    string newStr("applyCurveBoundary");
    word newW(newStr);

    correctRegIOobject tmp(io);
    correctRegIOobject res(newW, tmp, true);

    //- Construct polyMesh from IOobject
    polyMesh polymesh
    (
        io
    );

    //- Construct dgMesh from IOobject
    dgMesh mesh
    (
        io
    );

    const polyBoundaryMesh& polyBMesh = polymesh.boundaryMesh();
    const dgBoundaryMesh& bMesh = mesh.boundary();

    //*****************Search Every Patch in The BoundaryMesh****************
    pointField& polyPoints = const_cast<pointField&>(mesh.points());

    forAll(bMesh, patchI){
        const polyPatch& polypatch = polyBMesh[patchI];
        if(bMesh[patchI].curvedPatch()){

            labelList& boundaryPoints = const_cast<labelList&>(polypatch.meshPoints());
            vectorList stdDofLoc(boundaryPoints.size());
            List<bool> isEnd(boundaryPoints.size());
            forAll(boundaryPoints, pointI){
                stdDofLoc[pointI] = polyPoints[boundaryPoints[pointI]];

                if(mesh.meshDim()<3 && stdDofLoc[pointI][2] != 0){
                    isEnd[pointI] = true;
                }
                else{
                    isEnd[pointI] = false;
                }
           }
            
            //-Move the point to the right pointed according to the parameter equation
            vectorList tmpPatchShift(bMesh[patchI].positions(stdDofLoc, isEnd));

            forAll(tmpPatchShift, pointI){
                polyPoints[boundaryPoints[pointI]] = tmpPatchShift[pointI];
                        
            }

        }
    }

    pointIOField points_
    (
        IOobject
        (
            "points",
            polymesh.instance(),
            polymesh.meshSubDir,
            polymesh,
            IOobject::NO_READ,
            IOobject::AUTO_WRITE
        ),
        xferCopy(polyPoints)
    );
    fileName meshFilesPath = polymesh.thisDb().time().path()/polymesh.instance()/polymesh.meshDir();

    //- Delete the points file in origin polyMesh folder
    rm(meshFilesPath/"points");
    
    //- Create new points file in polyMesh folder.
    if (!points_.write())
    {
        FatalErrorInFunction
            << "Failed correctring polyMesh."
            << exit(FatalError);
    }   

    return 0;
}


// ************************************************************************* //
