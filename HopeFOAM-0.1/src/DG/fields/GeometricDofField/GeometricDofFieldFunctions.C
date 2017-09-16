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

#include "GeometricDofFieldReuseFunctions.H"


#define TEMPLATE \
    template<class Type, template<class> class PatchField, class GeoMesh>
#include "GeometricDofFieldFunctionsM.C"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * Global functions  * * * * * * * * * * * * * //




template<class Type, template<class> class PatchField, class GeoMesh>
void component
(
    GeometricDofField
    <
        typename GeometricDofField<Type, PatchField, GeoMesh>::cmptType,
        PatchField,
        GeoMesh
    >& gcf,
    const GeometricDofField<Type, PatchField, GeoMesh>& gf,
    const direction d
)
{
    component(gcf.primitiveFieldRef(), gf.primitiveField(), d);
    component(gcf.boundaryFieldRef(), gf.boundaryField(), d);
}


template<class Type, template<class> class PatchField, class GeoMesh>
void T
(
     GeometricDofField<Type, PatchField, GeoMesh>& gf,
     const GeometricDofField<Type, PatchField, GeoMesh>& gf1
)
{
    T(gf.primitiveFieldRef(), gf1.primitiveField());
    T(gf.boundaryFieldRef(), gf1.boundaryField());
}


template
<
    class Type,
    template<class> class PatchField,
    class GeoMesh,
    direction r
>
void pow
(
    GeometricDofField<typename powProduct<Type, r>::type, PatchField, GeoMesh>& gf,
    const GeometricDofField<Type, PatchField, GeoMesh>& gf1
)
{
    pow(gf.primitiveFieldRef(), gf1.primitiveField(), r);
    pow(gf.boundaryFieldRef(), gf1.boundaryField(), r);
}

template
<
    class Type,
    template<class> class PatchField,
    class GeoMesh,
    direction r
