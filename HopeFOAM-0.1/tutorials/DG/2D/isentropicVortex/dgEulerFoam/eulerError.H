
{	
	//init field to sin(x)
	scalar pi = constant::mathematical::pi;
    vector origin(5.0, 0, 0);
    scalar beta=5.0;
    vector timeLoc(runTime.value(), 0, 0);
	
	//scalar lam = (0.5/nu.value()) - std::sqrt( (0.25/(sqr(nu.value()))) + 4*pi*pi );
	dgTree<physicalCellElement>::iteratorEnd end = mesh.cellElementsTree().end();
	int cellI = 0;
	for(dgTree<physicalCellElement>::leafIterator iter = mesh.cellElementsTree().leafBegin(); iter != end; ++iter, ++cellI)
	{
		const physicalCellElement& cellEle = iter()->value();
		label nDof = cellEle.nDof();
		label dofStart = cellEle.dofStart();
		const List<vector>& px = cellEle.dofLocation();
	
		SubField<scalar> cellDataDGRHO(rhoExact.internalField(), nDof, dofStart);
		SubField<vector> cellDataRHOU(rhoUExact.internalField(), nDof, dofStart);
		
		for(int pointI=0; pointI<nDof; pointI++){
	        scalar r = magSqr(px[pointI] - origin - timeLoc);
	        cellDataDGRHO[pointI] = std::pow(1.0-(gamma.value()-1.0)*(beta*beta)*std::exp(2.0*(1.0-r))/(16.0*gamma.value()*pi*pi), 1.0/(gamma.value()-1.0));

	        cellDataRHOU[pointI] = vector(1-beta*std::exp(1-r)*(px[pointI].y()-0)/(2.0*pi),
	                                beta*std::exp(1-r)*(px[pointI].x()-5 - runTime.value())/(2.0*pi), 0.0) * cellDataDGRHO[pointI];
		}
	}
}

// normal 2
rhoExact -= rho;
rhoUExact -= rhoU;

// output
Info << "rhoError: "<<gSum(mag(rhoExact.primitiveField()))/mesh.localRange().second()<<endl;
Info << "rhoUError: "<<gSum(mag(rhoUExact.primitiveField()))/mesh.localRange().second()<<endl;

