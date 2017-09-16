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

#include "dgGaussFieldReuseFunctions.H"


#define TEMPLATE \
    template<class Type>
#include "dgGaussFieldFunctionsM.C"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * Global functions  * * * * * * * * * * * * * //
template<class Type>
void component
(
    dgGaussField
    <
    typename dgGaussField<Type>::cmptType

    >& gcf,
    const dgGaussField<Type>& gf,
    const direction d
)
{
	component(gcf.cellFieldRef(), gf.cellField(), d);
	component(gcf.ownerFaceFieldRef(), gf.ownerFaceField(), d);
	component(gcf.neighborFaceFieldRef(), gf.neighborFaceField(), d);
}


template<class Type>
void T
(
    dgGaussField<Type>& gf,
    const dgGaussField<Type>& gf1
)
{
	T(gf.cellFieldRef(), gf1.cellField());
	T(gf.ownerFaceFieldRef(), gf1.ownerFaceField());
	T(gf.neighborFaceFieldRef(), gf1.neighborFaceField());
}


template
<
    class Type,

    direction r
    >
void pow
(
    dgGaussField<typename powProduct<Type, r>::type>& gf,
    const dgGaussField<Type>& gf1
)
{
	pow(gf.cellFieldRef(), gf1.cellField(), r);
	pow(gf.ownerFaceFieldRef(), gf1.ownerFaceField(), r);
	pow(gf.neighborFaceFieldRef(), gf1.neighborFaceField(), r);
}

template
<
    class Type,

    direction r
    >
tmp<dgGaussField<typename powProduct<Type, r>::type>>
        pow
        (
            const dgGaussField<Type>& gf,
            typename powProduct<Type, r>::type
        )
{
	typedef typename powProduct<Type, r>::type powProductType;

	tmp<dgGaussField<powProductType>> tPow
	                               (
	                                   new  dgGaussField<powProductType>
	                                   (
	                                       GeometricDofField<powProductType, dgPatchField, dgGeoMesh>
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
	                                   )
	                               );

	pow<Type, r>(tPow.ref(), gf);

	return tPow;
}


template
<
    class Type,

    direction r
    >
tmp<dgGaussField<typename powProduct<Type, r>::type>>
        pow
        (
            const tmp<dgGaussField<Type>>& tgf,
            typename powProduct<Type, r>::type
        )
{
	typedef typename powProduct<Type, r>::type powProductType;

	const dgGaussField<Type>& gf = tgf();

	tmp< dgGaussField<powProductType>> tPow
	                                (
	                                    new   dgGaussField<powProductType>
	                                    (
	                                        GeometricDofField<powProductType, dgPatchField, dgGeoMesh>
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
	                                    )
	                                );

	pow<Type, r>(tPow.ref(), gf);

	tgf.clear();

	return tPow;
}


template<class Type>
void sqr
(
    dgGaussField
    <typename outerProduct<Type, Type>::type>& gf,
    const dgGaussField<Type>& gf1
)
{
	sqr(gf.cellFieldRef(), gf1.cellField());
	sqr(gf.ownerFaceFieldRef(), gf1.ownerFaceField());
	sqr(gf.neighborFaceFieldRef(), gf1.neighborFaceField());
}

template<class Type>
tmp
<
dgGaussField
<
typename outerProduct<Type, Type>::type
>
>
sqr(const dgGaussField<Type>& gf)
{
	typedef typename outerProduct<Type, Type>::type outerProductType;

	tmp<dgGaussField<outerProductType>> tSqr(
	                                     new   dgGaussField<outerProductType>      (
	                                             GeometricDofField<outerProductType, dgPatchField, dgGeoMesh>
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
	                                     )
	                                 );

	sqr(tSqr.ref(), gf);

	return tSqr;
}

