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

#include "dgForces.H"
#include "dgcGrad.H"
#include "dgmSup.H"
#include "Equations.H"
//#include "porosityModel.H"
//#include "turbulentTransportModel.H"
//#include "turbulentFluidThermoModel.H"
#include "addToRunTimeSelectionTable.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
namespace functionObjects
{
    defineTypeNameAndDebug(dgForces, 0);

    addToRunTimeSelectionTable(functionObject, dgForces, dictionary);
}
}


// * * * * * * * * * * * * Protected Member Functions  * * * * * * * * * * * //

Foam::wordList Foam::functionObjects::dgForces::createFileNames
(
    const dictionary& dict
) const
{
    DynamicList<word> names(1);

    const word forceType(dict.lookup("type"));

    if (dict.found("binData"))
    {
        const dictionary& binDict(dict.subDict("binData"));
        label nb = readLabel(binDict.lookup("nBin"));
        if (nb > 0)
        {
            names.append(forceType + "_bins");
        }
    }

    names.append(forceType);

    return names;
}


void Foam::functionObjects::dgForces::writeFileHeader(const label i)
{
    if (i == 0)
    {
        // force data

        writeHeader(file(i), "Forces");
        writeHeaderValue(file(i), "CofR", coordSys_.origin());
        writeCommented(file(i), "Time");

        file(i)
            << "forces(pressure viscous porous) "
            << "moment(pressure viscous porous)";

        if (localSystem_)
        {
            file(i)
                << tab
                << "localForces(pressure,viscous,porous) "
                << "localMoments(pressure,viscous,porous)";
        }
    }
    else if (i == 1)
    {
        // bin data

        writeHeader(file(i), "Force bins");
        writeHeaderValue(file(i), "bins", nBin_);
        writeHeaderValue(file(i), "start", binMin_);
        writeHeaderValue(file(i), "delta", binDx_);
        writeHeaderValue(file(i), "direction", binDir_);

        vectorField binPoints(nBin_);
        writeCommented(file(i), "x co-ords  :");
        forAll(binPoints, pointi)
        {
            binPoints[pointi] = (binMin_ + (pointi + 1)*binDx_)*binDir_;
            file(i) << tab << binPoints[pointi].x();
        }
        file(i) << nl;

        writeCommented(file(i), "y co-ords  :");
        forAll(binPoints, pointi)
        {
            file(i) << tab << binPoints[pointi].y();
        }
        file(i) << nl;

        writeCommented(file(i), "z co-ords  :");
        forAll(binPoints, pointi)
        {
            file(i) << tab << binPoints[pointi].z();
        }
        file(i) << nl;

        writeCommented(file(i), "Time");

        for (label j = 0; j < nBin_; j++)
        {
            const word jn('(' + Foam::name(j) + ')');
            const word f("forces" + jn + "[pressure,viscous,porous]");
            const word m("moments" + jn + "[pressure,viscous,porous]");

            file(i)<< tab << f << tab << m;
        }
        if (localSystem_)
        {
            for (label j = 0; j < nBin_; j++)
            {
                const word jn('(' + Foam::name(j) + ')');
                const word f("localForces" + jn + "[pressure,viscous,porous]");
                const word m("localMoments" + jn + "[pressure,viscous,porous]");

                file(i)<< tab << f << tab << m;
            }
        }
    }
    else
    {
        FatalErrorInFunction
            << "Unhandled file index: " << i
            << abort(FatalError);
    }

    file(i)<< endl;
}


void Foam::functionObjects::dgForces::initialise()
{
    if (initialised_)
    {
        return;
    }

    if (directForceDensity_)
    {
        if (!obr_.foundObject<dgVectorField>(fDName_))
        {
            FatalErrorInFunction
                << "Could not find " << fDName_ << " in database."
                << exit(FatalError);
        }
    }
    else
    {
        if
        (
            !obr_.foundObject<dgVectorField>(UName_)
         || !obr_.foundObject<dgScalarField>(pName_)

        )
        {
            FatalErrorInFunction
                << "Could not find " << UName_ << ", " << pName_
                << exit(FatalError);
        }

        if
        (
            rhoName_ != "rhoInf"
         && !obr_.foundObject<dgScalarField>(rhoName_)
        )
        {
            FatalErrorInFunction
                << "Could not find " << rhoName_
                << exit(FatalError);
        }
    }

    initialised_ = true;
}


