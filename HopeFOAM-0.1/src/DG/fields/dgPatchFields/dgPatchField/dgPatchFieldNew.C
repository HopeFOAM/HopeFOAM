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

// * * * * * * * * * * * * * * * * Selectors * * * * * * * * * * * * * * * * //

template<class Type>
Foam::tmp<Foam::dgPatchField<Type> > Foam::dgPatchField<Type>::New
(
    const word& patchFieldType,
    const word& actualPatchType,
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF
)
{
    if (debug)
    {
        Info<< "dgPatchField<Type>::New(const word&, const word&, "
               "const dgPatch&, const DimensionedField<Type, dgGeoMesh>&) :"
               " patchFieldType="
            << patchFieldType
            << " : " << p.type()
            << endl;
    }

    typename patchConstructorTable::iterator cstrIter =
        patchConstructorTablePtr_->find(patchFieldType);

    //Info<<"patchFieldType : "<<patchFieldType<<endl;

    if (cstrIter == patchConstructorTablePtr_->end())
    {
        FatalErrorIn
        (
            "dgPatchField<Type>::New(const word&, const word&, const dgPatch&,"
            "const DimensionedField<Type, dgGeoMesh>&)"
        )   << "Unknown patchField type "
            << patchFieldType << nl << nl
            << "Valid patchField types are :" << endl
            << patchConstructorTablePtr_->sortedToc()
            << exit(FatalError);
    }

    typename patchConstructorTable::iterator patchTypeCstrIter =
        patchConstructorTablePtr_->find(p.type());

    if
    (
        actualPatchType == word::null
     || actualPatchType != p.type()
    )
    {
        if (patchTypeCstrIter != patchConstructorTablePtr_->end())
        {
            return patchTypeCstrIter()(p, iF);
        }
        else
        {
            return cstrIter()(p, iF);
        }
    }
    else
    {
        tmp<dgPatchField<Type> > tdgp = cstrIter()(p, iF);

        // Check if constraint type override and store patchType if so
        if ((patchTypeCstrIter != patchConstructorTablePtr_->end()))
        {
            tdgp.ref().patchType() = actualPatchType;
        }
        return tdgp;
    }
}


template<class Type>
Foam::tmp<Foam::dgPatchField<Type> > Foam::dgPatchField<Type>::New
(
    const word& patchFieldType,
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF
)
{
    return New(patchFieldType, word::null, p, iF);
}


template<class Type>
Foam::tmp<Foam::dgPatchField<Type> > Foam::dgPatchField<Type>::New
(
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF,
    const dictionary& dict
)
{
    const word patchFieldType(dict.lookup("type"));

    if (debug)
    {
        Info<< "dgPatchField<Type>::New(const dgPatch&, "
               "const DimensionedField<Type, dgGeoMesh>&, "
               "const dictionary&) : patchFieldType="  << patchFieldType
            << endl;
    }

    typename dictionaryConstructorTable::iterator cstrIter
        = dictionaryConstructorTablePtr_->find(patchFieldType);

    if (cstrIter == dictionaryConstructorTablePtr_->end())
    {
        if (!disallowGenericDgPatchField)
        {
            cstrIter = dictionaryConstructorTablePtr_->find("generic");
        }

        if (cstrIter == dictionaryConstructorTablePtr_->end())
        {
            FatalIOErrorIn
            (
                "dgPatchField<Type>::New(const dgPatch&, "
                "const DimensionedField<Type, dgGeoMesh>&, "
                "const dictionary&)",
                dict
            )   << "Unknown patchField type " << patchFieldType
                << " for patch type " << p.type() << nl << nl
                << "Valid patchField types are :" << endl
                << dictionaryConstructorTablePtr_->sortedToc()
                << exit(FatalIOError);
        }
    }

    if
    (
       !dict.found("patchType")
     || word(dict.lookup("patchType")) != p.type()
    )
    {
        typename dictionaryConstructorTable::iterator patchTypeCstrIter
            = dictionaryConstructorTablePtr_->find(p.type());

        if
        (
            patchTypeCstrIter != dictionaryConstructorTablePtr_->end()
         && patchTypeCstrIter() != cstrIter()
        )
        {
            FatalIOErrorIn
            (
                "dgPatchField<Type>::New(const dgPatch&, "
                "const DimensionedField<Type, dgGeoMesh>&, "
                "const dictionary&)",
                dict
            )   << "inconsistent patch and patchField types for \n"
                   "    patch type " << p.type()
                << " and patchField type " << patchFieldType
                << exit(FatalIOError);
        }
    }

    return cstrIter()(p, iF, dict);
}


template<class Type>
Foam::tmp<Foam::dgPatchField<Type> > Foam::dgPatchField<Type>::New
(
    const dgPatchField<Type>& ptf,
    const dgPatch& p,
    const DimensionedField<Type, dgGeoMesh>& iF,
    const dgPatchFieldMapper& pfMapper
)
{
    if (debug)
    {
        Info<< "dgPatchField<Type>::New(const dgPatchField<Type>&, "
               "const dgPatch&, const DimensionedField<Type, dgGeoMesh>&, "
               "const dgPatchFieldMapper&) : "
               "constructing dgPatchField<Type>"
            << endl;
    }

    typename patchMapperConstructorTable::iterator cstrIter =
        patchMapperConstructorTablePtr_->find(ptf.type());

    if (cstrIter == patchMapperConstructorTablePtr_->end())
    {
        FatalErrorIn
        (
            "dgPatchField<Type>::New(const dgPatchField<Type>&, "
            "const dgPatch&, const DimensionedField<Type, dgGeoMesh>&, "
            "const dgPatchFieldMapper&)"
        )   << "Unknown patchField type " << ptf.type() << nl << nl
            << "Valid patchField types are :" << endl
            << patchMapperConstructorTablePtr_->sortedToc()
            << exit(FatalError);
    }

    return cstrIter()(ptf, p, iF, pfMapper);
}


// ************************************************************************* //
