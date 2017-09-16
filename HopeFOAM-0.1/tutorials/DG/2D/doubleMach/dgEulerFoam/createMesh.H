
    Foam::Info
        << "Create mesh for time = "
        << runTime.timeName() << Foam::nl << Foam::endl;

    Foam::dgMesh mesh
    (
        Foam::IOobject
        (
            Foam::dgMesh::defaultRegion,
            runTime.timeName(),
            runTime,
            Foam::IOobject::MUST_READ
        )
    );