template<class Type>
tmp
<
dgGaussField
<
typename outerProduct<Type, Type>::type
>
>
sqr(const tmp<dgGaussField<Type>>& tgf)
{
	typedef typename outerProduct<Type, Type>::type outerProductType;

	const dgGaussField<Type>& gf = tgf();

	tmp<dgGaussField<outerProductType>> tSqr
	                                 (
	                                     new  dgGaussField<outerProductType>
	                                     (
	                                             GeometricDofField<outerProductType, dgPatchField, dgGeoMesh>
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
	                                     )
	                                 );

	sqr(tSqr.ref(), gf);

	tgf.clear();

	return tSqr;
}


template<class Type>
void magSqr
(
    dgGaussField<scalar>& gsf,
    const dgGaussField<Type>& gf
)
{
	magSqr(gsf.cellFieldRef(), gf.cellField());
	magSqr(gsf.ownerFaceFieldRef(), gf.ownerFaceField());
	magSqr(gsf.neighborFaceFieldRef(), gf.neighborFaceField());
}

template<class Type>
tmp<dgGaussField<scalar>> magSqr
                       (
                           const dgGaussField<Type>& gf
                       )
{
	tmp<dgGaussField<scalar>> tMagSqr
	                       (
	                           new  dgGaussField<scalar>
	                           (
	                               GeometricDofField<scalar, dgPatchField, dgGeoMesh>
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
	                           )
	                       );

	magSqr(tMagSqr.ref(), gf);

	return tMagSqr;
}

