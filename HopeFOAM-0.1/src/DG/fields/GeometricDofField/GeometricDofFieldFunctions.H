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

#include "GeometricDofScalarField.H"

#define TEMPLATE \
    template<class Type, template<class> class PatchField, class GeoMesh>
#include "GeometricDofFieldFunctionsM.H"

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
);

template<class Type, template<class> class PatchField, class GeoMesh>
void T
(
     GeometricDofField<Type, PatchField, GeoMesh>& gf,
     const GeometricDofField<Type, PatchField, GeoMesh>& gf1
);

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
);

template
<
    class Type,
    template<class> class PatchField,
    class GeoMesh,
    direction r
>
tmp
<
    GeometricDofField
    <typename powProduct<Type, r>::type, PatchField, GeoMesh>
>
pow
(
    const GeometricDofField<Type, PatchField, GeoMesh>& gf,
    typename powProduct<Type, r>::type
);

template
<
    class Type,
    template<class> class PatchField,
    class GeoMesh,
    direction r
>
tmp
<
    GeometricDofField
    <typename powProduct<Type, r>::type, PatchField, GeoMesh>
>
pow
(
    const tmp<GeometricDofField<Type, PatchField, GeoMesh>>& tgf,
    typename powProduct<Type, r>::type
);

template<class Type, template<class> class PatchField, class GeoMesh>
void sqr
(
    GeometricDofField
    <typename outerProduct<Type, Type>::type, PatchField, GeoMesh>& gf,
    const GeometricDofField<Type, PatchField, GeoMesh>& gf1
);

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
sqr(const GeometricDofField<Type, PatchField, GeoMesh>& gf);

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
sqr(const tmp<GeometricDofField<Type, PatchField, GeoMesh>>& tgf);

template<class Type, template<class> class PatchField, class GeoMesh>
void magSqr
(
    GeometricDofField<scalar, PatchField, GeoMesh>& gsf,
    const GeometricDofField<Type, PatchField, GeoMesh>& gf
);

template<class Type, template<class> class PatchField, class GeoMesh>
tmp<GeometricDofField<scalar, PatchField, GeoMesh>> magSqr
(
    const GeometricDofField<Type, PatchField, GeoMesh>& gf
);

template<class Type, template<class> class PatchField, class GeoMesh>
tmp<GeometricDofField<scalar, PatchField, GeoMesh>> magSqr
(
    const tmp<GeometricDofField<Type, PatchField, GeoMesh>>& tgf
);

template<class Type, template<class> class PatchField, class GeoMesh>
void mag
(
    GeometricDofField<scalar, PatchField, GeoMesh>& gsf,
    const GeometricDofField<Type, PatchField, GeoMesh>& gf
);

template<class Type, template<class> class PatchField, class GeoMesh>
tmp<GeometricDofField<scalar, PatchField, GeoMesh>> mag
(
    const GeometricDofField<Type, PatchField, GeoMesh>& gf
);

template<class Type, template<class> class PatchField, class GeoMesh>
tmp<GeometricDofField<scalar, PatchField, GeoMesh>> mag
(
    const tmp<GeometricDofField<Type, PatchField, GeoMesh>>& tgf
);

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
);

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
cmptAv(const GeometricDofField<Type, PatchField, GeoMesh>& gf);

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
cmptAv(const tmp<GeometricDofField<Type, PatchField, GeoMesh>>& tgf);


#define UNARY_REDUCTION_FUNCTION_WITH_BOUNDARY(returnType, func, gFunc)        \
                                                                               \
template<class Type, template<class> class PatchField, class GeoMesh>          \
dimensioned<returnType> func                                                   \
(                                                                              \
    const GeometricDofField<Type, PatchField, GeoMesh>& gf                        \
);                                                                             \
                                                                               \
template<class Type, template<class> class PatchField, class GeoMesh>          \
dimensioned<returnType> func                                                   \
(                                                                              \
    const tmp<GeometricDofField<Type, PatchField, GeoMesh>>& tgf1                 \
);

UNARY_REDUCTION_FUNCTION_WITH_BOUNDARY(Type, max, gMax)
UNARY_REDUCTION_FUNCTION_WITH_BOUNDARY(Type, min, gMin)

#undef UNARY_REDUCTION_FUNCTION_WITH_BOUNDARY


#define UNARY_REDUCTION_FUNCTION(returnType, func, gFunc)                      \
                                                                               \
