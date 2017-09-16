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

#ifndef GeometricDofFieldReuseFunctions_H
#define GeometricDofFieldReuseFunctions_H

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

template
<
    class Type,
    template<class> class PatchField,
    class GeoMesh
>
bool reusable(const tmp<GeometricDofField<Type, PatchField, GeoMesh>>& tgf)
{
    if (tgf.isTmp())
    {
        if (GeometricDofField<Type, PatchField, GeoMesh>::debug)
        {
            const GeometricDofField<Type, PatchField, GeoMesh>& gf = tgf();
            const typename GeometricDofField<Type, PatchField, GeoMesh>::
                Boundary& gbf = gf.boundaryField();

            forAll(gbf, patchi)
            {
                if
                (
                    !polyPatch::constraintType(gbf[patchi].patch().type())
                 && !isA<typename PatchField<Type>::Calculated>(gbf[patchi])
                )
                {
                    WarningInFunction
                        << "Attempt to reuse temporary with non-reusable BC "
                        << gbf[patchi].type() << endl;

                    return false;
                }
            }
        }

        return true;
    }
    else
    {
        return false;
    }
}


template<class TypeR, template<class> class PatchField, class GeoMesh>
tmp<GeometricDofField<TypeR, PatchField, GeoMesh>> New
(
    const tmp<GeometricDofField<TypeR, PatchField, GeoMesh>>& tgf1,
    const word& name,
    const dimensionSet& dimensions,
    const bool initRet = false
)
{
    GeometricDofField<TypeR, PatchField, GeoMesh>& gf1 =
        const_cast<GeometricDofField<TypeR, PatchField, GeoMesh>& >(tgf1());

    if (reusable(tgf1))
    {
        gf1.rename(name);
        gf1.dimensions().reset(dimensions);
        return tgf1;
    }
    else
    {
        tmp<GeometricDofField<TypeR, PatchField, GeoMesh>> rtgf
        (
            new GeometricDofField<TypeR, PatchField, GeoMesh>
            (
                IOobject
                (
                    name,
                    gf1.instance(),
                    gf1.db()
                ),
                gf1.mesh(),
                dimensions
            )
        );

        if (initRet)
        {
            rtgf.ref() == tgf1();
        }

        return rtgf;
    }
}


template
<
    class TypeR,
    class Type1,
    template<class> class PatchField,
    class GeoMesh
>
class reuseTmpGeometricDofField
{
public:

    static tmp<GeometricDofField<TypeR, PatchField, GeoMesh>> New
    (
        const tmp<GeometricDofField<Type1, PatchField, GeoMesh>>& tgf1,
        const word& name,
        const dimensionSet& dimensions
    )
    {
        const GeometricDofField<Type1, PatchField, GeoMesh>& gf1 = tgf1();

        return tmp<GeometricDofField<TypeR, PatchField, GeoMesh>>
        (
            new GeometricDofField<TypeR, PatchField, GeoMesh>
            (
                IOobject
                (
                    name,
                    gf1.instance(),
                    gf1.db()
                ),
                gf1.mesh(),
                dimensions
            )
        );
    }
};


template<class TypeR, template<class> class PatchField, class GeoMesh>
class reuseTmpGeometricDofField<TypeR, TypeR, PatchField, GeoMesh>
{
public:

    static tmp<GeometricDofField<TypeR, PatchField, GeoMesh>> New
    (
        const tmp<GeometricDofField<TypeR, PatchField, GeoMesh>>& tgf1,
        const word& name,
        const dimensionSet& dimensions
    )
    {
        GeometricDofField<TypeR, PatchField, GeoMesh>& gf1 =
            const_cast<GeometricDofField<TypeR, PatchField, GeoMesh>& >(tgf1());

        if (reusable(tgf1))
        {
            gf1.rename(name);
            gf1.dimensions().reset(dimensions);
            return tgf1;
        }
        else
        {
            return tmp<GeometricDofField<TypeR, PatchField, GeoMesh>>
            (
                new GeometricDofField<TypeR, PatchField, GeoMesh>
                (
                    IOobject
                    (
                        name,
                        gf1.instance(),
                        gf1.db()
                    ),
                    gf1.mesh(),
                    dimensions
                )
            );
        }
    }
};


template
<
    class TypeR,
    class Type1,
    class Type12,
    class Type2,
    template<class> class PatchField,
    class GeoMesh
>
class reuseTmpTmpGeometricDofField
{
public:

