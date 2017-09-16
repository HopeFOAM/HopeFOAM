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

#define TEMPLATE \
    template<class Type>
#include "dgGaussFieldFunctionsM.H"

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
);

template<class Type>
void T
(
     dgGaussField<Type>& gf,
     const dgGaussField<Type>& gf1
);

template
<
    class Type,
    direction r
>
void pow
(
    dgGaussField<typename powProduct<Type, r>::type>& gf,
    const dgGaussField<Type>& gf1
);

template
<
    class Type,
    direction r
>
tmp
<
    dgGaussField
    <typename powProduct<Type, r>::type>
>
pow
(
    const dgGaussField<Type>& gf,
    typename powProduct<Type, r>::type
);

template
<
    class Type,
 
    direction r
>
tmp
<
    dgGaussField
    <typename powProduct<Type, r>::type>
>
pow
(
    const tmp<dgGaussField<Type>>& tgf,
    typename powProduct<Type, r>::type
);

template<class Type>
void sqr
(
    dgGaussField
    <typename outerProduct<Type, Type>::type>& gf,
    const dgGaussField<Type>& gf1
);

template<class Type>
tmp
<
    dgGaussField
    <
        typename outerProduct<Type, Type>::type
    >
>
sqr(const dgGaussField<Type>& gf);

template<class Type>
tmp
<
    dgGaussField
    <
        typename outerProduct<Type, Type>::type
    >
>
sqr(const tmp<dgGaussField<Type>>& tgf);

template<class Type>
void magSqr
(
    dgGaussField<scalar>& gsf,
    const dgGaussField<Type>& gf
);

template<class Type>
tmp<dgGaussField<scalar>> magSqr
(
    const dgGaussField<Type>& gf
);

template<class Type>
tmp<dgGaussField<scalar>> magSqr
(
    const tmp<dgGaussField<Type>>& tgf
);

template<class Type>
void mag
(
    dgGaussField<scalar>& gsf,
    const dgGaussField<Type>& gf
);

template<class Type>
tmp<dgGaussField<scalar>> mag
(
    const dgGaussField<Type>& gf
);

template<class Type>
tmp<dgGaussField<scalar>> mag
(
    const tmp<dgGaussField<Type>>& tgf
);

template<class Type>
void cmptAv
(
    dgGaussField
    <
        typename dgGaussField<Type>::cmptType
    >& gcf,
    const dgGaussField<Type>& gf
);

template<class Type>
tmp
<
    dgGaussField
    <
        typename dgGaussField<Type>::cmptType
    >
>
cmptAv(const dgGaussField<Type>& gf);

template<class Type>
tmp
<
    dgGaussField
    <
        typename dgGaussField<Type>::cmptType
    >
>
cmptAv(const tmp<dgGaussField<Type>>& tgf);


#define UNARY_REDUCTION_FUNCTION_WITH_BOUNDARY(returnType, func, gFunc)        \
                                                                               \
template<class Type>          \
dimensioned<returnType> func                                                   \
(                                                                              \
    const dgGaussField<Type>& gf                        \
);                                                                             \
                                                                               \
template<class Type>          \
dimensioned<returnType> func                                                   \
(                                                                              \
    const tmp<dgGaussField<Type>>& tgf1                 \
);

UNARY_REDUCTION_FUNCTION_WITH_BOUNDARY(Type, max, gMax)
UNARY_REDUCTION_FUNCTION_WITH_BOUNDARY(Type, min, gMin)

#undef UNARY_REDUCTION_FUNCTION_WITH_BOUNDARY


#define UNARY_REDUCTION_FUNCTION(returnType, func, gFunc)                      \
                                                                               \
template<class Type>          \
dimensioned<returnType> func                                                   \
(                                                                              \
    const dgGaussField<Type>& gf                        \
);                                                                             \
                                                                               \
template<class Type>          \
dimensioned<returnType> func                                                   \
(                                                                              \
    const tmp<dgGaussField<Type>>& tgf1                 \
);

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
<class Type1, class Type2>    												   \
void opFunc                                                                    \
(                                                                              \
    dgGaussField                                                               \
    <typename product<Type1, Type2>::type>& gf,          					   \
    const dgGaussField<Type1>& gf1,                     \
    const dgGaussField<Type2>& gf2                      \
);                                                                             \
                                                                               \
template                                                                       \
<class Type1, class Type2>    												   \
tmp                                                                            \
<                                                                              \
    dgGaussField                                                               \
        <typename product<Type1, Type2>::type>                                 \
>                                                                              \
operator op                                                                    \
(                                                                              \
    const dgGaussField<Type1>& gf1,                                            \
    const dgGaussField<Type2>& gf2                                             \
);                                                                             \
                                                                               \
template                                                                       \
<class Type1, class Type2>                                                     \
tmp                                                                            \
<                                                                              \
    dgGaussField                                                             \
    <typename product<Type1, Type2>::type>                \
>                                                                              \
operator op                                                                    \
(                                                                              \
    const dgGaussField<Type1>& gf1,                     \
    const tmp<dgGaussField<Type2>>& tgf2                \
);                                                                             \
                                                                               \
