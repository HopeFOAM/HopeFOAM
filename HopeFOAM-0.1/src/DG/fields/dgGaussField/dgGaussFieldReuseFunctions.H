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

#ifndef dgGaussFieldReuseFunctions_H
#define dgGaussFieldReuseFunctions_H

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

template
<
    class Type
>
bool reusable(const tmp<dgGaussField<Type>>& tgf)
{
    if (tgf.isTmp())
    {
        if (dgGaussField<Type>::debug)
        {
            const dgGaussField<Type>& gf = tgf();
            const typename dgMesh::
                BoundaryMesh& gbf = gf.mesh().boundary();

            forAll(gbf, patchi)
            {
                if
                (
                    !polyPatch::constraintType(gbf[patchi].patch().type())
                 && !isA<typename dgPatchField<Type>::Calculated>(gbf[patchi])
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


template<class TypeR>
tmp<dgGaussField<TypeR>> New
(
    const tmp<dgGaussField<TypeR>>& tgf1,
    const word& name,
    const dimensionSet& dimensions,
    const bool initRet = false
)
{
    dgGaussField<TypeR>& gf1 =
        const_cast<dgGaussField<TypeR>& >(tgf1());

    if (reusable(tgf1))
    {
        gf1.rename(name);
        gf1.dimensions().reset(dimensions);
        return tgf1;
    }
    else
    {
        tmp<dgGaussField<TypeR>> rtgf 
        (
        	new dgGaussField<TypeR>
        	(
             GeometricDofField<TypeR, dgPatchField, dgGeoMesh>
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
            )
        );

        if (initRet)
        {
            rtgf.ref() ==tgf1();
        }

        return rtgf;
    }
}


template
<
    class TypeR,
    class Type1
    
>
class reuseTmpdgGaussField
{
public:

    static tmp<dgGaussField<TypeR>> New
    (
        const tmp<dgGaussField<Type1>>& tgf1,
        const word& name,
        const dimensionSet& dimensions
    )
    {
        const dgGaussField<Type1>& gf1 = tgf1();

        return tmp<dgGaussField<TypeR>> 
	(
		new dgGaussField<TypeR>
		(
             GeometricDofField<TypeR, dgPatchField, dgGeoMesh>
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
            )
        );
    }
};


template<class TypeR>
class reuseTmpdgGaussField<TypeR, TypeR>
{
public:

    static tmp<dgGaussField<TypeR>> New
    (
        const tmp<dgGaussField<TypeR>>& tgf1,
        const word& name,
        const dimensionSet& dimensions
    )
    {
        dgGaussField<TypeR>& gf1 =
            const_cast<dgGaussField<TypeR>& >(tgf1());

        if (reusable(tgf1))
        {
            gf1.rename(name);
            gf1.dimensions().reset(dimensions);
            return tgf1;
        }
        else
        {
            return tmp<dgGaussField<TypeR>> 
            (
            new dgGaussField<TypeR>
            (
                GeometricDofField<TypeR, dgPatchField, dgGeoMesh>
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
    class Type2
   
>
class reuseTmpTmpdgGaussField
{
public:

    static tmp<dgGaussField<TypeR>> New
    (
        const tmp<dgGaussField<Type1>>& tgf1,
        const tmp<dgGaussField<Type2>>& tgf2,
        const word& name,
        const dimensionSet& dimensions
    )
    {
        const dgGaussField<Type1>& gf1 = tgf1();

        return tmp<dgGaussField<TypeR>> 
        (
        new dgGaussField<TypeR>
            (
            GeometricDofField<TypeR, dgPatchField, dgGeoMesh>
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
            )
        );
    }
};


template
<
    class TypeR,
    class Type1,
    class Type12
   
>
class reuseTmpTmpdgGaussField
    <TypeR, Type1, Type12, TypeR>
{
public:

    static tmp<dgGaussField<TypeR>> New
    (
        const tmp<dgGaussField<Type1>>& tgf1,
        const tmp<dgGaussField<TypeR>>& tgf2,
        const word& name,
        const dimensionSet& dimensions
    )
    {
        const dgGaussField<Type1>& gf1 = tgf1();
        dgGaussField<TypeR>& gf2 =
            const_cast<dgGaussField<TypeR>& >(tgf2());

        if (reusable(tgf2))
        {
            gf2.rename(name);
            gf2.dimensions().reset(dimensions);
            return tgf2;
        }
        else
        {
            return tmp<dgGaussField<TypeR>> 
            (
            new dgGaussField<TypeR>
            (
                 GeometricDofField<TypeR, dgPatchField, dgGeoMesh>
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
			)
            );
        }
    }
};


template
<
    class TypeR,
    class Type2
   
>
class reuseTmpTmpdgGaussField<TypeR, TypeR, TypeR, Type2>
{
public:

    static tmp<dgGaussField<TypeR>> New
    (
        const tmp<dgGaussField<TypeR>>& tgf1,
        const tmp<dgGaussField<Type2>>& tgf2,
        const word& name,
        const dimensionSet& dimensions
    )
    {
        dgGaussField<TypeR>& gf1 =
            const_cast<dgGaussField<TypeR>& >(tgf1());

        if (reusable(tgf1))
        {
            gf1.rename(name);
            gf1.dimensions().reset(dimensions);
            return tgf1;
        }
        else
        {
            return tmp<dgGaussField<TypeR>>
            (
            new dgGaussField<TypeR>
            (
                 GeometricDofField<TypeR, dgPatchField, dgGeoMesh>
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
                )
            );
        }
    }
};


template<class TypeR>
class reuseTmpTmpdgGaussField<TypeR, TypeR, TypeR, TypeR>
{
public:

    static tmp<dgGaussField<TypeR>> New
    (
        const tmp<dgGaussField<TypeR>>& tgf1,
        const tmp<dgGaussField<TypeR>>& tgf2,
        const word& name,
        const dimensionSet& dimensions
    )
    {
        dgGaussField<TypeR>& gf1 =
            const_cast<dgGaussField<TypeR>& >(tgf1());
        dgGaussField<TypeR>& gf2 =
            const_cast<dgGaussField<TypeR>& >(tgf2());

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
            return tmp<dgGaussField<TypeR>>
            (
            new dgGaussField<TypeR>
            (
                 GeometricDofField<TypeR,dgPatchField, dgGeoMesh>
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
