
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

    dimensionedScalar gamma
    (
        transportProperties.lookup("gamma")
    );


	// dg field
    Info<< "Reading field rho\n" << endl;
    dgScalarField rho
    (
        IOobject
        (
            "rho",
            runTime.timeName(),
            mesh,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        ),
        mesh
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

    dgVectorField rhoU
    (
        IOobject
        (
            "rhoU",
            runTime.timeName(),
            mesh,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        ),
        mesh
    );

    dgScalarField Ener
    (
        IOobject
        (
            "Ener",
            runTime.timeName(),
            mesh,
            IOobject::MUST_READ,
            IOobject::AUTO_WRITE
        ),
        mesh
    );
	
	/*********************gauss field*******************/
    rho.updateGaussField();
    rhoU.updateGaussField();
    Ener.updateGaussField();
	dgGaussVectorField gther_U("gther_U",U);
    dgGaussScalarField gther_p("gther_p",p);
	
    Info<< "Create Riemann solver\n" << endl;

    Foam::dg::godunovScheme Godunov(mesh, gther_U, gther_p, gamma.value(), rho.gaussField(), rhoU.gaussField(), Ener.gaussField());
	
    /* * * * * * * * output data * * * * * * * * * * */
 
    dgScalarField resu
    (
        IOobject
        (
            "resu",
            runTime.timeName(),
            mesh,
            IOobject::NO_READ,
            IOobject::NO_WRITE
        ),
        mesh,
        dimensionedScalar("zero",dimensionSet(1,-3,0,0,0,0,0), 0.0)
    );
   
	dgScalarField rhoExact("rhoExact", rho);
	dgVectorField rhoUExact("rhoUExact", rhoU); 
  
    scalar rk4a[5] = {
         0.0,
        -567301805773.0/1357537059087.0,
        -2404267990393.0/2016746695238.0,
        -3550918686646.0/2091501179385.0,
        -1275806237668.0/842570457699.0};

    scalar rk4b[5] = {
        1432997174477.0/9575080441755.0,
        5161836677717.0/13612068292357.0,
        1720146321549.0/2090206949498.0,
        3134564353537.0/4481467310338.0,
        2277821191437.0/14882151754819.0};

    scalar rk4c[5] = {
        0.0,
        1432997174477.0/9575080441755.0,
        2526269341429.0/6820363962896.0,
        2006345519317.0/3224310063776.0,
        2802321613138.0/2924317926251.0};

