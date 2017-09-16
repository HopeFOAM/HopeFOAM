paraT1 = std::exp(-nu.value()*8*std::pow(pi,2)*runTime.value());
paraT2 = std::exp(-nu.value()*4*std::pow(pi,2)*runTime.value());
paraT3 = std::exp(-nu.value()*8*std::pow(pi,2)*runTime.deltaT().value());
paraT4 = std::exp(-nu.value()*4*std::pow(pi,2)*runTime.deltaT().value());
