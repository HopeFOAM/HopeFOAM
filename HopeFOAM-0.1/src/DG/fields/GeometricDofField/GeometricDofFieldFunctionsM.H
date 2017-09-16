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

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#define UNARY_FUNCTION(ReturnType, Type1, Func, Dfunc)                         \
                                                                               \
TEMPLATE                                                                       \
tmp<GeometricDofField<ReturnType, PatchField, GeoMesh>> Func                      \
(                                                                              \
    const GeometricDofField<Type1, PatchField, GeoMesh>& df                       \
);                                                                             \
                                                                               \
TEMPLATE                                                                       \
tmp<GeometricDofField<ReturnType, PatchField, GeoMesh>> Func                      \
(                                                                              \
    const tmp<GeometricDofField<Type1, PatchField, GeoMesh>>& tdf1                \
);


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#define UNARY_OPERATOR(ReturnType, Type1, Op, opFunc, Dfunc)                   \
                                                                               \
TEMPLATE                                                                       \
tmp<GeometricDofField<ReturnType, PatchField, GeoMesh>> operator Op               \
(                                                                              \
    const GeometricDofField<Type1, PatchField, GeoMesh>& df1                      \
);                                                                             \
                                                                               \
TEMPLATE                                                                       \
tmp<GeometricDofField<ReturnType, PatchField, GeoMesh>> operator Op               \
(                                                                              \
    const tmp<GeometricDofField<Type1, PatchField, GeoMesh>>& tdf1                \
);


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#define BINARY_FUNCTION(ReturnType, Type1, Type2, Func)                        \
                                                                               \
TEMPLATE                                                                       \
tmp<GeometricDofField<ReturnType, PatchField, GeoMesh>> Func                      \
(                                                                              \
    const GeometricDofField<Type1, PatchField, GeoMesh>& df1,                     \
    const GeometricDofField<Type2, PatchField, GeoMesh>& df2                      \
);                                                                             \
                                                                               \
TEMPLATE                                                                       \
tmp<GeometricDofField<ReturnType, PatchField, GeoMesh>> Func                      \
(                                                                              \
    const GeometricDofField<Type1, PatchField, GeoMesh>& df1,                     \
    const tmp<GeometricDofField<Type2, PatchField, GeoMesh>>& tdf2                \
);                                                                             \
                                                                               \
TEMPLATE                                                                       \
tmp<GeometricDofField<ReturnType, PatchField, GeoMesh>> Func                      \
(                                                                              \
    const tmp<GeometricDofField<Type1, PatchField, GeoMesh>>& tdf1,               \
    const GeometricDofField<Type2, PatchField, GeoMesh>& df2                      \
);                                                                             \
                                                                               \
TEMPLATE                                                                       \
tmp<GeometricDofField<ReturnType, PatchField, GeoMesh>> Func                      \
(                                                                              \
    const tmp<GeometricDofField<Type1, PatchField, GeoMesh>>& tdf1,               \
    const tmp<GeometricDofField<Type2, PatchField, GeoMesh>>& tdf2                \
);


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#define BINARY_TYPE_FUNCTION_SF(ReturnType, Type1, Type2, Func)                \
                                                                               \
TEMPLATE                                                                       \
tmp<GeometricDofField<ReturnType, PatchField, GeoMesh>> Func                      \
(                                                                              \
    const dimensioned<Type1>& dt1,                                             \
    const GeometricDofField<Type2, PatchField, GeoMesh>& df2                      \
);                                                                             \
                                                                               \
TEMPLATE                                                                       \
tmp<GeometricDofField<ReturnType, PatchField, GeoMesh>> Func                      \
(                                                                              \
    const Type1& t1,                                                           \
    const GeometricDofField<Type2, PatchField, GeoMesh>& df2                      \
);                                                                             \
                                                                               \
TEMPLATE                                                                       \
tmp<GeometricDofField<ReturnType, PatchField, GeoMesh>> Func                      \
(                                                                              \
    const dimensioned<Type1>& dt1,                                             \
    const tmp<GeometricDofField<Type2, PatchField, GeoMesh>>& tdf2                \
);                                                                             \
                                                                               \
TEMPLATE                                                                       \
tmp<GeometricDofField<ReturnType, PatchField, GeoMesh>> Func                      \
(                                                                              \
    const Type1& t1,                                                           \
    const tmp<GeometricDofField<Type2, PatchField, GeoMesh>>& tdf2                \
);


#define BINARY_TYPE_FUNCTION_FS(ReturnType, Type1, Type2, Func)                \
                                                                               \
TEMPLATE                                                                       \
tmp<GeometricDofField<ReturnType, PatchField, GeoMesh>> Func                      \
(                                                                              \
    const GeometricDofField<Type1, PatchField, GeoMesh>& df1,                     \
    const dimensioned<Type2>& dt2                                              \
);                                                                             \
                                                                               \
TEMPLATE                                                                       \
tmp<GeometricDofField<ReturnType, PatchField, GeoMesh>> Func                      \
(                                                                              \
    const GeometricDofField<Type1, PatchField, GeoMesh>& df1,                     \
    const Type2& t2                                                            \
);                                                                             \
                                                                               \
TEMPLATE                                                                       \
tmp<GeometricDofField<ReturnType, PatchField, GeoMesh>> Func                      \
(                                                                              \
    const tmp<GeometricDofField<Type1, PatchField, GeoMesh>>& tdf1,               \
    const dimensioned<Type2>& dt2                                              \
);                                                                             \
                                                                               \