template<class Type>
tmp<dgGaussField<scalar>> magSqr
                       (
                           const tmp<dgGaussField<Type>>& tgf
                       )
{
	const dgGaussField<Type>& gf = tgf();

	tmp<dgGaussField<scalar>> tMagSqr
	                       (
	                           new GeometricDofField<scalar, dgPatchField, dgGeoMesh>
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


template<class Type>
void mag
(
    dgGaussField<scalar>& gsf,
    const dgGaussField<Type>& gf
)
{
	mag(gsf.cellFieldRef(), gf.cellField());
	mag(gsf.ownerFaceFieldRef(), gf.ownerFaceField());
	mag(gsf.neighborFaceFieldRef(), gf.neighborFaceField());
}

template<class Type>
tmp<dgGaussField<scalar>> mag
                       (
                           const dgGaussField<Type>& gf
                       )
{
	tmp<dgGaussField<scalar>> tMag(
	                           new  dgGaussField<scalar>        (
	                               GeometricDofField<scalar, dgPatchField, dgGeoMesh>
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
	                           )
	                       );

	mag(tMag.ref(), gf);

	return tMag;
}

template<class Type>
tmp<dgGaussField<scalar>> mag
                       (
                           const tmp<dgGaussField<Type>>& tgf
                       )
{
	const dgGaussField<Type>& gf = tgf();

	tmp<dgGaussField<scalar>> tMag
	                       ( new dgGaussField<scalar>
	                         (
	                             GeometricDofField<scalar, dgPatchField, dgGeoMesh>
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
	                         )
	                       );

	mag(tMag.ref(), gf);

	tgf.clear();

	return tMag;
}


template<class Type>
void cmptAv
(
    dgGaussField
    <
    typename dgGaussField<Type>::cmptType
    >& gcf,
    const dgGaussField<Type>& gf
)
{
	cmptAv(gcf.cellFieldRef(), gf.cellField());
	cmptAv(gcf.ownerFaceFieldRef(), gf.ownerFaceField());
	cmptAv(gcf.neighborFaceFieldRef(), gf.neighborFaceField());
}

template<class Type>
tmp
<
dgGaussField
<
typename dgGaussField<Type>::cmptType
>
>
cmptAv(const dgGaussField<Type>& gf)
{
	typedef typename dgGaussField<Type>::cmptType
	cmptType;

	tmp<dgGaussField<cmptType>> CmptAv
	                         (new   dgGaussField<cmptType>       (
	                              GeometricDofField<scalar, dgPatchField, dgGeoMesh>
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
	                          )
	                         );

	cmptAv(CmptAv.ref(), gf);

	return CmptAv;
}

template<class Type>
tmp
<
dgGaussField
<
typename dgGaussField<Type>::cmptType
>
>
cmptAv(const tmp<dgGaussField<Type>>& tgf)
{
	typedef typename dgGaussField<Type>::cmptType
	cmptType;

	const dgGaussField<Type>& gf = tgf();

	tmp<dgGaussField<cmptType>> CmptAv
	                         ( new dgGaussField<cmptType>
	                           (
	                               GeometricDofField<scalar, dgPatchField, dgGeoMesh>
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
	                           )
	                         );

	cmptAv(CmptAv.ref(), gf);

	tgf.clear();

	return CmptAv;
}





#define UNARY_REDUCTION_FUNCTION_WITH_BOUNDARY(returnType, func, gFunc)        \
                                                                               \
template<class Type>          \
dimensioned<returnType> func                                                   \
(                                                                              \
    const dgGaussField<Type>& gf                        \
)                                                                              \
{                                                                              \
    return dimensioned<Type>                                                   \
    (                                                                          \
        #func "(" + gf.name() + ')',                                           \
        gf.dimensions(),                                                       \
        Foam::func(gFunc(gf.cellField()), gFunc(gf.ownerFaceField()),gFunc(gf.neighborFaceField()))      \
    );                                                                         \
}                                                                              \
                                                                               \
template<class Type>          \
dimensioned<returnType> func                                                   \
(                                                                              \
    const tmp<dgGaussField<Type>>& tgf1                 \
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
template<class Type>          \
dimensioned<returnType> func                                                   \
(                                                                              \
    const dgGaussField<Type>& gf                        \
)                                                                              \
{                                                                              \
    return dimensioned<Type>                                                   \
    (                                                                          \
        #func "(" + gf.name() + ')',                                           \
        gf.dimensions(),                                                       \
        gFunc(gf.cellField())                                             \
    );                                                                         \
}                                                                              \
                                                                               \
template<class Type>          \
dimensioned<returnType> func                                                   \
(                                                                              \
    const tmp<dgGaussField<Type>>& tgf1                 \
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
<class Type1, class Type2>    \
void opFunc                                                                    \
(                                                                              \
    dgGaussField                                                             \
    <typename product<Type1, Type2>::type>& gf,           \
    const dgGaussField<Type1>& gf1,                     \
    const dgGaussField<Type2>& gf2                      \
)                                                                              \
{                                                                              \
    Foam::opFunc                                                               \
    (                                                                          \
        gf.cellFieldRef(),                                                \
        gf1.cellField(),                                                  \
        gf2.cellField()                                                   \
    );                                                                         \
    Foam::opFunc                                                               \
    (                                                                          \
        gf.ownerFaceFieldRef(),                                                 \
        gf1.ownerFaceField(),                                                   \
        gf2.ownerFaceField()                                                    \
    );                                                                         \
      Foam::opFunc                                                               \
    (                                                                          \
        gf.neighborFaceFieldRef(),                                                 \
        gf1.neighborFaceField(),                                                   \
        gf2.neighborFaceField()                                                    \
    );                                                                         \
}                                                                              \
                                                                             \
template                                                                       \
<class Type1, class Type2>    \
tmp                                                                            \
<                                                                              \
    dgGaussField<typename product<Type1, Type2>::type>  \
>                                                                              \
operator op                                                                    \
(                                                                              \
    const dgGaussField<Type1>& gf1,                     \
    const dgGaussField<Type2>& gf2                      \
)                                                                              \
{                                                                              \
    typedef typename product<Type1, Type2>::type productType;                  \
    tmp<dgGaussField<productType>> tRes                 \
    (                                                                           \
    new dgGaussField<productType> \
    (                                       \
     GeometricDofField<productType, dgPatchField, dgGeoMesh>                   \
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
        )                                                                   \
        )                       \
    );                                                                         \
                                                                               \
    Foam::opFunc(tRes.ref(), gf1, gf2);                                        \
                                                                               \
    return tRes;                                                               \
}                                                                              \
                                                                              \
template                                                                       \
<class Type1, class Type2>    \
tmp                                                                            \
<                                                                              \
    dgGaussField<typename product<Type1, Type2>::type>  \
>                                                                              \
operator op                                                                    \
(                                                                              \
    const dgGaussField<Type1>& gf1,                     \
    const tmp<dgGaussField<Type2>>& tgf2                \
)                                                                              \
{                                                                              \
    typedef typename product<Type1, Type2>::type productType;                  \
                                                                               \
    const dgGaussField<Type2>& gf2 = tgf2();            \
                                                                               \
    tmp<dgGaussField<productType>> tRes =               \
        reuseTmpdgGaussField<productType, Type2>::New   \
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
<class Type1, class Type2>    \
tmp                                                                            \
<                                                                              \
    dgGaussField<typename product<Type1, Type2>::type>  \
>                                                                              \
operator op                                                                    \
(                                                                              \
    const tmp<dgGaussField<Type1>>& tgf1,               \
    const dgGaussField<Type2>& gf2                      \
)                                                                              \
{                                                                              \
    typedef typename product<Type1, Type2>::type productType;                  \
                                                                               \
    const dgGaussField<Type1>& gf1 = tgf1();            \
                                                                               \
    tmp<dgGaussField<productType>> tRes =               \
        reuseTmpdgGaussField<productType, Type1>::New   \
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
<class Type1, class Type2>    \
tmp                                                                            \
<                                                                              \
    dgGaussField<typename product<Type1, Type2>::type>  \
>                                                                              \
operator op                                                                    \
(                                                                              \
    const tmp<dgGaussField<Type1>>& tgf1,               \
    const tmp<dgGaussField<Type2>>& tgf2                \
)                                                                              \
{                                                                              \
    typedef typename product<Type1, Type2>::type productType;                  \
                                                                               \
    const dgGaussField<Type1>& gf1 = tgf1();            \
    const dgGaussField<Type2>& gf2 = tgf2();            \
                                                                               \
    tmp<dgGaussField<productType>> tRes =               \
        reuseTmpTmpdgGaussField                                          \
        <productType, Type1, Type1, Type2>::New           \
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
<class Form, class Type>      \
void opFunc                                                                    \
(                                                                              \
    dgGaussField                                                             \
    <typename product<Type, Form>::type>& gf,             \
    const dgGaussField<Type>& gf1,                      \
    const dimensioned<Form>& dvs                                               \
)                                                                              \
{                                                                              \
    Foam::opFunc(gf.cellFieldRef(), gf1.cellField(), dvs.value());   \
    Foam::opFunc(gf.ownerFaceFieldRef(), gf1.ownerFaceField(), dvs.value());     \
    Foam::opFunc(gf.neighborFaceFieldRef(), gf1.neighborFaceField(), dvs.value());     \
}                                                                              \
                                                                               \
template                                                                       \
<class Form, class Type>      \
tmp<dgGaussField<typename product<Type, Form>::type>>   \
operator op                                                                    \
(                                                                              \
    const dgGaussField<Type>& gf1,                      \
    const dimensioned<Form>& dvs                                               \
)                                                                              \
{                                                                              \
    typedef typename product<Type, Form>::type productType;                    \
                                                                               \
    tmp<dgGaussField<productType>> tRes                 \
    (                       \
    new dgGaussField<productType> (                                                                      \
       ( GeometricDofField<productType, dgPatchField, dgGeoMesh>                   \
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
        )                                               \
        )                                                  \
        )                                        \
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
    direction nCmpt  ,                                  \
    class Type                                      \
>                                                                              \
tmp<dgGaussField<typename product<Form, Type>::type>>   \
operator op                                                                    \
(                                                                              \
    const dgGaussField<Type>& gf1,                      \
    const VectorSpace<Form,Cmpt,nCmpt>& vs                                     \
)                                                                              \
{                                                                              \
    return gf1 op dimensioned<Form>(static_cast<const Form&>(vs));             \
}                                                                              \
                                                                               \
                                                                               \
template                                                                       \
<class Form, class Type>                                                       \
tmp<dgGaussField<typename product<Type, Form>::type>>                          \
operator op                                                                    \
(                                                                              \
    const tmp<dgGaussField<Type>>& tgf1,                                       \
    const dimensioned<Form>& dvs                                               \
)                                                                              \
{                                                                              \
    typedef typename product<Type, Form>::type productType;                    \
                                                                               \
    const dgGaussField<Type>& gf1 = tgf1();                                    \
                                                                               \
    tmp<dgGaussField<productType>> tRes =               \
        reuseTmpdgGaussField<productType, Type>::New    \
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
    direction nCmpt ,                                                         \
    class Type                                                                  \
>                                                                              \
tmp<dgGaussField<typename product<Form, Type>::type>>   \
operator op                                                                    \
(                                                                              \
    const tmp<dgGaussField<Type>>& tgf1,                \
    const VectorSpace<Form,Cmpt,nCmpt>& vs                                     \
)                                                                              \
{                                                                              \
    return tgf1 op dimensioned<Form>(static_cast<const Form&>(vs));            \
}                                                                              \
                                                                               \
                                                                               \
template                                                                       \
<class Form, class Type>      \
void opFunc                                                                    \
(                                                                              \
    dgGaussField                                                             \
    <typename product<Form, Type>::type>& gf,             \
    const dimensioned<Form>& dvs,                                              \
    const dgGaussField<Type>& gf1                       \
)                                                                              \
{                                                                              \
    Foam::opFunc(gf.cellFieldRef(), dvs.value(), gf1.cellField());   \
    Foam::opFunc(gf.ownerFaceFieldRef(), dvs.value(), gf1.ownerFaceField());     \
    Foam::opFunc(gf.neighborFaceFieldRef(), dvs.value(), gf1.neighborFaceField());     \
}                                                                              \
                                                                               \
template                                                                       \
<class Form, class Type>      \
tmp<dgGaussField<typename product<Form, Type>::type>>   \
operator op                                                                    \
(                                                                              \
    const dimensioned<Form>& dvs,                                              \
    const dgGaussField<Type>& gf1                       \
)                                                                              \
{                                                                              \
    typedef typename product<Form, Type>::type productType;                    \
    tmp<dgGaussField<productType>> tRes                 \
    (                       \
        new     dgGaussField<productType>    \
        (                           \
        new GeometricDofField<productType, dgPatchField, dgGeoMesh>            \
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
        )      \
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
    class Type                                                                 \
>                                                                              \
tmp<dgGaussField<typename product<Form, Type>::type>>   \
operator op                                                                    \
(                                                                              \
    const VectorSpace<Form,Cmpt,nCmpt>& vs,                                    \
    const dgGaussField<Type>& gf1                       \
)                                                                              \
{                                                                              \
    return dimensioned<Form>(static_cast<const Form&>(vs)) op gf1;             \
}                                                                              \
                                                                               \
template                                                                       \
<class Form, class Type>      \
tmp<dgGaussField<typename product<Form, Type>::type>>   \
operator op                                                                    \
(                                                                              \
    const dimensioned<Form>& dvs,                                              \
    const tmp<dgGaussField<Type>>& tgf1                 \
)                                                                              \
{                                                                              \
    typedef typename product<Form, Type>::type productType;                    \
                                                                               \
    const dgGaussField<Type>& gf1 = tgf1();             \
                                                                               \
    tmp<dgGaussField<productType>> tRes =               \
        reuseTmpdgGaussField<productType, Type>::New    \
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
    class Type                                                                 \
>                                                                              \
tmp<dgGaussField<typename product<Form, Type>::type>>   \
operator op                                                                    \
(                                                                              \
    const VectorSpace<Form,Cmpt,nCmpt>& vs,                                    \
    const tmp<dgGaussField<Type>>& tgf1                 \
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

#include "undefdgGaussFieldFunctionsM.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