Foam::tmp<Foam::dgSymmTensorField>
Foam::functionObjects::dgForces::devRhoReff() const
{

    if (obr_.foundObject<dictionary>("transportProperties"))
    {
        const dictionary& transportProperties =
             obr_.lookupObject<dictionary>("transportProperties");

        dimensionedScalar nu(transportProperties.lookup("nu"));

        const dgVectorField& U = obr_.lookupObject<dgVectorField>(UName_);

        IOobject gradIOobject
        (
            "grad("+U.name()+')',
            U.instance(),
            U.mesh(),
            IOobject::NO_READ,
            IOobject::NO_WRITE
        );

        tmp<GeometricDofField<tensor, dgPatchField, dgGeoMesh> > tgGrad
        (
            new GeometricDofField<tensor, dgPatchField, dgGeoMesh>
            (
                gradIOobject,
                U.mesh(),
                dimensioned<tensor>
                (
                    "0",
                    U.dimensions()/dimLength,
                    pTraits<tensor>::zero
                )
            )
        );
        GeometricDofField<tensor, dgPatchField, dgGeoMesh>& gGrad = tgGrad.ref();
        
        dg::solveEquation(dgm::Sp(gGrad) == dgc::grad(U));

        return -rho()*nu*dev(twoSymm(tgGrad));
    }
    else
    {
        FatalErrorInFunction
            << "No valid model for viscous stress calculation"
            << exit(FatalError);

        return dgSymmTensorField::null();
    }
}


Foam::tmp<Foam::dgScalarField> Foam::functionObjects::dgForces::mu() const
{
    if (obr_.foundObject<dictionary>("transportProperties"))
    {
        const dictionary& transportProperties =
             obr_.lookupObject<dictionary>("transportProperties");

        dimensionedScalar nu(transportProperties.lookup("nu"));

        return rho()*nu;
    }
    else
    {
        FatalErrorInFunction
            << "No valid model for dynamic viscosity calculation"
            << exit(FatalError);

        return dgScalarField::null();
    }
}


Foam::tmp<Foam::dgScalarField> Foam::functionObjects::dgForces::rho() const
{
    if (rhoName_ == "rhoInf")
    {
        const dgMesh& mesh = refCast<const dgMesh>(obr_);

        return tmp<dgScalarField>
        (
            new dgScalarField
            (
                IOobject
                (
                    "rho",
                    mesh.time().timeName(),
                    mesh
                ),
                mesh,
                dimensionedScalar("rho", dimDensity, rhoRef_)
            )
        );
    }
    else
    {
        return(obr_.lookupObject<dgScalarField>(rhoName_));
    }
}


Foam::scalar Foam::functionObjects::dgForces::rho(const dgScalarField& p) const
{
    if (p.dimensions() == dimPressure)
    {
        return 1.0;
    }
    else
    {
        if (rhoName_ != "rhoInf")
        {
            FatalErrorInFunction
                << "Dynamic pressure is expected but kinematic is provided."
                << exit(FatalError);
        }

        return rhoRef_;
    }
}


