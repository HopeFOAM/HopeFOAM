{
//init field to sin(x)
scalar pi = constant::mathematical::pi;
scalar gTime = (1+20*runTime.value())/std::sqrt(3.0);
scalar position;

//scalar lam = (0.5/nu.value()) - std::sqrt( (0.25/(sqr(nu.value()))) + 4*pi*pi );
dgTree<physicalCellElement>::iteratorEnd end = mesh.cellElementsTree().end();
int cellI = 0;
for(dgTree<physicalCellElement>::leafIterator iter = mesh.cellElementsTree().leafBegin(); iter != end; ++iter, ++cellI)
{
    const physicalCellElement& cellEle = iter()->value();
    label nDof = cellEle.nDof();
    label dofStart = cellEle.dofStart();
    const List<vector>& px = cellEle.dofLocation();

    SubField<scalar> cellDataRHO(rho.internalField(), nDof, dofStart);
    SubField<vector> cellDataRHOU(rhoU.internalField(), nDof, dofStart);
    SubField<scalar> cellDataE(Ener.internalField(), nDof, dofStart);
    for(int i=0; i<nDof; i++){
        scalar x = px[i].x();
        scalar y = px[i].y();
        position = 1.0/gTime*(x-1.0/6.0)-y;

        if(position < 0){
            cellDataRHO[i] = 8.0;

            cellDataRHOU[i] = vector(8.25*std::cos(pi/6.0)*8.0, -8.25*std::sin(pi/6.0)*8.0, 0.0);

            cellDataE[i] = 116.5/(gamma.value()-1.0) + (magSqr(cellDataRHOU[i]))/(2*cellDataRHO[i]);
        }
        else{
            cellDataRHO[i] = 1.4;

            cellDataRHOU[i] = vector::zero;
            
            cellDataE[i] = 1.0/(gamma.value() -1.0) + (magSqr(cellDataRHOU[i]))/(2*cellDataRHO[i]);            
        }
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

}
