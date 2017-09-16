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

#include "patchWriter.H"
#include "writeFuns.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

template<class Type>
void Foam::patchWriter::write
(
    const UPtrList<const GeometricDofField<Type, dgPatchField, dgGeoMesh>>& flds
)
{
    forAll(flds, fieldi)
    {
        const GeometricDofField<Type, dgPatchField, dgGeoMesh>& fld =
            flds[fieldi];

        os_ << fld.name() << ' ' << pTraits<Type>::nComponents << ' '
            << nPoints_ << " float" << std::endl;

        DynamicList<floatScalar> fField(pTraits<Type>::nComponents*nPoints_);

        forAll(patchIDs_, j)
        {
            label patchi = patchIDs_[j];

            const dgPatchField<Type>& pfld = fld.boundaryField()[patchi];

            writeFuns::insert(pfld, fField);
        }
        writeFuns::write(os_, binary_, fField);
    }
}


// ************************************************************************* //