TEMPLATE                                                                       \
tmp<GeometricDofField<ReturnType, PatchField, GeoMesh>> Func                      \
(                                                                              \
    const tmp<GeometricDofField<Type1, PatchField, GeoMesh>>& tdf2,               \
    const Type2& t2                                                            \
);


#define BINARY_TYPE_FUNCTION(ReturnType, Type1, Type2, Func)                   \
    BINARY_TYPE_FUNCTION_SF(ReturnType, Type1, Type2, Func)                    \
    BINARY_TYPE_FUNCTION_FS(ReturnType, Type1, Type2, Func)


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#define BINARY_OPERATOR(ReturnType, Type1, Type2, Op, OpName, OpFunc)          \
                                                                               \
TEMPLATE                                                                       \
tmp<GeometricDofField<ReturnType, PatchField, GeoMesh>> operator Op               \
(                                                                              \
    const GeometricDofField<Type1, PatchField, GeoMesh>& df1,                     \
    const GeometricDofField<Type2, PatchField, GeoMesh>& df2                      \
);                                                                             \
                                                                               \
TEMPLATE                                                                       \
tmp<GeometricDofField<ReturnType, PatchField, GeoMesh>> operator Op               \
(                                                                              \
    const GeometricDofField<Type1, PatchField, GeoMesh>& df1,                     \
    const tmp<GeometricDofField<Type2, PatchField, GeoMesh>>& tdf2                \
);                                                                             \
                                                                               \
TEMPLATE                                                                       \
tmp<GeometricDofField<ReturnType, PatchField, GeoMesh>> operator Op               \
(                                                                              \
    const tmp<GeometricDofField<Type1, PatchField, GeoMesh>>& tdf1,               \
    const GeometricDofField<Type2, PatchField, GeoMesh>& df2                      \
);                                                                             \
                                                                               \
TEMPLATE                                                                       \
tmp<GeometricDofField<ReturnType, PatchField, GeoMesh>> operator Op               \
(                                                                              \
    const tmp<GeometricDofField<Type1, PatchField, GeoMesh>>& tdf1,               \
    const tmp<GeometricDofField<Type2, PatchField, GeoMesh>>& tdf2                \
);


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#define BINARY_TYPE_OPERATOR_SF(ReturnType, Type1, Type2, Op, OpName, OpFunc) \
                                                                               \
TEMPLATE                                                                       \
tmp<GeometricDofField<ReturnType, PatchField, GeoMesh>> operator Op               \
(                                                                              \
    const dimensioned<Type1>& dt1,                                             \
    const GeometricDofField<Type2, PatchField, GeoMesh>& df2                      \
);                                                                             \
                                                                               \
TEMPLATE                                                                       \
tmp<GeometricDofField<ReturnType, PatchField, GeoMesh>> operator Op               \
(                                                                              \
    const Type1& t1,                                                           \
    const GeometricDofField<Type2, PatchField, GeoMesh>& df2                      \
);                                                                             \
                                                                               \
TEMPLATE                                                                       \
tmp<GeometricDofField<ReturnType, PatchField, GeoMesh>> operator Op               \
(                                                                              \
    const dimensioned<Type1>& dt1,                                             \
    const tmp<GeometricDofField<Type2, PatchField, GeoMesh>>& tdf2                \
);                                                                             \
                                                                               \
TEMPLATE                                                                       \
tmp<GeometricDofField<ReturnType, PatchField, GeoMesh>> operator Op               \
(                                                                              \
    const Type1& t1,                                                           \
    const tmp<GeometricDofField<Type2, PatchField, GeoMesh>>& tdf2                \
);


#define BINARY_TYPE_OPERATOR_FS(ReturnType, Type1, Type2, Op, OpName, OpFunc) \
                                                                               \
TEMPLATE                                                                       \
tmp<GeometricDofField<ReturnType, PatchField, GeoMesh>> operator Op               \
(                                                                              \
    const GeometricDofField<Type1, PatchField, GeoMesh>& df1,                     \
    const dimensioned<Type2>& dt2                                              \
);                                                                             \
                                                                               \
TEMPLATE                                                                       \
tmp<GeometricDofField<ReturnType, PatchField, GeoMesh>> operator Op               \
(                                                                              \
    const GeometricDofField<Type1, PatchField, GeoMesh>& df1,                     \
    const Type2& t2                                                            \
);                                                                             \
                                                                               \
TEMPLATE                                                                       \
tmp<GeometricDofField<ReturnType, PatchField, GeoMesh>> operator Op               \
(                                                                              \
    const tmp<GeometricDofField<Type1, PatchField, GeoMesh>>& tdf1,               \
    const dimensioned<Type2>& dt2                                              \
);                                                                             \
                                                                               \
TEMPLATE                                                                       \
tmp<GeometricDofField<ReturnType, PatchField, GeoMesh>> operator Op               \
(                                                                              \
    const tmp<GeometricDofField<Type1, PatchField, GeoMesh>>& tdf1,               \
    const Type2& t2                                                            \
);


#define BINARY_TYPE_OPERATOR(ReturnType, Type1, Type2, Op, OpName, OpFunc)     \
    BINARY_TYPE_OPERATOR_SF(ReturnType, Type1, Type2, Op, OpName, OpFunc)      \
    BINARY_TYPE_OPERATOR_FS(ReturnType, Type1, Type2, Op, OpName, OpFunc)


// ************************************************************************* //
