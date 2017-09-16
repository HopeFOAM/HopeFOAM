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
    Foam::fixedGradientFvPatchField

Group
    grpGenericBoundaryConditions

Description
    This boundary condition supplies a fixed gradient condition, such that
    the patch values are calculated using:

        \f[
            x_p = x_c + \frac{\nabla(x)}{\Delta}
        \f]

    where
    \vartable
        x_p      | patch values
        x_c      | internal field values
        \nabla(x)| gradient (user-specified)
        \Delta   | inverse distance from patch face centre to cell centre
    \endvartable

Usage
    \table
        Property     | Description             | Required    | Default value
        gradient     | gradient                | yes         |
    \endtable

    Example of the boundary condition specification:
    \verbatim
    <patchName>
    {
        type            fixedGradient;
        gradient        uniform 0;
    }
    \endverbatim

SourceFiles
    fixedGradientDgPatchField.C

\*---------------------------------------------------------------------------*/

#ifndef fixedGradientDgPatchField_H
#define fixedGradientDgPatchField_H

#include "dgPatchField.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                 Class fixedGradientFvPatchField Declaration
\*---------------------------------------------------------------------------*/

template<class Type>
class fixedGradientDgPatchField
:
    public dgPatchField<Type>
{
    // Private data

        Field<Type> gradient_;


public:

    //- Runtime type information
    TypeName("fixedGradient");


    // Constructors

        //- Construct from patch and internal field
        fixedGradientDgPatchField
        (
            const dgPatch&,
            const DimensionedField<Type, dgGeoMesh>&
        );

        //- Construct from patch, internal field and dictionary
        fixedGradientDgPatchField
        (
            const dgPatch&,
            const DimensionedField<Type, dgGeoMesh>&,
            const dictionary&
        );

        //- Construct by mapping the given fixedGradientFvPatchField
        //  onto a new patch
        fixedGradientDgPatchField
        (
            const fixedGradientDgPatchField<Type>&,
            const dgPatch&,
            const DimensionedField<Type, dgGeoMesh>&,
            const dgPatchFieldMapper&
        );

        //- Construct as copy
        fixedGradientDgPatchField
        (
            const fixedGradientDgPatchField<Type>&
        );

        //- Construct and return a clone
        virtual tmp<dgPatchField<Type>> clone() const
        {
            return tmp<dgPatchField<Type>>
            (
                new fixedGradientDgPatchField<Type>(*this)
            );
        }

        //- Construct as copy setting internal field reference
        fixedGradientDgPatchField
        (
            const fixedGradientDgPatchField<Type>&,
            const DimensionedField<Type, dgGeoMesh>&
        );

        //- Construct and return a clone setting internal field reference
        virtual tmp<dgPatchField<Type>> clone
        (
            const DimensionedField<Type, dgGeoMesh>& iF
        ) const
        {
            return tmp<dgPatchField<Type>>
            (
                new fixedGradientDgPatchField<Type>(*this, iF)
            );
        }


    // Member functions

        // Return defining fields

            //- Return gradient at boundary
            virtual Field<Type>& gradient()
            {
                return gradient_;
            }

            virtual const Field<Type>& gradient() const
            {
                return gradient_;
            }

            virtual bool fixesGradient() const
            {
                return true;
            }

        // Mapping functions

            //- Map (and resize as needed) from self given a mapping object
            virtual void autoMap
            (
                const dgPatchFieldMapper&
            );

            //- Reverse map the given fvPatchField onto this fvPatchField
            virtual void rmap
            (
                const dgPatchField<Type>&,
                const labelList&
            );


        // Evaluation functions

            //- Return gradient at boundary
            virtual tmp<Field<Type>> snGrad() const
            {
                return gradient_;
            }

            //- Evaluate the patch field
            virtual void evaluate
            (
                const Pstream::commsTypes commsType=Pstream::blocking
            );

            //- Return the matrix diagonal coefficients corresponding to the
            //  evaluation of the value of this patchField with given weights
virtual tmp<Field<Type> > valueInternalCoeffs
            (
                const tmp<Field<scalar> >&
            ) const
            {
                 notImplemented
                (
                    type()
                  + "::valueInternalCoeffs(const tmp<Field<scalar> >&)"
                );
                return *this;//modified by RXG
            }

            //- Return the matrix source coefficients corresponding to the
            //  evaluation of the value of this patchField with given weights
            virtual tmp<Field<Type> > valueBoundaryCoeffs
            (
                const tmp<Field<scalar> >&
            ) const
            {
                notImplemented
                (
                    type()
                  + "::valueBoundaryCoeffs(const tmp<Field<scalar> >&)"
                );
                return *this;
            }

            //- Return the matrix diagonal coefficients corresponding to the
            //  evaluation of the gradient of this patchField
            virtual tmp<Field<Type> > gradientInternalCoeffs() const
            {
                notImplemented(type() + "::gradientInternalCoeffs()");
                return *this;
            }

            //- Return the matrix diagonal coefficients corresponding to the
            //  evaluation of the gradient of this coupled patchField
            //  using the deltaCoeffs provided
            virtual tmp<Field<Type> > gradientInternalCoeffs
            (
                const scalarField& deltaCoeffs
            ) const
            {
                notImplemented
                (
                    type()
                  + "::gradientInternalCoeffs(const scalarField& deltaCoeffs)"
                );
                return *this;
            }

            //- Return the matrix source coefficients corresponding to the
            //  evaluation of the gradient of this patchField
            virtual tmp<Field<Type> > gradientBoundaryCoeffs() const
            {
                notImplemented(type() + "::gradientBoundaryCoeffs()");
                return *this;
            }

            //- Return the matrix source coefficients corresponding to the
            //  evaluation of the gradient of this coupled patchField
            //  using the deltaCoeffs provided
            virtual tmp<Field<Type> > gradientBoundaryCoeffs
            (
                const scalarField& deltaCoeffs
            ) const
            {
                notImplemented
                (
                    type()
                  + "::gradientBoundaryCoeffs(const scalarField& deltaCoeffs)"
                );
                return *this;
            }



        //- Write
        virtual void write(Ostream&) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
    #include "fixedGradientDgPatchField.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