    static tmp<GeometricDofField<TypeR, PatchField, GeoMesh>> New
    (
        const tmp<GeometricDofField<Type1, PatchField, GeoMesh>>& tgf1,
        const tmp<GeometricDofField<Type2, PatchField, GeoMesh>>& tgf2,
        const word& name,
        const dimensionSet& dimensions
    )
    {
        const GeometricDofField<Type1, PatchField, GeoMesh>& gf1 = tgf1();

        return tmp<GeometricDofField<TypeR, PatchField, GeoMesh>>
        (
            new GeometricDofField<TypeR, PatchField, GeoMesh>
            (
                IOobject
                (
                    name,
                    gf1.instance(),
                    gf1.db()
                ),
                gf1.mesh(),
                dimensions
            )
        );
    }
};


template
<
    class TypeR,
    class Type1,
    class Type12,
    template<class> class PatchField,
    class GeoMesh
>
class reuseTmpTmpGeometricDofField
    <TypeR, Type1, Type12, TypeR, PatchField, GeoMesh>
{
public:

    static tmp<GeometricDofField<TypeR, PatchField, GeoMesh>> New
    (
        const tmp<GeometricDofField<Type1, PatchField, GeoMesh>>& tgf1,
        const tmp<GeometricDofField<TypeR, PatchField, GeoMesh>>& tgf2,
        const word& name,
        const dimensionSet& dimensions
    )
    {
        const GeometricDofField<Type1, PatchField, GeoMesh>& gf1 = tgf1();
        GeometricDofField<TypeR, PatchField, GeoMesh>& gf2 =
            const_cast<GeometricDofField<TypeR, PatchField, GeoMesh>& >(tgf2());

        if (reusable(tgf2))
        {
            gf2.rename(name);
            gf2.dimensions().reset(dimensions);
            return tgf2;
        }
        else
        {
            return tmp<GeometricDofField<TypeR, PatchField, GeoMesh>>
            (
                new GeometricDofField<TypeR, PatchField, GeoMesh>
                (
                    IOobject
                    (
                        name,
                        gf1.instance(),
                        gf1.db()
                    ),
                    gf1.mesh(),
                    dimensions
                )
            );
        }
    }
};


template
<
    class TypeR,
    class Type2,
    template<class> class PatchField,
    class GeoMesh
>
class reuseTmpTmpGeometricDofField<TypeR, TypeR, TypeR, Type2, PatchField, GeoMesh>
{
public:

    static tmp<GeometricDofField<TypeR, PatchField, GeoMesh>> New
    (
        const tmp<GeometricDofField<TypeR, PatchField, GeoMesh>>& tgf1,
        const tmp<GeometricDofField<Type2, PatchField, GeoMesh>>& tgf2,
        const word& name,
        const dimensionSet& dimensions
    )
    {
        GeometricDofField<TypeR, PatchField, GeoMesh>& gf1 =
            const_cast<GeometricDofField<TypeR, PatchField, GeoMesh>& >(tgf1());

        if (reusable(tgf1))
        {
            gf1.rename(name);
            gf1.dimensions().reset(dimensions);
            return tgf1;
        }
        else
        {
            return tmp<GeometricDofField<TypeR, PatchField, GeoMesh>>
            (
                new GeometricDofField<TypeR, PatchField, GeoMesh>
                (
                    IOobject
                    (
                        name,
                        gf1.instance(),
                        gf1.db()
                    ),
                    gf1.mesh(),
                    dimensions
                )
            );
        }
    }
};


template<class TypeR, template<class> class PatchField, class GeoMesh>
class reuseTmpTmpGeometricDofField<TypeR, TypeR, TypeR, TypeR, PatchField, GeoMesh>
{
public:

    static tmp<GeometricDofField<TypeR, PatchField, GeoMesh>> New
    (
        const tmp<GeometricDofField<TypeR, PatchField, GeoMesh>>& tgf1,
        const tmp<GeometricDofField<TypeR, PatchField, GeoMesh>>& tgf2,
        const word& name,
        const dimensionSet& dimensions
    )
    {
        GeometricDofField<TypeR, PatchField, GeoMesh>& gf1 =
            const_cast<GeometricDofField<TypeR, PatchField, GeoMesh>& >(tgf1());
        GeometricDofField<TypeR, PatchField, GeoMesh>& gf2 =
            const_cast<GeometricDofField<TypeR, PatchField, GeoMesh>& >(tgf2());

        if (reusable(tgf1))
        {
            gf1.rename(name);
            gf1.dimensions().reset(dimensions);
            return tgf1;
        }
        else if (reusable(tgf2))
        {
            gf2.rename(name);
            gf2.dimensions().reset(dimensions);
            return tgf2;
        }
        else
        {
            return tmp<GeometricDofField<TypeR, PatchField, GeoMesh>>
            (
                new GeometricDofField<TypeR, PatchField, GeoMesh>
                (
                    IOobject
                    (
                        name,
                        gf1.instance(),
                        gf1.db()
                    ),
                    gf1.mesh(),
                    dimensions
                )
            );
        }
    }
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
