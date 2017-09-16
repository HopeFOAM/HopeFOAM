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
    Foam::coupledDgPatchField

Group
    grpCoupledBoundaryConditions

Description
    Abstract base class for coupled patches.

SourceFiles
    coupledDgPatchField.C

\*---------------------------------------------------------------------------*/

#ifndef coupledDgPatchField_H
#define coupledDgPatchField_H

#include "LduInterfaceField.H"
#include "dgPatchField.H"
#include "coupledDgPatch.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                       Class coupledDgPatch Declaration
\*---------------------------------------------------------------------------*/

template<class Type>
class coupledDgPatchField
:
    public LduInterfaceField<Type>,
    public dgPatchField<Type>
{	

public:

    //- Runtime type information
    TypeName(coupledDgPatch::typeName_());


    // Constructors

        //- Construct from patch and internal field
        coupledDgPatchField
        (
            const dgPatch&,
            const DimensionedField<Type, dgGeoMesh>&
        );

        //- Construct from patch and internal field and patch field
        coupledDgPatchField
        (
            const dgPatch&,
            const DimensionedField<Type, dgGeoMesh>&,
            const Field<Type>&
        );

        //- Construct from patch, internal field and dictionary
        coupledDgPatchField
        (
            const dgPatch&,
            const DimensionedField<Type, dgGeoMesh>&,
            const dictionary&
        );

        //- Construct by mapping the given coupledDgPatchField onto a new patch
        coupledDgPatchField
        (
            const coupledDgPatchField<Type>&,
            const dgPatch&,
            const DimensionedField<Type, dgGeoMesh>&,
            const dgPatchFieldMapper&
        );

        //- Construct as copy
        coupledDgPatchField
        (
            const coupledDgPatchField<Type>&
        );

        //- Construct and return a clone
        virtual tmp<dgPatchField<Type> > clone() const = 0;

        //- Construct as copy setting internal field reference
        coupledDgPatchField
        (
            const coupledDgPatchField<Type>&,
            const DimensionedField<Type, dgGeoMesh>&
        );

        //- Construct and return a clone
        virtual tmp<dgPatchField<Type> > clone
        (
            const DimensionedField<Type, dgGeoMesh>&
        ) const = 0;


    // Member functions

        // Access

            //- Return true if this patch field is derived from
            //  coupledDgPatchField<Type>.
            virtual bool coupled() const
            {
                return true;
            }

            //- Return neighbour field of internal field
            virtual tmp<Field<Type> > patchNeighbourField() const = 0;


		

        // Evaluation functions

            //- Return patch-normal gradient
            virtual tmp<Field<Type> > snGrad
            (
                const scalarField& deltaCoeffs
            ) const;

            //- Return patch-normal gradient
            virtual tmp<Field<Type> > snGrad() const
            {
                notImplemented
                (
                    type() + "::coupledDgPatchField<Type>::snGrad()"
                );
                return *this;
            }

            //- Initialise the evaluation of the patch field
            virtual void initEvaluate
            (
                const Pstream::commsTypes commsType
            );

            //- Evaluate the patch field
            virtual void evaluate
            (
                const Pstream::commsTypes commsType
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
            virtual tmp<Field<Type> > gradientInternalCoeffs
            (
                const scalarField& deltaCoeffs
            ) const;

            //- Return the matrix diagonal coefficients corresponding to the
            //  evaluation of the gradient of this patchField
            virtual tmp<Field<Type> > gradientInternalCoeffs() const;

            //- Return the matrix source coefficients corresponding to the
            //  evaluation of the gradient of this patchField
            virtual tmp<Field<Type> > gradientBoundaryCoeffs
            (
                const scalarField& deltaCoeffs
            ) const;

            //- Return the matrix source coefficients corresponding to the
            //  evaluation of the gradient of this patchField
            virtual tmp<Field<Type> > gradientBoundaryCoeffs() const;


        // Coupled interface functionality

            //- Update result field based on interface functionality
            virtual void updateInterfaceMatrix
            (
                scalarField& result,
                const scalarField& psiInternal,
                const scalarField& coeffs,
                const direction,
                const Pstream::commsTypes commsType
            ) const = 0;

            //- Update result field based on interface functionality
            virtual void updateInterfaceMatrix
            (
                Field<Type>&,
                const Field<Type>&,
                const scalarField&,
                const Pstream::commsTypes commsType
            ) const = 0;


        //- Write
        virtual void write(Ostream&) const;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
#   include "coupledDgPatchField.C"
#endif

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
