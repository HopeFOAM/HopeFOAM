    Info<< "Reading transportProperties\n" << endl;

    IOdictionary transportProperties
    (
        IOobject
        (
            "transportProperties",
            runTime.constant(),
            mesh,
            IOobject::MUST_READ_IF_MODIFIED,
            IOobject::NO_WRITE
        )
    );

    dimensionedScalar nu
    (
        transportProperties.lookup("nu")
    );

    Info<< "Reading field p\n" << endl;
    dgScalarField p
    (
        IOobject
        (
            "p",
            runTime.timeName(),
            mesh,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        ),
        mesh
    );

    dgScalarField exact_p("exact_p", p);

    dgScalarField pZero
    (
        IOobject
        (
            "pZero",
            runTime.timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        mesh,
        dimensionedScalar("zero",dimensionSet(0,0,-2,0,0,0,0), 0)

    );


    Info<< "Reading field U\n" << endl;
    dgVectorField U
    (
        IOobject
        (
            "U",
            runTime.timeName(),
            mesh,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        ),
        mesh
    );

    dgVectorField UZero(
        IOobject
        (
            "UT",
            runTime.timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        mesh,
        dimensionedVector("zero",dimensionSet(0,1,-1,0,0,0,0), vector::zero)
    );

    dgVectorField UT(
        IOobject
        (
            "UT",
            runTime.timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        mesh,
        dimensionedVector("zero",dimensionSet(0,1,-1,0,0,0,0), vector::zero),
        "zeroGradient"
    );

    dgVectorField convecU
    (
        IOobject
        (
            "convecU",
            runTime.timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        mesh,
        dimensionedVector("zero",dimensionSet(0,1,-2,0,0,0,0), vector::zero)
    );

    dgVectorField convecUold
    (
        "convecUold",
        convecU
    );

    dgVectorField curlU
    (
        IOobject
        (
            "curlU",
            runTime.timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        mesh,
        dimensionedVector("zero",dimensionSet(0,1,-2,0,0,0,0), vector::zero)
    );

    dgScalarField nbp
    (
        IOobject
        (
            "nbp",
            runTime.timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        mesh,
        dimensionedScalar("zero",dimensionSet(0,1,-2,0,0,0,0), 0)
    );

    dgScalarField nbpOld
    (
        "nbpOld",
        nbp
    );

    scalar g0 = 1.0, a0 = 1.0, a1 = 0, b0 = 1.0, b1 = 0;
    

    label pRefCell = 0;
    scalar pRefValue = 0.0;

