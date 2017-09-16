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
    dgReconstructPar

Description
    Reconstructs dgFields of a case that is decomposed for parallel
    execution of HopeFOAM.

\*---------------------------------------------------------------------------*/

#include "argList.H"
#include "timeSelector.H"
#include "dgFieldReconstructor.H"

#include "fvCFD.H"
#include "IOobjectList.H"
#include "processorDgMeshes.H"
#include "regionProperties.H"

#include "cellSet.H"
#include "faceSet.H"
#include "pointSet.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

bool haveAllTimes
(
    const HashSet<word>& masterTimeDirSet,
    const instantList& timeDirs
)
{
    // Loop over all times
    forAll(timeDirs, timeI)
    {
        if (!masterTimeDirSet.found(timeDirs[timeI].name()))
        {
            return false;
        }
    }
    return true;
}


int main(int argc, char *argv[])
{
    argList::addNote
    (
        "Reconstruct fields of a parallel case"
    );

    // enable -constant ... if someone really wants it
    // enable -zeroTime to prevent accidentally trashing the initial fields
    timeSelector::addOptions(true, true);
    argList::noParallel();
    #include "addRegionOption.H"
    argList::addBoolOption
    (
        "allRegions",
        "operate on all regions in regionProperties"
    );
    argList::addOption
    (
        "fields",
        "list",
        "specify a list of fields to be reconstructed. Eg, '(U T p)' - "
        "regular expressions not currently supported"
    );

    argList::addBoolOption
    (
        "noSets",
        "skip reconstructing cellSets, faceSets, pointSets"
    );
    argList::addBoolOption
    (
        "newTimes",
        "only reconstruct new times (i.e. that do not exist already)"
    );

    #include "setRootCase.H"
    #include "createTime.H"

    HashSet<word> selectedFields;
    if (args.optionFound("fields"))
    {
        args.optionLookup("fields")() >> selectedFields;
    }

    const bool noReconstructSets = args.optionFound("noSets");

    if (noReconstructSets)
    {
        Info<< "Skipping reconstructing cellSets, faceSets and pointSets"
            << nl << endl;
    }

    const bool newTimes   = args.optionFound("newTimes");
    const bool allRegions = args.optionFound("allRegions");


    // determine the processor count directly
    label nProcs = 0;
    while (isDir(args.path()/(word("processor") + name(nProcs))))
    {
        ++nProcs;
    }

    if (!nProcs)
    {
        FatalErrorIn(args.executable())
            << "No processor* directories found"
            << exit(FatalError);
    }

    // Create the processor databases
    PtrList<Time> databases(nProcs);

    forAll(databases, procI)
    {
        databases.set
        (
            procI,
            new Time
            (
                Time::controlDictName,
                args.rootPath(),
                args.caseName()/fileName(word("processor") + name(procI))
            )
        );
    }

    // use the times list from the master processor
    // and select a subset based on the command-line options
    instantList timeDirs = timeSelector::select
    (
        databases[0].times(),
        args
    );

    // Note that we do not set the runTime time so it is still the
    // one set through the controlDict. The -time option
    // only affects the selected set of times from processor0.
    // - can be illogical
    // + any point motion handled through mesh.readUpdate


    if (timeDirs.empty())
    {
        FatalErrorIn(args.executable())
            << "No times selected"
            << exit(FatalError);
    }


    // Get current times if -newTimes
    instantList masterTimeDirs;
    if (newTimes)
    {
        masterTimeDirs = runTime.times();
    }
    HashSet<word> masterTimeDirSet(2*masterTimeDirs.size());
    forAll(masterTimeDirs, i)
    {
        masterTimeDirSet.insert(masterTimeDirs[i].name());
    }


    // Set all times on processor meshes equal to reconstructed mesh
    forAll(databases, procI)
    {
        databases[procI].setTime(runTime);
    }


    wordList regionNames;
    wordList regionDirs;
    if (allRegions)
    {
        Info<< "Reconstructing for all regions in regionProperties" << nl
            << endl;
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

        Info<< "\n\nReconstructing fields for mesh " << regionName << nl
            << endl;

        if
        (
            newTimes
         && regionNames.size() == 1
         && regionDirs[0].empty()
         && haveAllTimes(masterTimeDirSet, timeDirs)
        )
        {
            Info<< "Skipping region " << regionName
                << " since already have all times"
                << endl << endl;
            continue;
        }


        fvMesh mesh
        (
            IOobject
            (
                regionName,
                runTime.timeName(),
                runTime,
                Foam::IOobject::MUST_READ
            )
        );

        dgMesh mesh2
        (
            IOobject
            (
                regionName,
                runTime.timeName(),
                runTime,
                Foam::IOobject::MUST_READ
            )
        );


	    Info<<" regionName:"<<regionName<<endl;
        // Read all meshes and addressing to reconstructed mesh
        processorDgMeshes procMeshes(databases, regionName);


        // check face addressing for meshes that have been decomposed
        // with a very old foam version
        #include "checkFaceAddressingComp.H"

        // Loop over all times
        forAll(timeDirs, timeI)
        {
            if (newTimes && masterTimeDirSet.found(timeDirs[timeI].name()))
            {
                Info<< "Skipping time " << timeDirs[timeI].name()
                    << endl << endl;
                continue;
            }


            // Set time for global database
            runTime.setTime(timeDirs[timeI], timeI);

            Info<< "Time = " << runTime.timeName() << endl << endl;

            // Set time for all databases
            forAll(databases, procI)
            {
                databases[procI].setTime(timeDirs[timeI], timeI);
            }

            // Check if any new meshes need to be read.
            dgMesh::readUpdateState meshStat = mesh2.readUpdate();

            dgMesh::readUpdateState procStat = procMeshes.readUpdate();

            if (procStat == dgMesh::POINTS_MOVED)
            {
                // Reconstruct the points for moving mesh cases and write
                // them out
                procMeshes.reconstructPoints(mesh2);
            }
            else if (meshStat != procStat)
            {
                WarningIn(args.executable())
                    << "readUpdate for the reconstructed mesh:"
                    << meshStat << nl
                    << "readUpdate for the processor meshes  :"
                    << procStat << nl
                    << "These should be equal or your addressing"
                    << " might be incorrect."
                    << " Please check your time directories for any "
                    << "mesh directories." << endl;
            }


            // Get list of objects from processor0 database
            IOobjectList objects
            (
                procMeshes.meshes()[0],
                databases[0].timeName()
            );

            {
                // If there are any FV fields, reconstruct them
                Info<< "Reconstructing DG fields" << nl << endl;

				dgFieldReconstructor dgReconstructor
                (
                    mesh2,
                    procMeshes.meshes(),
                    procMeshes.faceProcAddressing(),
                    procMeshes.cellProcAddressing(),
                    procMeshes.boundaryProcAddressing()
                );
				
		        dgReconstructor.reconstructDgVolumeFields<scalar>
                (
                    objects,
                    selectedFields
                );
 		        dgReconstructor.reconstructDgVolumeFields<vector>
                (
                    objects,
                    selectedFields
                );
		
                dgReconstructor.reconstructDgVolumeFields<sphericalTensor>
                (
                    objects,
                    selectedFields
                );
                dgReconstructor.reconstructDgVolumeFields<symmTensor>
                (
                    objects,
                    selectedFields
                );
                dgReconstructor.reconstructDgVolumeFields<tensor>
                (
                    objects,
                    selectedFields
                );

            }
            if (!noReconstructSets)
            {
                // Scan to find all sets
                HashTable<label> cSetNames;
                HashTable<label> fSetNames;
                HashTable<label> pSetNames;

                forAll(procMeshes.meshes(), procI)
                {
                    const dgMesh& procMesh = procMeshes.meshes()[procI];

                    // Note: look at sets in current time only or between
                    // mesh and current time?. For now current time. This will
                    // miss out on sets in intermediate times that have not
                    // been reconstructed.
                    IOobjectList objects
                    (
                        procMesh,
                        databases[0].timeName(),    //procMesh.facesInstance()
                        polyMesh::meshSubDir/"sets"
                    );

                    IOobjectList cSets(objects.lookupClass(cellSet::typeName));
                    forAllConstIter(IOobjectList, cSets, iter)
                    {
                        cSetNames.insert(iter.key(), cSetNames.size());
                    }

                    IOobjectList fSets(objects.lookupClass(faceSet::typeName));
                    forAllConstIter(IOobjectList, fSets, iter)
                    {
                        fSetNames.insert(iter.key(), fSetNames.size());
                    }
                    IOobjectList pSets(objects.lookupClass(pointSet::typeName));
                    forAllConstIter(IOobjectList, pSets, iter)
                    {
                        pSetNames.insert(iter.key(), pSetNames.size());
                    }
                }

                // Construct all sets
                PtrList<cellSet> cellSets(cSetNames.size());
                PtrList<faceSet> faceSets(fSetNames.size());
                PtrList<pointSet> pointSets(pSetNames.size());

                Info<< "Reconstructing sets:" << endl;
                if (cSetNames.size())
                {
                    Info<< "    cellSets " << cSetNames.sortedToc() << endl;
                }
                if (fSetNames.size())
                {
                    Info<< "    faceSets " << fSetNames.sortedToc() << endl;
                }
                if (pSetNames.size())
                {
                    Info<< "    pointSets " << pSetNames.sortedToc() << endl;
                }

                // Load sets
                forAll(procMeshes.meshes(), procI)
                {
                    const dgMesh& procMesh = procMeshes.meshes()[procI];

                    IOobjectList objects
                    (
                        procMesh,
                        databases[0].timeName(),    //procMesh.facesInstance(),
                        polyMesh::meshSubDir/"sets"
                    );

                    // cellSets
                    const labelList& cellMap =
                        procMeshes.cellProcAddressing()[procI];

                    IOobjectList cSets(objects.lookupClass(cellSet::typeName));
                    forAllConstIter(IOobjectList, cSets, iter)
                    {
                        // Load cellSet
                        const cellSet procSet(*iter());
                        label setI = cSetNames[iter.key()];
                        if (!cellSets.set(setI))
                        {
                            cellSets.set
                            (
                                setI,
                                new cellSet(mesh, iter.key(), procSet.size())
                            );
                        }
                        cellSet& cSet = cellSets[setI];

                        forAllConstIter(cellSet, procSet, iter)
                        {
                            cSet.insert(cellMap[iter.key()]);
                        }
                    }

                    // faceSets
                    const labelList& faceMap =
                        procMeshes.faceProcAddressing()[procI];

                    IOobjectList fSets(objects.lookupClass(faceSet::typeName));
                    forAllConstIter(IOobjectList, fSets, iter)
                    {
                        // Load faceSet
                        const faceSet procSet(*iter());
                        label setI = fSetNames[iter.key()];
                        if (!faceSets.set(setI))
                        {
                            faceSets.set
                            (
                                setI,
                                new faceSet(mesh, iter.key(), procSet.size())
                            );
                        }
                        faceSet& fSet = faceSets[setI];

                        forAllConstIter(faceSet, procSet, iter)
                        {
                            fSet.insert(mag(faceMap[iter.key()])-1);
                        }
                    }
                    // pointSets
                    const labelList& pointMap =
                        procMeshes.pointProcAddressing()[procI];

                    IOobjectList pSets(objects.lookupClass(pointSet::typeName));
                    forAllConstIter(IOobjectList, pSets, iter)
                    {
                        // Load pointSet
                        const pointSet propSet(*iter());
                        label setI = pSetNames[iter.key()];
                        if (!pointSets.set(setI))
                        {
                            pointSets.set
                            (
                                setI,
                                new pointSet(mesh, iter.key(), propSet.size())
                            );
                        }
                        pointSet& pSet = pointSets[setI];

                        forAllConstIter(pointSet, propSet, iter)
                        {
                            pSet.insert(pointMap[iter.key()]);
                        }
                    }
                }

                // Write sets
                forAll(cellSets, i)
                {
                    cellSets[i].write();
                }
                forAll(faceSets, i)
                {
                    faceSets[i].write();
                }
                forAll(pointSets, i)
                {
                    pointSets[i].write();
                }
            }
        }
    }

    // If there are any "uniform" directories copy them from
    // the master processor
    forAll(timeDirs, timeI)
    {
        runTime.setTime(timeDirs[timeI], timeI);
        databases[0].setTime(timeDirs[timeI], timeI);

        fileName uniformDir0 = databases[0].timePath()/"uniform";
        if (isDir(uniformDir0))
        {
            cp(uniformDir0, runTime.timePath());
        }
    }

    Info<< "End.\n" << endl;

    return 0;
}


// ************************************************************************* //
