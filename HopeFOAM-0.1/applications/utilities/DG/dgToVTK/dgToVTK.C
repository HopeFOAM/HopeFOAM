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
    dgToVTK

Description
    Legacy VTK file format writer.

    - handles dgScalar, dgVector fields.
    - mesh topo changes.
    - both ascii and binary.
    - single time step writing.
    - automatic decomposition of high-order cells according to high-order points.
    - Visualize with quadratic cells in VTK.

Usage

    - dgToVTK [OPTION]
    
    Options:
        -ascii
            Write VTK data in ASCII format instead of binary.
        
        -fields <fields>
            Convert selected fields only. For example,
                -fields "( p T U )"
            The quoting is required to avoid shell expansions and to pass the
            information as a single argument.

        -noInternal
            Do not generate file for mesh, only for patches

        -noPointValues
            No dgPointFields

        -allPatches
            Combine all patches into a single file

        -excludePatches <patchNames>
            Specify patches (wildcards) to exclude. For example,
                -excludePatches '( inlet_1 inlet_2 "proc.*")'
            The quoting is required to avoid shell expansions and to pass the
            information as a single argument. The double quotes denote a regular
            expression.

        -useTimeName
            use the time index in the VTK file name instead of the time index
        
        -quadCell
            use quadratic cell to show field in VTK file

\*---------------------------------------------------------------------------*/

#include "timeSelector.H"
#include "pointMesh.H"
#include "volPointInterpolation.H"
#include "emptyPolyPatch.H"
#include "labelIOField.H"
#include "scalarIOField.H"
#include "sphericalTensorIOField.H"
#include "symmTensorIOField.H"
#include "tensorIOField.H"
#include "faceZoneMesh.H"
#include "Cloud.H"
#include "passiveParticle.H"
#include "stringListOps.H"

#include "vtkMesh.H"
#include "readFields.H"
#include "patchWriter.H"
#include "internalWriter.H"

#include "Time.H"
#include "dgGeoMesh.H"
#include "fixedValueDgPatchFields.H"
#include "zeroGradientDgPatchFields.H"

#include "dgFields.H"
#include "constants.H"
#include "OSspecific.H"
#include "argList.H"

#ifndef namespaceFoam
#define namespaceFoam
    using namespace Foam;
#endif

//*****************************************************************************//
template<class GeoField>
void print(const char* msg, Ostream& os, const PtrList<const GeoField>& flds)
{
    if (flds.size())
    {
        os  << msg;
        forAll(flds, i)
        {
            os  << ' ' << flds[i].name();
        }
        os  << endl;
    }
}


void print(Ostream& os, const wordList& flds)
{
    forAll(flds, i)
    {
        os  << ' ' << flds[i];
    }
    os  << endl;
}

labelList getSelectedPatches
(
    const dgBoundaryMesh& patches,
    const List<wordRe>& excludePatches
)
{
    DynamicList<label> patchIDs(patches.size());

    Info<< "Combining patches:" << endl;

    forAll(patches, patchi)
    {
        const dgPatch& pp = patches[patchi];

        if
        (
            isType<emptyPolyPatch>(pp.patch())
         || (Pstream::parRun() && isType<processorPolyPatch>(pp.patch()))
        )
        {
            Info<< "    discarding empty/processor patch " << patchi
                << " " << pp.patch().name() << endl;
        }
        else if (findStrings(excludePatches, pp.patch().name()))
        {
            Info<< "    excluding patch " << patchi
                << " " << pp.patch().name() << endl;
        }
        else
        {
            patchIDs.append(patchi);
            Info<< "    patch " << patchi << " " << pp.patch().name() << endl;
        }
    }
    return patchIDs.shrink();
}


