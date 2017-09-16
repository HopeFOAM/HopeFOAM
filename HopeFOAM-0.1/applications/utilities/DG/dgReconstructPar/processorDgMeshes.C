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

#include "processorDgMeshes.H"
#include "Time.H"
#include "primitiveMesh.H"

// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

void Foam::processorDgMeshes::read()
{
    // Make sure to clear (and hence unregister) any previously loaded meshes
    // and fields
    forAll(databases_, proci)
    {
        meshes_.set(proci, NULL);
        pointProcAddressing_.set(proci, NULL);
        faceProcAddressing_.set(proci, NULL);
        cellProcAddressing_.set(proci, NULL);
        boundaryProcAddressing_.set(proci, NULL);
    }

    forAll(databases_, proci)
    {

	//Info<<"----"<<   databases_[proci].timeName()<<endl;
        meshes_.set
        (
            proci,
            new dgMesh
            (
                IOobject
                (
                    meshName_,
                    databases_[proci].timeName(),
                    databases_[proci],
                    Foam::IOobject::MUST_READ
                )
            )
        );

        pointProcAddressing_.set
        (
            proci,
            new labelIOList
            (
                IOobject
                (
                    "pointProcAddressing",
                    meshes_[proci].facesInstance(),
                    meshes_[proci].meshSubDir(),
                    meshes_[proci],
                    IOobject::MUST_READ,
                    IOobject::NO_WRITE
                )
            )
        );

        faceProcAddressing_.set
        (
            proci,
            new labelIOList
            (
                IOobject
                (
                    "faceProcAddressing",
                    meshes_[proci].facesInstance(),
                    meshes_[proci].meshSubDir(),
                    meshes_[proci],
                    IOobject::MUST_READ,
                    IOobject::NO_WRITE
                )
            )
        );

        cellProcAddressing_.set
        (
            proci,
            new labelIOList
            (
                IOobject
                (
                    "cellProcAddressing",
                    meshes_[proci].facesInstance(),
                    meshes_[proci].meshSubDir(),
                    meshes_[proci],
                    IOobject::MUST_READ,
                    IOobject::NO_WRITE
                )
            )
        );

        boundaryProcAddressing_.set
        (
            proci,
            new labelIOList
            (
                IOobject
                (
                    "boundaryProcAddressing",
                    meshes_[proci].facesInstance(),
                    meshes_[proci].meshSubDir(),
                    meshes_[proci],
                    IOobject::MUST_READ,
                    IOobject::NO_WRITE
                )
            )
        );
    }
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::processorDgMeshes::processorDgMeshes
(
    PtrList<Time>& databases,
    const word& meshName
)
:
    meshName_(meshName),
    databases_(databases),
    meshes_(databases.size()),
    pointProcAddressing_(databases.size()),
    faceProcAddressing_(databases.size()),
    cellProcAddressing_(databases.size()),
    boundaryProcAddressing_(databases.size())
{
    read();
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

Foam::dgMesh::readUpdateState Foam::processorDgMeshes::readUpdate()
{
    dgMesh::readUpdateState stat = dgMesh::UNCHANGED;

    forAll(databases_, proci)
    {
        // Check if any new meshes need to be read.
        dgMesh::readUpdateState procStat = meshes_[proci].readUpdate();

        // Combine into overall mesh change status
        if (stat == dgMesh::UNCHANGED)
        {
            stat = procStat;
        }
        else if (stat != procStat)
        {
            FatalErrorInFunction
                << "Processor " << proci
                << " has a different polyMesh at time "
                << databases_[proci].timeName()
                << " compared to any previous processors." << nl
                << "Please check time " << databases_[proci].timeName()
                << " directories on all processors for consistent"
                << " mesh files."
                << exit(FatalError);
        }
    }

    if
    (
        stat == dgMesh::TOPO_CHANGE
     || stat == dgMesh::TOPO_PATCH_CHANGE
    )
    {
        // Reread all meshes and addresssing
        read();
    }
    return stat;
}


void Foam::processorDgMeshes::reconstructPoints(dgMesh& mesh)
{
    // Read the field for all the processors
    PtrList<pointIOField> procsPoints(meshes_.size());

    forAll(meshes_, proci)
    {
        procsPoints.set
        (
            proci,
            new pointIOField
            (
                IOobject
                (
                    "points",
                    meshes_[proci].time().timeName(),
                    polyMesh::meshSubDir,
                    meshes_[proci],
                    IOobject::MUST_READ,
                    IOobject::NO_WRITE,
                    false
                )
            )
        );
    }

    // Create the new points
    vectorField newPoints(mesh.nPoints());

    forAll(meshes_, proci)
    {
        const vectorField& procPoints = procsPoints[proci];

        // Set the cell values in the reconstructed field

        const labelList& pointProcAddressingI = pointProcAddressing_[proci];

        if (pointProcAddressingI.size() != procPoints.size())
        {
            FatalErrorInFunction
                << "problem :"
                << " pointProcAddressingI:" << pointProcAddressingI.size()
                << " procPoints:" << procPoints.size()
                << abort(FatalError);
        }

        forAll(pointProcAddressingI, pointi)
        {
            newPoints[pointProcAddressingI[pointi]] = procPoints[pointi];
        }
    }

    mesh.movePoints(newPoints);
    mesh.write();
}


// ************************************************************************* //