>
tmp<GeometricDofField<typename powProduct<Type, r>::type, PatchField, GeoMesh>>
pow
(
    const GeometricDofField<Type, PatchField, GeoMesh>& gf,
    typename powProduct<Type, r>::type
)
{
    typedef typename powProduct<Type, r>::type powProductType;

    tmp<GeometricDofField<powProductType, PatchField, GeoMesh>> tPow
    (
        new GeometricDofField<powProductType, PatchField, GeoMesh>
        (
            IOobject
            (
                "pow(" + gf.name() + ',' + name(r) + ')',
                gf.instance(),
                gf.db(),
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            gf.mesh(),
            pow(gf.dimensions(), r)
        )
    );

    pow<Type, r, PatchField, GeoMesh>(tPow.ref(), gf);

    return tPow;
}


template
<
    class Type,
    template<class> class PatchField,
    class GeoMesh,
    direction r
>
tmp<GeometricDofField<typename powProduct<Type, r>::type, PatchField, GeoMesh>>
pow
(
    const tmp<GeometricDofField<Type, PatchField, GeoMesh>>& tgf,
    typename powProduct<Type, r>::type
)
{
    typedef typename powProduct<Type, r>::type powProductType;

    const GeometricDofField<Type, PatchField, GeoMesh>& gf = tgf();

    tmp<GeometricDofField<powProductType, PatchField, GeoMesh>> tPow
    (
        new GeometricDofField<powProductType, PatchField, GeoMesh>
        (
            IOobject
            (
                "pow(" + gf.name() + ',' + name(r) + ')',
                gf.instance(),
                gf.db(),
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            gf.mesh(),
            pow(gf.dimensions(), r)
        )
    );

    pow<Type, r, PatchField, GeoMesh>(tPow.ref(), gf);

    tgf.clear();

    return tPow;
}


template<class Type, template<class> class PatchField, class GeoMesh>
void sqr
(
    GeometricDofField
    <typename outerProduct<Type, Type>::type, PatchField, GeoMesh>& gf,
    const GeometricDofField<Type, PatchField, GeoMesh>& gf1
)
{
    sqr(gf.primitiveFieldRef(), gf1.primitiveField());
    sqr(gf.boundaryFieldRef(), gf1.boundaryField());
}

template<class Type, template<class> class PatchField, class GeoMesh>
tmp
<
    GeometricDofField
    <
        typename outerProduct<Type, Type>::type,
        PatchField,
        GeoMesh
    >
>
sqr(const GeometricDofField<Type, PatchField, GeoMesh>& gf)
{
    typedef typename outerProduct<Type, Type>::type outerProductType;

    tmp<GeometricDofField<outerProductType, PatchField, GeoMesh>> tSqr
    (
        new GeometricDofField<outerProductType, PatchField, GeoMesh>
        (
            IOobject
            (
                "sqr(" + gf.name() + ')',
                gf.instance(),
                gf.db(),
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            gf.mesh(),
            sqr(gf.dimensions())
        )
    );

    sqr(tSqr.ref(), gf);

    return tSqr;
}

template<class Type, template<class> class PatchField, class GeoMesh>
tmp
<
    GeometricDofField
    <
        typename outerProduct<Type, Type>::type,
        PatchField,
        GeoMesh
    >
>
sqr(const tmp<GeometricDofField<Type, PatchField, GeoMesh>>& tgf)
{
    typedef typename outerProduct<Type, Type>::type outerProductType;

    const GeometricDofField<Type, PatchField, GeoMesh>& gf = tgf();

    tmp<GeometricDofField<outerProductType, PatchField, GeoMesh>> tSqr
    (
        new GeometricDofField<outerProductType, PatchField, GeoMesh>
        (
            IOobject
            (
                "sqr(" + gf.name() + ')',
                gf.instance(),
                gf.db(),
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            gf.mesh(),
            sqr(gf.dimensions())
        )
    );

    sqr(tSqr.ref(), gf);

    tgf.clear();

    return tSqr;
}


template<class Type, template<class> class PatchField, class GeoMesh>
void magSqr
(
    GeometricDofField<scalar, PatchField, GeoMesh>& gsf,
    const GeometricDofField<Type, PatchField, GeoMesh>& gf
)
{
    magSqr(gsf.primitiveFieldRef(), gf.primitiveField());
    magSqr(gsf.boundaryFieldRef(), gf.boundaryField());
}

template<class Type, template<class> class PatchField, class GeoMesh>
tmp<GeometricDofField<scalar, PatchField, GeoMesh>> magSqr
(
    const GeometricDofField<Type, PatchField, GeoMesh>& gf
)
{
    tmp<GeometricDofField<scalar, PatchField, GeoMesh>> tMagSqr
    (
        new GeometricDofField<scalar, PatchField, GeoMesh>
        (
            IOobject
            (
                "magSqr(" + gf.name() + ')',
                gf.instance(),
                gf.db(),
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            gf.mesh(),
            sqr(gf.dimensions())
        )
    );

    magSqr(tMagSqr.ref(), gf);

    return tMagSqr;
}

template<class Type, template<class> class PatchField, class GeoMesh>
tmp<GeometricDofField<scalar, PatchField, GeoMesh>> magSqr
(
    const tmp<GeometricDofField<Type, PatchField, GeoMesh>>& tgf
)
{
    const GeometricDofField<Type, PatchField, GeoMesh>& gf = tgf();

    tmp<GeometricDofField<scalar, PatchField, GeoMesh>> tMagSqr
    (
        new GeometricDofField<scalar, PatchField, GeoMesh>
        (
            IOobject
            (
                "magSqr(" + gf.name() + ')',
                gf.instance(),
                gf.db(),
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            gf.mesh(),
            sqr(gf.dimensions())
        )
    );

    magSqr(tMagSqr.ref(), gf);

    tgf.clear();

    return tMagSqr;
}


template<class Type, template<class> class PatchField, class GeoMesh>
void mag
(
    GeometricDofField<scalar, PatchField, GeoMesh>& gsf,
    const GeometricDofField<Type, PatchField, GeoMesh>& gf
)
{
    mag(gsf.primitiveFieldRef(), gf.primitiveField());
    mag(gsf.boundaryFieldRef(), gf.boundaryField());
}

template<class Type, template<class> class PatchField, class GeoMesh>
tmp<GeometricDofField<scalar, PatchField, GeoMesh>> mag
(
    const GeometricDofField<Type, PatchField, GeoMesh>& gf
)
{
    tmp<GeometricDofField<scalar, PatchField, GeoMesh>> tMag
    (
        new GeometricDofField<scalar, PatchField, GeoMesh>
        (
            IOobject
            (
                "mag(" + gf.name() + ')',
                gf.instance(),
                gf.db(),
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            gf.mesh(),
            gf.dimensions()
        )
    );

    mag(tMag.ref(), gf);

    return tMag;
}

template<class Type, template<class> class PatchField, class GeoMesh>
tmp<GeometricDofField<scalar, PatchField, GeoMesh>> mag
(
    const tmp<GeometricDofField<Type, PatchField, GeoMesh>>& tgf
)
{
    const GeometricDofField<Type, PatchField, GeoMesh>& gf = tgf();

    tmp<GeometricDofField<scalar, PatchField, GeoMesh>> tMag
    (
        new GeometricDofField<scalar, PatchField, GeoMesh>
        (
            IOobject
            (
                "mag(" + gf.name() + ')',
                gf.instance(),
                gf.db(),
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            gf.mesh(),
            gf.dimensions()
        )
    );

    mag(tMag.ref(), gf);

    tgf.clear();

    return tMag;
}


template<class Type, template<class> class PatchField, class GeoMesh>
void cmptAv
(
    GeometricDofField
    <
        typename GeometricDofField<Type, PatchField, GeoMesh>::cmptType,
        PatchField,
        GeoMesh
    >& gcf,
    const GeometricDofField<Type, PatchField, GeoMesh>& gf
)
{
    cmptAv(gcf.primitiveFieldRef(), gf.primitiveField());
    cmptAv(gcf.boundaryFieldRef(), gf.boundaryField());
}

template<class Type, template<class> class PatchField, class GeoMesh>
tmp
<
    GeometricDofField
    <
        typename GeometricDofField<Type, PatchField, GeoMesh>::cmptType,
        PatchField,
        GeoMesh
    >
>
cmptAv(const GeometricDofField<Type, PatchField, GeoMesh>& gf)
{
    typedef typename GeometricDofField<Type, PatchField, GeoMesh>::cmptType
        cmptType;

    tmp<GeometricDofField<cmptType, PatchField, GeoMesh>> CmptAv
    (
        new GeometricDofField<scalar, PatchField, GeoMesh>
        (
            IOobject
            (
                "cmptAv(" + gf.name() + ')',
                gf.instance(),
                gf.db(),
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            gf.mesh(),
            gf.dimensions()
        )
    );

    cmptAv(CmptAv.ref(), gf);

    return CmptAv;
}

template<class Type, template<class> class PatchField, class GeoMesh>
tmp
<
    GeometricDofField
    <
        typename GeometricDofField<Type, PatchField, GeoMesh>::cmptType,
        PatchField,
        GeoMesh
    >
>
cmptAv(const tmp<GeometricDofField<Type, PatchField, GeoMesh>>& tgf)
{
    typedef typename GeometricDofField<Type, PatchField, GeoMesh>::cmptType
        cmptType;

    const GeometricDofField<Type, PatchField, GeoMesh>& gf = tgf();

    tmp<GeometricDofField<cmptType, PatchField, GeoMesh>> CmptAv
    (
        new GeometricDofField<scalar, PatchField, GeoMesh>
        (
            IOobject
            (
                "cmptAv(" + gf.name() + ')',
                gf.instance(),
                gf.db(),
                IOobject::NO_READ,
                IOobject::NO_WRITE
            ),
            gf.mesh(),
            gf.dimensions()
        )
    );

    cmptAv(CmptAv.ref(), gf);

    tgf.clear();

    return CmptAv;
}


#define UNARY_REDUCTION_FUNCTION_WITH_BOUNDARY(returnType, func, gFunc)        \
                                                                               \
template<class Type, template<class> class PatchField, class GeoMesh>          \
dimensioned<returnType> func                                                   \
(                                                                              \
    const GeometricDofField<Type, PatchField, GeoMesh>& gf                        \
)                                                                              \
{                                                                              \
    return dimensioned<Type>                                                   \
    (                                                                          \
        #func "(" + gf.name() + ')',                                           \
        gf.dimensions(),                                                       \
        Foam::func(gFunc(gf.primitiveField()), gFunc(gf.boundaryField()))      \
    );                                                                         \
}                                                                              \
                                                                               \
template<class Type, template<class> class PatchField, class GeoMesh>          \
dimensioned<returnType> func                                                   \
(                                                                              \
    const tmp<GeometricDofField<Type, PatchField, GeoMesh>>& tgf1                 \
)                                                                              \
{                                                                              \
    dimensioned<returnType> res = func(tgf1());                                \
    tgf1.clear();                                                              \
    return res;                                                                \
}

UNARY_REDUCTION_FUNCTION_WITH_BOUNDARY(Type, max, gMax)
UNARY_REDUCTION_FUNCTION_WITH_BOUNDARY(Type, min, gMin)

#undef UNARY_REDUCTION_FUNCTION_WITH_BOUNDARY


#define UNARY_REDUCTION_FUNCTION(returnType, func, gFunc)                      \
                                                                               \
template<class Type, template<class> class PatchField, class GeoMesh>          \
dimensioned<returnType> func                                                   \
(                                                                              \
    const GeometricDofField<Type, PatchField, GeoMesh>& gf                        \
)                                                                              \
{                                                                              \
    return dimensioned<Type>                                                   \
    (                                                                          \
        #func "(" + gf.name() + ')',                                           \
        gf.dimensions(),                                                       \
        gFunc(gf.primitiveField())                                             \
    );                                                                         \
}                                                                              \
                                                                               \
template<class Type, template<class> class PatchField, class GeoMesh>          \
dimensioned<returnType> func                                                   \
(                                                                              \
    const tmp<GeometricDofField<Type, PatchField, GeoMesh>>& tgf1                 \
)                                                                              \
{                                                                              \
    dimensioned<returnType> res = func(tgf1());                                \
    tgf1.clear();                                                              \
    return res;                                                                \
}

UNARY_REDUCTION_FUNCTION(Type, sum, gSum)
UNARY_REDUCTION_FUNCTION(scalar, sumMag, gSumMag)
UNARY_REDUCTION_FUNCTION(Type, average, gAverage)

#undef UNARY_REDUCTION_FUNCTION


BINARY_FUNCTION(Type, Type, Type, max)
BINARY_FUNCTION(Type, Type, Type, min)
BINARY_FUNCTION(Type, Type, Type, cmptMultiply)
BINARY_FUNCTION(Type, Type, Type, cmptDivide)

BINARY_TYPE_FUNCTION(Type, Type, Type, max)
BINARY_TYPE_FUNCTION(Type, Type, Type, min)
BINARY_TYPE_FUNCTION(Type, Type, Type, cmptMultiply)
BINARY_TYPE_FUNCTION(Type, Type, Type, cmptDivide)


// * * * * * * * * * * * * * * * Global operators  * * * * * * * * * * * * * //

UNARY_OPERATOR(Type, Type, -, negate, transform)

BINARY_OPERATOR(Type, Type, scalar, *, '*', multiply)
BINARY_OPERATOR(Type, scalar, Type, *, '*', multiply)
BINARY_OPERATOR(Type, Type, scalar, /, '|', divide)

BINARY_TYPE_OPERATOR_SF(Type, scalar, Type, *, '*', multiply)
BINARY_TYPE_OPERATOR_FS(Type, Type, scalar, *, '*', multiply)

BINARY_TYPE_OPERATOR_FS(Type, Type, scalar, /, '|', divide)


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#define PRODUCT_OPERATOR(product, op, opFunc)                                  \
                                                                               \
template                                                                       \
<class Type1, class Type2, template<class> class PatchField, class GeoMesh>    \
void opFunc                                                                    \
(                                                                              \
    GeometricDofField                                                             \
    <typename product<Type1, Type2>::type, PatchField, GeoMesh>& gf,           \
    const GeometricDofField<Type1, PatchField, GeoMesh>& gf1,                     \
    const GeometricDofField<Type2, PatchField, GeoMesh>& gf2                      \
)                                                                              \
{                                                                              \
    Foam::opFunc                                                               \
    (                                                                          \
        gf.primitiveFieldRef(),                                                \
        gf1.primitiveField(),                                                  \
        gf2.primitiveField()                                                   \
    );                                                                         \
    Foam::opFunc                                                               \
    (                                                                          \
        gf.boundaryFieldRef(),                                                 \
        gf1.boundaryField(),                                                   \
        gf2.boundaryField()                                                    \
    );                                                                         \
}                                                                              \
                                                                               \
template                                                                       \
<class Type1, class Type2, template<class> class PatchField, class GeoMesh>    \
tmp                                                                            \
<                                                                              \
    GeometricDofField<typename product<Type1, Type2>::type, PatchField, GeoMesh>  \
>                                                                              \
operator op                                                                    \
(                                                                              \
    const GeometricDofField<Type1, PatchField, GeoMesh>& gf1,                     \
    const GeometricDofField<Type2, PatchField, GeoMesh>& gf2                      \
)                                                                              \
{                                                                              \
    typedef typename product<Type1, Type2>::type productType;                  \
    tmp<GeometricDofField<productType, PatchField, GeoMesh>> tRes                 \
    (                                                                          \
        new GeometricDofField<productType, PatchField, GeoMesh>                   \
        (                                                                      \
            IOobject                                                           \
            (                                                                  \
                '(' + gf1.name() + #op + gf2.name() + ')',                     \
                gf1.instance(),                                                \
                gf1.db(),                                                      \
                IOobject::NO_READ,                                             \
                IOobject::NO_WRITE                                             \
            ),                                                                 \
            gf1.mesh(),                                                        \
            gf1.dimensions() op gf2.dimensions()                               \
        )                                                                      \
    );                                                                         \
                                                                               \
    Foam::opFunc(tRes.ref(), gf1, gf2);                                        \
                                                                               \
    return tRes;                                                               \
}                                                                              \
                                                                               \
template                                                                       \
<class Type1, class Type2, template<class> class PatchField, class GeoMesh>    \
tmp                                                                            \
<                                                                              \
    GeometricDofField<typename product<Type1, Type2>::type, PatchField, GeoMesh>  \
>                                                                              \
operator op                                                                    \
(                                                                              \
    const GeometricDofField<Type1, PatchField, GeoMesh>& gf1,                     \
    const tmp<GeometricDofField<Type2, PatchField, GeoMesh>>& tgf2                \
)                                                                              \
{                                                                              \
    typedef typename product<Type1, Type2>::type productType;                  \
                                                                               \
    const GeometricDofField<Type2, PatchField, GeoMesh>& gf2 = tgf2();            \
                                                                               \
    tmp<GeometricDofField<productType, PatchField, GeoMesh>> tRes =               \
        reuseTmpGeometricDofField<productType, Type2, PatchField, GeoMesh>::New   \
        (                                                                      \
            tgf2,                                                              \
            '(' + gf1.name() + #op + gf2.name() + ')',                         \
            gf1.dimensions() op gf2.dimensions()                               \
        );                                                                     \
                                                                               \
    Foam::opFunc(tRes.ref(), gf1, gf2);                                        \
                                                                               \
    tgf2.clear();                                                              \
                                                                               \
    return tRes;                                                               \
}                                                                              \
                                                                               \
template                                                                       \
<class Type1, class Type2, template<class> class PatchField, class GeoMesh>    \
tmp                                                                            \
<                                                                              \
    GeometricDofField<typename product<Type1, Type2>::type, PatchField, GeoMesh>  \
>                                                                              \
operator op                                                                    \
(                                                                              \
    const tmp<GeometricDofField<Type1, PatchField, GeoMesh>>& tgf1,               \
    const GeometricDofField<Type2, PatchField, GeoMesh>& gf2                      \
)                                                                              \
{                                                                              \
    typedef typename product<Type1, Type2>::type productType;                  \
                                                                               \
    const GeometricDofField<Type1, PatchField, GeoMesh>& gf1 = tgf1();            \
                                                                               \
    tmp<GeometricDofField<productType, PatchField, GeoMesh>> tRes =               \
        reuseTmpGeometricDofField<productType, Type1, PatchField, GeoMesh>::New   \
        (                                                                      \
            tgf1,                                                              \
            '(' + gf1.name() + #op + gf2.name() + ')',                         \
            gf1.dimensions() op gf2.dimensions()                               \
        );                                                                     \
                                                                               \
    Foam::opFunc(tRes.ref(), gf1, gf2);                                        \
                                                                               \
    tgf1.clear();                                                              \
                                                                               \
    return tRes;                                                               \
}                                                                              \
                                                                               \
template                                                                       \
<class Type1, class Type2, template<class> class PatchField, class GeoMesh>    \
tmp                                                                            \
<                                                                              \
    GeometricDofField<typename product<Type1, Type2>::type, PatchField, GeoMesh>  \
>                                                                              \
operator op                                                                    \
(                                                                              \
    const tmp<GeometricDofField<Type1, PatchField, GeoMesh>>& tgf1,               \
    const tmp<GeometricDofField<Type2, PatchField, GeoMesh>>& tgf2                \
)                                                                              \
{                                                                              \
    typedef typename product<Type1, Type2>::type productType;                  \
                                                                               \
    const GeometricDofField<Type1, PatchField, GeoMesh>& gf1 = tgf1();            \
    const GeometricDofField<Type2, PatchField, GeoMesh>& gf2 = tgf2();            \
                                                                               \
    tmp<GeometricDofField<productType, PatchField, GeoMesh>> tRes =               \
        reuseTmpTmpGeometricDofField                                              \
        <productType, Type1, Type1, Type2, PatchField, GeoMesh>::New           \
        (                                                                      \
            tgf1,                                                              \
            tgf2,                                                              \
            '(' + gf1.name() + #op + gf2.name() + ')',                         \
            gf1.dimensions() op gf2.dimensions()                               \
        );                                                                     \
                                                                               \
    Foam::opFunc(tRes.ref(), gf1, gf2);                                        \
                                                                               \
    tgf1.clear();                                                              \
    tgf2.clear();                                                              \
                                                                               \
    return tRes;                                                               \
}                                                                              \
                                                                               \
template                                                                       \
<class Form, class Type, template<class> class PatchField, class GeoMesh>      \
void opFunc                                                                    \
(                                                                              \
    GeometricDofField                                                             \
    <typename product<Type, Form>::type, PatchField, GeoMesh>& gf,             \
    const GeometricDofField<Type, PatchField, GeoMesh>& gf1,                      \
    const dimensioned<Form>& dvs                                               \
)                                                                              \
{                                                                              \
    Foam::opFunc(gf.primitiveFieldRef(), gf1.primitiveField(), dvs.value());   \
    Foam::opFunc(gf.boundaryFieldRef(), gf1.boundaryField(), dvs.value());     \
}                                                                              \
                                                                               \
template                                                                       \
<class Form, class Type, template<class> class PatchField, class GeoMesh>      \
tmp<GeometricDofField<typename product<Type, Form>::type, PatchField, GeoMesh>>   \
operator op                                                                    \
(                                                                              \
    const GeometricDofField<Type, PatchField, GeoMesh>& gf1,                      \
    const dimensioned<Form>& dvs                                               \
)                                                                              \
{                                                                              \
    typedef typename product<Type, Form>::type productType;                    \
                                                                               \
    tmp<GeometricDofField<productType, PatchField, GeoMesh>> tRes                 \
    (                                                                          \
        new GeometricDofField<productType, PatchField, GeoMesh>                   \
        (                                                                      \
            IOobject                                                           \
            (                                                                  \
                '(' + gf1.name() + #op + dvs.name() + ')',                     \
                gf1.instance(),                                                \
                gf1.db(),                                                      \
                IOobject::NO_READ,                                             \
                IOobject::NO_WRITE                                             \
            ),                                                                 \
            gf1.mesh(),                                                        \
            gf1.dimensions() op dvs.dimensions()                               \
        )                                                                      \
    );                                                                         \
                                                                               \
    Foam::opFunc(tRes.ref(), gf1, dvs);                                        \
                                                                               \
    return tRes;                                                               \
}                                                                              \
                                                                               \
template                                                                       \
<                                                                              \
    class Form,                                                                \
    class Cmpt,                                                                \
    direction nCmpt,                                                           \
    class Type, template<class> class PatchField,                              \
    class GeoMesh                                                              \
>                                                                              \
tmp<GeometricDofField<typename product<Form, Type>::type, PatchField, GeoMesh>>   \
operator op                                                                    \
(                                                                              \
    const GeometricDofField<Type, PatchField, GeoMesh>& gf1,                      \
    const VectorSpace<Form,Cmpt,nCmpt>& vs                                     \
)                                                                              \
{                                                                              \
    return gf1 op dimensioned<Form>(static_cast<const Form&>(vs));             \
}                                                                              \
                                                                               \
                                                                               \
template                                                                       \
<class Form, class Type, template<class> class PatchField, class GeoMesh>      \
tmp<GeometricDofField<typename product<Type, Form>::type, PatchField, GeoMesh>>   \
operator op                                                                    \
(                                                                              \
    const tmp<GeometricDofField<Type, PatchField, GeoMesh>>& tgf1,                \
    const dimensioned<Form>& dvs                                               \
)                                                                              \
{                                                                              \
    typedef typename product<Type, Form>::type productType;                    \
                                                                               \
    const GeometricDofField<Type, PatchField, GeoMesh>& gf1 = tgf1();             \
                                                                               \
    tmp<GeometricDofField<productType, PatchField, GeoMesh>> tRes =               \
        reuseTmpGeometricDofField<productType, Type, PatchField, GeoMesh>::New    \
        (                                                                      \
            tgf1,                                                              \
            '(' + gf1.name() + #op + dvs.name() + ')',                         \
            gf1.dimensions() op dvs.dimensions()                               \
        );                                                                     \
                                                                               \
    Foam::opFunc(tRes.ref(), gf1, dvs);                                        \
                                                                               \
    tgf1.clear();                                                              \
                                                                               \
    return tRes;                                                               \
}                                                                              \
                                                                               \
template                                                                       \
<                                                                              \
    class Form,                                                                \
    class Cmpt,                                                                \
    direction nCmpt,                                                           \
    class Type, template<class> class PatchField,                              \
    class GeoMesh                                                              \
>                                                                              \
tmp<GeometricDofField<typename product<Form, Type>::type, PatchField, GeoMesh>>   \
operator op                                                                    \
(                                                                              \
    const tmp<GeometricDofField<Type, PatchField, GeoMesh>>& tgf1,                \
    const VectorSpace<Form,Cmpt,nCmpt>& vs                                     \
)                                                                              \
{                                                                              \
    return tgf1 op dimensioned<Form>(static_cast<const Form&>(vs));            \
}                                                                              \
                                                                               \
                                                                               \
template                                                                       \
<class Form, class Type, template<class> class PatchField, class GeoMesh>      \
void opFunc                                                                    \
(                                                                              \
    GeometricDofField                                                             \
    <typename product<Form, Type>::type, PatchField, GeoMesh>& gf,             \
    const dimensioned<Form>& dvs,                                              \
    const GeometricDofField<Type, PatchField, GeoMesh>& gf1                       \
)                                                                              \
{                                                                              \
    Foam::opFunc(gf.primitiveFieldRef(), dvs.value(), gf1.primitiveField());   \
    Foam::opFunc(gf.boundaryFieldRef(), dvs.value(), gf1.boundaryField());     \
}                                                                              \
                                                                               \
template                                                                       \
<class Form, class Type, template<class> class PatchField, class GeoMesh>      \
tmp<GeometricDofField<typename product<Form, Type>::type, PatchField, GeoMesh>>   \
operator op                                                                    \
(                                                                              \
    const dimensioned<Form>& dvs,                                              \
    const GeometricDofField<Type, PatchField, GeoMesh>& gf1                       \
)                                                                              \
{                                                                              \
    typedef typename product<Form, Type>::type productType;                    \
    tmp<GeometricDofField<productType, PatchField, GeoMesh>> tRes                 \
    (                                                                          \
        new GeometricDofField<productType, PatchField, GeoMesh>                   \
        (                                                                      \
            IOobject                                                           \
            (                                                                  \
                '(' + dvs.name() + #op + gf1.name() + ')',                     \
                gf1.instance(),                                                \
                gf1.db(),                                                      \
                IOobject::NO_READ,                                             \
                IOobject::NO_WRITE                                             \
            ),                                                                 \
            gf1.mesh(),                                                        \
            dvs.dimensions() op gf1.dimensions()                               \
        )                                                                      \
    );                                                                         \
                                                                               \
    Foam::opFunc(tRes.ref(), dvs, gf1);                                        \
                                                                               \
    return tRes;                                                               \
}                                                                              \
                                                                               \
template                                                                       \
<                                                                              \
    class Form,                                                                \
    class Cmpt,                                                                \
    direction nCmpt,                                                           \
    class Type, template<class> class PatchField,                              \
    class GeoMesh                                                              \
>                                                                              \
tmp<GeometricDofField<typename product<Form, Type>::type, PatchField, GeoMesh>>   \
operator op                                                                    \
(                                                                              \
    const VectorSpace<Form,Cmpt,nCmpt>& vs,                                    \
    const GeometricDofField<Type, PatchField, GeoMesh>& gf1                       \
)                                                                              \
{                                                                              \
    return dimensioned<Form>(static_cast<const Form&>(vs)) op gf1;             \
}                                                                              \
                                                                               \
template                                                                       \
<class Form, class Type, template<class> class PatchField, class GeoMesh>      \
tmp<GeometricDofField<typename product<Form, Type>::type, PatchField, GeoMesh>>   \
operator op                                                                    \
(                                                                              \
    const dimensioned<Form>& dvs,                                              \
    const tmp<GeometricDofField<Type, PatchField, GeoMesh>>& tgf1                 \
)                                                                              \
{                                                                              \
    typedef typename product<Form, Type>::type productType;                    \
                                                                               \
    const GeometricDofField<Type, PatchField, GeoMesh>& gf1 = tgf1();             \
                                                                               \
    tmp<GeometricDofField<productType, PatchField, GeoMesh>> tRes =               \
        reuseTmpGeometricDofField<productType, Type, PatchField, GeoMesh>::New    \
        (                                                                      \
            tgf1,                                                              \
            '(' + dvs.name() + #op + gf1.name() + ')',                         \
            dvs.dimensions() op gf1.dimensions()                               \
        );                                                                     \
                                                                               \
    Foam::opFunc(tRes.ref(), dvs, gf1);                                        \
                                                                               \
    tgf1.clear();                                                              \
                                                                               \
    return tRes;                                                               \
}                                                                              \
                                                                               \
template                                                                       \
<                                                                              \
    class Form,                                                                \
    class Cmpt,                                                                \
    direction nCmpt,                                                           \
    class Type, template<class> class PatchField,                              \
    class GeoMesh                                                              \
>                                                                              \
tmp<GeometricDofField<typename product<Form, Type>::type, PatchField, GeoMesh>>   \
operator op                                                                    \
(                                                                              \
    const VectorSpace<Form,Cmpt,nCmpt>& vs,                                    \
    const tmp<GeometricDofField<Type, PatchField, GeoMesh>>& tgf1                 \
)                                                                              \
{                                                                              \
    return dimensioned<Form>(static_cast<const Form&>(vs)) op tgf1;            \
}

PRODUCT_OPERATOR(typeOfSum, +, add)
PRODUCT_OPERATOR(typeOfSum, -, subtract)

PRODUCT_OPERATOR(outerProduct, *, outer)
PRODUCT_OPERATOR(crossProduct, ^, cross)
PRODUCT_OPERATOR(innerProduct, &, dot)
PRODUCT_OPERATOR(scalarProduct, &&, dotdot)

#undef PRODUCT_OPERATOR


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "undefDofFieldFunctionsM.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
