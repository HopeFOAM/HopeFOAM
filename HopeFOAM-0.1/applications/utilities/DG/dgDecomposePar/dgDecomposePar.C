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
    dgDecomposePar

Description
    Automatically decomposes a mesh and fields of a case for parallel
    execution of HopeFOAM.

Usage

    - dgDecomposePar [OPTION]

    Options:
        -fields
            Use existing geometry dgDecompostion and convert dgFields only.

        -force
            Remove any existing processor subdirectories before decomposing
            the geometry.

\*---------------------------------------------------------------------------*/
#include "dgFieldDecomposer.H"
#include "OSspecific.H"
#include "fvCFD.H"

#include "IOobjectList.H"
#include "domainDecomposition.H"
#include "labelIOField.H"
#include "labelFieldIOField.H"
#include "scalarIOField.H"
#include "scalarFieldIOField.H"
#include "vectorIOField.H"
#include "vectorFieldIOField.H"
#include "sphericalTensorIOField.H"
#include "sphericalTensorFieldIOField.H"
#include "symmTensorIOField.H"
#include "symmTensorFieldIOField.H"
#include "tensorIOField.H"
#include "tensorFieldIOField.H"
#include "regionProperties.H"

#include "readFields.H"
#include "fvFieldDecomposer.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

const labelIOList& procAddressing
(
    const PtrList<fvMesh>& procMeshList,
    const label procI,
    const word& name,
    PtrList<labelIOList>& procAddressingList
)
{
    const fvMesh& procMesh = procMeshList[procI];

    if (!procAddressingList.set(procI))
    {
        procAddressingList.set
        (
            procI,
            new labelIOList
            (
                IOobject
                (
                    name,
                    procMesh.facesInstance(),
                    procMesh.meshSubDir,
                    procMesh,
                    IOobject::MUST_READ,
                    IOobject::NO_WRITE
                )
            )
        );
    }
    return procAddressingList[procI];
}


const labelIOList& dgProcAddressing
(
    const PtrList<dgMesh>& procMeshList,
    const label procI,
    const word& name,
    PtrList<labelIOList>& procAddressingList
)
{
    const dgMesh& procMesh = procMeshList[procI];

    if (!procAddressingList.set(procI))
    {
        procAddressingList.set
        (
            procI,
            new labelIOList
            (
                IOobject
                (
                    name,
                    procMesh.facesInstance(),
                    procMesh.meshSubDir(),
                    procMesh,
                    IOobject::MUST_READ,
                    IOobject::NO_WRITE
                )
            )
        );
    }

    return procAddressingList[procI];
}


