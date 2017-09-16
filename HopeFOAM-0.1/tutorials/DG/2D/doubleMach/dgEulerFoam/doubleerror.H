        scalar residualRho = gSum(mag(rho-rho.oldTime()).ref())/mesh.localRange().second();
        scalar residualRhoU = gSum(mag(rhoU-rhoU.oldTime()).ref())/mesh.localRange().second();
        scalar residualEner = gSum(mag(Ener-Ener.oldTime()).ref())/mesh.localRange().second();
        Info << "resitual: Solving for residualRho, Initial residual = "<<residualRho<<", Final residual = "<< 0<<", No Iteration 1"<<endl;
        Info << "resitual: Solving for residualRhoU, Initial residual = "<<residualRhoU<<", Final residual = "<< 0<<", No Iteration 1"<<endl;
        Info << "resitual: Solving for residualEner, Initial residual = "<<residualEner<<", Final residual = "<< 0<<", No Iteration 1"<<endl;