template                                                                       \
<class Type1, class Type2>    \
tmp                                                                            \
<                                                                              \
    dgGaussField                                                             \
    <typename product<Type1, Type2>::type>                \
>                                                                              \
operator op                                                                    \
(                                                                              \
    const tmp<dgGaussField<Type1>>& tgf1,               \
    const dgGaussField<Type2>& gf2                      \
);                                                                             \
                                                                               \
template                                                                       \
<class Type1, class Type2>    \
tmp                                                                            \
<                                                                              \
    dgGaussField                                                             \
    <typename product<Type1, Type2>::type>                \
>                                                                              \
operator op                                                                    \
(                                                                              \
    const tmp<dgGaussField<Type1>>& tgf1,               \
    const tmp<dgGaussField<Type2>>& tgf2                \
);                                                                             \
                                                                               \
template                                                                       \
<class Form, class Type>      \
void opFunc                                                                    \
(                                                                              \
    dgGaussField                                                             \
    <typename product<Type, Form>::type>& gf,             \
    const dgGaussField<Type>& gf1,                      \
    const dimensioned<Form>& dvs                                               \
);                                                                             \
                                                                               \
template                                                                       \
<class Form, class Type>      \
tmp                                                                            \
<                                                                              \
    dgGaussField                                                             \
    <typename product<Type, Form>::type>                  \
>                                                                              \
operator op                                                                    \
(                                                                              \
    const dgGaussField<Type>& gf1,                      \
    const dimensioned<Form>& dvs                                               \
);                                                                             \
                                                                               \
template                                                                       \
<                                                                              \
    class Form,                                                                \
    class Cmpt,                                                                \
    direction nCmpt,                                                         \
    class Type                                                     \
>                                                                              \
tmp                                                                            \
<                                                                              \
    dgGaussField                                                             \
    <typename product<Form, Type>::type>                  \
>                                                                              \
operator op                                                                    \
(                                                                              \
    const dgGaussField<Type>& gf1,                      \
    const VectorSpace<Form,Cmpt,nCmpt>& vs                                     \
);                                                                             \
                                                                               \
template                                                                       \
<class Form, class Type>      \
tmp                                                                            \
<                                                                              \
    dgGaussField                                                             \
    <typename product<Type, Form>::type>                  \
>                                                                              \
operator op                                                                    \
(                                                                              \
    const tmp<dgGaussField<Type>>& tgf1,                \
    const dimensioned<Form>& dvs                                               \
);                                                                             \
                                                                               \
template                                                                       \
<                                                                              \
    class Form,                                                                \
    class Cmpt,                                                                \
    direction nCmpt ,                                                                                \
    class Type                                                                 \
>                                                                              \
tmp                                                                            \
<                                                                              \
    dgGaussField                                                             \
    <typename product<Form, Type>::type>                  \
>                                                                              \
operator op                                                                    \
(                                                                              \
    const tmp<dgGaussField<Type>>& tgf1,                \
    const VectorSpace<Form,Cmpt,nCmpt>& vs                                     \
);                                                                             \
                                                                               \
template                                                                       \
<class Form, class Type>      \
void opFunc                                                                    \
(                                                                              \
    dgGaussField                                                             \
    <typename product<Form, Type>::type>& gf,             \
    const dimensioned<Form>& dvs,                                              \
    const dgGaussField<Type>& gf1                       \
);                                                                             \
                                                                               \
template                                                                       \
<class Form, class Type>      \
tmp                                                                            \
<                                                                              \
    dgGaussField                                                             \
    <typename product<Form, Type>::type>                  \
>                                                                              \
operator op                                                                    \
(                                                                              \
    const dimensioned<Form>& dvs,                                              \
    const dgGaussField<Type>& gf1                       \
);                                                                             \
                                                                               \
template                                                                       \
<                                                                              \
    class Form,                                                                \
    class Cmpt,                                                                \
    direction nCmpt ,                                                        \
    class Type                                                           \
>                                                                              \
tmp                                                                            \
<                                                                              \
    dgGaussField                                                             \
    <typename product<Form, Type>::type>                  \
>                                                                              \
operator op                                                                    \
(                                                                              \
    const VectorSpace<Form,Cmpt,nCmpt>& vs,                                    \
    const dgGaussField<Type>& gf1                       \
);                                                                             \
                                                                               \
template                                                                       \
<class Form, class Type>      \
tmp                                                                            \
<                                                                              \
    dgGaussField                                                             \
    <typename product<Form, Type>::type>                  \
>                                                                              \
operator op                                                                    \
(                                                                              \
    const dimensioned<Form>& dvs,                                              \
    const tmp<dgGaussField<Type>>& tgf1                 \
);                                                                             \
                                                                               \
template                                                                       \
<                                                                              \
    class Form,                                                                \
    class Cmpt,                                                                \
    direction nCmpt    ,                                                    \
     class Type                                                      \
>                                                                              \
tmp                                                                            \
<                                                                              \
    dgGaussField                                                             \
    <typename product<Form, Type>::type>                  \
>                                                                              \
operator op                                                                    \
(                                                                              \
    const VectorSpace<Form,Cmpt,nCmpt>& vs,                                    \
    const tmp<dgGaussField<Type>>& tgf1                 \
);

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

// ************************************************************************* //
