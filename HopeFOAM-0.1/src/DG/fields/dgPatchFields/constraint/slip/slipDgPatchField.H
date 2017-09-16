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

Class
    Foam::slipDgPatchField

Description
    Foam::slipDgPatchField
    slip boundary condition: the vector is symmetry to the normal of the patch.

SourceFiles
    slipDgPatchField.C

\*---------------------------------------------------------------------------*/

#ifndef slipDgPatchField_H
#define slipDgPatchField_H

#include "dgPatchField.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                           Class slipDgPatch Declaration
\*---------------------------------------------------------------------------*/

template<class Type>
class slipDgPatchField
:
    public dgPatchField<Type>
{

public:

    //- Runtime type information
    TypeName("slip");


    // Constructors

        //- Construct from patch and internal field
        slipDgPatchField
        (
            const dgPatch&,
            const DimensionedField<Type, dgGeoMesh>&
        );

        //- Construct from patch, internal field and dictionary
        slipDgPatchField
        (
            const dgPatch&,
            const DimensionedField<Type, dgGeoMesh>&,
            const dictionary&
        );

        //- Construct by mapping the given slipDgPatchField<Type>
        //  onto a new patch
        slipDgPatchField
        (
            const slipDgPatchField<Type>&,
            const dgPatch&,
            const DimensionedField<Type, dgGeoMesh>&,
            const dgPatchFieldMapper&
        );

        //- Construct as copy
        slipDgPatchField
        (
            const slipDgPatchField<Type>&
        );

        //- Construct and return a clone
        virtual tmp<dgPatchField<Type> > clone() const
        {
            return tmp<dgPatchField<Type> >
            (
                new slipDgPatchField<Type>(*this)
            );
        }

        //- Construct as copy setting internal field reference
        slipDgPatchField
        (
            const slipDgPatchField<Type>&,
            const DimensionedField<Type, dgGeoMesh>&
        );

        //- Construct and return a clone setting internal field reference
        virtual tmp<dgPatchField<Type> > clone
        (
            const DimensionedField<Type, dgGeoMesh>& iF
        ) const
        {
            return tmp<dgPatchField<Type> >
            (
                new slipDgPatchField<Type>(*this, iF)
            );
        }


    // Member functions

        // Access

            //- Return true if this patch field fixes a value.
            //  Needed to check if a level has to be specified while solving
            //  Poissons equations.
     


        // Evaluation functions
            virtual void evaluate
            (
                const Pstream::commsTypes commsType=Pstream::blocking
            );

            //- Return the matrix diagonal coefficients corresponding to the
            //  evaluation of the value of this patchField with given weights
            virtual tmp<Field<Type> > valueInternalCoeffs
            (
                const tmp<scalarField>&
            ) const;

            //- Return the matrix source coefficients corresponding to the
            //  evaluation of the value of this patchField with given weights
            virtual tmp<Field<Type> > valueBoundaryCoeffs
            (
                const tmp<scalarField>&
            ) const;

            //- Return the matrix diagonal coefficients corresponding to the
            //  evaluation of the gradient of this patchField
            virtual tmp<Field<Type> > gradientInternalCoeffs() const;

            //- Return the matrix source coefficients corresponding to the
            //  evaluation of the gradient of this patchField
            virtual tmp<Field<Type> > gradientBoundaryCoeffs() const;

        //- Write
        virtual void write(Ostream&) const;

    // Member operators

        virtual void operator=(const UList<Type>&) {}

        virtual void operator=(const dgPatchField<Type>&) {}
        virtual void operator+=(const dgPatchField<Type>&) {}
        virtual void operator-=(const dgPatchField<Type>&) {}
        virtual void operator*=(const dgPatchField<scalar>&) {}
        virtual void operator/=(const dgPatchField<scalar>&) {}

        virtual void operator+=(const Field<Type>&) {}
        virtual void operator-=(const Field<Type>&) {}

        virtual void operator*=(const Field<scalar>&) {}
        virtual void operator/=(const Field<scalar>&) {}

        virtual void operator=(const Type&) {}
        virtual void operator+=(const Type&) {}
        virtual void operator-=(const Type&) {}
        virtual void operator*=(const scalar) {}
        virtual void operator/=(const scalar) {}
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
#   include "slipDgPatchField.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