int main(int argc, char *argv[])
{
    argList::addNote
    (
        "legacy VTK file format writer"
    );
    timeSelector::addOptions();

    #include "addRegionOption.H"
    argList::addOption
    (
        "fields",
        "wordList",
        "only convert the specified fields - eg '(p T U)'"
    );
    argList::addOption
    (
        "cellSet",
        "name",
        "convert a mesh subset corresponding to the specified cellSet"
    );
    argList::addOption
    (
        "faceSet",
        "name",
        "restrict conversion to the specified faceSet"
    );
    argList::addOption
    (
        "pointSet",
        "name",
        "restrict conversion to the specified pointSet"
    );
    argList::addBoolOption
    (
        "ascii",
        "write in ASCII format instead of binary"
    );
    argList::addBoolOption
    (
        "quadCell",
        "write the cell with two order cell"
    );
    argList::addBoolOption
    (
        "poly",
        "write polyhedral cells without tet/pyramid decomposition"
    );
    argList::addBoolOption
    (
        "surfaceFields",
        "write surfaceScalarFields (e.g., phi)"
    );
    argList::addBoolOption
    (
        "nearCellValue",
        "use cell value on patches instead of patch value itself"
    );
    argList::addBoolOption
    (
        "noInternal",
        "do not generate file for mesh, only for patches"
    );
    argList::addBoolOption
    (
        "noPointValues",
        "no pointFields"
    );
    argList::addBoolOption
    (
        "allPatches",
        "combine all patches into a single file"
    );
    argList::addOption
    (
        "excludePatches",
        "wordReList",
        "a list of patches to exclude - eg '( inlet \".*Wall\" )' "
    );
    argList::addBoolOption
    (
        "noFaceZones",
        "no faceZones"
    );
    argList::addBoolOption
    (
        "noLinks",
        "don't link processor VTK files - parallel only"
    );
    argList::addBoolOption
    (
        "useTimeName",
        "use the time name instead of the time index when naming the files"
    );

    #include "setRootCase.H"
    #include "createTime.H"

    const bool doWriteInternal = !args.optionFound("noInternal");
    const bool binary          = !args.optionFound("ascii");
    const bool useTimeName     = args.optionFound("useTimeName");
    const bool quadCell          = args.optionFound("quadCell");


    // decomposition of polyhedral cells into tets/pyramids cells
    vtkTopo::decomposePoly     = !args.optionFound("poly");

    if (binary && (sizeof(floatScalar) != 4 || sizeof(label) != 4))
    {
        FatalErrorIn(args.executable())
            << "floatScalar and/or label are not 4 bytes in size" << nl
            << "Hence cannot use binary VTK format. Please use -ascii"
            << exit(FatalError);
    }

    const bool nearCellValue = args.optionFound("nearCellValue");

    if (nearCellValue)
    {
        WarningIn(args.executable())
            << "Using neighbouring cell value instead of patch value"
            << nl << endl;
    }

    const bool noPointValues = args.optionFound("noPointValues");

    if (noPointValues)
    {
        WarningIn(args.executable())
            << "Outputting cell values only" << nl << endl;
    }

    const bool allPatches = args.optionFound("allPatches");

    List<wordRe> excludePatches;
    if (args.optionFound("excludePatches"))
    {
        args.optionLookup("excludePatches")() >> excludePatches;

        Info<< "Not including patches " << excludePatches << nl << endl;
    }

    word cellSetName;
    word faceSetName;
    word pointSetName;
    string vtkName = runTime.caseName();

    if (args.optionReadIfPresent("cellSet", cellSetName))
    {
        vtkName = cellSetName;
    }
    else if (Pstream::parRun())
    {
        // Strip off leading casename, leaving just processor_DDD ending.
        string::size_type i = vtkName.rfind("processor");

        if (i != string::npos)
        {
            vtkName = vtkName.substr(i);
        }
    }
    args.optionReadIfPresent("faceSet", faceSetName);
    args.optionReadIfPresent("pointSet", pointSetName);

    instantList timeDirs = timeSelector::select0(runTime, args);

    #include "createNamedDgMesh.H"

    //VTK/directory in the case
    fileName dgPath(runTime.path()/"VTK");
    //Directory of dgMesh (region0 gets filtered out)
    fileName regionPrefix = "";

    if (regionName != polyMesh::defaultRegion)
    {
        dgPath = dgPath/regionName;
        regionPrefix = regionName;
    }

    if(isDir(dgPath))
    {
        if 
        (
            args.optionFound("time")
        ||  args.optionFound("latestTime")
        ||  cellSetName.size()
        ||  faceSetName.size()
        ||  pointSetName.size()
        ||  regionName != polyMesh::defaultRegion
        )
        {
            /* code */
            Info<<"Keeping old VTK files in "<<dgPath<<nl<<endl;
        }
        else
        {
            Info<<"Deleting lod VTK files in "<<dgPath<<nl<<endl;
            rmDir(dgPath);
        }
    }//end if isDir(dgPath)

    mkDir(dgPath);

    // mesh wrapper; does subsetting and decomposition
    vtkMesh vMesh(mesh, cellSetName, quadCell);

    forAll(timeDirs, timeI)
    {
        runTime.setTime(timeDirs[timeI], timeI);

        Info<< "Time: " << runTime.timeName() << endl;

        word timeDesc =
            useTimeName ? runTime.timeName() : Foam::name(runTime.timeIndex());

        // Check for new polyMesh/ and update mesh, fvMeshSubset and cell
        // decomposition.
        polyMesh::readUpdateState meshState = vMesh.readUpdate();

        const dgMesh& mesh = vMesh.mesh();

        if
        (
            meshState == polyMesh::TOPO_CHANGE
         || meshState == polyMesh::TOPO_PATCH_CHANGE
        )
        {
            Info<< "    Read new mesh" << nl << endl;
        }

        // Search for list of objects for this time
        IOobjectList objects(mesh, runTime.timeName());

        HashSet<word> selectedFields;
        bool specifiedFields = args.optionReadIfPresent
        (
            "fields",
            selectedFields
        );

        PtrList<const dgScalarField> dgsf;
        PtrList<const dgVectorField> dgvf;
        PtrList<const dgSphericalTensorField> dgSpheretf;
        PtrList<const dgSymmTensorField> dgSymmtf;
        PtrList<const dgTensorField> dgtf;

        if (!noPointValues && !(specifiedFields && selectedFields.empty()))
        {
            readFields
            (
                vMesh,
                vMesh.baseMesh(),
                objects,
                selectedFields,
                dgsf
            );
            print("    dgScalarFields          :", Info, dgsf);

            readFields
            (
                vMesh,
                vMesh.baseMesh(),
                objects,
                selectedFields,
                dgvf
            );
            print("    dgVectorFields          :", Info, dgvf);

            readFields
            (
                vMesh,
                vMesh.baseMesh(),
                objects,
                selectedFields,
                dgSpheretf
            );
            print("    dgSphericalTensorFields :", Info, dgSpheretf);

            readFields
            (
                vMesh,
                vMesh.baseMesh(),
                objects,
                selectedFields,
                dgSymmtf
            );
            print("    dgSymmTensorFields      :", Info, dgSymmtf);

            readFields
            (
                vMesh,
                vMesh.baseMesh(),
                objects,
                selectedFields,
                dgtf
            );
            print("    dgTensorFields          :", Info, dgtf);
        }
        Info<< endl;

        label nDgFields =
            dgsf.size()
          + dgvf.size()
          + dgSpheretf.size()
          + dgSymmtf.size()
          + dgtf.size();

        Info <<"dgFields Num : " <<nDgFields <<endl;

        if (doWriteInternal)
        {
            // Create file and write header
            fileName vtkFileName
            (
                dgPath/vtkName
              + "_"
              + timeDesc
              + ".vtk"
            );

            Info<< "    Internal  : " << vtkFileName << endl;

            // Write mesh  xxxxxxx
            internalWriter writer(vMesh, binary, quadCell, vtkFileName);

            // write cellID and cellField generated by dgFields interpolation
            writeFuns::writeCellDataHeader
            (
                writer.os(),
                vMesh.nFieldCells(),
                1+nDgFields
            );
        

            // Write cellID field
            writer.writeCellIDs();
            
            Info<<"cellFields Num Generated by dgFields interpolation : "<<nDgFields<<endl;
            // Write cellField generated by dgFields interpolation 
            writer.writeDgToVol(dgsf);
            writer.writeDgToVol(dgvf);
            writer.writeDgToVol(dgSpheretf);
            writer.writeDgToVol(dgSymmtf);
            writer.writeDgToVol(dgtf);

            if (!noPointValues)
            {
                writeFuns::writeDgDataHeader
                (
                    writer.os(),
                    vMesh.nFieldPoints(),
                    nDgFields
                );

                // dgFields
                writer.write(dgsf);
                writer.write(dgvf);
                writer.write(dgSpheretf);
                writer.write(dgSymmtf);
                writer.write(dgtf);
            }
        }
        //---------------------------------------------------------------------
        //
        // Write patches (POLYDATA file, one for each patch)
        //
        //---------------------------------------------------------------------

        const dgBoundaryMesh& patches = mesh.boundary();

        if (allPatches)
        {
            mkDir(dgPath/"allPatches");

            fileName patchFileName;

            if (vMesh.useSubMesh())
            {
                patchFileName =
                    dgPath/"allPatches"/cellSetName
                  + "_"
                  + timeDesc
                  + ".vtk";
            }
            else
            {
                patchFileName =
                    dgPath/"allPatches"/"allPatches"
                  + "_"
                  + timeDesc
                  + ".vtk";
            }

            Info<< "    Combined patches     : " << patchFileName << endl;

            patchWriter writer
            (
                vMesh,
                binary,
                nearCellValue,
                patchFileName,
                getSelectedPatches(patches, excludePatches)
            );

            // patchID
            writeFuns::writeCellDataHeader
            (
                writer.os(),
                writer.nFaces(),
                1
            );

            // Write patchID field
            writer.writePatchIDs();

            if (!noPointValues)
            {
                writeFuns::writeDgDataHeader
                (
                    writer.os(),
                    writer.nPoints(),
                    nDgFields
                );

                // Write dgFields
                writer.write(dgsf);
                writer.write(dgvf);
                writer.write(dgSpheretf);
                writer.write(dgSymmtf);
                writer.write(dgtf);

                // no interpolated volFields since I cannot be bothered to
                // create the patchInterpolation for all subpatches.
            }
        }
        else
        {
            forAll(patches, patchi)
            {
                const dgPatch& pp = patches[patchi];

                if (!findStrings(excludePatches, pp.name()))
                {
                    mkDir(dgPath/pp.name());

                    fileName patchFileName;

                    if (vMesh.useSubMesh())
                    {
                        patchFileName =
                            dgPath/pp.name()/cellSetName
                          + "_"
                          + timeDesc
                          + ".vtk";
                    }
                    else
                    {
                        patchFileName =
                            dgPath/pp.name()/pp.name()
                          + "_"
                          + timeDesc
                          + ".vtk";
                    }

                    Info<< "    Patch     : " << patchFileName << endl;

                    patchWriter writer
                    (
                        vMesh,
                        binary,
                        nearCellValue,
                        patchFileName,
                        labelList(1, patchi)
                    );

                    if (!isA<emptyPolyPatch>(pp))
                    {
                        //patchID
                        writeFuns::writeCellDataHeader
                        (
                            writer.os(),
                            writer.nFaces(),
                            1
                        );

                        // Write patchID field
                        writer.writePatchIDs();

                        if (!noPointValues)
                        {
                            writeFuns::writeDgDataHeader
                            (
                                writer.os(),
                                writer.nPoints(),
                                nDgFields
                            );

                            // Write dgFields
                            writer.write(dgsf);
                            writer.write(dgvf);
                            writer.write(dgSpheretf);
                            writer.write(dgSymmtf);
                            writer.write(dgtf);
                        }
                    }
                }
            }
        }
    }
}