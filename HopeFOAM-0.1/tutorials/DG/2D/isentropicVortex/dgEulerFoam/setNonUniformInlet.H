
//init field to sin(x)
scalar pi = constant::mathematical::pi;
vector origin(5.0, 0, 0);
scalar beta=5.0;

dgTree<physicalCellElement>::iteratorEnd end = mesh.cellElementsTree().end();
int cellI = 0;
for(dgTree<physicalCellElement>::leafIterator iter = mesh.cellElementsTree().leafBegin(); iter != end; ++iter, ++cellI)
{
    const physicalCellElement& cellEle = iter()->value();
    label nDof = cellEle.nDof();
    label dofStart = cellEle.dofStart();
    const List<vector>& px = cellEle.dofLocation();

    SubField<scalar> cellDataDGRHO(rho.internalField(), nDof, dofStart);
    SubField<vector> cellDataRHOU(rhoU.internalField(), nDof, dofStart);
    SubField<scalar> cellDataE(Ener.internalField(), nDof, dofStart);
    for(int pointI=0; pointI<nDof; pointI++){
        scalar r = magSqr(px[pointI] - origin);
        cellDataDGRHO[pointI] = std::pow(1.0-(gamma.value()-1.0)*(beta*beta)*std::exp(2.0*(1.0-r))/(16.0*gamma.value()*pi*pi), 1.0/(gamma.value()-1.0));

        cellDataRHOU[pointI] = vector(1-beta*std::exp(1-r)*(px[pointI].y()-0)/(2.0*pi),
                                beta*std::exp(1-r)*(px[pointI].x()-5)/(2.0*pi), 0.0) * cellDataDGRHO[pointI];

        cellDataE[pointI] = std::pow(cellDataDGRHO[pointI], gamma.value())/(gamma.value()-1.0)+0.5*magSqr(cellDataRHOU[pointI])/cellDataDGRHO[pointI];
    }
}

//set boundary value
const List<shared_ptr<dgTreeUnit<physicalFaceElement>>>& faceElement = mesh.faceElements();
forAll(p.boundaryFieldRef(), patchI){
    rho.boundaryField()[patchI].patchInternalField(rho.boundaryFieldRef()[patchI]);

    rhoU.boundaryField()[patchI].patchInternalField(rhoU.boundaryFieldRef()[patchI]);

    Ener.boundaryField()[patchI].patchInternalField(Ener.boundaryFieldRef()[patchI]);
}

rho.correctBoundaryConditions();
rhoU.correctBoundaryConditions();
Ener.correctBoundaryConditions();

