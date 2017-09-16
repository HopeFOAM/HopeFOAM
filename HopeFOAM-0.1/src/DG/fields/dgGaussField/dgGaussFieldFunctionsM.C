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

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
//*******unary_function: one operator funciton (  function type1  ||  function op type1 )
 //*******binary_function: two operator function || function  field field 
//										     || function   field dimension   (fs)
//											|| function  dimension field  (fs)					
//											||operator with the parameter as above
namespace Foam
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#define UNARY_FUNCTION(ReturnType, Type1, Func, Dfunc)                         \
                                                                               \
TEMPLATE                                                                       \
void Func                                                                      \
(                                                                              \
    dgGaussField<ReturnType >& res,                      \
    const dgGaussField<Type1 >& gf1                      \
)                                                                              \
{                                                                              \
    Foam::Func(res.cellFieldRef(), gf1.cellField());                 \
    Foam::Func(res.ownerFaceFieldRef(), gf1.ownerFaceField());                   \
    Foam::Func(res.neighborFaceFieldRef(), gf1.neighborFaceField());                   \
}                                                                              \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> Func                      \
(                                                                              \
    const dgGaussField<Type1 >& gf1                      \
)                                                                              \
{                                                                              \
    tmp<dgGaussField<ReturnType >> tRes                  \
    (                                                                          \
    	new dgGaussField<ReturnType >			\
    	(							\
         GeometricDofField<ReturnType,  dgPatchField, dgGeoMesh>                    \
        (                                                                      \
            IOobject                                                           \
            (                                                                  \
                #Func "(" + gf1.name() + ')',                                  \
                gf1.instance(),                                                \
                gf1.db(),                                                      \
                IOobject::NO_READ,                                             \
                IOobject::NO_WRITE                                             \
            ),                                                                 \
            gf1.mesh(),                                                        \
            Dfunc(gf1.dimensions())                                            \
        )                                                                      \
        )			\
    );                                                                         \
                                                                               \
    Foam::Func(tRes.ref(), gf1);                                               \
                                                                               \
    return tRes;                                                               \
}                                                                              \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> Func                      \
(                                                                              \
    const tmp<dgGaussField<Type1 >>& tgf1                \
)                                                                              \
{                                                                              \
    const dgGaussField<Type1 >& gf1 = tgf1();            \
                                                                               \
    tmp<dgGaussField<ReturnType >> tRes                  \
    (                                                                          \
        reuseTmpdgGaussField<ReturnType, Type1 >::New    \
        (                                                                      \
            tgf1,                                                              \
            #Func "(" + gf1.name() + ')',                                      \
            Dfunc(gf1.dimensions())                                            \
        )                                                                      \
    );                                                                         \
                                                                               \
    Foam::Func(tRes.ref(), gf1);                                               \
                                                                               \
    tgf1.clear();                                                              \
                                                                               \
    return tRes;                                                               \
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#define UNARY_OPERATOR(ReturnType, Type1, Op, OpFunc, Dfunc)                   \
                                                                               \
TEMPLATE                                                                       \
void OpFunc                                                                    \
(                                                                              \
    dgGaussField<ReturnType >& res,                      \
    const dgGaussField<Type1 >& gf1                      \
)                                                                              \
{                                                                              \
    Foam::OpFunc(res.cellFieldRef(), gf1.cellField());               \
    Foam::OpFunc(res.ownerFaceFieldRef(), gf1.ownerFaceField());                 \
    Foam::OpFunc(res.neighborFaceFieldRef(), gf1.neighborFaceField());                 \
}                                                                              \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> operator Op               \
(                                                                              \
    const dgGaussField<Type1 >& gf1                      \
)                                                                              \
{                                                                              \
    tmp<dgGaussField<ReturnType >> tRes                  \
    (                                                                          \
    	new dgGaussField<ReturnType >							\
    	(							\
         GeometricDofField<ReturnType,  dgPatchField, dgGeoMesh>                    \
        (                                                                      \
            IOobject                                                           \
            (                                                                  \
                #Op + gf1.name(),                                              \
                gf1.instance(),                                                \
                gf1.db(),                                                      \
                IOobject::NO_READ,                                             \
                IOobject::NO_WRITE                                             \
            ),                                                                 \
            gf1.mesh(),                                                        \
            Dfunc(gf1.dimensions())                                            \
        )                                                                      \
        )   	\
    );                                                                         \
                                                                               \
    Foam::OpFunc(tRes.ref(), gf1);                                             \
                                                                               \
    return tRes;                                                               \
}                                                                              \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> operator Op               \
(                                                                              \
    const tmp<dgGaussField<Type1 >>& tgf1                \
)                                                                              \
{                                                                              \
    const dgGaussField<Type1 >& gf1 = tgf1();            \
                                                                               \
    tmp<dgGaussField<ReturnType >> tRes                  \
    (                                                                          \
        reuseTmpdgGaussField<ReturnType, Type1 >::New    \
        (                                                                      \
            tgf1,                                                              \
            #Op + gf1.name(),                                                  \
            Dfunc(gf1.dimensions())                                            \
        )                                                                      \
    );                                                                         \
                                                                               \
    Foam::OpFunc(tRes.ref(), gf1);                                             \
                                                                               \
    tgf1.clear();                                                              \
                                                                               \
    return tRes;                                                               \
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#define BINARY_FUNCTION(ReturnType, Type1, Type2, Func)                        \
                                                                               \
TEMPLATE                                                                       \
void Func                                                                      \
(                                                                              \
    dgGaussField<ReturnType >& res,                      \
    const dgGaussField<Type1 >& gf1,                     \
    const dgGaussField<Type2 >& gf2                      \
)                                                                              \
{                                                                              \
    Foam::Func                                                                 \
    (                                                                          \
        res.cellFieldRef(),                                               \
        gf1.cellField(),                                                  \
        gf2.cellField()                                                   \
    );                                                                         \
    Foam::Func                                                                 \
    (                                                                          \
        res.ownerFaceFieldRef(),                                                \
        gf1.ownerFaceField(),                                                   \
        gf2.ownerFaceField()                                                    \
    );                                                                         \
        Foam::Func                                                                 \
    (                                                                          \
        res.neighborFaceFieldRef(),                                                \
        gf1.neighborFaceField(),                                                   \
        gf2.neighborFaceField()                                                    \
    );                                                                         \
}                                                                              \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> Func                      \
(                                                                              \
    const dgGaussField<Type1 >& gf1,                     \
    const dgGaussField<Type2 >& gf2                      \
)                                                                              \
{                                                                              \
    tmp<dgGaussField<ReturnType >> tRes                  \
    (                                                                          \
        new dgGaussField<ReturnType >			\
        (						\
        	GeometricDofField<ReturnType,  dgPatchField, dgGeoMesh>                    \
        (                                                                      \
            IOobject                                                           \
            (                                                                  \
                #Func "(" + gf1.name() + ',' + gf2.name() + ')',               \
                gf1.instance(),                                                \
                gf1.db(),                                                      \
                IOobject::NO_READ,                                             \
                IOobject::NO_WRITE                                             \
            ),                                                                 \
            gf1.mesh(),                                                        \
            Func(gf1.dimensions(), gf2.dimensions())                           \
        )                                                                      \
        )			\
    );                                                                         \
                                                                               \
    Foam::Func(tRes.ref(), gf1, gf2);                                          \
                                                                               \
    return tRes;                                                               \
}                                                                              \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> Func                      \
(                                                                              \
    const dgGaussField<Type1 >& gf1,                     \
    const tmp<dgGaussField<Type2 >>& tgf2                \
)                                                                              \
{                                                                              \
    const dgGaussField<Type2 >& gf2 = tgf2();            \
                                                                               \
    tmp<dgGaussField<ReturnType >> tRes                  \
    (                                                                          \
        reuseTmpdgGaussField<ReturnType, Type2 >::New    \
        (                                                                      \
            tgf2,                                                              \
            #Func "(" + gf1.name() + ',' + gf2.name() + ')',                   \
            Func(gf1.dimensions(), gf2.dimensions())                           \
        )                                                                      \
    );                                                                         \
                                                                               \
    Foam::Func(tRes.ref(), gf1, gf2);                                          \
                                                                               \
    tgf2.clear();                                                              \
                                                                               \
    return tRes;                                                               \
}                                                                              \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> Func                      \
(                                                                              \
    const tmp<dgGaussField<Type1 >>& tgf1,               \
    const dgGaussField<Type2 >& gf2                      \
)                                                                              \
{                                                                              \
    const dgGaussField<Type1 >& gf1 = tgf1();            \
                                                                               \
    tmp<dgGaussField<ReturnType >> tRes                  \
    (                                                                          \
        reuseTmpdgGaussField<ReturnType, Type1 >::New    \
        (                                                                      \
            tgf1,                                                              \
            #Func "(" + gf1.name() + ',' + gf2.name() + ')',                   \
            Func(gf1.dimensions(), gf2.dimensions())                           \
        )                                                                      \
    );                                                                         \
                                                                               \
    Foam::Func(tRes.ref(), gf1, gf2);                                          \
                                                                               \
    tgf1.clear();                                                              \
                                                                               \
    return tRes;                                                               \
}                                                                              \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> Func                      \
(                                                                              \
    const tmp<dgGaussField<Type1 >>& tgf1,               \
    const tmp<dgGaussField<Type2 >>& tgf2                \
)                                                                              \
{                                                                              \
    const dgGaussField<Type1 >& gf1 = tgf1();            \
    const dgGaussField<Type2 >& gf2 = tgf2();            \
                                                                               \
    tmp<dgGaussField<ReturnType >> tRes                  \
    (                                                                          \
        reuseTmpTmpdgGaussField                                              \
            <ReturnType, Type1, Type1, Type2 >             \
        ::New                                                                  \
        (                                                                      \
            tgf1,                                                              \
            tgf2,                                                              \
            #Func "(" + gf1.name() + ',' + gf2.name() + ')',                   \
            Func(gf1.dimensions(), gf2.dimensions())                           \
        )                                                                      \
    );                                                                         \
                                                                               \
    Foam::Func(tRes.ref(), gf1, gf2);                                          \
                                                                               \
    tgf1.clear();                                                              \
    tgf2.clear();                                                              \
                                                                               \
    return tRes;                                                               \
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#define BINARY_TYPE_FUNCTION_SF(ReturnType, Type1, Type2, Func)                \
                                                                               \
TEMPLATE                                                                       \
void Func                                                                      \
(                                                                              \
    dgGaussField<ReturnType >& res,                      \
    const dimensioned<Type1>& dt1,                                             \
    const dgGaussField<Type2 >& gf2                      \
)                                                                              \
{                                                                              \
    Foam::Func(res.cellFieldRef(), dt1.value(), gf2.cellField());    \
    Foam::Func(res.ownerFaceFieldRef(), dt1.value(), gf2.ownerFaceField());      \
    Foam::Func(res.neighborFieldRef(), dt1.value(), gf2.neighborField());      \
}                                                                              \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> Func                      \
(                                                                              \
    const dimensioned<Type1>& dt1,                                             \
    const dgGaussField<Type2 >& gf2                      \
)                                                                              \
{                                                                              \
    tmp<dgGaussField<ReturnType >> tRes                  \
    (                                                                          \
	new  dgGaussField<ReturnType >		\
		(		\
	GeometricDofField<ReturnType,  dgPatchField, dgGeoMesh>                    \
        (                                                                      \
            IOobject                                                           \
            (                                                                  \
                #Func "(" + dt1.name() + ',' + gf2.name() + ')',               \
                gf2.instance(),                                                \
                gf2.db(),                                                      \
                IOobject::NO_READ,                                             \
                IOobject::NO_WRITE                                             \
            ),                                                                 \
            gf2.mesh(),                                                        \
            Func(dt1.dimensions(), gf2.dimensions())                           \
        )                                                                      \
        )		\
    );                                                                         \
                                                                               \
    Foam::Func(tRes.ref(), dt1, gf2);                                          \
                                                                               \
    return tRes;                                                               \
}                                                                              \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> Func                      \
(                                                                              \
    const Type1& t1,                                                           \
    const dgGaussField<Type2 >& gf2                      \
)                                                                              \
{                                                                              \
    return Func(dimensioned<Type1>(t1), gf2);                                  \
}                                                                              \
                                                                               \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> Func                      \
(                                                                              \
    const dimensioned<Type1>& dt1,                                             \
    const tmp<dgGaussField<Type2 >>& tgf2                \
)                                                                              \
{                                                                              \
    const dgGaussField<Type2 >& gf2 = tgf2();            \
                                                                               \
    tmp<dgGaussField<ReturnType >> tRes                  \
    (                                                                          \
        reuseTmpdgGaussField<ReturnType, Type2 >::New    \
        (                                                                      \
            tgf2,                                                              \
            #Func "(" + dt1.name() + gf2.name() + ',' + ')',                   \
            Func(dt1.dimensions(), gf2.dimensions())                           \
        )                                                                      \
    );                                                                         \
                                                                               \
    Foam::Func(tRes.ref(), dt1, gf2);                                          \
                                                                               \
    tgf2.clear();                                                              \
                                                                               \
    return tRes;                                                               \
}                                                                              \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> Func                      \
(                                                                              \
    const Type1& t1,                                                           \
    const tmp<dgGaussField<Type2 >>& tgf2                \
)                                                                              \
{                                                                              \
    return Func(dimensioned<Type1>(t1), tgf2);                                 \
}


#define BINARY_TYPE_FUNCTION_FS(ReturnType, Type1, Type2, Func)                \
                                                                               \
TEMPLATE                                                                       \
void Func                                                                      \
(                                                                              \
    dgGaussField<ReturnType >& res,                      \
    const dgGaussField<Type1 >& gf1,                     \
    const dimensioned<Type2>& dt2                                              \
)                                                                              \
{                                                                              \
    Foam::Func(res.cellFieldRef(), gf1.cellField(), dt2.value());    \
    Foam::Func(res.ownerFaceFieldRef(), gf1.ownerFaceField(), dt2.value());      \
    Foam::Func(res.neighborFaceFieldRef(), gf1.neighborFaceField(), dt2.value());      \
}                                                                              \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> Func                      \
(                                                                              \
    const dgGaussField<Type1 >& gf1,                     \
    const dimensioned<Type2>& dt2                                              \
)                                                                              \
{                                                                              \
    tmp<dgGaussField<ReturnType >> tRes                  \
    (                                                                          \
	new  dgGaussField<ReturnType >		\
	(	\
	GeometricDofField<ReturnType,  dgPatchField, dgGeoMesh>                    \
        (                                                                      \
            IOobject                                                           \
            (                                                                  \
                #Func "(" + gf1.name() + ',' + dt2.name() + ')',               \
                gf1.instance(),                                                \
                gf1.db(),                                                      \
                IOobject::NO_READ,                                             \
                IOobject::NO_WRITE                                             \
            ),                                                                 \
            gf1.mesh(),                                                        \
            Func(gf1.dimensions(), dt2.dimensions())                           \
        )                                                                      \
        )		\
    );                                                                         \
                                                                               \
    Foam::Func(tRes.ref(), gf1, dt2);                                          \
                                                                               \
    return tRes;                                                               \
}                                                                              \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> Func                      \
(                                                                              \
    const dgGaussField<Type1 >& gf1,                     \
    const Type2& t2                                                            \
)                                                                              \
{                                                                              \
    return Func(gf1, dimensioned<Type2>(t2));                                  \
}                                                                              \
                                                                               \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> Func                      \
(                                                                              \
    const tmp<dgGaussField<Type1 >>& tgf1,               \
    const dimensioned<Type2>& dt2                                              \
)                                                                              \
{                                                                              \
    const dgGaussField<Type1 >& gf1 = tgf1();            \
                                                                               \
    tmp<dgGaussField<ReturnType >> tRes                  \
    (                                                                          \
        reuseTmpdgGaussField<ReturnType, Type1 >::New    \
        (                                                                      \
            tgf1,                                                              \
            #Func "(" + gf1.name() + ',' + dt2.name() + ')',                   \
            Func(gf1.dimensions(), dt2.dimensions())                           \
        )                                                                      \
    );                                                                         \
                                                                               \
    Foam::Func(tRes.ref(), gf1, dt2);                                          \
                                                                               \
    tgf1.clear();                                                              \
                                                                               \
    return tRes;                                                               \
}                                                                              \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> Func                      \
(                                                                              \
    const tmp<dgGaussField<Type1 >>& tgf1,               \
    const Type2& t2                                                            \
)                                                                              \
{                                                                              \
    return Func(tgf1, dimensioned<Type2>(t2));                                 \
}


#define BINARY_TYPE_FUNCTION(ReturnType, Type1, Type2, Func)                   \
    BINARY_TYPE_FUNCTION_SF(ReturnType, Type1, Type2, Func)                    \
    BINARY_TYPE_FUNCTION_FS(ReturnType, Type1, Type2, Func)


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#define BINARY_OPERATOR(ReturnType, Type1, Type2, Op, OpName, OpFunc)          \
                                                                               \
TEMPLATE                                                                       \
void OpFunc                                                                    \
(                                                                              \
    dgGaussField<ReturnType >& res,                      \
    const dgGaussField<Type1 >& gf1,                     \
    const dgGaussField<Type2 >& gf2                      \
)                                                                              \
{                                                                              \
    Foam::OpFunc                                                               \
    (res.cellFieldRef(), gf1.cellField(), gf2.cellField());     \
    Foam::OpFunc                                                               \
    (res.ownerFaceFieldRef(), gf1.ownerFaceField(), gf2.ownerFaceField());        \
     Foam::OpFunc                                                               \
    (res.neighborFaceFieldRef(), gf1.neighborFaceField(), gf2.neighborFaceField());        \
}                                                                              \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> operator Op               \
(                                                                              \
    const dgGaussField<Type1 >& gf1,                     \
    const dgGaussField<Type2 >& gf2                      \
)                                                                              \
{                                                                              \
    tmp<dgGaussField<ReturnType >> tRes                  \
    (                                                                          \
    	new dgGaussField<ReturnType >											\
    	(						\
         GeometricDofField<ReturnType,  dgPatchField, dgGeoMesh>                    \
        (                                                                      \
            IOobject                                                           \
            (                                                                  \
                '(' + gf1.name() + OpName + gf2.name() + ')',                  \
                gf1.instance(),                                                \
                gf1.db(),                                                      \
                IOobject::NO_READ,                                             \
                IOobject::NO_WRITE                                             \
            ),                                                                 \
            gf1.mesh(),                                                        \
            gf1.dimensions() Op gf2.dimensions()                               \
        )                                                                      \
        )	\
    );                                                                         \
                                                                               \
    Foam::OpFunc(tRes.ref(), gf1, gf2);                                        \
                                                                               \
    return tRes;                                                               \
}                                                                              \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> operator Op               \
(                                                                              \
    const dgGaussField<Type1 >& gf1,                     \
    const tmp<dgGaussField<Type2 >>& tgf2                \
)                                                                              \
{                                                                              \
    const dgGaussField<Type2 >& gf2 = tgf2();            \
                                                                               \
    tmp<dgGaussField<ReturnType >> tRes                  \
    (                                                                          \
        reuseTmpdgGaussField<ReturnType, Type2 >::New    \
        (                                                                      \
            tgf2,                                                              \
            '(' + gf1.name() + OpName + gf2.name() + ')',                      \
            gf1.dimensions() Op gf2.dimensions()                               \
        )                                                                      \
    );                                                                         \
                                                                               \
    Foam::OpFunc(tRes.ref(), gf1, gf2);                                        \
                                                                               \
    tgf2.clear();                                                              \
                                                                               \
    return tRes;                                                               \
}                                                                              \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> operator Op               \
(                                                                              \
    const tmp<dgGaussField<Type1 >>& tgf1,               \
    const dgGaussField<Type2 >& gf2                      \
)                                                                              \
{                                                                              \
    const dgGaussField<Type1 >& gf1 = tgf1();            \
                                                                               \
    tmp<dgGaussField<ReturnType >> tRes                  \
    (                                                                          \
        reuseTmpdgGaussField<ReturnType, Type1 >::New    \
        (                                                                      \
            tgf1,                                                              \
            '(' + gf1.name() + OpName + gf2.name() + ')',                      \
            gf1.dimensions() Op gf2.dimensions()                               \
        )                                                                      \
    );                                                                         \
                                                                               \
    Foam::OpFunc(tRes.ref(), gf1, gf2);                                        \
                                                                               \
    tgf1.clear();                                                              \
                                                                               \
    return tRes;                                                               \
}                                                                              \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> operator Op               \
(                                                                              \
    const tmp<dgGaussField<Type1 >>& tgf1,               \
    const tmp<dgGaussField<Type2 >>& tgf2                \
)                                                                              \
{                                                                              \
    const dgGaussField<Type1 >& gf1 = tgf1();            \
    const dgGaussField<Type2 >& gf2 = tgf2();            \
                                                                               \
    tmp<dgGaussField<ReturnType >> tRes                  \
    (                                                                          \
        reuseTmpTmpdgGaussField                                              \
            <ReturnType, Type1, Type1, Type2 >::New        \
        (                                                                      \
            tgf1,                                                              \
            tgf2,                                                              \
            '(' + gf1.name() + OpName + gf2.name() + ')',                      \
            gf1.dimensions() Op gf2.dimensions()                               \
        )                                                                      \
    );                                                                         \
                                                                               \
    Foam::OpFunc(tRes.ref(), gf1, gf2);                                        \
                                                                               \
    tgf1.clear();                                                              \
    tgf2.clear();                                                              \
                                                                               \
    return tRes;                                                               \
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#define BINARY_TYPE_OPERATOR_SF(ReturnType, Type1, Type2, Op, OpName, OpFunc)  \
                                                                               \
TEMPLATE                                                                       \
void OpFunc                                                                    \
(                                                                              \
    dgGaussField<ReturnType >& res,                      \
    const dimensioned<Type1>& dt1,                                             \
    const dgGaussField<Type2 >& gf2                      \
)                                                                              \
{                                                                              \
    Foam::OpFunc(res.cellFieldRef(), dt1.value(), gf2.cellField());  \
    Foam::OpFunc(res.ownerFaceFieldRef(), dt1.value(), gf2.ownerFaceField());    \
    Foam::OpFunc(res.neighborFaceFieldRef(), dt1.value(), gf2.neighborFaceField());    \
}                                                                              \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> operator Op               \
(                                                                              \
    const dimensioned<Type1>& dt1,                                             \
    const dgGaussField<Type2 >& gf2                      \
)                                                                              \
{                                                                              \
    tmp<dgGaussField<ReturnType >> tRes                  \
    (                                                                          \
        new dgGaussField<ReturnType >											\
       (							\
       GeometricDofField<ReturnType,  dgPatchField, dgGeoMesh>                    \
        (                                                                      \
            IOobject                                                           \
            (                                                                  \
                '(' + dt1.name() + OpName + gf2.name() + ')',                  \
                gf2.instance(),                                                \
                gf2.db(),                                                      \
                IOobject::NO_READ,                                             \
                IOobject::NO_WRITE                                             \
            ),                                                                 \
            gf2.mesh(),                                                        \
            dt1.dimensions() Op gf2.dimensions()                               \
        )                                                                      \
        )				\
    );                                                                         \
                                                                               \
    Foam::OpFunc(tRes.ref(), dt1, gf2);                                        \
                                                                               \
    return tRes;                                                               \
}                                                                              \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> operator Op               \
(                                                                              \
    const Type1& t1,                                                           \
    const dgGaussField<Type2 >& gf2                      \
)                                                                              \
{                                                                              \
    return dimensioned<Type1>(t1) Op gf2;                                      \
}                                                                              \
                                                                               \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> operator Op               \
(                                                                              \
    const dimensioned<Type1>& dt1,                                             \
    const tmp<dgGaussField<Type2 >>& tgf2                \
)                                                                              \
{                                                                              \
    const dgGaussField<Type2 >& gf2 = tgf2();            \
                                                                               \
    tmp<dgGaussField<ReturnType >> tRes                  \
    (                                                                          \
        reuseTmpdgGaussField<ReturnType, Type2 >::New    \
        (                                                                      \
            tgf2,                                                              \
            '(' + dt1.name() + OpName + gf2.name() + ')',                      \
            dt1.dimensions() Op gf2.dimensions()                               \
        )                                                                      \
    );                                                                         \
                                                                               \
    Foam::OpFunc(tRes.ref(), dt1, gf2);                                        \
                                                                               \
    tgf2.clear();                                                              \
                                                                               \
    return tRes;                                                               \
}                                                                              \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> operator Op               \
(                                                                              \
    const Type1& t1,                                                           \
    const tmp<dgGaussField<Type2 >>& tgf2                \
)                                                                              \
{                                                                              \
    return dimensioned<Type1>(t1) Op tgf2;                                     \
}


#define BINARY_TYPE_OPERATOR_FS(ReturnType, Type1, Type2, Op, OpName, OpFunc)  \
                                                                               \
TEMPLATE                                                                       \
void OpFunc                                                                    \
(                                                                              \
    dgGaussField<ReturnType >& res,                      \
    const dgGaussField<Type1 >& gf1,                     \
    const dimensioned<Type2>& dt2                                              \
)                                                                              \
{                                                                              \
    Foam::OpFunc(res.cellFieldRef(), gf1.cellField(), dt2.value());  \
    Foam::OpFunc(res.ownerFaceFieldRef(), gf1.ownerFaceField(), dt2.value());    \
    Foam::OpFunc(res.neighborFaceFieldRef(), gf1.neighborFaceField(), dt2.value());    \
}                                                                              \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> operator Op               \
(                                                                              \
    const dgGaussField<Type1 >& gf1,                     \
    const dimensioned<Type2>& dt2                                              \
)                                                                              \
{                                                                              \
    tmp<dgGaussField<ReturnType >> tRes                  \
    (                                                                          \
 		   new dgGaussField<ReturnType >							\
 		   (						\
	GeometricDofField<ReturnType,  dgPatchField, dgGeoMesh>                    \
        (                                                                      \
            IOobject                                                           \
            (                                                                  \
                '(' + gf1.name() + OpName + dt2.name() + ')',                  \
                gf1.instance(),                                                \
                gf1.db(),                                                      \
                IOobject::NO_READ,                                             \
                IOobject::NO_WRITE                                             \
            ),                                                                 \
            gf1.mesh(),                                                        \
            gf1.dimensions() Op dt2.dimensions()                               \
        )                                                                      \
        )	\
    );                                                                         \
                                                                               \
    Foam::OpFunc(tRes.ref(), gf1, dt2);                                        \
                                                                               \
    return tRes;                                                               \
}                                                                              \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> operator Op               \
(                                                                              \
    const dgGaussField<Type1 >& gf1,                     \
    const Type2& t2                                                            \
)                                                                              \
{                                                                              \
    return gf1 Op dimensioned<Type2>(t2);                                      \
}                                                                              \
                                                                               \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> operator Op               \
(                                                                              \
    const tmp<dgGaussField<Type1 >>& tgf1,               \
    const dimensioned<Type2>& dt2                                              \
)                                                                              \
{                                                                              \
    const dgGaussField<Type1 >& gf1 = tgf1();            \
                                                                               \
    tmp<dgGaussField<ReturnType >> tRes                  \
    (                                                                          \
        reuseTmpdgGaussField<ReturnType, Type1 >::New    \
        (                                                                      \
            tgf1,                                                              \
            '(' + gf1.name() + OpName + dt2.name() + ')',                      \
            gf1.dimensions() Op dt2.dimensions()                               \
        )                                                                      \
    );                                                                         \
                                                                               \
    Foam::OpFunc(tRes.ref(), gf1, dt2);                                        \
                                                                               \
    tgf1.clear();                                                              \
                                                                               \
    return tRes;                                                               \
}                                                                              \
                                                                               \
TEMPLATE                                                                       \
tmp<dgGaussField<ReturnType >> operator Op               \
(                                                                              \
    const tmp<dgGaussField<Type1 >>& tgf1,               \
    const Type2& t2                                                            \
)                                                                              \
{                                                                              \
    return tgf1 Op dimensioned<Type2>(t2);                                     \
}


#define BINARY_TYPE_OPERATOR(ReturnType, Type1, Type2, Op, OpName, OpFunc)     \
    BINARY_TYPE_OPERATOR_SF(ReturnType, Type1, Type2, Op, OpName, OpFunc)      \
    BINARY_TYPE_OPERATOR_FS(ReturnType, Type1, Type2, Op, OpName, OpFunc)


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //
