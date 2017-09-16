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


#include "dgGaussFieldScalarField.H"
#include "dgFields.H"
#define TEMPLATE  inline
#include "dgGaussFieldFunctionsM.C"



// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{
//template
//class GeometricDofField;
// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //


void stabilise
(
    dgGaussField<scalar >& result,
    const dgGaussField<scalar >& gsf,
    const dimensioned<scalar>& ds
)
{
    stabilise(result.cellFieldRef(), gsf.cellField(), ds.value());
    stabilise(result.ownerFaceFieldRef(), gsf.ownerFaceField(), ds.value());
	  stabilise(result.neighborFaceFieldRef(), gsf.neighborFaceField(), ds.value());
}



tmp<dgGaussField<scalar >> stabilise
(
    const dgGaussField<scalar >& gsf,
    const dimensioned<scalar>& ds
)
{
    tmp<dgGaussField<scalar >> tRes
    (
        new dgGaussField<scalar >
        (
        GeometricDofField<scalar, dgPatchField, dgGeoMesh>
        (
            IOobject
            (
                "stabilise(" + gsf.name() + ',' + ds.name() + ')',
                gsf.instance(),
                gsf.db(),
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            gsf.mesh(),
            ds.dimensions() + gsf.dimensions()
        )
        )
    );

    stabilise(tRes.ref(), gsf, ds);

    return tRes;
}



tmp<dgGaussField<scalar >> stabilise
(
    const tmp<dgGaussField<scalar >>& tgsf,
    const dimensioned<scalar>& ds
)
{
    const dgGaussField<scalar >& gsf = tgsf();

    tmp<dgGaussField<scalar >> tRes
    (
        New
        (
            tgsf,
            "stabilise(" + gsf.name() + ',' + ds.name() + ')',
            ds.dimensions() + gsf.dimensions()
        )
    );

    stabilise(tRes.ref(), gsf, ds);

    tgsf.clear();

    return tRes;
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

BINARY_TYPE_OPERATOR(scalar, scalar, scalar, +, '+', add)
BINARY_TYPE_OPERATOR(scalar, scalar, scalar, -, '-', subtract)

BINARY_OPERATOR(scalar, scalar, scalar, *, '*', multiply)
BINARY_OPERATOR(scalar, scalar, scalar, /, '|', divide)

BINARY_TYPE_OPERATOR_SF(scalar, scalar, scalar, /, '|', divide)


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

TEMPLATE
void pow
(
    dgGaussField<scalar >& Pow,
    const dgGaussField<scalar >& gsf1,
    const dgGaussField<scalar >& gsf2
)
{
    pow(Pow.cellFieldRef(), gsf1.cellField(), gsf2.cellField());
    pow(Pow.ownerFaceFieldRef(), gsf1.ownerFaceField(), gsf2.ownerFaceField());
    pow(Pow.neighborFaceFieldRef(), gsf1.neighborFaceField(), gsf2.neighborFaceField());
}

TEMPLATE
tmp<dgGaussField<scalar >> pow
(
    const dgGaussField<scalar >& gsf1,
    const dgGaussField<scalar >& gsf2
)
{
    if (!gsf1.dimensions().dimensionless())
    {
        FatalErrorInFunction
            << "Base field is not dimensionless: " << gsf1.dimensions()
            << exit(FatalError);
    }

    if (!gsf2.dimensions().dimensionless())
    {
        FatalErrorInFunction
            << "Exponent field is not dimensionless: " << gsf2.dimensions()
            << exit(FatalError);
    }

    tmp<dgGaussField<scalar >> tPow
    (
        new dgGaussField<scalar >
        (
        GeometricDofField<scalar, dgPatchField, dgGeoMesh>
        (
            IOobject
            (
                "pow(" + gsf1.name() + ',' + gsf2.name() + ')',
                gsf1.instance(),
                gsf1.db(),
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            gsf1.mesh(),
            dimless
        )
        )
    );

    pow(tPow.ref(), gsf1, gsf2);

    return tPow;
}


TEMPLATE
tmp<dgGaussField<scalar >> pow
(
    const tmp<dgGaussField<scalar >>& tgsf1,
    const dgGaussField<scalar >& gsf2
)
{
    const dgGaussField<scalar >& gsf1 = tgsf1();

    if (!gsf1.dimensions().dimensionless())
    {
        FatalErrorInFunction
            << "Base field is not dimensionless: " << gsf1.dimensions()
            << exit(FatalError);
    }

    if (!gsf2.dimensions().dimensionless())
    {
        FatalErrorInFunction
            << "Exponent field is not dimensionless: " << gsf2.dimensions()
            << exit(FatalError);
    }
	
    tmp<dgGaussField<scalar >> tPow
    (
        New
        (
            tgsf1,
            "pow(" + gsf1.name() + ',' + gsf2.name() + ')',
            dimless
        )
    );

    pow(tPow.ref(), gsf1, gsf2);

    tgsf1.clear();

    return tPow;
}


TEMPLATE
tmp<dgGaussField<scalar >> pow
(
    const dgGaussField<scalar >& gsf1,
    const tmp<dgGaussField<scalar >>& tgsf2
)
{
    const dgGaussField<scalar >& gsf2 = tgsf2();

    if (!gsf1.dimensions().dimensionless())
    {
        FatalErrorInFunction
            << "Base field is not dimensionless: " << gsf1.dimensions()
            << exit(FatalError);
    }

    if (!gsf2.dimensions().dimensionless())
    {
        FatalErrorInFunction
            << "Exponent field is not dimensionless: " << gsf2.dimensions()
            << exit(FatalError);
    }

    tmp<dgGaussField<scalar >> tPow
    (
        New
        (
            tgsf2,
            "pow(" + gsf1.name() + ',' + gsf2.name() + ')',
            dimless
        )
    );

    pow(tPow.ref(), gsf1, gsf2);

    tgsf2.clear();

    return tPow;
}

TEMPLATE
tmp<dgGaussField<scalar >> pow
(
    const tmp<dgGaussField<scalar >>& tgsf1,
    const tmp<dgGaussField<scalar >>& tgsf2
)
{
    const dgGaussField<scalar >& gsf1 = tgsf1();
    const dgGaussField<scalar >& gsf2 = tgsf2();

    if (!gsf1.dimensions().dimensionless())
    {
        FatalErrorInFunction
            << "Base field is not dimensionless: " << gsf1.dimensions()
            << exit(FatalError);
    }

    if (!gsf2.dimensions().dimensionless())
    {
        FatalErrorInFunction
            << "Exponent field is not dimensionless: " << gsf2.dimensions()
            << exit(FatalError);
    }

    tmp<dgGaussField<scalar >> tPow
    (
        reuseTmpTmpdgGaussField
            <scalar, scalar, scalar, scalar >::New
        (
            tgsf1,
            tgsf2,
            "pow(" + gsf1.name() + ',' + gsf2.name() + ')',
            dimless
        )
    );

    pow(tPow.ref(), gsf1, gsf2);

    tgsf1.clear();
    tgsf2.clear();

    return tPow;
}


TEMPLATE
void pow
(
    dgGaussField<scalar >& tPow,
    const dgGaussField<scalar >& gsf,
    const dimensioned<scalar>& ds
)
{
    pow(tPow.cellFieldRef(), gsf.cellField(), ds.value());
    pow(tPow.ownerFaceFieldRef(), gsf.ownerFaceField(), ds.value());
	pow(tPow.neighborFaceFieldRef(), gsf.neighborFaceField(), ds.value());
}

TEMPLATE
tmp<dgGaussField<scalar >> pow
(
    const dgGaussField<scalar >& gsf,
    const dimensionedScalar& ds
)
{
    if (!ds.dimensions().dimensionless())
    {
        FatalErrorInFunction
            << "Exponent is not dimensionless: " << ds.dimensions()
            << exit(FatalError);
    }

    tmp<dgGaussField<scalar >> tPow
    (
        new dgGaussField<scalar >
        (
          GeometricDofField<scalar, dgPatchField, dgGeoMesh>
          (
            IOobject
            (
                "pow(" + gsf.name() + ',' + ds.name() + ')',
                gsf.instance(),
                gsf.db(),
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            gsf.mesh(),
            pow(gsf.dimensions(), ds)
        )
        )
    );

    pow(tPow.ref(), gsf, ds);

    return tPow;
}

TEMPLATE
tmp<dgGaussField<scalar >> pow
(
    const tmp<dgGaussField<scalar >>& tgsf,
    const dimensionedScalar& ds
)
{
    if (!ds.dimensions().dimensionless())
    {
        FatalErrorInFunction
            << "Exponent is not dimensionless: " << ds.dimensions()
            << exit(FatalError);
    }

    const dgGaussField<scalar >& gsf = tgsf();

    tmp<dgGaussField<scalar >> tPow
    (
        New
        (
            tgsf,
            "pow(" + gsf.name() + ',' + ds.name() + ')',
            pow(gsf.dimensions(), ds)
        )
    );

    pow(tPow.ref(), gsf, ds);

    tgsf.clear();

    return tPow;
}

TEMPLATE
tmp<dgGaussField<scalar >> pow
(
    const dgGaussField<scalar >& gsf,
    const scalar& s
)
{
    return pow(gsf, dimensionedScalar(s));
}

TEMPLATE
tmp<dgGaussField<scalar >> pow
(
    const tmp<dgGaussField<scalar >>& tgsf,
    const scalar& s
)
{
    return pow(tgsf, dimensionedScalar(s));
}


TEMPLATE
void pow
(
    dgGaussField<scalar >& tPow,
    const dimensioned<scalar>& ds,
    const dgGaussField<scalar >& gsf
)
{

	  pow(tPow.cellFieldRef(), ds.value(), gsf.cellField());
    pow(tPow.ownerFaceFieldRef(), ds.value(), gsf.ownerFaceField());
	pow(tPow.neighborFaceFieldRef(), ds.value(), gsf.neighborFaceField());
}


TEMPLATE
tmp<dgGaussField<scalar >> pow
(
    const dimensionedScalar& ds,
    const dgGaussField<scalar >& gsf
)
{
    if (!ds.dimensions().dimensionless())
    {
        FatalErrorInFunction
            << "Base scalar is not dimensionless: " << ds.dimensions()
            << exit(FatalError);
    }

    if (!gsf.dimensions().dimensionless())
    {
        FatalErrorInFunction
            << "Exponent field is not dimensionless: " << gsf.dimensions()
            << exit(FatalError);
    }

    tmp<dgGaussField<scalar >> tPow
    (
        new dgGaussField<scalar >
        (
         GeometricDofField<scalar, dgPatchField, dgGeoMesh>
         (
            IOobject
            (
                "pow(" + ds.name() + ',' + gsf.name() + ')',
                gsf.instance(),
                gsf.db(),
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            gsf.mesh(),
            dimless
        )
        )
    );

    pow(tPow.ref(), ds, gsf);

    return tPow;
}


TEMPLATE
tmp<dgGaussField<scalar >> pow
(
    const dimensionedScalar& ds,
    const tmp<dgGaussField<scalar >>& tgsf
)
{
    const dgGaussField<scalar >& gsf = tgsf();

    if (!ds.dimensions().dimensionless())
    {
        FatalErrorInFunction
            << "Base scalar is not dimensionless: " << ds.dimensions()
            << exit(FatalError);
    }

    if (!gsf.dimensions().dimensionless())
    {
        FatalErrorInFunction
            << "Exponent field is not dimensionless: " << gsf.dimensions()
            << exit(FatalError);
    }

    tmp<dgGaussField<scalar >> tPow
    (
        New
        (
            tgsf,
            "pow(" + ds.name() + ',' + gsf.name() + ')',
            dimless
        )
    );

    pow(tPow.ref(), ds, gsf);

    tgsf.clear();

    return tPow;
}

TEMPLATE
tmp<dgGaussField<scalar >> pow
(
    const scalar& s,
    const dgGaussField<scalar >& gsf
)
{
    return pow(dimensionedScalar(s), gsf);
}

TEMPLATE
tmp<dgGaussField<scalar >> pow
(
    const scalar& s,
    const tmp<dgGaussField<scalar >>& tgsf
)
{
    return pow(dimensionedScalar(s), tgsf);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
TEMPLATE
void atan2
(
    dgGaussField<scalar >& Atan2,
    const dgGaussField<scalar >& gsf1,
    const dgGaussField<scalar >& gsf2
)
{
    atan2
    (
        Atan2.cellFieldRef(),
        gsf1.cellField(),
        gsf2.cellField()
    );
    atan2
    (
        Atan2.ownerFaceFieldRef(),
        gsf1.ownerFaceField(),
        gsf2.ownerFaceField()
    );
	    atan2
    (
        Atan2.neighborFaceFieldRef(),
        gsf1.neighborFaceField(),
        gsf2.neighborFaceField()
    );
}

TEMPLATE
tmp<dgGaussField<scalar >> atan2
(
    const dgGaussField<scalar >& gsf1,
    const dgGaussField<scalar >& gsf2
)
{
    tmp<dgGaussField<scalar >> tAtan2
    (
        new dgGaussField<scalar >
        (
        GeometricDofField<scalar, dgPatchField, dgGeoMesh>
        (
            IOobject
            (
                "atan2(" + gsf1.name() + ',' + gsf2.name() + ')',
                gsf1.instance(),
                gsf1.db(),
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            gsf1.mesh(),
            atan2(gsf1.dimensions(), gsf2.dimensions())
        )
        )
    );

    atan2(tAtan2.ref(), gsf1, gsf2);

    return tAtan2;
}

TEMPLATE
tmp<dgGaussField<scalar >> atan2
(
    const tmp<dgGaussField<scalar >>& tgsf1,
    const dgGaussField<scalar >& gsf2
)
{
    const dgGaussField<scalar >& gsf1 = tgsf1();

    tmp<dgGaussField<scalar >> tAtan2
    (
        New
        (
            tgsf1,
            "atan2(" + gsf1.name() + ',' + gsf2.name() + ')',
            atan2(gsf1.dimensions(), gsf2.dimensions())
        )
    );

    atan2(tAtan2.ref(), gsf1, gsf2);

    tgsf1.clear();

    return tAtan2;
}

TEMPLATE
tmp<dgGaussField<scalar >> atan2
(
    const dgGaussField<scalar >& gsf1,
    const tmp<dgGaussField<scalar >>& tgsf2
)
{
    const dgGaussField<scalar >& gsf2 = tgsf2();

    tmp<dgGaussField<scalar >> tAtan2
    (
        New
        (
            tgsf2,
            "atan2(" + gsf1.name() + ',' + gsf2.name() + ')',
            atan2( gsf1.dimensions(), gsf2.dimensions())
        )
    );

    atan2(tAtan2.ref(), gsf1, gsf2);

    tgsf2.clear();

    return tAtan2;
}

TEMPLATE
tmp<dgGaussField<scalar >> atan2
(
    const tmp<dgGaussField<scalar >>& tgsf1,
    const tmp<dgGaussField<scalar >>& tgsf2
)
{
    const dgGaussField<scalar >& gsf1 = tgsf1();
    const dgGaussField<scalar >& gsf2 = tgsf2();

    tmp<dgGaussField<scalar >> tAtan2
    (
        reuseTmpTmpdgGaussField
            <scalar, scalar, scalar, scalar >::New
        (
            tgsf1,
            tgsf2,
            "atan2(" + gsf1.name() + ',' + gsf2.name() + ')',
            atan2(gsf1.dimensions(), gsf2.dimensions())
        )
    );

    atan2(tAtan2.ref(), gsf1, gsf2);

    tgsf1.clear();
    tgsf2.clear();

    return tAtan2;
}

TEMPLATE
void atan2
(
    dgGaussField<scalar >& tAtan2,
    const dgGaussField<scalar >& gsf,
    const dimensioned<scalar>& ds
)
{
    atan2(tAtan2.cellFieldRef(), gsf.cellField(), ds.value());
    atan2(tAtan2.ownerFaceFieldRef(), gsf.ownerFaceField(), ds.value());
	    atan2(tAtan2.neighborFaceFieldRef(), gsf.neighborFaceField(), ds.value());
}

/*
tmp<dgGaussField<scalar >> atan2
(
    const dgGaussField<scalar >& gsf,
    const dimensionedScalar& ds
)
{
    tmp<dgGaussField<scalar >> tAtan2
    (
        new dgGaussField<scalar >
        (
        GeometricDofField<scalar, dgPatchField, dgGeoMesh>
        (
            IOobject
            (
                "atan2(" + gsf.name() + ',' + ds.name() + ')',
                gsf.instance(),
                gsf.db(),
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            gsf.mesh(),
            atan2(gsf.dimensions(), ds)
        )
        )
    );

    atan2(tAtan2.ref(), gsf, ds);

    return tAtan2;
}


tmp<dgGaussField<scalar >> atan2
(
    const tmp<dgGaussField<scalar >>& tgsf,
    const dimensionedScalar& ds
)
{
    const dgGaussField<scalar >& gsf = tgsf();

    tmp<dgGaussField<scalar >> tAtan2
    (
        New
        (
            tgsf,
            "atan2(" + gsf.name() + ',' + ds.name() + ')',
            atan2(gsf.dimensions(), ds)
        )
    );

    atan2(tAtan2.ref(), gsf, ds);

    tgsf.clear();

    return tAtan2;
}

tmp<dgGaussField<scalar >> atan2
(
    const dgGaussField<scalar >& gsf,
    const scalar& s
)
{
    return atan2(gsf, dimensionedScalar(s));
}

tmp<dgGaussField<scalar >> atan2
(
    const tmp<dgGaussField<scalar >>& tgsf,
    const scalar& s
)
{
    return atan2(tgsf, dimensionedScalar(s));
}*/

TEMPLATE
void atan2
(
    dgGaussField<scalar >& tAtan2,
    const dimensioned<scalar>& ds,
    const dgGaussField<scalar >& gsf
)
{
   
    atan2(tAtan2.cellFieldRef(), ds.value(), gsf.cellField());
    atan2(tAtan2.ownerFaceFieldRef(), ds.value(), gsf.ownerFaceField());
    atan2(tAtan2.neighborFaceFieldRef(), ds.value(), gsf.neighborFaceField());
}
/*
tmp<dgGaussField<scalar >> atan2
(
    const dimensionedScalar& ds,
    const dgGaussField<scalar >& gsf
)
{
    tmp<dgGaussField<scalar >> tAtan2
    (
        new dgGaussField<scalar >
        (
        GeometricDofField<scalar, dgPatchField, dgGeoMesh>
        (
            IOobject
            (
                "atan2(" + ds.name() + ',' + gsf.name() + ')',
                gsf.instance(),
                gsf.db(),
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            gsf.mesh(),
            atan2(ds, gsf.dimensions())
        )
        )
    );

    atan2(tAtan2.ref(), ds, gsf);

    return tAtan2;
}


tmp<dgGaussField<scalar >> atan2
(
    const dimensionedScalar& ds,
    const tmp<dgGaussField<scalar >>& tgsf
)
{
    const dgGaussField<scalar >& gsf = tgsf();

    tmp<dgGaussField<scalar >> tAtan2
    (
        New
        (
            tgsf,
            "atan2(" + ds.name() + ',' + gsf.name() + ')',
            atan2(ds, gsf.dimensions())
        )
    );

    atan2(tAtan2.ref(), ds, gsf);

    tgsf.clear();

    return tAtan2;
}

tmp<dgGaussField<scalar >> atan2
(
    const scalar& s,
    const dgGaussField<scalar >& gsf
)
{
    return atan2(dimensionedScalar(s), gsf);
}


tmp<dgGaussField<scalar >> atan2
(
    const scalar& s,
    const tmp<dgGaussField<scalar >>& tgsf
)
{
    return atan2(dimensionedScalar(s), tgsf);
}
*/

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

UNARY_FUNCTION(scalar, scalar, pow3, pow3)
UNARY_FUNCTION(scalar, scalar, pow4, pow4)
UNARY_FUNCTION(scalar, scalar, pow5, pow5)
UNARY_FUNCTION(scalar, scalar, pow6, pow6)
UNARY_FUNCTION(scalar, scalar, pow025, pow025)
UNARY_FUNCTION(scalar, scalar, sqrt, sqrt)
UNARY_FUNCTION(scalar, scalar, cbrt, cbrt)
UNARY_FUNCTION(scalar, scalar, sign, sign)
UNARY_FUNCTION(scalar, scalar, pos, pos)
UNARY_FUNCTION(scalar, scalar, neg, neg)
UNARY_FUNCTION(scalar, scalar, posPart, posPart)
UNARY_FUNCTION(scalar, scalar, negPart, negPart)

UNARY_FUNCTION(scalar, scalar, exp, trans)
UNARY_FUNCTION(scalar, scalar, log, trans)
UNARY_FUNCTION(scalar, scalar, log10, trans)
UNARY_FUNCTION(scalar, scalar, sin, trans)
UNARY_FUNCTION(scalar, scalar, cos, trans)
UNARY_FUNCTION(scalar, scalar, tan, trans)
UNARY_FUNCTION(scalar, scalar, asin, trans)
UNARY_FUNCTION(scalar, scalar, acos, trans)
UNARY_FUNCTION(scalar, scalar, atan, trans)
UNARY_FUNCTION(scalar, scalar, sinh, trans)
UNARY_FUNCTION(scalar, scalar, cosh, trans)
UNARY_FUNCTION(scalar, scalar, tanh, trans)
UNARY_FUNCTION(scalar, scalar, asinh, trans)
UNARY_FUNCTION(scalar, scalar, acosh, trans)
UNARY_FUNCTION(scalar, scalar, atanh, trans)
UNARY_FUNCTION(scalar, scalar, erf, trans)
UNARY_FUNCTION(scalar, scalar, erfc, trans)
UNARY_FUNCTION(scalar, scalar, lgamma, trans)
UNARY_FUNCTION(scalar, scalar, j0, trans)
UNARY_FUNCTION(scalar, scalar, j1, trans)
UNARY_FUNCTION(scalar, scalar, y0, trans)
UNARY_FUNCTION(scalar, scalar, y1, trans)


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#define BesselFunc(func)                                                       \
  TEMPLATE                                                                       \
void func                                                                      \
(                                                                              \
    dgGaussField<scalar >& gsf,                          \
    const int n,                                                               \
    const dgGaussField<scalar >& gsf1                    \
)                                                                              \
{                                                                              \
    func(gsf.cellFieldRef(), n, gsf1.cellField());                   \
    func(gsf.ownerFaceFieldRef(), n, gsf1.ownerFaceField());                     \
        func(gsf.neighborFaceFieldRef(), n, gsf1.neighborFaceField());                     \
}                                                                              \
    TEMPLATE                                                                      \
tmp<dgGaussField<scalar >> func                          \
(                                                                              \
    const int n,                                                               \
    const dgGaussField<scalar >& gsf                     \
)                                                                              \
{                                                                              \
    if (!gsf.dimensions().dimensionless())                                     \
    {                                                                          \
        FatalErrorInFunction                                                   \
            << "gsf not dimensionless"                                         \
            << abort(FatalError);                                              \
    }                                                                          \
                                                                               \
    tmp<dgGaussField<scalar >> tFunc                     \
    (                                                                          \
        new dgGaussField<scalar >                        \
        (            					\
        GeometricDofField<scalar, dgPatchField, dgGeoMesh> \
        (							\
            IOobject                                                           \
            (                                                                  \
                #func "(" + gsf.name() + ')',                                  \
                gsf.instance(),                                                \
                gsf.db(),                                                      \
                IOobject::NO_READ,                                             \
                IOobject::NO_WRITE                                             \
            ),                                                                 \
            gsf.mesh(),                                                        \
            dimless                                                            \
        )                                                                      \
        )					\
    );                                                                         \
                                                                               \
    func(tFunc.ref(), n, gsf);                                                 \
                                                                               \
    return tFunc;                                                              \
}                                                                              \
     TEMPLATE                                                              \
tmp<dgGaussField<scalar >> func                          \
(                                                                              \
    const int n,                                                               \
    const tmp<dgGaussField<scalar >>& tgsf               \
)                                                                              \
{                                                                              \
    const dgGaussField<scalar >& gsf = tgsf();           \
                                                                               \
    if (!gsf.dimensions().dimensionless())                                     \
    {                                                                          \
        FatalErrorInFunction                                                   \
            << " : gsf not dimensionless"                                      \
            << abort(FatalError);                                              \
    }                                                                          \
                                                                     \
    tmp<dgGaussField<scalar >> tFunc                     \
    (                                                                          \
        New                                                                    \
        (                                                                      \
            tgsf,                                                              \
            #func "(" + gsf.name() + ')',                                      \
            dimless                                                            \
        )                                                                      \
    );                                                                         \
                                                                               \
    func(tFunc.ref(), n, gsf);                                                 \
                                                                               \
    tgsf.clear();                                                              \
                                                                               \
    return tFunc;                                                              \
}

BesselFunc(jn)
BesselFunc(yn)

#undef BesselFunc


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "undefFieldFunctionsM.H"

// ************************************************************************* //
