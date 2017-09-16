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
    Foam::dgPatchField

Description
    An abstract base class with a fat-interface to all derived classes
    covering all possible ways in which they might be used.

    The first level of derivation is to basic patchFields which cover
    zero-gradient, fixed-gradient, fixed-value and mixed conditions.

    The next level of derivation covers all the specialised typed with
    specific evaluation proceedures, particularly with respect to specific
    fields.

SourceFiles
    dgPatchField.C
    dgPatchFieldNew.C

\*---------------------------------------------------------------------------*/

#ifndef dgPatchField_H
#define dgPatchField_H

#include "dgPatch.H"
#include "DimensionedField.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// Forward declaration of classes

class objectRegistry;
class dictionary;
class dgPatchFieldMapper;
class dgGeoMesh;


// Forward declaration of friend functions and operators

template<class Type>
class dgPatchField;


template<class Type>
class calculatedDgPatchField;

template<class Type>
Ostream& operator<<(Ostream&, const dgPatchField<Type>&);


/*---------------------------------------------------------------------------*\
                           Class patch Declaration
\*---------------------------------------------------------------------------*/

template<class Type>
class dgPatchField
:
    public Field<Type>
{
    // Private data

        //- Reference to patch
        const dgPatch& patch_;

        //- Reference to internal field
        const DimensionedField<Type, dgGeoMesh>& internalField_;
		

        //- Update index used so that updateCoeffs is called only once during
        //  the construction of the matrix
        bool updated_;

        //- Update index used so that manipulateMatrix is called only once
        //  during the construction of the matrix
        //bool manipulatedMatrix_;

        //- Optional patch type, used to allow specified boundary conditions
        //  to be applied to constraint patches by providing the constraint
        //  patch type as 'patchType'
        word patchType_;

public:

    typedef dgPatch Patch;
	 typedef calculatedDgPatchField<Type> Calculated;


    //- Runtime type information
    TypeName("dgPatchField");

    //- Debug switch to disallow the use of genericDgPatchField
    static int disallowGenericDgPatchField;


    // Declare run-time constructor selection tables

        declareRunTimeSelectionTable
        (
            tmp,
            dgPatchField,
            patch,
            (
                const dgPatch& p,
                const DimensionedField<Type, dgGeoMesh>& iF
            ),
            (p, iF)
        );

        declareRunTimeSelectionTable
        (
            tmp,
            dgPatchField,
            patchMapper,
            (
                const dgPatchField<Type>& ptf,
                const dgPatch& p,
                const DimensionedField<Type, dgGeoMesh>& iF,
                const dgPatchFieldMapper& m
            ),
            (dynamic_cast<const dgPatchFieldType&>(ptf), p, iF, m)
        );

        declareRunTimeSelectionTable
        (
            tmp,
            dgPatchField,
            dictionary,
            (
                const dgPatch& p,
                const DimensionedField<Type, dgGeoMesh>& iF,
                const dictionary& dict
            ),
            (p, iF, dict)
        );


    // Constructors

        //- Construct from patch and internal field
        dgPatchField
        (
            const dgPatch&,
            const DimensionedField<Type, dgGeoMesh>&
        );

        //- Construct from patch and internal field and patch type
        dgPatchField
        (
            const dgPatch&,
            const DimensionedField<Type, dgGeoMesh>&,
            const word& patchType
        );

		        //- Construct from patch, internal field and value
        dgPatchField
        (
            const dgPatch&,
            const DimensionedField<Type, dgGeoMesh>&,
            const Type& value
        );

        //- Construct from patch and internal field and patch field
        dgPatchField
        (
            const dgPatch&,
            const DimensionedField<Type, dgGeoMesh>&,
            const Field<Type>&
        );

        //- Construct from patch, internal field and dictionary
        dgPatchField
        (
            const dgPatch&,
            const DimensionedField<Type, dgGeoMesh>&,
            const dictionary&,
            const bool valueRequired=false
        );

        //- Construct by mapping the given dgPatchField onto a new patch
        dgPatchField
        (
            const dgPatchField<Type>&,
            const dgPatch&,
            const DimensionedField<Type, dgGeoMesh>&,
            const dgPatchFieldMapper&
        );

        //- Construct as copy
        dgPatchField(const dgPatchField<Type>&);

        //- Construct and return a clone
        virtual tmp<dgPatchField<Type> > clone() const
        {
            return tmp<dgPatchField<Type> >(new dgPatchField<Type>(*this));
        }

        //- Construct as copy setting internal field reference
        dgPatchField
        (
            const dgPatchField<Type>&,
            const DimensionedField<Type, dgGeoMesh>&
        );

        //- Construct and return a clone setting internal field reference
        virtual tmp<dgPatchField<Type> > clone
        (
            const DimensionedField<Type, dgGeoMesh>& iF
        ) const
        {
            return tmp<dgPatchField<Type> >(new dgPatchField<Type>(*this, iF));
        }


    // Selectors

        //- Return a pointer to a new patchField created on freestore given
        //  patch and internal field
        //  (does not set the patch field values)
        static tmp<dgPatchField<Type> > New
        (
            const word&,
            const dgPatch&,
            const DimensionedField<Type, dgGeoMesh>&
        );

        //- Return a pointer to a new patchField created on freestore given
        //  patch and internal field
        //  (does not set the patch field values)
        //  Allows override of constraint type
        static tmp<dgPatchField<Type> > New
        (
            const word&,
            const word& actualPatchType,
            const dgPatch&,
            const DimensionedField<Type, dgGeoMesh>&
        );

        //- Return a pointer to a new patchField created on freestore from
        //  a given dgPatchField mapped onto a new patch
        static tmp<dgPatchField<Type> > New
        (
            const dgPatchField<Type>&,
            const dgPatch&,
            const DimensionedField<Type, dgGeoMesh>&,
            const dgPatchFieldMapper&
        );

        //- Return a pointer to a new patchField created on freestore
        //  from dictionary
        static tmp<dgPatchField<Type> > New
        (
            const dgPatch&,
            const DimensionedField<Type, dgGeoMesh>&,
            const dictionary&
        );

        //- Return a pointer to a new calculatedDgPatchField created on
        //  freestore without setting patchField values
        static tmp<dgPatchField<Type> > NewCalculatedType
        (
            const dgPatch&
        );

        //- Return a pointer to a new calculatedDgPatchField created on
        //  freestore without setting patchField values
        template<class Type2>
        static tmp<dgPatchField<Type> > NewCalculatedType
        (
            const dgPatchField<Type2>&
        );


    //- Destructor
    virtual ~dgPatchField<Type>()
    {}


    // Member functions

        // Access

            //- Return local objectRegistry
            const objectRegistry& db() const;

            //- Return patch
            const dgPatch& patch() const
            {
                return patch_;
            }

            //- Return dimensioned internal field reference
            const DimensionedField<Type, dgGeoMesh>&
            dimensionedInternalField() const
            {
                return internalField_;
            }

            //- Return internal field reference
            const Field<Type>& internalField() const
            {
                return internalField_;
            }

            //- Optional patch type
            const word& patchType() const
            {
                return patchType_;
            }

            //- Optional patch type
            word& patchType()
            {
                return patchType_;
            }

            //- Return the type of the calculated for of dgPatchField
            static const word& calculatedType();

            //- Return true if this patch field fixes a value.
            //  Needed to check if a level has to be specified while solving
            //  Poissons equations.
            virtual bool fixesValue() const
            {
                return false;
            }

            virtual bool fixesGradient() const
            {
                return false;
            }

            virtual bool reflective() const
            {
                return false;
            }
            //- Return true if this patch field is coupled
            virtual bool coupled() const
            {
                return false;
            }

            //- Return true if this patch field is distributed on processors
            virtual bool processored() const
            {
                return false;
            }

	


            //- Return true if the boundary condition has already been updated
            bool updated() const
            {
                return updated_;
            }

			label totalDofNum() const{
                return patch_.totalDofNum();
            }

        // Mapping functions

            //- Map (and resize as needed) from self given a mapping object
            virtual void autoMap
            (
                const dgPatchFieldMapper&
            );

            //- Reverse map the given dgPatchField onto this dgPatchField
            virtual void rmap
            (
                const dgPatchField<Type>&,
                const labelList&
            );

        // Evaluation functions

            //- Return patch-normal gradient
            virtual tmp<Field<Type> > snGrad() const;

            //- Return patch-normal gradient for coupled-patches
            //  using the deltaCoeffs provided
            virtual tmp<Field<Type> > snGrad
            (
                const scalarField& deltaCoeffs
            ) const
            {
                notImplemented
                (
                    type() + "::snGrad(const scalarField& deltaCoeffs)"
                );
                return *this;
            }

            //- Update the coefficients associated with the patch field
            //  Sets Updated to true
            virtual void updateCoeffs();

            //- Update the coefficients associated with the patch field
            //  and apply weight field
            //  Sets Updated to true
            virtual void updateCoeffs(const scalarField& weights);

            //- Return internal field next to patch as patch field
            virtual tmp<Field<Type> > patchInternalField() const;

            //- Return internal field next to patch as patch field
            virtual void patchInternalField(Field<Type>&) const;
            
            //- set fixed or calculated valued patchfield to internalField
            void setPatchToInternalField(Field<Type>&);

            //- Return patchField on the opposite patch of a coupled patch
            virtual tmp<Field<Type> > patchNeighbourField() const
            {
                notImplemented(type() + "patchNeighbourField()");
                return *this;
            }

            //- Initialise the evaluation of the patch field
            virtual void initEvaluate
            (
                const Pstream::commsTypes commsType=Pstream::blocking
            )
            {}

            //- Evaluate the patch field, sets Updated to false
            virtual void evaluate
            (
                const Pstream::commsTypes commsType=Pstream::blocking
            );


		//inline const List<vector>& dofLocation()const{ return dofLocation_;}

   		
			//return the gradient
            virtual Field<Type>& gradient()
            {
                Field<Type> tmpGradient(this->size(), Zero);
                return tmpGradient;
            }

            virtual const Field<Type>& gradient() const
            {
                Field<Type> tmpGradient(this->size(), Zero);
                return tmpGradient;
            }

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

		            //- Helper function to write the keyword and entry only if the
            //  values are not equal. The value is then output as value2
            template<class EntryType>
            void writeEntryIfDifferent
            (
                Ostream& os,
                const word& entryName,
                const EntryType& value1,
                const EntryType& value2
            ) const;


        // Check

            //- Check dgPatchField<Type> against given dgPatchField<Type>
            void check(const dgPatchField<Type>&) const;


    // Member operators

        virtual void operator=(const UList<Type>&);

        virtual void operator=(const dgPatchField<Type>&);
        virtual void operator+=(const dgPatchField<Type>&);
        virtual void operator-=(const dgPatchField<Type>&);
        virtual void operator*=(const dgPatchField<scalar>&);
        virtual void operator/=(const dgPatchField<scalar>&);

        virtual void operator+=(const Field<Type>&);
        virtual void operator-=(const Field<Type>&);

        virtual void operator*=(const Field<scalar>&);
        virtual void operator/=(const Field<scalar>&);

        virtual void operator=(const Type&);
        virtual void operator+=(const Type&);
        virtual void operator-=(const Type&);
        virtual void operator*=(const scalar);
        virtual void operator/=(const scalar);


        // Force an assignment irrespective of form of patch

        virtual void operator==(const dgPatchField<Type>&);
        virtual void operator==(const Field<Type>&);
        virtual void operator==(const Type&);


    // Ostream operator

        friend Ostream& operator<< <Type>(Ostream&, const dgPatchField<Type>&);
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#ifdef NoRepository
#   include "dgPatchField.C"
#   include "calculatedDgPatchField.H"
#endif


#define makeDgPatchField(dgPatchTypeField)                                    \
                                                                              \
defineNamedTemplateTypeNameAndDebug(dgPatchTypeField, 0);                     \
template<>                                                                    \
int dgPatchTypeField::disallowGenericDgPatchField                             \
(                                                                             \
    debug::debugSwitch("disallowGenericDgPatchField", 0)                      \
);                                                                            \
defineTemplateRunTimeSelectionTable(dgPatchTypeField, patch);                 \
defineTemplateRunTimeSelectionTable(dgPatchTypeField, patchMapper);           \
defineTemplateRunTimeSelectionTable(dgPatchTypeField, dictionary);


#define addToPatchFieldRunTimeSelection(PatchTypeField, typePatchTypeField)   \
    addToRunTimeSelectionTable                                                \
    (                                                                         \
        PatchTypeField,                                                       \
        typePatchTypeField,                                                   \
        patch                                                                 \
    );                                                                        \
    addToRunTimeSelectionTable                                                \
    (                                                                         \
        PatchTypeField,                                                       \
        typePatchTypeField,                                                   \
        patchMapper                                                           \
    );                                                                        \
    addToRunTimeSelectionTable                                                \
    (                                                                         \
        PatchTypeField,                                                       \
        typePatchTypeField,                                                   \
        dictionary                                                            \
    );


// use with caution
#define addRemovableToPatchFieldRunTimeSelection\
(PatchTypeField, typePatchTypeField)                                          \
                                                                              \
    addRemovableToRunTimeSelectionTable                                       \
    (                                                                         \
        PatchTypeField,                                                       \
        typePatchTypeField,                                                   \
        patch                                                                 \
    );                                                                        \
    addRemovableToRunTimeSelectionTable                                       \
    (                                                                         \
        PatchTypeField,                                                       \
        typePatchTypeField,                                                   \
        patchMapper                                                           \
    );                                                                        \
    addRemovableToRunTimeSelectionTable                                       \
    (                                                                         \
        PatchTypeField,                                                       \
        typePatchTypeField,                                                   \
        dictionary                                                            \
    );


// for non-templated patch fields
#define makePatchTypeField(PatchTypeField, typePatchTypeField)                \
    defineTypeNameAndDebug(typePatchTypeField, 0);                            \
    addToPatchFieldRunTimeSelection(PatchTypeField, typePatchTypeField)

// for non-templated patch fields - use with caution
#define makeRemovablePatchTypeField(PatchTypeField, typePatchTypeField)       \
    defineTypeNameAndDebug(typePatchTypeField, 0);                            \
    addRemovableToPatchFieldRunTimeSelection(PatchTypeField, typePatchTypeField)


// for templated patch fields
#define makeTemplatePatchTypeField(PatchTypeField, typePatchTypeField)        \
    defineNamedTemplateTypeNameAndDebug(typePatchTypeField, 0);               \
    addToPatchFieldRunTimeSelection(PatchTypeField, typePatchTypeField)


#define makePatchFields(type)                                                 \
    makeTemplatePatchTypeField                                                \
    (                                                                         \
        dgPatchScalarField,                                                   \
        type##DgPatchScalarField                                              \
    );                                                                        \
    makeTemplatePatchTypeField                                                \
    (                                                                         \
        dgPatchVectorField,                                                   \
        type##DgPatchVectorField                                              \
    );                                                                        \
    makeTemplatePatchTypeField                                                \
    (                                                                         \
        dgPatchSphericalTensorField,                                          \
        type##DgPatchSphericalTensorField                                     \
    );                                                                        \
    makeTemplatePatchTypeField                                                \
    (                                                                         \
        dgPatchSymmTensorField,                                               \
        type##DgPatchSymmTensorField                                          \
    );                                                                        \
    makeTemplatePatchTypeField                                                \
    (                                                                         \
        dgPatchTensorField,                                                   \
        type##DgPatchTensorField                                              \
    );


#define makePatchFieldsTypeName(type)                                         \
    defineNamedTemplateTypeNameAndDebug(type##DgPatchScalarField, 0);         \
    defineNamedTemplateTypeNameAndDebug(type##DgPatchVectorField, 0);         \
    defineNamedTemplateTypeNameAndDebug(type##DgPatchSphericalTensorField, 0);\
    defineNamedTemplateTypeNameAndDebug(type##DgPatchSymmTensorField, 0);     \
    defineNamedTemplateTypeNameAndDebug(type##DgPatchTensorField, 0)


#define makePatchTypeFieldTypedefs(type)                                      \
    typedef type##DgPatchField<scalar> type##DgPatchScalarField;              \
    typedef type##DgPatchField<vector> type##DgPatchVectorField;              \
    typedef type##DgPatchField<sphericalTensor>                               \
        type##DgPatchSphericalTensorField;                                    \
    typedef type##DgPatchField<symmTensor> type##DgPatchSymmTensorField;      \
    typedef type##DgPatchField<tensor> type##DgPatchTensorField;



// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
