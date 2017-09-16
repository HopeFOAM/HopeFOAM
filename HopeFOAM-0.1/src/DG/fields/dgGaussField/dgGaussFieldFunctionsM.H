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
tmp<dgGaussField<ReturnType>> Func                      \
(                                                                              \
    const dgGaussField<Type1>& df                       \
);                                                                             \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType>> Func                      \
(                                                                              \
    const tmp<dgGaussField<Type1>>& tdf1                \
);


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#define UNARY_OPERATOR(ReturnType, Type1, Op, opFunc, Dfunc)                   \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType>> operator Op               \
(                                                                              \
    const dgGaussField<Type1>& df1                      \
);                                                                             \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType>> operator Op               \
(                                                                              \
    const tmp<dgGaussField<Type1>>& tdf1                \
);


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#define BINARY_FUNCTION(ReturnType, Type1, Type2, Func)                        \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType>> Func                      \
(                                                                              \
    const dgGaussField<Type1>& df1,                     \
    const dgGaussField<Type2>& df2                      \
);                                                                             \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType>> Func                      \
(                                                                              \
    const dgGaussField<Type1>& df1,                     \
    const tmp<dgGaussField<Type2>>& tdf2                \
);                                                                             \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType>> Func                      \
(                                                                              \
    const tmp<dgGaussField<Type1>>& tdf1,               \
    const dgGaussField<Type2>& df2                      \
);                                                                             \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType>> Func                      \
(                                                                              \
    const tmp<dgGaussField<Type1>>& tdf1,               \
    const tmp<dgGaussField<Type2>>& tdf2                \
);


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#define BINARY_TYPE_FUNCTION_SF(ReturnType, Type1, Type2, Func)                \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType>> Func                      \
(                                                                              \
    const dimensioned<Type1>& dt1,                                             \
    const dgGaussField<Type2>& df2                      \
);                                                                             \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType>> Func                      \
(                                                                              \
    const Type1& t1,                                                           \
    const dgGaussField<Type2>& df2                      \
);                                                                             \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> Func                      \
(                                                                              \
    const dimensioned<Type1>& dt1,                                             \
    const tmp<dgGaussField<Type2 >>& tdf2                \
);                                                                             \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> Func                      \
(                                                                              \
    const Type1& t1,                                                           \
    const tmp<dgGaussField<Type2 >>& tdf2                \
);


#define BINARY_TYPE_FUNCTION_FS(ReturnType, Type1, Type2, Func)                \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> Func                      \
(                                                                              \
    const dgGaussField<Type1 >& df1,                     \
    const dimensioned<Type2>& dt2                                              \
);                                                                             \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> Func                      \
(                                                                              \
    const dgGaussField<Type1 >& df1,                     \
    const Type2& t2                                                            \
);                                                                             \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> Func                      \
(                                                                              \
    const tmp<dgGaussField<Type1 >>& tdf1,               \
    const dimensioned<Type2>& dt2                                              \
);                                                                             \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> Func                      \
(                                                                              \
    const tmp<dgGaussField<Type1 >>& tdf2,               \
    const Type2& t2                                                            \
);


#define BINARY_TYPE_FUNCTION(ReturnType, Type1, Type2, Func)                   \
    BINARY_TYPE_FUNCTION_SF(ReturnType, Type1, Type2, Func)                    \
    BINARY_TYPE_FUNCTION_FS(ReturnType, Type1, Type2, Func)


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#define BINARY_OPERATOR(ReturnType, Type1, Type2, Op, OpName, OpFunc)          \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> operator Op               \
(                                                                              \
    const dgGaussField<Type1 >& df1,                     \
    const dgGaussField<Type2 >& df2                      \
);                                                                             \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> operator Op               \
(                                                                              \
    const dgGaussField<Type1 >& df1,                     \
    const tmp<dgGaussField<Type2 >>& tdf2                \
);                                                                             \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> operator Op               \
(                                                                              \
    const tmp<dgGaussField<Type1 >>& tdf1,               \
    const dgGaussField<Type2 >& df2                      \
);                                                                             \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> operator Op               \
(                                                                              \
    const tmp<dgGaussField<Type1 >>& tdf1,               \
    const tmp<dgGaussField<Type2 >>& tdf2                \
);


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#define BINARY_TYPE_OPERATOR_SF(ReturnType, Type1, Type2, Op, OpName, OpFunc) \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> operator Op               \
(                                                                              \
    const dimensioned<Type1>& dt1,                                             \
    const dgGaussField<Type2 >& df2                      \
);                                                                             \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> operator Op               \
(                                                                              \
    const Type1& t1,                                                           \
    const dgGaussField<Type2 >& df2                      \
);                                                                             \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> operator Op               \
(                                                                              \
    const dimensioned<Type1>& dt1,                                             \
    const tmp<dgGaussField<Type2 >>& tdf2                \
);                                                                             \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> operator Op               \
(                                                                              \
    const Type1& t1,                                                           \
    const tmp<dgGaussField<Type2 >>& tdf2                \
);


#define BINARY_TYPE_OPERATOR_FS(ReturnType, Type1, Type2, Op, OpName, OpFunc) \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> operator Op               \
(                                                                              \
    const dgGaussField<Type1 >& df1,                     \
    const dimensioned<Type2>& dt2                                              \
);                                                                             \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> operator Op               \
(                                                                              \
    const dgGaussField<Type1 >& df1,                     \
    const Type2& t2                                                            \
);                                                                             \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> operator Op               \
(                                                                              \
    const tmp<dgGaussField<Type1 >>& tdf1,               \
    const dimensioned<Type2>& dt2                                              \
);                                                                             \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> operator Op               \
(                                                                              \
    const tmp<dgGaussField<Type1 >>& tdf1,               \
    const Type2& t2                                                            \
);


#define BINARY_TYPE_OPERATOR(ReturnType, Type1, Type2, Op, OpName, OpFunc)     \
    BINARY_TYPE_OPERATOR_SF(ReturnType, Type1, Type2, Op, OpName, OpFunc)      \
    BINARY_TYPE_OPERATOR_FS(ReturnType, Type1, Type2, Op, OpName, OpFunc)


// ************************************************************************* //