void Foam::functionObjects::dgForces::applyBins
(
    const vectorField& Md,
    const vectorField& fN,
    const vectorField& fT,
    const vectorField& fP,
    const vectorField& d
)
{
    if (nBin_ == 1)
    {
        force_[0][0] += sum(fN);
        force_[1][0] += sum(fT);
        force_[2][0] += sum(fP);
        moment_[0][0] += sum(Md^fN);
        moment_[1][0] += sum(Md^fT);
        moment_[2][0] += sum(Md^fP);
    }
    else
    {
        scalarField dd((d & binDir_) - binMin_);

        forAll(dd, i)
        {
            label bini = min(max(floor(dd[i]/binDx_), 0), force_[0].size() - 1);

            force_[0][bini] += fN[i];
            force_[1][bini] += fT[i];
            force_[2][bini] += fP[i];
            moment_[0][bini] += Md[i]^fN[i];
            moment_[1][bini] += Md[i]^fT[i];
            moment_[2][bini] += Md[i]^fP[i];
        }
    }
}


void Foam::functionObjects::dgForces::writeForces()
{
    Log << type() << " " << name() << " write:" << nl
        << "    sum of forces:" << nl
        << "        pressure : " << sum(force_[0]) << nl
        << "        viscous  : " << sum(force_[1]) << nl
        << "        porous   : " << sum(force_[2]) << nl
        << "    sum of moments:" << nl
        << "        pressure : " << sum(moment_[0]) << nl
        << "        viscous  : " << sum(moment_[1]) << nl
        << "        porous   : " << sum(moment_[2])
        << endl;

    writeTime(file(0));
    file(0) << tab << setw(1) << '('
        << sum(force_[0]) << setw(1) << ' '
        << sum(force_[1]) << setw(1) << ' '
        << sum(force_[2]) << setw(3) << ") ("
        << sum(moment_[0]) << setw(1) << ' '
        << sum(moment_[1]) << setw(1) << ' '
        << sum(moment_[2]) << setw(1) << ')'
        << endl;

    if (localSystem_)
    {
        vectorField localForceN(coordSys_.localVector(force_[0]));
        vectorField localForceT(coordSys_.localVector(force_[1]));
        vectorField localForceP(coordSys_.localVector(force_[2]));
        vectorField localMomentN(coordSys_.localVector(moment_[0]));
        vectorField localMomentT(coordSys_.localVector(moment_[1]));
        vectorField localMomentP(coordSys_.localVector(moment_[2]));

        writeTime(file(0));
        file(0) << tab << setw(1) << '('
            << sum(localForceN) << setw(1) << ' '
            << sum(localForceT) << setw(1) << ' '
            << sum(localForceP) << setw(3) << ") ("
            << sum(localMomentN) << setw(1) << ' '
            << sum(localMomentT) << setw(1) << ' '
            << sum(localMomentP) << setw(1) << ')'
            << endl;
    }
}