int main(int argc, char *argv[])
{


    argList::addNote
    (
        "decompose a mesh and fields of a case for parallel execution"
    );

    argList::noParallel();
    #include "addRegionOption.H"
    argList::addBoolOption
    (
        "allRegions",
        "operate on all regions in regionProperties"
    );
    argList::addBoolOption
    (
        "cellDist",
        "write cell distribution as a labelList - for use with 'manual' "
        "decomposition method or as a volScalarField for post-processing."
    );
    argList::addBoolOption
    (
        "copyUniform",
        "copy any uniform/ directories too"
    );
    argList::addBoolOption
    (
        "fields",
        "use existing geometry decomposition and convert fields only"
    );
    argList::addBoolOption
    (
        "noSets",
        "skip decomposing cellSets, faceSets, pointSets"
    );
    argList::addBoolOption
    (
        "force",
        "remove existing processor*/ subdirs before decomposing the geometry"
    );
    argList::addBoolOption
    (
        "ifRequired",
        "only decompose geometry if the number of domains has changed"
    );

    // Include explicit constant options, have zero from time range
    timeSelector::addOptions(true, false);

    #include "setRootCase.H"

    bool allRegions              = args.optionFound("allRegions");
    bool writeCellDist           = args.optionFound("cellDist");
    bool copyUniform             = args.optionFound("copyUniform");
    bool decomposeFieldsOnly     = args.optionFound("fields");
    bool decomposeSets           = !args.optionFound("noSets");
    bool forceOverwrite          = args.optionFound("force");
    bool ifRequiredDecomposition = args.optionFound("ifRequired");

    // Set time from database
    #include "createTime.H"
    // Allow override of time
    instantList times = timeSelector::selectIfPresent(runTime, args);


    wordList regionNames;
    wordList regionDirs;
    if (allRegions)
    {
        Info<< "Decomposing all regions in regionProperties" << nl << endl;
        regionProperties rp(runTime);
        forAllConstIter(HashTable<wordList>, rp, iter)
        {
            const wordList& regions = iter();
            forAll(regions, i)
            {
                if (findIndex(regionNames, regions[i]) == -1)
                {
                    regionNames.append(regions[i]);
                }
            }
        }
        regionDirs = regionNames;
    }
    else
    {
        word regionName;
        if (args.optionReadIfPresent("region", regionName))
        {
            regionNames = wordList(1, regionName);
            regionDirs = regionNames;
        }
        else
        {
            regionNames = wordList(1, fvMesh::defaultRegion);
            regionDirs = wordList(1, word::null);
        }
    }

    forAll(regionNames, regionI)
    {
        const word& regionName = regionNames[regionI];
        const word& regionDir = regionDirs[regionI];

        Info<< "\n\nDecomposing mesh " << regionName << nl << endl;


        // determine the existing processor count directly
        label nProcs = 0;
        while
        (
            isDir
            (
                runTime.path()
              / (word("processor") + name(nProcs))
              / runTime.constant()
              / regionDir
              / polyMesh::meshSubDir
            )
        )
        {
            ++nProcs;
        }

        // get requested numberOfSubdomains
        const label nDomains = readLabel
        (
            IOdictionary
            (
                IOobject
                (
                    "decomposeParDict",
                    runTime.time().system(),
                    regionDir,          // use region if non-standard
                    runTime,
                    IOobject::MUST_READ_IF_MODIFIED,
                    IOobject::NO_WRITE,
                    false
                )
            ).lookup("numberOfSubdomains")
        );

        if (decomposeFieldsOnly)
        {
            // Sanity check on previously decomposed case
            if (nProcs != nDomains)
            {
                FatalErrorIn(args.executable())
                    << "Specified -fields, but the case was decomposed with "
                    << nProcs << " domains"
                    << nl
                    << "instead of " << nDomains
                    << " domains as specified in decomposeParDict"
                    << nl
                    << exit(FatalError);
            }
        }
        else if (nProcs)
        {
            bool procDirsProblem = true;

            if (ifRequiredDecomposition && nProcs == nDomains)
            {
                // we can reuse the decomposition
                decomposeFieldsOnly = true;
                procDirsProblem = false;
                forceOverwrite = false;

                Info<< "Using existing processor directories" << nl;
            }

            if (forceOverwrite)
            {
                Info<< "Removing " << nProcs
                    << " existing processor directories" << endl;

                // remove existing processor dirs
                // reverse order to avoid gaps if someone interrupts the process
                for (label procI = nProcs-1; procI >= 0; --procI)
                {
                    fileName procDir
                    (
                        runTime.path()/(word("processor") + name(procI))
                    );

                    rmDir(procDir);
                }

                procDirsProblem = false;
            }

            if (procDirsProblem)
            {
                FatalErrorIn(args.executable())
                    << "Case is already decomposed with " << nProcs
                    << " domains, use the -force option or manually" << nl
                    << "remove processor directories before decomposing. e.g.,"
                    << nl
                    << "    rm -rf " << runTime.path().c_str() << "/processor*"
                    << nl
                    << exit(FatalError);
            }
        }

        Info<< "Create mesh" << endl;
        domainDecomposition mesh
        (
            IOobject
            (
                regionName,
                runTime.timeName(),
                runTime
            )
        );

        Info<< "Create dgmesh" << endl;
        Foam::dgMesh dgmesh
        (
            Foam::IOobject
            (
                "",
                runTime.timeName(),
                runTime,
                Foam::IOobject::MUST_READ
            )
        );
		
        // Decompose the mesh
        if (!decomposeFieldsOnly)
        {
            mesh.decomposeMesh();

            mesh.writeDecomposition(decomposeSets);

            if (writeCellDist)
            {
                const labelList& procIds = mesh.cellToProc();

                // Write the decomposition as labelList for use with 'manual'
                // decomposition method.
                labelIOList cellDecomposition
                (
                    IOobject
                    (
                        "cellDecomposition",
                        mesh.facesInstance(),
                        mesh,
                        IOobject::NO_READ,
                        IOobject::NO_WRITE,
                        false
                    ),
                    procIds
                );
                cellDecomposition.write();

                Info<< nl << "Wrote decomposition to "
                    << cellDecomposition.objectPath()
                    << " for use in manual decomposition." << endl;

                // Write as volScalarField for postprocessing.
                volScalarField cellDist
                (
                    IOobject
                    (
                        "cellDist",
                        runTime.timeName(),
                        mesh,
                        IOobject::NO_READ,
                        IOobject::AUTO_WRITE
                    ),
                    mesh,
                    dimensionedScalar("cellDist", dimless, 0),
                    zeroGradientFvPatchScalarField::typeName
                );

                forAll(procIds, celli)
                {
                   cellDist[celli] = procIds[celli];
                }

                cellDist.write();

                Info<< nl << "Wrote decomposition as volScalarField to "
                    << cellDist.name() << " for use in postprocessing."
                    << endl;
            }
        }

        // Caches
        // Cached processor meshes and maps. These are only preserved if running
        // with multiple times.
        PtrList<Time> processorDbList(mesh.nProcs());
        PtrList<fvMesh> procMeshList(mesh.nProcs());        
        PtrList<labelIOList> faceProcAddressingList(mesh.nProcs());
        PtrList<labelIOList> cellProcAddressingList(mesh.nProcs());
        PtrList<labelIOList> boundaryProcAddressingList(mesh.nProcs());        
        PtrList<fvFieldDecomposer> fieldDecomposerList(mesh.nProcs());	        
        PtrList<labelIOList> pointProcAddressingList(mesh.nProcs());

        //- Data for dgDecomposePar
        PtrList<dgMesh> dgProcMeshList(mesh.nProcs());//add by RXG
        PtrList<dgFieldDecomposer> dgFieldDecomposerList(mesh.nProcs());//add by RXG
        PtrList<labelIOList> dgFaceProcAddressingList(mesh.nProcs());
        PtrList<labelIOList> dgCellProcAddressingList(mesh.nProcs());
        PtrList<labelIOList> dgBoundaryProcAddressingList(mesh.nProcs());

        // Loop over all times
        forAll(times, timeI)
        {
            runTime.setTime(times[timeI], timeI);

            Info<< "Time = " << runTime.timeName() << endl;

            // Search for list of objects for this time
            IOobjectList objects(mesh, runTime.timeName());
	        IOobjectList dgObjects(dgmesh, runTime.timeName());

	        // Construct the DG fields
            PtrList<dgScalarField> dgScalarFields;
            readFields(dgmesh, dgObjects, dgScalarFields);
            PtrList<dgVectorField> dgVectorFields;
            readFields(dgmesh, dgObjects, dgVectorFields);
            PtrList<dgSphericalTensorField> dgSphericalTensorFields;
            readFields(dgmesh, dgObjects, dgSphericalTensorFields);
            PtrList<dgSymmTensorField> dgSymmTensorFields;
            readFields(dgmesh, dgObjects, dgSymmTensorFields);
            PtrList<dgTensorField> dgTensorFields;
            readFields(dgmesh, dgObjects, dgTensorFields);


            // Any uniform data to copy/link?
            fileName uniformDir("uniform");

            if (isDir(runTime.timePath()/uniformDir))
            {
                Info<< "Detected additional non-decomposed files in "
                    << runTime.timePath()/uniformDir
                    << endl;
            }
            else
            {
                uniformDir.clear();
            }

            Info<< endl;

            // split the fields over processors
            for (label procI = 0; procI < mesh.nProcs(); procI++)
            {
                Info<< "Processor " << procI << ": field transfer" << endl;


                // open the database
                if (!processorDbList.set(procI))
                {
                    processorDbList.set
                    (
                        procI,
                        new Time
                        (
                            Time::controlDictName,
                            args.rootPath(),
                            args.caseName()
                           /fileName(word("processor") + name(procI))
                        )
                    );
                }
                Time& processorDb = processorDbList[procI];


                processorDb.setTime(runTime);

                // remove files remnants that can cause horrible problems
                // - mut and nut are used to mark the new turbulence models,
                //   their existence prevents old models from being upgraded
                {
                    fileName timeDir(processorDb.path()/processorDb.timeName());

                    rm(timeDir/"mut");
                    rm(timeDir/"nut");
                }

                // read the mesh
                if (!dgProcMeshList.set(procI))
                {
                    dgProcMeshList.set
                    (
                        procI,
                        new dgMesh
                        (
                            IOobject
                            (
                                "",
                                processorDb.timeName(),
                                processorDb,
                                Foam::IOobject::MUST_READ
                            )
                        )
                    );
                }
                const dgMesh& procdgmesh = dgProcMeshList[procI];

                const labelIOList& faceDgProcAddressing = dgProcAddressing
                (
                    dgProcMeshList,
                    procI,
                    "faceProcAddressing",
                    dgFaceProcAddressingList
                );
                const labelIOList& cellDgProcAddressing = dgProcAddressing
                (
                    dgProcMeshList,
                    procI,
                    "cellProcAddressing",
                    dgCellProcAddressingList
                );

                const labelIOList& boundaryDgProcAddressing = dgProcAddressing
                (
                    dgProcMeshList,
                    procI,
                    "boundaryProcAddressing",
                    dgBoundaryProcAddressingList
                );

                // DG fields
                {
                    if (!dgFieldDecomposerList.set(procI))
                    {
                        dgFieldDecomposerList.set
                        (
                            procI,
                            new dgFieldDecomposer
                            (
                                dgmesh,
                                procdgmesh,
                                faceDgProcAddressing,
                                cellDgProcAddressing,
                                boundaryDgProcAddressing
                            )
                        );
                    }
                    const dgFieldDecomposer& dgFieldDecomposer = dgFieldDecomposerList[procI];
                    
                    dgFieldDecomposer.decomposeFields(dgScalarFields);
                    dgFieldDecomposer.decomposeFields(dgVectorFields);
                    dgFieldDecomposer.decomposeFields(dgSphericalTensorFields);
                    dgFieldDecomposer.decomposeFields(dgSymmTensorFields);
                    dgFieldDecomposer.decomposeFields(dgTensorFields);

                    if (times.size() == 1)
                    {
                        // Clear cached decomposer
                        dgFieldDecomposerList.set(procI, NULL);
                    }
                }

                // Any non-decomposed data to copy?
                if (uniformDir.size())
                {
                    const fileName timePath = processorDb.timePath();

                    if (copyUniform || mesh.distributed())
                    {
                        cp
                        (
                            runTime.timePath()/uniformDir,
                            timePath/uniformDir
                        );
                    }
                    else
                    {
                        // link with relative paths
                        const string parentPath = string("..")/"..";

                        fileName currentDir(cwd());
                        chDir(timePath);
                        ln
                        (
                            parentPath/runTime.timeName()/uniformDir,
                            uniformDir
                        );
                        chDir(currentDir);
                    }
                }

			    const string parentPath = string("..");
			
			    fileName currentDir(cwd());
			    fileName systemDir(currentDir/"system");
        		mkDir(processorDb.path()/"system");
		        cp
                (
                    systemDir/"dgMeshCtrDict",
                    processorDb.path()/"system"
                );
				cp
                (
                    systemDir/"dgSolution",
                    processorDb.path()/"system"
                );		  
                // We have cached all the constant mesh data for the current
                // processor. This is only important if running with multiple
                // times, otherwise it is just extra storage.
                if (times.size() == 1)
                {
                    boundaryProcAddressingList.set(procI, NULL);
                    cellProcAddressingList.set(procI, NULL);
                    faceProcAddressingList.set(procI, NULL);

		            dgBoundaryProcAddressingList.set(procI, NULL);
                    dgCellProcAddressingList.set(procI, NULL);
                    dgFaceProcAddressingList.set(procI, NULL);
					
                    procMeshList.set(procI, NULL);
		            dgProcMeshList.set(procI, NULL);
                    processorDbList.set(procI, NULL);
                }
            }
        }
    }

    Info<< "\nEnd.\n" << endl;

    return 0;
}


// ************************************************************************* //