template<class Type, template<class> class PatchField, class GeoMesh>          \
dimensioned<returnType> func                                                   \
(                                                                              \
    const GeometricDofField<Type, PatchField, GeoMesh>& gf                        \
);                                                                             \
                                                                               \
template<class Type, template<class> class PatchField, class GeoMesh>          \
dimensioned<returnType> func                                                   \
(                                                                              \
    const tmp<GeometricDofField<Type, PatchField, GeoMesh>>& tgf1                 \
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
<class Type1, class Type2, template<class> class PatchField, class GeoMesh>    \
void opFunc                                                                    \
(                                                                              \
    GeometricDofField                                                             \
    <typename product<Type1, Type2>::type, PatchField, GeoMesh>& gf,           \
    const GeometricDofField<Type1, PatchField, GeoMesh>& gf1,                     \
    const GeometricDofField<Type2, PatchField, GeoMesh>& gf2                      \
);                                                                             \
                                                                               \
template                                                                       \
<class Type1, class Type2, template<class> class PatchField, class GeoMesh>    \
tmp                                                                            \
<                                                                              \
    GeometricDofField                                                             \
        <typename product<Type1, Type2>::type, PatchField, GeoMesh>            \
>                                                                              \
operator op                                                                    \
(                                                                              \
    const GeometricDofField<Type1, PatchField, GeoMesh>& gf1,                     \
    const GeometricDofField<Type2, PatchField, GeoMesh>& gf2                      \
);                                                                             \
                                                                               \
template                                                                       \
<class Type1, class Type2, template<class> class PatchField, class GeoMesh>    \
tmp                                                                            \
<                                                                              \
    GeometricDofField                                                             \
    <typename product<Type1, Type2>::type, PatchField, GeoMesh>                \
>                                                                              \
operator op                                                                    \
(                                                                              \
    const GeometricDofField<Type1, PatchField, GeoMesh>& gf1,                     \
    const tmp<GeometricDofField<Type2, PatchField, GeoMesh>>& tgf2                \
);                                                                             \
                                                                               \
template                                                                       \
<class Type1, class Type2, template<class> class PatchField, class GeoMesh>    \
tmp                                                                            \
<                                                                              \
    GeometricDofField                                                             \
    <typename product<Type1, Type2>::type, PatchField, GeoMesh>                \
>                                                                              \
operator op                                                                    \
(                                                                              \
    const tmp<GeometricDofField<Type1, PatchField, GeoMesh>>& tgf1,               \
    const GeometricDofField<Type2, PatchField, GeoMesh>& gf2                      \
);                                                                             \
                                                                               \
template                                                                       \
<class Type1, class Type2, template<class> class PatchField, class GeoMesh>    \
tmp                                                                            \
<                                                                              \
    GeometricDofField                                                             \
    <typename product<Type1, Type2>::type, PatchField, GeoMesh>                \
>                                                                              \
operator op                                                                    \
(                                                                              \
    const tmp<GeometricDofField<Type1, PatchField, GeoMesh>>& tgf1,               \
    const tmp<GeometricDofField<Type2, PatchField, GeoMesh>>& tgf2                \
);                                                                             \
                                                                               \
template                                                                       \
<class Form, class Type, template<class> class PatchField, class GeoMesh>      \
void opFunc                                                                    \
(                                                                              \
    GeometricDofField                                                             \
    <typename product<Type, Form>::type, PatchField, GeoMesh>& gf,             \
    const GeometricDofField<Type, PatchField, GeoMesh>& gf1,                      \
    const dimensioned<Form>& dvs                                               \
);                                                                             \
                                                                               \
template                                                                       \
<class Form, class Type, template<class> class PatchField, class GeoMesh>      \
tmp                                                                            \
<                                                                              \
    GeometricDofField                                                             \
    <typename product<Type, Form>::type, PatchField, GeoMesh>                  \
>                                                                              \
operator op                                                                    \
(                                                                              \
    const GeometricDofField<Type, PatchField, GeoMesh>& gf1,                      \
    const dimensioned<Form>& dvs                                               \
);                                                                             \
                                                                               \
template                                                                       \
<                                                                              \
    class Form,                                                                \
    class Cmpt,                                                                \
    direction nCmpt,                                                           \
    class Type, template<class> class PatchField,                              \
    class GeoMesh                                                              \
>                                                                              \
tmp                                                                            \
<                                                                              \
    GeometricDofField                                                             \
    <typename product<Form, Type>::type, PatchField, GeoMesh>                  \
>                                                                              \
operator op                                                                    \
(                                                                              \
    const GeometricDofField<Type, PatchField, GeoMesh>& gf1,                      \
    const VectorSpace<Form,Cmpt,nCmpt>& vs                                     \
);                                                                             \
                                                                               \
template                                                                       \
<class Form, class Type, template<class> class PatchField, class GeoMesh>      \
tmp                                                                            \
<                                                                              \
    GeometricDofField                                                             \
    <typename product<Type, Form>::type, PatchField, GeoMesh>                  \
>                                                                              \
operator op                                                                    \
(                                                                              \
    const tmp<GeometricDofField<Type, PatchField, GeoMesh>>& tgf1,                \
    const dimensioned<Form>& dvs                                               \
);                                                                             \
                                                                               \
template                                                                       \
<                                                                              \
    class Form,                                                                \
    class Cmpt,                                                                \
    direction nCmpt,                                                           \
    class Type, template<class> class PatchField,                              \
    class GeoMesh                                                              \
>                                                                              \
tmp                                                                            \
<                                                                              \
    GeometricDofField                                                             \
    <typename product<Form, Type>::type, PatchField, GeoMesh>                  \
>                                                                              \
operator op                                                                    \
(                                                                              \
    const tmp<GeometricDofField<Type, PatchField, GeoMesh>>& tgf1,                \
    const VectorSpace<Form,Cmpt,nCmpt>& vs                                     \
);                                                                             \
                                                                               \
template                                                                       \
<class Form, class Type, template<class> class PatchField, class GeoMesh>      \
void opFunc                                                                    \
(                                                                              \
    GeometricDofField                                                             \
    <typename product<Form, Type>::type, PatchField, GeoMesh>& gf,             \
    const dimensioned<Form>& dvs,                                              \
    const GeometricDofField<Type, PatchField, GeoMesh>& gf1                       \
);                                                                             \
                                                                               \
template                                                                       \
<class Form, class Type, template<class> class PatchField, class GeoMesh>      \
tmp                                                                            \
<                                                                              \
    GeometricDofField                                                             \
    <typename product<Form, Type>::type, PatchField, GeoMesh>                  \
>                                                                              \
operator op                                                                    \
(                                                                              \
    const dimensioned<Form>& dvs,                                              \
    const GeometricDofField<Type, PatchField, GeoMesh>& gf1                       \
);                                                                             \
                                                                               \
template                                                                       \
<                                                                              \
    class Form,                                                                \
    class Cmpt,                                                                \
    direction nCmpt,                                                           \
    class Type, template<class> class PatchField,                              \
    class GeoMesh                                                              \
>                                                                              \
tmp                                                                            \
<                                                                              \
    GeometricDofField                                                             \
    <typename product<Form, Type>::type, PatchField, GeoMesh>                  \
>                                                                              \
operator op                                                                    \
(                                                                              \
    const VectorSpace<Form,Cmpt,nCmpt>& vs,                                    \
    const GeometricDofField<Type, PatchField, GeoMesh>& gf1                       \
);                                                                             \
                                                                               \
template                                                                       \
<class Form, class Type, template<class> class PatchField, class GeoMesh>      \
tmp                                                                            \
<                                                                              \
    GeometricDofField                                                             \
    <typename product<Form, Type>::type, PatchField, GeoMesh>                  \
>                                                                              \
operator op                                                                    \
(                                                                              \
    const dimensioned<Form>& dvs,                                              \
    const tmp<GeometricDofField<Type, PatchField, GeoMesh>>& tgf1                 \
);                                                                             \
                                                                               \
template                                                                       \
<                                                                              \
    class Form,                                                                \
    class Cmpt,                                                                \
    direction nCmpt,                                                           \
    class Type, template<class> class PatchField,                              \
    class GeoMesh                                                              \
>                                                                              \
tmp                                                                            \
<                                                                              \
    GeometricDofField                                                             \
    <typename product<Form, Type>::type, PatchField, GeoMesh>                  \
>                                                                              \
operator op                                                                    \
(                                                                              \
    const VectorSpace<Form,Cmpt,nCmpt>& vs,                                    \
    const tmp<GeometricDofField<Type, PatchField, GeoMesh>>& tgf1                 \
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

#include "undefDofFieldFunctionsM.H"

// ************************************************************************* //