void Foam::functionObjects::dgForces::writeBins()
{
    if (nBin_ == 1)
    {
        return;
    }

    List<Field<vector>> f(force_);
    List<Field<vector>> m(moment_);

    if (binCumulative_)
    {
        for (label i = 1; i < f[0].size(); i++)
        {
            f[0][i] += f[0][i-1];
            f[1][i] += f[1][i-1];
            f[2][i] += f[2][i-1];

            m[0][i] += m[0][i-1];
            m[1][i] += m[1][i-1];
            m[2][i] += m[2][i-1];
        }
    }

    writeTime(file(1));

    forAll(f[0], i)
    {
        file(1)
            << tab << setw(1) << '('
            << f[0][i] << setw(1) << ' '
            << f[1][i] << setw(1) << ' '
            << f[2][i] << setw(3) << ") ("
            << m[0][i] << setw(1) << ' '
            << m[1][i] << setw(1) << ' '
            << m[2][i] << setw(1) << ')';
    }

    if (localSystem_)
    {
        List<Field<vector>> lf(3);
        List<Field<vector>> lm(3);
        lf[0] = coordSys_.localVector(force_[0]);
        lf[1] = coordSys_.localVector(force_[1]);
        lf[2] = coordSys_.localVector(force_[2]);
        lm[0] = coordSys_.localVector(moment_[0]);
        lm[1] = coordSys_.localVector(moment_[1]);
        lm[2] = coordSys_.localVector(moment_[2]);

        if (binCumulative_)
        {
            for (label i = 1; i < lf[0].size(); i++)
            {
                lf[0][i] += lf[0][i-1];
                lf[1][i] += lf[1][i-1];
                lf[2][i] += lf[2][i-1];
                lm[0][i] += lm[0][i-1];
                lm[1][i] += lm[1][i-1];
                lm[2][i] += lm[2][i-1];
            }
        }

        forAll(lf[0], i)
        {
            file(1)
                << tab << setw(1) << '('
                << lf[0][i] << setw(1) << ' '
                << lf[1][i] << setw(1) << ' '
                << lf[2][i] << setw(3) << ") ("
                << lm[0][i] << setw(1) << ' '
                << lm[1][i] << setw(1) << ' '
                << lm[2][i] << setw(1) << ')';
        }
    }

    file(1) << endl;
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::functionObjects::dgForces::dgForces
(
    const word& name,
    const Time& runTime,
    const dictionary& dict
)
:
    writeFiles(name, runTime, dict, name),
    force_(3),
    moment_(3),
    patchSet_(),
    pName_(word::null),
    UName_(word::null),
    rhoName_(word::null),
    directForceDensity_(false),
    fDName_(""),
    rhoRef_(VGREAT),
    pRef_(0),
    coordSys_(),
    localSystem_(false),
    porosity_(false),
    nBin_(1),
    binDir_(Zero),
    binDx_(0.0),
    binMin_(GREAT),
    binPoints_(),
    binCumulative_(true),
    initialised_(false)
{
    if (!isA<dgMesh>(obr_))
    {
        FatalErrorInFunction
            << "objectRegistry is not an dgMesh" << exit(FatalError);
    }

    read(dict);
    resetNames(createFileNames(dict));
}


Foam::functionObjects::dgForces::dgForces
(
    const word& name,
    const objectRegistry& obr,
    const dictionary& dict
)
:
    writeFiles(name, obr, dict, name),
    force_(3),
    moment_(3),
    patchSet_(),
    pName_(word::null),
    UName_(word::null),
    rhoName_(word::null),
    directForceDensity_(false),
    fDName_(""),
    rhoRef_(VGREAT),
    pRef_(0),
    coordSys_(),
    localSystem_(false),
    porosity_(false),
    nBin_(1),
    binDir_(Zero),
    binDx_(0.0),
    binMin_(GREAT),
    binPoints_(),
    binCumulative_(true),
    initialised_(false)
{
    if (!isA<dgMesh>(obr_))
    {
        FatalErrorInFunction
            << "objectRegistry is not an dgMesh" << exit(FatalError);
    }

    read(dict);
    resetNames(createFileNames(dict));
}


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

Foam::functionObjects::dgForces::~dgForces()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

bool Foam::functionObjects::dgForces::read(const dictionary& dict)
{
    writeFiles::read(dict);

    initialised_ = false;

    Log << type() << " " << name() << ":" << nl;

    directForceDensity_ = dict.lookupOrDefault("directForceDensity", false);

    const dgMesh& mesh = refCast<const dgMesh>(obr_);
    const polyBoundaryMesh& pbm = mesh.boundaryMesh();

    patchSet_ = pbm.patchSet(wordReList(dict.lookup("patches")));

    if (directForceDensity_)
    {
        // Optional entry for fDName
        fDName_ = dict.lookupOrDefault<word>("fD", "fD");
    }
    else
    {
        // Optional entries U and p
        pName_ = dict.lookupOrDefault<word>("p", "p");
        UName_ = dict.lookupOrDefault<word>("U", "U");
        rhoName_ = dict.lookupOrDefault<word>("rho", "rho");

        // Reference density needed for incompressible calculations
        if (rhoName_ == "rhoInf")
        {
            rhoRef_ = readScalar(dict.lookup("rhoInf"));
        }

        // Reference pressure, 0 by default
        pRef_ = dict.lookupOrDefault<scalar>("pRef", 0.0);
    }

    coordSys_.clear();

    // Centre of rotation for moment calculations
    // specified directly, from coordinate system, or implicitly (0 0 0)
    if (!dict.readIfPresent<point>("CofR", coordSys_.origin()))
    {
        coordSys_ = coordinateSystem(obr_, dict);
        localSystem_ = true;
    }

    dict.readIfPresent("porosity", porosity_);
    if (porosity_)
    {
        Log << "    Including porosity effects" << endl;
    }
    else
    {
        Log << "    Not including porosity effects" << endl;
    }

    if (dict.found("binData"))
    {
        const dictionary& binDict(dict.subDict("binData"));
        binDict.lookup("nBin") >> nBin_;

        if (nBin_ < 0)
        {
            FatalIOErrorInFunction(dict)
                << "Number of bins (nBin) must be zero or greater"
                << exit(FatalIOError);
        }
        else if ((nBin_ == 0) || (nBin_ == 1))
        {
            nBin_ = 1;
            forAll(force_, i)
            {
                force_[i].setSize(1);
                moment_[i].setSize(1);
            }
        }

        if (nBin_ > 1)
        {
            binDict.lookup("direction") >> binDir_;
            binDir_ /= mag(binDir_);

            binMin_ = GREAT;
            scalar binMax = -GREAT;
            forAllConstIter(labelHashSet, patchSet_, iter)
            {
                label patchi = iter.key();
                const polyPatch& pp = pbm[patchi];
                scalarField d(pp.faceCentres() & binDir_);
                binMin_ = min(min(d), binMin_);
                binMax = max(max(d), binMax);
            }
            reduce(binMin_, minOp<scalar>());
            reduce(binMax, maxOp<scalar>());

            // slightly boost binMax so that region of interest is fully
            // within bounds
            binMax = 1.0001*(binMax - binMin_) + binMin_;

            binDx_ = (binMax - binMin_)/scalar(nBin_);

            // create the bin points used for writing
            binPoints_.setSize(nBin_);
            forAll(binPoints_, i)
            {
                binPoints_[i] = (i + 0.5)*binDir_*binDx_;
            }

            binDict.lookup("cumulative") >> binCumulative_;

            // allocate storage for forces and moments
            forAll(force_, i)
            {
                force_[i].setSize(nBin_);
                moment_[i].setSize(nBin_);
            }
        }
    }

    if (nBin_ == 1)
    {
        // allocate storage for forces and moments
        force_[0].setSize(1);
        force_[1].setSize(1);
        force_[2].setSize(1);
        moment_[0].setSize(1);
        moment_[1].setSize(1);
        moment_[2].setSize(1);
    }

    return true;
}


void Foam::functionObjects::dgForces::calcForcesMoment()
{
    initialise();

    force_[0] = Zero;
    force_[1] = Zero;
    force_[2] = Zero;

    moment_[0] = Zero;
    moment_[1] = Zero;
    moment_[2] = Zero;

    if (directForceDensity_)
    {
        const dgVectorField& fD = obr_.lookupObject<dgVectorField>(fDName_);

        const dgMesh& mesh = fD.mesh();

        const dgBoundaryMesh& bMesh = mesh.boundary();

        const typename Foam::GeometricField<vector, dgPatchField, dgGeoMesh>::Boundary& bData = fD.boundaryField(); 
        const dgTree<physicalFaceElement>& faceEleTree = mesh.faceElementsTree();

        forAllConstIter(labelHashSet, patchSet_, iter){
            label patchI = iter.key();
            const labelList& dgFaceIndex = bMesh[patchI].dgFaceIndex();
            label totalGaussDof = 0;
            for(dgTree<physicalFaceElement>::leafIterator iterT = faceEleTree.leafBegin(dgFaceIndex) ; iterT != faceEleTree.end() ; ++iterT){
                totalGaussDof += iterT()->value().nGaussPoints_;
            }
            vectorField stdDofLoc(totalGaussDof);

            vectorField fN(totalGaussDof);
            vectorField fT(totalGaussDof);
            label gaussBase = 0;
            for(dgTree<physicalFaceElement>::leafIterator iterT = faceEleTree.leafBegin(dgFaceIndex) ; iterT != faceEleTree.end() ; ++iterT){
                const physicalFaceElement& dgFaceI = iterT()->value();
                const physicalCellElement& dgCellI = dgFaceI.ownerEle_->value();
                label ownerDof = dgFaceI.nOwnerDof();
                label gaussOrder = dgFaceI.gaussOrder_;
                const Pair<label> sequenceIndex = dgFaceI.sequenceIndex();
                SubList<vector> subBoundary(bData[patchI], ownerDof, sequenceIndex[1]);
                const labelList& ownerDofMapping = const_cast<physicalFaceElement&>(dgFaceI).ownerDofMapping();

                const denseMatrix<scalar>& gaussFaceInterp_O =  const_cast<stdElement&>(dgCellI.baseFunction()).gaussFaceInterpolateMatrix(gaussOrder, 0);
                vectorField facePoint(ownerDof);
                forAll(facePoint, pointI){
                    facePoint[pointI] = dgCellI.dofLocation()[ownerDofMapping[pointI]];
                }
                vectorField gaussPoint(dgFaceI.nGaussPoints_, Zero);
                vectorField gaussPartFD(dgFaceI.nGaussPoints_, Zero);
                for(int i=0, ptr=0; i<dgFaceI.nGaussPoints_; i++){
                    for(int j=0; j<ownerDof; j++, ptr++){
                        gaussPoint[i] += gaussFaceInterp_O[ptr] * facePoint[j];
                        gaussPartFD[i] += gaussFaceInterp_O[ptr] * subBoundary[j];
                    }
                }

                forAll(gaussPoint, pointI){
                    stdDofLoc[gaussBase+pointI] = gaussPoint[pointI];
                    fN[gaussBase+pointI] = dgFaceI.faceWJ_[pointI] * dgFaceI.faceNx_[pointI] * (gaussPartFD[pointI] & dgFaceI.faceNx_[pointI]);
                    fT[gaussBase+pointI] = dgFaceI.faceWJ_[pointI] * gaussPartFD[pointI] - fN[gaussBase+pointI];
                }
                gaussBase += dgFaceI.nGaussPoints_;
            }

            vectorField Md
            (
                stdDofLoc - coordSys_.origin()
            );

            //- Porous force
            vectorField fP(Md.size(), Zero);

            applyBins(Md, fN, fT, fP, stdDofLoc);
        }
    }
    else
    {
        const dgVectorField& U = obr_.lookupObject<dgVectorField>(UName_);
        const dgScalarField& p = obr_.lookupObject<dgScalarField>(pName_);
        const dgMesh& mesh = U.mesh();

        tmp<dgSymmTensorField> tdevRhoReff = devRhoReff();
        const dgSymmTensorField::Boundary& devRhoReffb
            = tdevRhoReff().boundaryField();

        // Scale pRef by density for incompressible simulations
        scalar pRef = pRef_/rho(p);
        const dgScalarField::Boundary& bP = p.boundaryField(); 
        const dgBoundaryMesh& bMesh = mesh.boundary();
        const dgTree<physicalFaceElement>& faceEleTree = mesh.faceElementsTree();

        forAllConstIter(labelHashSet, patchSet_, iter){
            label patchI = iter.key();
            const labelList& dgFaceIndex = bMesh[patchI].dgFaceIndex();
            label totalGaussDof = 0;
            for(dgTree<physicalFaceElement>::leafIterator iterT = faceEleTree.leafBegin(dgFaceIndex) ; iterT != faceEleTree.end() ; ++iterT){
                totalGaussDof += iterT()->value().nGaussPoints_;
            }
            vectorField stdDofLoc(totalGaussDof);
            label gaussBase = 0;

            // Normal force = surfaceUnitNormal*(surfaceNormal & forceDensity)
            vectorField fN(totalGaussDof);
            // Tangential force (total force minus normal fN)
            vectorField fT(totalGaussDof);
            for(dgTree<physicalFaceElement>::leafIterator iterT = faceEleTree.leafBegin(dgFaceIndex) ; iterT != faceEleTree.end() ; ++iterT){
                const physicalFaceElement& dgFaceI = iterT()->value();
                const physicalCellElement& dgCellI = dgFaceI.ownerEle_->value();
                label ownerDof = dgFaceI.nOwnerDof();
                label gaussOrder = dgFaceI.gaussOrder_;
                const Pair<label> sequenceIndex = dgFaceI.sequenceIndex();
                SubList<symmTensor> subDev(devRhoReffb[patchI], ownerDof, sequenceIndex[1]);
                SubList<scalar> subBP(bP[patchI], ownerDof, sequenceIndex[1]);
                const labelList& ownerDofMapping = const_cast<physicalFaceElement&>(dgFaceI).ownerDofMapping();

                const denseMatrix<scalar>& gaussFaceInterp_O = const_cast<stdElement&>(dgCellI.baseFunction()).gaussFaceInterpolateMatrix(gaussOrder, 0);
                vectorField facePoint(ownerDof);
                forAll(facePoint, pointI){
                    facePoint[pointI] = dgCellI.dofLocation()[ownerDofMapping[pointI]];
                }
                vectorField gaussPoint(dgFaceI.nGaussPoints_, Zero);
                symmTensorField gaussPartDEV(dgFaceI.nGaussPoints_, Zero);
                scalarField gaussPartBP(dgFaceI.nGaussPoints_, 0.0);

                for(int i=0, ptr=0; i<dgFaceI.nGaussPoints_; i++){
                    for(int j=0; j<ownerDof; j++, ptr++){
                        gaussPoint[i] += gaussFaceInterp_O[ptr] * facePoint[j];
                        gaussPartDEV[i] += gaussFaceInterp_O[ptr] * subDev[j];
                        gaussPartBP[i] += rho(p) * gaussFaceInterp_O[ptr] * (subBP[j]-pRef);
                    }
                }
                forAll(gaussPoint, pointI){
                    stdDofLoc[gaussBase+pointI] = gaussPoint[pointI];
                    fT[gaussBase+pointI] = dgFaceI.faceWJ_[pointI] * (dgFaceI.faceNx_[pointI] & gaussPartDEV[pointI]);
                    fN[gaussBase+pointI] = dgFaceI.faceWJ_[pointI] * (dgFaceI.faceNx_[pointI] * gaussPartBP[pointI]);
                }

                gaussBase += gaussPoint.size();
            }
            vectorField Md
            (
                stdDofLoc - coordSys_.origin()
            );

            //- Porous force
            vectorField fP(Md.size(), Zero);
            applyBins(Md, fN, fT, fP, stdDofLoc);
        }

    }
    Pstream::listCombineGather(force_, plusEqOp<vectorField>());
    Pstream::listCombineGather(moment_, plusEqOp<vectorField>());
    Pstream::listCombineScatter(force_);
    Pstream::listCombineScatter(moment_);
}


Foam::vector Foam::functionObjects::dgForces::forceEff() const
{
    return sum(force_[0]) + sum(force_[1]) + sum(force_[2]);
}


Foam::vector Foam::functionObjects::dgForces::momentEff() const
{
    return sum(moment_[0]) + sum(moment_[1]) + sum(moment_[2]);
}


bool Foam::functionObjects::dgForces::execute()
{
    return true;
}


bool Foam::functionObjects::dgForces::write()
{
    calcForcesMoment();

    if (Pstream::master())
    {
        writeFiles::write();

        writeForces();

        writeBins();

        Log << endl;
    }

    return true;
}


// ************************************************************************* //
